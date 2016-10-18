CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

plotincludedir = $(pkgincludedir)/libs

plotlibdir = $(pkglibdir)/libs

plotinclude_HEADERS = \
		basicplot.h \
		incrementalplot.h \
		plotdialog.h \
		scatterplot.h \
		scrollbar.h \
		scrollzoomer.h

plotlib_LTLIBRARIES = librtplot.la

librtplot_la_LDFLAGS = -lQt5OpenGL -lQt5PrintSupport -lQt5Xml -lQt5Svg -lQt5Widgets -lQt5Gui -lQt5Network -lQt5Core -lqwt-qt5 -lgsl -lgslcblas -lm -ldl -lpthread -lGL -shared -avoid-version

librtplot_la_SOURCES = \
		basicplot.cpp \
		incrementalplot.cpp \
		plotdialog.cpp \
		scatterplot.cpp \
		scrollbar.cpp \
		scrollzoomer.cpp

nodist_librtplot_la_SOURCES = \
		moc_basicplot.cpp \
		moc_incrementalplot.cpp \
		moc_plotdialog.cpp \
		moc_scatterplot.cpp \
		moc_scrollbar.cpp \
		moc_scrollzoomer.cpp

# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<
