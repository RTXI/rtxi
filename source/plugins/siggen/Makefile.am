CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pkglib_LTLIBRARIES = siggen.la

siggen_la_LDFLAGS = -module -avoid-version -lqwt

siggen_la_SOURCES = \
		siggen.h \
		siggen.cpp\
        ../include/gen_sine.cpp\
        ../include/gen_sine.h\
        ../include/gen_biphase.cpp\
        ../include/gen_biphase.h\
        ../include/gen_mono.cpp\
        ../include/gen_mono.h\
        ../include/gen_saw.cpp\
        ../include/gen_saw.h\
        ../include/gen_zap.cpp\
        ../include/gen_zap.h\
        ../include/generator.cpp\
        ../include/generator.h

nodist_siggen_la_SOURCES = \
   		moc_siggen.cpp
		
# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<
		