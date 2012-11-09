CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pkglib_LTLIBRARIES = axon_multiclamp700_commander.la

axon_multiclamp700_commander_la_LDFLAGS = -module -avoid-version -lgsl -lgslcblas

axon_multiclamp700_commander_la_SOURCES = \
		axon_multiclamp700_commander.h \
		axon_multiclamp700_commander.cpp \
		axon_multiclamp700_commanderUI.cpp	
nodist_axon_multiclamp700_commander_la_SOURCES = \
		moc_axon_multiclamp700_commander.cpp \
		moc_axon_multiclamp700_commanderUI.cpp

# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<