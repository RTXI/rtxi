CLEANFILES = *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pkglib_LTLIBRARIES = librtgen.la

librtgen_la_LDFLAGS = -lgsl -lgslcblas -lm -ldl -lpthread -lGL -shared

librtgen_la_SOURCES = \
		gen_biphase.cpp \
		generator.cpp \
		gen_mono.cpp \
		gen_saw.cpp \
		gen_sine.cpp \
		gen_whitenoise.cpp \
		gen_zap.cpp
