CLEANFILES = *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/.buildvars

pkglib_LTLIBRARIES = neuron.la

neuron_la_LDFLAGS = -module -avoid-version

neuron_la_SOURCES = \
		neuron.h \
		neuron.cpp
