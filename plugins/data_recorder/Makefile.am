CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pkglib_LTLIBRARIES = data_recorder.la

LIBS = $(HDF_LIBS)

data_recorder_la_LDFLAGS = -module -avoid-version

data_recorder_la_SOURCES = \
		data_recorder.h \
		data_recorder.cpp
nodist_data_recorder_la_SOURCES = \
		moc_data_recorder.cpp

# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<
