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
		../include/DSP/bessfunc.cpp\
		../include/DSP/bilinear.cpp\
		../include/DSP/filtfunc.cpp\
		../include/DSP/cmpxpoly.cpp\
		../include/DSP/poly.cpp\
		../include/DSP/pause.cpp\
		../include/DSP/complex.cpp\
		../include/DSP/unwrap.cpp\
		../include/DSP/laguerre.cpp\
		../include/DSP/log2.cpp\
		../include/DSP/iir_dsgn.h\
		../include/DSP/dir1_iir.h\
		../include/DSP/unq_iir.h\
		../include/DSP/buttfunc.h\
		../include/DSP/chebfunc.h\
		../include/DSP/elipfunc.h\
		../include/DSP/bessfunc.h\
		../include/DSP/bilinear.h\
		../include/DSP/filtfunc.h\
		../include/DSP/cmpxpoly.h\
		../include/DSP/poly.h\
		../include/DSP/pause.h\
		../include/DSP/complex.h\
		../include/DSP/unwrap.h\
		../include/DSP/laguerre.h\
		../include/DSP/log2.h

nodist_IIRfilter_la_SOURCES = \
   		moc_IIRfilter.cpp
		
# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<
		