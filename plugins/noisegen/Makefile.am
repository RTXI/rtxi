CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pkglib_LTLIBRARIES = noisegen.la

noisegen_la_LDFLAGS = -module -avoid-version -lgsl

noisegen_la_SOURCES = \
		noisegen.h \
		noisegen.cpp\
        ../include/gen_whitenoise.cpp\
        ../include/gen_whitenoise.h\
        ../include/generator.cpp\
        ../include/generator.h

nodist_noisegen_la_SOURCES = \
   		moc_noisegen.cpp

		
# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<
		