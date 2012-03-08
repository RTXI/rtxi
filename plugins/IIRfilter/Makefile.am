CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pkglib_LTLIBRARIES = IIRfilter.la

IIRfilter_la_LDFLAGS = -module -avoid-version -lqwt

IIRfilter_la_SOURCES = \
		IIRfilter.h \
		IIRfilter.cpp\
		../include/DSP/iir_dsgn.cpp\
		../include/DSP/iir_resp.cpp

nodist_IIRfilter_la_SOURCES = \
   		moc_IIRfilter.cpp
		
# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<
		