CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pkglib_LTLIBRARIES = am_2400_commander.la

am_2400_commander_la_LDFLAGS = -module -avoid-version -lgsl -lgslcblas

am_2400_commander_la_SOURCES = \
		am_2400_commander.h \
		am_2400_commander.cpp \
		am_2400_commanderUI.cpp	
nodist_am_2400_commander_la_SOURCES = \
		moc_am_2400_commander.cpp \
		moc_am_2400_commanderUI.cpp

# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<