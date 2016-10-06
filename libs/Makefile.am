CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)../../Makefile.buildvars

librtxi_la_LDFLAGS = -module -avoid-version -lqwt

librtxi_la_SOURCES = \
		basicplot.h \
		basicplot.cpp \
		gen_biphase.h \
		gen_biphase.cpp \
		generator.h \
		generator.cpp \
		gen_mono.h \
		gen_mono.cpp \
		gen_saw.h \
		gen_saw.cpp \
		gen_sine.h \
		gen_sine.cpp \
		gen_whitenoise.h \
		gen_whitenoise.cpp \
		gen_zap.h \
		gen_zap.cpp \
		incrementalplot.h \
		incrementalplot.cpp \
		plotdialog.h \
		plotdialog.cpp \
		powfast.hpp \
		powfast.cpp \
		rtmath.h \
		rtmath.cpp \
		runningstat.h \
		runningstat.cpp \
		scatterplot.h \
		scatterplot.cpp \
		scrollbar.h \
		scrollbar.cpp \
		scrollzoomer.h \
		scrollzoomer.cpp 

nodist_librtxi_la_SOURCES = \
		moc_basicplot.cpp \
		moc_incrementalplot.cpp \
		moc_plotdialog.cpp
		moc_scatterplot.cpp \
		moc_scrollbar.cpp \
		moc_scrollzommer.cpp \

moc_%.cpp: %h
	$(MOC) -o $@ $<
