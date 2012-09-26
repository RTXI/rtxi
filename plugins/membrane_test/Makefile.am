CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pkglib_LTLIBRARIES = clamp_utilities.la

clamp_utilities_la_LDFLAGS = -module -avoid-version -lgsl -lgslcblas

clamp_utilities_la_SOURCES = \
		clamp_utilities.h \
		clamp_utilities.cpp \
		clamp_utilitiesUI.cpp	
nodist_clamp_utilities_la_SOURCES = \
		moc_clamp_utilities.cpp \
		moc_clamp_utilitiesUI.cpp

# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<