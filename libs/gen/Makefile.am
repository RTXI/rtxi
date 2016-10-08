CLEANFILES = *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

genincludedir = $(pkgincludedir)/libs

genlibdir = $(pkglibdir)/libs

geninclude_HEADERS = \
		gen_biphase.h \
		generator.h \
		gen_mono.h \
		gen_saw.h \
		gen_sine.h \
		gen_whitenoise.h \
		gen_zap.h

genlib_LTLIBRARIES = librtgen.la

librtgen_la_LDFLAGS = -lgsl -lgslcblas -lm -ldl -lpthread -lGL -shared -avoid-version

librtgen_la_SOURCES = \
		gen_biphase.cpp \
		generator.cpp \
		gen_mono.cpp \
		gen_saw.cpp \
		gen_sine.cpp \
		gen_whitenoise.cpp \
		gen_zap.cpp
