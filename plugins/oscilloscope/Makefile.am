CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pluginincludedir = $(pkgincludedir)/plugins

plugininclude_HEADERS = \
		oscilloscope.h \
		scope.h

pkglib_LTLIBRARIES = oscilloscope.la

oscilloscope_la_LDFLAGS = -lqwt-qt5 -module -avoid-version 

oscilloscope_la_SOURCES = \
		oscilloscope.h \
		scope .h \
		oscilloscope.cpp \
		scope.cpp 
nodist_oscilloscope_la_SOURCES = \
		moc_oscilloscope.cpp \
		moc_scope.cpp

# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<
