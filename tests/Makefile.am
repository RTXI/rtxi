# TESTS -- Programs run automatically by "make check"
# check_PROGRAMS -- Programs built by "make check" but not necessarily run

include $(top_srcdir)/Makefile.buildvars

AM_CXXFLAGS += -I$(top_srcdir)/libs -I$(top_srcdir)/plugins

TESTS = rtxiTests 
check_PROGRAMS = rtxiTests

bin_PROGRAMS = rtxiTests

pkginclude_HEADERS = system_tests.h \
                     event_tests.h \
                     io_tests.h \
                     daq_tests.h \
                     settings_tests.h \
                     workspace_tests.h \
                     plugin_tests.h \
                     fakePlugin.h \
                     performance_measurement_plugin_tests.h \
                     connector_plugin_tests.h

rtxiTests_SOURCES = rtxiTests.cpp \
                    system_tests.cpp \
                    event_tests.cpp \
                    io_tests.cpp \
                    daq_tests.cpp \
                    settings_tests.cpp \
                    workspace_tests.cpp \
                    plugin_tests.cpp \
                    performance_measurement_plugin_tests.cpp \
                    connector_plugin_tests.cpp

OBJS = $(top_srcdir)/src/rt.o \
       $(top_srcdir)/src/event.o \
       $(top_srcdir)/src/atomic_fifo.o \
       $(top_srcdir)/src/daq.o \
       $(top_srcdir)/src/fifo.o \
       $(top_srcdir)/src/io.o \
       $(top_srcdir)/src/mutex.o \
       $(top_srcdir)/src/plugin.o \
       $(top_srcdir)/src/sem.o \
       $(top_srcdir)/src/settings.o \
       $(top_srcdir)/src/workspace.o \
       $(top_srcdir)/src/main_window.o \
       $(top_srcdir)/src/default_gui_model.o \
       $(top_srcdir)/plugins/performance_measurement/*.o \
       $(top_srcdir)/libs/math/runningstat.o \
       $(top_srcdir)/src/moc_*.o

noinst_LTLIBRARIES = fakePlugin.la
fakePlugin_la_SOURCES = fakePlugin.h fakePlugin.cpp

if XENOMAI
OBJS += $(top_srcdir)/src/rt_os-xenomai.o 
else
OBJS += $(top_srcdir)/src/rt_os-posix.o 
endif

rtxiTests_LDADD = -lgtest -lgmock $(OBJS)

# Sometimes in life you encounter the most complex and
# crazy ways of doing the most simple tasks. Exibit A:
# all i wanted to do was to generate a test plugin based
# on Plugin::Object to run my tests on. It should not have
# taken this long to figure out.
fakePlugin_la_LDFLAGS = $(AM_LDFLAGS) \
                        -module \
                        -shared \
                        -avoid-version \
                        -export-dynamic \
                        -rpath $(abs_builddir) \
                        $(OBJS) 



