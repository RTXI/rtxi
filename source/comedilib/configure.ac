
AC_INIT([comedilib], [0.9.0])
AC_CANONICAL_TARGET([])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([.])
#COMEDILIB_DEBUG="-Wall -Werror"
COMEDILIB_DEBUG="-Wall"
AM_INIT_AUTOMAKE([-Wall -Werror])

# libtool version: current:revision:age
#
# If the library source code has changed at all since the last update, then
# increment revision (`c:r:a' becomes `c:r+1:a').
#
# If any interfaces have been added, removed, or changed since the last update,
# increment current, and set revision to 0.
#
# If any interfaces have been added since the last public release, then
# increment age.
#
# If any interfaces have been removed since the last public release, then set
# age to 0.
#
# AS_LIBTOOL arguments are (prefix, current, revision, age)
AS_LIBTOOL(COMEDILIB, 9, 0, 9)
#libscxi c:r:a
SCXI_SO_VERSION=9:0:9
AC_SUBST(SCXI_SO_VERSION)

AM_CONFIG_HEADER(config.h)

ACLOCAL="$ACLOCAL -I m4"

AM_MAINTAINER_MODE

AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CC_STDC
AC_ISC_POSIX
AM_PROG_LEX
AC_PROG_YACC
AC_PROG_LIBTOOL

AC_HEADER_STDC([])

AX_TLS

COMEDILIB_CFLAGS="$COMEDILIB_CFLAGS -I\$(top_srcdir)/include $COMEDILIB_DEBUG"
COMEDILIB_LIBS="$COMEDILIB_LIBS \$(top_builddir)/lib/libcomedi.la -lm"
AC_SUBST(COMEDILIB_CFLAGS)
AC_SUBST(COMEDILIB_LIBS)

# autoconf backwards compatibility
if test "$datarootdir" = ""; then
	datarootdir='${datadir}'
	AC_SUBST(datarootdir)
fi

if test "$docdir" = ""; then
	docdir='${datarootdir}/doc/${PACKAGE}'
	AC_SUBST(docdir)
fi

if test "$htmldir" = ""; then
	htmldir='${docdir}'
	AC_SUBST(htmldir)
fi

if test "$pdfdir" = ""; then
	pdfdir='${docdir}'
	AC_SUBST(pdfdir)
fi

#swig
AC_PATH_PROG(SWIG, swig, "no")
if test "$SWIG" == "no" ; then
	AC_MSG_WARN([swig not found, will not be able to build swig based bindings])
fi

#python
AC_ARG_ENABLE([python-binding],[  --disable-python-binding	Disable building of Python binding],
	[ENABLE_PYTHON=$enableval],[ENABLE_PYTHON="yes"])
if test "$ENABLE_PYTHON" == "yes" && test "$SWIG" != "no"; then
	AM_PATH_PYTHON
	AM_CHECK_PYTHON_HEADERS(HAVE_PYTHON=yes,[HAVE_PYTHON=no;AC_MSG_WARN([python headers not found, disabling python binding])])
else
	HAVE_PYTHON="no"
fi
AM_CONDITIONAL(HAVE_PYTHON, [test "$HAVE_PYTHON" == "yes" && test "$ENABLE_PYTHON" == "yes"])

AS_COMPILER_FLAG([-fno-strict-aliasing],[PYTHON_QUIET="$PYTHON_QUIET -fno-strict-aliasing"], true )
AS_COMPILER_FLAG([-Wno-unused-function],[PYTHON_QUIET="$PYTHON_QUIET -Wno-unused-function"], true )
AC_SUBST(PYTHON_QUIET)

# ruby
AC_ARG_ENABLE([ruby-binding], [  --disable-ruby-binding	Disable building of Ruby binding],
	[ENABLE_RUBY=$enableval], [ENABLE_RUBY="yes"])
if test "$ENABLE_RUBY" == "yes" && test "$SWIG" != "no"; then
	AC_PATH_PROG(RUBY, ruby, no)
	if test "$RUBY" != "no" ; then
		RUBY_INC_DIR=`$RUBY -e "require 'rbconfig'; c = ::Config::CONFIG; print c[['archdir']];"`
		AC_CHECK_HEADER([$RUBY_INC_DIR/ruby.h],[],[ENABLE_RUBY="no";AC_MSG_WARN([ruby.h not found, disabling Ruby binding])])
	else
		AC_MSG_WARN([ruby not found, disabling ruby binding])
		ENABLE_RUBY="no"
	fi
else
	ENABLE_RUBY="no"
fi

AM_CONDITIONAL(HAVE_RUBY, [test "$ENABLE_RUBY" == "yes"])

AC_ARG_VAR(RUBY_PREFIX,[path prefix for Ruby binding])
AC_ARG_VAR(RUBY_SO_DIR,[path for Ruby extensions])
if test "$RUBY_PREFIX" != "" ; then
	RUBY_CONFIG_OPTIONS="$RUBY_CONFIG_OPTIONS --prefix=\$(RUBY_PREFIX)"
fi
if test "$RUBY_SO_DIR" != "" ; then
	RUBY_CONFIG_OPTIONS="$RUBY_CONFIG_OPTIONS --so-dir=\$(RUBY_SO_DIR)"
fi
AC_SUBST(RUBY_CONFIG_OPTIONS)

# scxi
AC_ARG_ENABLE([scxi], [  --enable-scxi  Enable SCXI convenience library],
	[ENABLE_SCXI=$enableval], [ENABLE_SCXI="no"])
AM_CONDITIONAL(BUILD_SCXI, [test "$ENABLE_SCXI" == "yes"])

# docbook
AC_ARG_ENABLE([docbook],[  --disable-docbook-binding        Disable docbook],[ENABLE_DOCBOOK=$enableval],[ENABLE_DOCBOOK="yes"])

if test "$ENABLE_DOCBOOK" == "yes"; then
	AC_PATH_PROG(XMLTO, xmlto, no)
	if test "$XMLTO" = "no" ; then
  		AC_MSG_WARN([xmlto not found, will not be able to rebuild documentation])
	fi
else
	XMLTO="no"
fi
AM_CONDITIONAL(HAVE_XMLTO, [test "$XMLTO" != "no"])

pcmciadir="\${sysconfdir}/pcmcia"
AC_SUBST(pcmciadir)



# new udev hotplug
AC_ARG_WITH([udev-hotplug], [  --with-udev-hotplug=[[/lib]]    enable udev hotplug],
	[ENABLE_UDEVHOTPLUG=$withval], [ENABLE_UDEVHOTPLUG="no"])
AM_CONDITIONAL(INSTALL_UDEVHOTPLUG, [test "$ENABLE_UDEVHOTPLUG" != "no"])
if test "$ENABLE_UDEVHOTPLUG" != "no"; then
	udevrulesdir="\${sysconfdir}/udev/rules.d/"
	if  test "$ENABLE_UDEVHOTPLUG" == "yes"; then
		AC_MSG_ERROR([udev-hotplug needs a path as an argument (usually: --enable-udev-hotplug=/lib).])
	fi
	if test "$ENABLE_UDEVHOTPLUG" != "/lib"; then
		AC_MSG_WARN([Installing the udev scripts in the non-standard location: $ENABLE_UDEVHOTPLUG (should be --enable-udev-hotplug=/lib)])
	fi
	if test "$sysconfdir" != "/etc"; then
		AC_MSG_WARN([udev hotplug works only if sysconfdir is set to /etc.])
	fi
	udevfirmwaredir="$ENABLE_UDEVHOTPLUG/firmware"
	udevscriptsdir="$ENABLE_UDEVHOTPLUG/udev"
	AC_SUBST(udevrulesdir)
	AC_SUBST(udevfirmwaredir)
	AC_SUBST(udevscriptsdir)
else
	ENABLE_UDEVHOTPLUG="no"
fi


# old hotplug mechanism
AC_ARG_ENABLE([etc-hotplug], [  --enable-etc-hotplug    enable old hotplug in /etc/hotplug],
	[ENABLE_ETCHOTPLUG=$enableval], [ENABLE_ETCHOTPLUG="no"])
AM_CONDITIONAL(INSTALL_ETCHOTPLUG, [test "$ENABLE_ETCHOTPLUG" != "no"])

if test "$ENABLE_ETCHOTPLUG" != "no"; then
#see: http://linux-hotplug.sourceforge.net/
#the hotplug expects the device dependent scripts here:
	usbhotplugdir="\${sysconfdir}/hotplug/usb"
	AC_SUBST(usbhotplugdir)

#firmware for the hotplug script
#see: http://linux-hotplug.sourceforge.net/
	usbfirmwaredir="\${datadir}/usb"
	AC_SUBST(usbfirmwaredir)
	if test "$sysconfdir" != "/etc"; then
		AC_MSG_WARN([hotplug works only if sysconfdir is set to /etc.])
	fi
fi

if test "$ENABLE_ETCHOTPLUG" = "no" && test "$ENABLE_UDEVHOTPLUG" = "no" ; then
		AC_MSG_WARN([No hotplug mechanism will be installed. Consult ./configure --help if you want hotplug.])
fi

AC_CONFIG_FILES(
Makefile
comedi_config/Makefile
c++/Makefile
c++/include/Makefile
etc/Makefile
etc/pcmcia/Makefile
etc/hotplug/Makefile
etc/hotplug/usb/Makefile
etc/hotplug/usb/usbdux/Makefile
etc/hotplug/usb/usbduxfast/Makefile
etc/udev/Makefile
doc/Makefile
demo/Makefile
include/Makefile
lib/Makefile
man/Makefile
scxi/Makefile
swig/Makefile
swig/python/Makefile
swig/ruby/Makefile
testing/Makefile
comedilib.spec
comedilib.pc
)

AC_OUTPUT
