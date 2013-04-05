CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pkglib_LTLIBRARIES = axon_axopatch200_commander.la

axon_axopatch200_commander_la_LDFLAGS = -module -avoid-version -lgsl -lgslcblas

axon_axopatch200_commander_la_SOURCES = \
		axon_axopatch200_commander.h \
		axon_axopatch200_commander.cpp \
		axon_axopatch200_commanderUI.cpp	
nodist_axon_axopatch200_commander_la_SOURCES = \
		moc_axon_axopatch200_commander.cpp \
		moc_axon_axopatch200_commanderUI.cpp

# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<