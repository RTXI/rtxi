CLEANFILES = *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pkglib_LTLIBRARIES = synch.la

synch_la_LDFLAGS = -module -avoid-version

synch_la_SOURCES = \
		synch.h \
		synch.cpp