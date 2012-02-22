CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pkglib_LTLIBRARIES = wave_maker.la

wave_maker_la_LDFLAGS = -module -avoid-version -lqwt

wave_maker_la_SOURCES = \
		wave_maker.h \
		wave_maker.cpp\
		../include/plotdialog.h\
		../include/plotdialog.cpp\
		../include/basicplot.h\
		../include/basicplot.cpp\
    	../include/scrollzoomer.h\
    	../include/scrollzoomer.cpp\
    	../include/scrollbar.h\
    	../include/scrollbar.cpp

nodist_wave_maker_la_SOURCES = \
   		moc_wave_maker.cpp\
    	../include/moc_scrollbar.cpp\
    	../include/moc_scrollzoomer.cpp\
    	../include/moc_basicplot.cpp\
		../include/moc_plotdialog.cpp
		
# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<
		