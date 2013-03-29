
SUBDIRS = . kcomedilib drivers

CLEANFILES = Modules.symvers Module.symvers

COMEDI_MOD_DIR = "comedi"

if CONFIG_KBUILD
module_PROGRAMS =
all-local:
#if builddir is not srcdir, make symbolic links in the builddir to the source files so
#the kernel's build system doesn't get lost.
	if test "$(srcdir)" != "."; then \
		for dir in . drivers kcomedilib drivers/addi-data; do \
			for file in `ls @abs_srcdir@/$$dir/*\.[ch] @abs_srcdir@/$$dir/Kbuild | $(EGREP) -v \.mod\.c`; do \
				LINK_NAME=$$dir/`basename "$$file"`; \
				if test ! -e $$LINK_NAME; then ln -vs $$file $$LINK_NAME; fi; \
			done; \
		done; \
	fi
#invoke kernel's build system
#ignore errors from kernel's make modules in all-local, to avoid failure of make install
#on nfs filesystem with root_squash
	-$(MAKE) -I@abs_builddir@ -C $(LINUX_DIR) M=@abs_builddir@ CC="$(LINUX_CC) -I@abs_top_builddir@/ \
		-I@abs_top_srcdir@/include $(RTAI_CFLAGS) $(RTLINUX_CFLAGS)" modules
install-data-local:
	$(MAKE) -I@abs_builddir@ -C $(LINUX_DIR) M=@abs_builddir@ CC="$(LINUX_CC) -I@abs_top_builddir@/ \
		-I@abs_top_srcdir@/include $(RTAI_CFLAGS) $(RTLINUX_CFLAGS)" INSTALL_MOD_DIR=$(COMEDI_MOD_DIR) \
		INSTALL_MOD_PATH="$(DESTDIR)$(prefix)" modules_install
clean-local:
	$(MAKE) -I@abs_builddir@ -C $(LINUX_DIR) M=@abs_builddir@ CC="$(LINUX_CC) -I@abs_top_builddir@/ \
		-I@abs_top_srcdir@/include $(RTAI_CFLAGS) $(RTLINUX_CFLAGS)" clean
uninstall-local:
	find $(DESTDIR)$(prefix)/$(LINUX_MODLIB)/$(COMEDI_MOD_DIR) -name '*.ko' -exec $(RM) {} \;
else
module_PROGRAMS = comedi.ko
all-local:
install-data-local:
clean-local:
uninstall-local:
endif

EXTRA_DIST = \
 Kbuild \
 rt_pend_tq.h \
 comedi_fops.h \
 comedi_compat32.h

if CONFIG_COMEDI_RT
RT_SOURCES = rt_pend_tq.c rt.c
else
RT_SOURCES =
endif

comedi_ko_SOURCES = \
 comedi_fops.c \
 proc.c \
 range.c \
 drivers.c \
 comedi_compat32.c \
 comedi_ksyms.c \
 $(RT_SOURCES)

comedi_ko_CFLAGS = $(COMEDI_CFLAGS) $(LINUX_CFLAGS) $(RTAI_CFLAGS) $(RTLINUX_CFLAGS)
comedi_ko_LINK = $(top_builddir)/modtool --link -o $@

#comedi_ksyms.o: comedi_ksyms.c
#	$(CC) -o $@ comedi_ksyms.c $(CFLAGS) $(COMEDI_CFLAGS) $(LINUX_CFLAGS) -DEXPORT_SYMTAB

clean-modulePROGRAMS:
	-test -z "$(module_PROGRAMS)" || rm -f $(module_PROGRAMS)
	-rm -rf .mods
