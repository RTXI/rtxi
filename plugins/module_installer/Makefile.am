CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pkglib_LTLIBRARIES = module_installer.la

LIBS = -lgit2 -lmarkdown -lQt5Network

module_installer_la_LDFLAGS = -module -avoid-version

module_installer_la_SOURCES = \
					rtxi_wizard.h \
          module_utils.h \
          rtxi_wizard.cpp \
          module_utils.cpp 
nodist_module_installer_la_SOURCES = \
          moc_rtxi_wizard.cpp

# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<
