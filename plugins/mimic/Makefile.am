CLEANFILES = *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pkglib_LTLIBRARIES = mimic.la

mimic_la_LDFLAGS = -module -avoid-version

mimic_la_SOURCES = \
		mimic.h \
		mimic.cpp
