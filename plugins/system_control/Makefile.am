CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pkglib_LTLIBRARIES = system_control.la

system_control_la_LDFLAGS = -module -avoid-version

system_control_la_SOURCES = \
		system_control.h \
		system_control_panel.h \
		system_control.cpp \
		system_control_panel.cpp
nodist_system_control_la_SOURCES = \
		moc_system_control.cpp \
		moc_system_control_panel.cpp

# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<
