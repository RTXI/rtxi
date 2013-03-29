CLEANFILES = *~
DISTCLEANFILES = 
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

bin_PROGRAMS = rtxi

LIBS = -rdynamic

pkginclude_HEADERS = \
                $(top_srcdir)/include/cmdline.h \
		$(top_srcdir)/include/compiler.h \
                $(top_srcdir)/include/daq.h \
		$(top_srcdir)/include/default_gui_model.h \
		$(top_srcdir)/include/debug.h \
		$(top_srcdir)/include/event.h \
		$(top_srcdir)/include/fifo.h \
		$(top_srcdir)/include/io.h \
		$(top_srcdir)/include/main_window.h \
		$(top_srcdir)/include/mutex.h \
		$(top_srcdir)/include/plugin.h \
		$(top_srcdir)/include/rt.h \
		$(top_srcdir)/include/rtfile.h \
		$(top_srcdir)/include/rwlock.h \
		$(top_srcdir)/include/sem.h \
		$(top_srcdir)/include/settings.h \
		$(top_srcdir)/include/workspace.h 

rtxi_SOURCES = \
		$(top_srcdir)/src/cmdline.cpp \
		$(top_srcdir)/src/daq.cpp \
		$(top_srcdir)/src/default_gui_model.cpp \
		$(top_srcdir)/src/event.cpp \
		$(top_srcdir)/src/fifo.cpp \
		$(top_srcdir)/src/io.cpp \
		$(top_srcdir)/src/main.cpp \
		$(top_srcdir)/src/main_window.cpp \
		$(top_srcdir)/src/mutex.cpp \
		$(top_srcdir)/src/plugin.cpp \
		$(top_srcdir)/src/rt.cpp \
		$(top_srcdir)/src/rtfile.cpp \
		$(top_srcdir)/src/rwlock.cpp \
		$(top_srcdir)/src/sem.cpp \
		$(top_srcdir)/src/settings.cpp \
		$(top_srcdir)/src/workspace.cpp

nodist_rtxi_SOURCES = \
		$(top_srcdir)/include/config.h \
		moc_default_gui_model.cpp \
		moc_main_window.cpp \
		moc_plugin.cpp 

EXTRA_DIST = \
		$(top_srcdir)/src/rt_os-posix.cpp \
		$(top_srcdir)/src/rt_os-rtai3.cpp \
		$(top_srcdir)/src/rt_os-xenomai.cpp

if RTAI3
rtxi_SOURCES += \
		$(top_srcdir)/src/rt_os-rtai3.cpp
endif
if XENOMAI
rtxi_SOURCES += \
		$(top_srcdir)/src/rt_os-xenomai.cpp
endif
if POSIX
rtxi_SOURCES += \
		$(top_srcdir)/src/rt_os-posix.cpp
endif

# MOC Rule - builds meta-object files as needed
moc_%.cpp: $(top_srcdir)/include/%.h
	$(MOC) -o $@ $<
