CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pluginincludedir = $(pkgincludedir)/plugins

AM_CPPFLAGS += \
                -I$(top_srcdir)/libs/math 

plugininclude_HEADERS = \
		performance_measurement.h

pkglib_LTLIBRARIES = performance_measurement.la

performance_measurement_la_LDFLAGS = -module -avoid-version

performance_measurement_LDADD = \
		$(top_srcdir)/libs/math/librtmath.la

performance_measurement_la_SOURCES = \
		performance_measurement.h \
		performance_measurement.cpp	
nodist_performance_measurement_la_SOURCES = \
		moc_performance_measurement.cpp

# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<
