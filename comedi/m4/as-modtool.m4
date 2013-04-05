dnl as-modtool.m4 0.0.1
dnl autostars m4 macro for building modtool, a linker for Linux kernel
dnl modules

dnl David Schleef <ds@schleef.org>
dnl Frank Mori Hess <fmhess@users.sourceforge.net>
dnl Thomas Vander Stichele <thomas@apestaart.org>

dnl $Id: as-modtool.m4,v 1.7 2006-07-10 10:06:34 abbotti Exp $

dnl AS_LINUX_MODTOOL()
dnl
dnl this macro defines:
dnl moduledir
dnl modulePROGRAMS_INSTALL
dnl modulePROGRAMS_UNINSTALL
dnl
dnl End of search list.

dnl
dnl FIXME:
dnl  How do you specify that the building of modtool should go to the
dnl  end of the configure script?
dnl

dnl SYMVERS_INCLUDES can be used to add additional .symvers files to the
dnl modpost step

AC_DEFUN([AS_LINUX_MODTOOL],
[
	AC_PATH_TOOL([STRIP], [strip])
	AC_PATH_PROG([DEPMOD], [depmod], [no], [$PATH:/sbin:/usr/sbin:/usr/local/sbin])

	dnl this can be overridden in Makefile.am
	dnl FIXME: it'd be nice if we could specify different target_PROGRAMS
	dnl and different targetdir
	moduledir="\$(modulesdir)/\$(PACKAGE)"
	modulePROGRAMS_INSTALL="\$(top_builddir)/modtool --install"
	modulePROGRAMS_UNINSTALL="\$(top_builddir)/modtool --uninstall"
	AC_SUBST(moduledir)
	AC_SUBST(modulePROGRAMS_INSTALL)

	AC_MSG_NOTICE(creating modtool)
	cat >modtool <<EOF
#!/bin/sh

set -e
#set -x

LINUX_LD="$LINUX_LD"
LINUX_MODPOST="$LINUX_MODPOST"
CC="$LINUX_CC"
INSTALL="$INSTALL"
LINUX_MODULE_EXT="$LINUX_MODULE_EXT"
STRIP="$STRIP"
CFLAGS="$CFLAGS $LINUX_CFLAGS"
LINUX_DIR="$LINUX_DIR"
LINUX_MODULE_STYLE="$LINUX_MODULE_STYLE"
DEPMOD="$DEPMOD"
LINUX_KERNELRELEASE="$LINUX_KERNELRELEASE"

mode=\$[1]
shift

case \$mode in
--link)
	# we accept -i (symvers) and -o (target) as options
	# at least -o (target) needs to be specified
	SYMVERS_INCLUDES=""
	done=false
	while test ! -z "\$[0]" -a "\$done" = "false"
	do
		case \$[1] in
		-i)
                        SYMVERS_INCLUDES="\$SYMVERS_INCLUDES \$[2]"
                        shift 2
                        ;;
                -o)
                        target=\$(echo \$[2] | sed s/.ko$//)
                        shift 2
                        ;;
                *)
                        done=true
                        ;;
                esac
        done

	case "\$LINUX_MODULE_STYLE" in
	2.6.6)
		set -x
		mkdir -p .mods

		echo \$LINUX_LD -r -o .mods/\$target.o \$[*]
		\$LINUX_LD -r -o .mods/\$target.o \$[*]

		echo "cat \$LINUX_DIR/Module.symvers \$SYMVERS_INCLUDES >.mods/symvers.tmp || touch .mods/symvers.tmp"
		cat \$LINUX_DIR/Module.symvers \$SYMVERS_INCLUDES >.mods/symvers.tmp || touch .mods/symvers.tmp

		echo "\$LINUX_MODPOST -o .mods/\$target.o.symvers.tmp -i .mods/symvers.tmp \$target.o"
		\$LINUX_MODPOST -o .mods/\$target.o.symvers.tmp -i .mods/symvers.tmp .mods/\$target.o

		echo "grep .mods/\$target .mods/\$target.o.symvers.tmp >.mods/\$target.o.symvers || true"
		grep .mods/\$target .mods/\$target.o.symvers.tmp >.mods/\$target.o.symvers || true

		echo "rm -f .mods/\$target.o.symvers.tmp .mods/symvers.tmp"
		rm -f .mods/\$target.o.symvers.tmp .mods/symvers.tmp
		
		echo \$CC \$CFLAGS -DKBUILD_MODNAME=\$target -c -o .mods/\$target.mod.o .mods/\$target.mod.c
		\$CC \$CFLAGS -DKBUILD_MODNAME=\$target -c -o .mods/\$target.mod.o .mods/\$target.mod.c

		echo \$LINUX_LD -r -o \$target.ko .mods/\$target.mod.o .mods/\$target.o
		\$LINUX_LD -r -o \$target.ko .mods/\$target.mod.o .mods/\$target.o
		set +x

		;;
	2.6.0)
		;;
	2.4.0)
		echo \$LINUX_LD -r -o \$target.ko \$[*]
		\$LINUX_LD -r -o \$target.ko \$[*]
		;;
	esac

	;;
--install)
	module_src=\$[1]
	module_dest=\`echo \$[2] | sed "s/\.ko$/\${LINUX_MODULE_EXT}/"\`
	echo \$INSTALL -m644 "\$module_src" "\$module_dest"
	\$INSTALL -m644 "\$module_src" "\$module_dest"
	\$STRIP -g "\$module_dest"
	;;
--uninstall)
	module_src=\$[1]
	module_dest=\`echo \$[2] | sed "s/\.ko$/\${LINUX_MODULE_EXT}/"\`
	echo uninstall "\$module_src" "\$module_dest"
	rm -f "\$module_dest"
	;;
--finish)
	# run depmod here
	if [ "$prefix" = "/" ] ; then
		\$DEPMOD -ae \$LINUX_KERNELRELEASE
	fi
	;;
*)
	echo Unknown mode \$mode >&2
	exit 1
esac

EOF
	chmod +x modtool


])


