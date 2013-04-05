CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pkglib_LTLIBRARIES = model_loader.la

model_loader_la_LDFLAGS = -module -avoid-version

model_loader_la_SOURCES = \
		model_loader.h \
		model_loader.cpp

nodist_model_loader_la_SOURCES = \
		moc_model_loader.cpp

# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<
