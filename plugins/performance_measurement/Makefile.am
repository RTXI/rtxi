CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pkglib_LTLIBRARIES = performance_measurement.la

performance_measurement_la_LDFLAGS = -module -avoid-version

performance_measurement_la_SOURCES = \
		performance_measurement.h \
		performance_measurement.cpp\
		../include/runningstat.h\
		../include/runningstat.cpp
		
nodist_performance_measurement_la_SOURCES = \
		moc_performance_measurement.cpp

# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<
