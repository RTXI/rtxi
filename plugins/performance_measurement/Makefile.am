CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pluginincludedir = $(pkgincludedir)/plugins

plugininclude_HEADERS = \
		performance_measurement.h

pkglib_LTLIBRARIES = performance_measurement.la

performance_measurement_la_LDFLAGS = -module -avoid-version

performance_measurement_la_SOURCES = \
		performance_measurement.h \
		runningstat.h \
		performance_measurement.cpp \
		runningstat.cpp 
nodist_performance_measurement_la_SOURCES = \
		moc_performance_measurement.cpp

# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<
