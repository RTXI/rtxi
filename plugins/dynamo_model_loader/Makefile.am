CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pkglib_LTLIBRARIES = dynamo_model_loader.la

dynamo_model_loader_la_LDFLAGS = -module -avoid-version -lpthread

dynamo_model_loader_la_SOURCES = \
		dynamo_model_loader.h \
		dynamo_model_loader.cpp

nodist_dynamo_model_loader_la_SOURCES = \
		moc_dynamo_model_loader.cpp

# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<
