CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pluginincludedir = $(pkgincludedir)/plugins

plugininclude_HEADERS = \
		rtxi_wizard.h

pkglib_LTLIBRARIES = module_installer.la

LIBS = -lgit2 -lmarkdown -lQt5Network

module_installer_la_LDFLAGS = -module -avoid-version

module_installer_la_SOURCES = \
		rtxi_wizard.h \
		rtxi_wizard.cpp \
nodist_module_installer_la_SOURCES = \
		moc_rtxi_wizard.cpp

# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<
