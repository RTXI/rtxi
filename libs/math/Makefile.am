CLEANFILES = *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pkglib_LTLIBRARIES = librtmath.la

librtmath_la_LDFLAGS = -lgsl -lgslcblas -lm -ldl -lpthread -lGL -shared

librtmath_la_SOURCES = \
		powfast.cpp \
		rtmath.cpp \
		runningstat.cpp 
