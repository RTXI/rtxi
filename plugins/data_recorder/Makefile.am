CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pluginincludedir = $(pkgincludedir)/plugins

plugininclude_HEADERS = \
		data_recorder.h

pkglib_LTLIBRARIES = data_recorder.la

HDF_LIBS = $(shell pkg-config --libs hdf5) -lhdf5_hl

data_recorder_la_LDFLAGS = -module -avoid-version $(HDF_LIBS)

data_recorder_la_SOURCES = \
		data_recorder.h \
		data_recorder.cpp
nodist_data_recorder_la_SOURCES = \
		moc_data_recorder.cpp

# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<
