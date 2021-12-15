CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

AM_CXXFLAGS += -I$(top_srcdir)/libs

pluginincludedir = $(pkgincludedir)/plugins

plugininclude_HEADERS = \
		performance_measurement.h

pkglib_LTLIBRARIES = performance_measurement.la

performance_measurement_la_LDFLAGS = -module -avoid-version $(top_srcdir)/libs/math/runningstat.o

performance_measurement_la_SOURCES = \
		performance_measurement.h \
		runningstat.h \
		performance_measurement.cpp 
nodist_performance_measurement_la_SOURCES = \
		moc_performance_measurement.cpp

# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<
