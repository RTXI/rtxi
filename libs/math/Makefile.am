CLEANFILES = *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

mathincludedir = $(pkgincludedir)/libs

mathlibdir = $(pkglibdir)/libs

mathinclude_HEADERS = \
		powfast.hpp \
		rtmath.h \
		runningstat.h

mathlib_LTLIBRARIES = librtmath.la

librtmath_la_LDFLAGS = -lgsl -lgslcblas -lm -ldl -lpthread -lGL -shared -avoid-version

librtmath_la_SOURCES = \
		powfast.cpp \
		rtmath.cpp \
		runningstat.cpp 
