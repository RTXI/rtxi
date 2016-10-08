CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pluginincludedir = $(pkgincludedir)/plugins

librtmath_dir = $(top_srcdir)/libs/math

AM_CXXFLAGS += \
		-I$(librtmath_dir)

plugininclude_HEADERS = \
		performance_measurement.h

pkglib_LTLIBRARIES = performance_measurement.la

performance_measurement_la_LDFLAGS = -module -avoid-version

performance_measurement_la_SOURCES = \
		performance_measurement.h \
		$(librtmath_dir)/runningstat.h \
		performance_measurement.cpp \
		$(librtmath_dir)/runningstat.cpp 
nodist_performance_measurement_la_SOURCES = \
		moc_performance_measurement.cpp

# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<
