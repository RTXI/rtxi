CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pkglib_LTLIBRARIES = axon_axopatch1d_commander.la

axon_axopatch1d_commander_la_LDFLAGS = -module -avoid-version -lgsl -lgslcblas

axon_axopatch1d_commander_la_SOURCES = \
		axon_axopatch1d_commander.h \
		axon_axopatch1d_commander.cpp \
		axon_axopatch1d_commanderUI.cpp	
nodist_axon_axopatch1d_commander_la_SOURCES = \
		moc_axon_axopatch1d_commander.cpp \
		moc_axon_axopatch1d_commanderUI.cpp

# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<