
SUBDIRS = lib comedi_config man testing demo doc swig \
	include etc scxi c++

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = comedilib.pc

## with automake 1.8, putting the m4 directory in EXTRA_DIST is not needed, and
## causes 'make distcheck' to fail, by way of 'make distdir' failing when builddir != srcdir 
## and srcdir is read-only.
EXTRA_DIST = debian comedilib.spec autogen.sh perl

ACLOCAL_AMFLAGS = -I m4

## ruby binding is excluded from distcheck because setup.rb doesn't support uninstall
DISTCHECK_CONFIGURE_FLAGS = RUBY_PREFIX='$${prefix}' --disable-ruby-binding

dist-hook:
	rm -rf `find $(distdir) -name CVS`

dev:
	-for i in `seq 0 15`; do \
		mknod -m 666 /dev/comedi$$i c 98 $$i \
		; \
	done;

