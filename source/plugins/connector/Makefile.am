CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pkglib_LTLIBRARIES = connector.la

connector_la_LDFLAGS = -module -avoid-version

connector_la_SOURCES = \
		connector.h \
		connector.cpp
nodist_connector_la_SOURCES = \
		moc_connector.cpp

# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<
