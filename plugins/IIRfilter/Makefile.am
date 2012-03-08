CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pkglib_LTLIBRARIES = IIRfilter.la

IIRfilter_la_LDFLAGS = -module -avoid-version 

IIRfilter_la_SOURCES = \
		IIRfilter.h \
		IIRfilter.cpp\
		../include/DSP/iir_dsgn.cpp\
		../include/DSP/dir1_iir.cpp\
		../include/DSP/unq_iir.cpp\
		../include/DSP/buttfunc.cpp\
		../include/DSP/chebfunc.cpp\
		../include/DSP/elipfunc.cpp\
		../include/DSP/bilinear.cpp\
		../include/DSP/iir_dsgn.h\
		../include/DSP/dir1_iir.h\
		../include/DSP/unq_iir.h\
		../include/DSP/buttfunc.h\
		../include/DSP/chebfunc.h\
		../include/DSP/elipfunc.h\
		../include/DSP/bilinear.h

nodist_IIRfilter_la_SOURCES = \
   		moc_IIRfilter.cpp
		
# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<
		