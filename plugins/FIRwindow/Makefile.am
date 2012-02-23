CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pkglib_LTLIBRARIES = FIRwindow.la

FIRwindow_la_LDFLAGS = -module -avoid-version -lqwt

FIRwindow_la_SOURCES = \
		FIRwindow.h \
		FIRwindow.cpp\
		../include/DSP/gen_win.cpp\
		../include/DSP/rectnglr.cpp\
		../include/DSP/trianglr.cpp\
		../include/DSP/hamming.cpp\
		../include/DSP/hann.cpp\
		../include/DSP/dolph.cpp\
		../include/DSP/kaiser.cpp\
		../include/DSP/acosh.cpp\
		../include/DSP/firideal.cpp\
		../include/DSP/fir_dsgn.cpp\
		../include/DSP/lin_dsgn.cpp\
		../include/DSP/fir_resp.cpp\
		../include/DSP/gen_win.h\
		../include/DSP/rectnglr.h\
		../include/DSP/trianglr.h\
		../include/DSP/hamming.h\
		../include/DSP/hann.h\
		../include/DSP/dolph.h\
		../include/DSP/kaiser.h\
		../include/DSP/acosh.h\
		../include/DSP/firideal.h\
		../include/DSP/fir_dsgn.h\
		../include/DSP/lin_dsgn.h\
		../include/DSP/fir_resp.h

nodist_FIRwindow_la_SOURCES = \
   		moc_FIRwindow.cpp
		
# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<
		