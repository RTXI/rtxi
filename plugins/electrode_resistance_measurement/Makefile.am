CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pkglib_LTLIBRARIES = electrode_resistance_measurement.la

electrode_resistance_measurement_la_LDFLAGS = -module -avoid-version

electrode_resistance_measurement_la_SOURCES = \
		electrode_resistance_measurement.h \
		electrode_resistance_measurement.cpp
nodist_electrode_resistance_measurement_la_SOURCES = \
		moc_electrode_resistance_measurement.cpp

# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<
