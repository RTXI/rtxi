include $(top_srcdir)/Makefile.buildvars

check_PROGRAMS = run
check_LTLIBRARIES = test_plugin_module.la

AM_CXXFLAGS += $(CPPUNIT_CFLAGS) -W -g -DDEBUG -DTESTSUITE_DIR=\"$(abs_top_srcdir)/testsuite\"
LIBS = -rdynamic $(CPPUNIT_LIBS)

run_SOURCES = \
		$(top_srcdir)/include/rt.h \
		$(top_srcdir)/include/fifo.h \
		$(top_srcdir)/include/rtfile.h \
		$(top_srcdir)/include/event.h \
		$(top_srcdir)/include/mutex.h \
		$(top_srcdir)/include/plugin.h \
		$(top_srcdir)/include/settings.h \
		$(top_srcdir)/incldue/io.h \
		\
		$(top_srcdir)/src/fifo.cpp \
		$(top_srcdir)/src/rtfile.cpp \
		$(top_srcdir)/src/rt.cpp \
		$(top_srcdir)/src/event.cpp \
		$(top_srcdir)/src/mutex.cpp \
		$(top_srcdir)/src/plugin.cpp \
		$(top_srcdir)/src/settings.cpp \
		$(top_srcdir)/src/io.cpp \
		\
		main.cpp \
		test_rt_os.h \
		test_rt_os.cpp \
		test_rt.h \
		test_rt.cpp \
		test_fifo.h \
		test_fifo.cpp \
		test_rtfile.h \
		test_rtfile.cpp \
		test_event.h \
		test_event.cpp \
		test_plugin.h \
		test_plugin.cpp \
		test_settings.h \
		test_settings.cpp

nodist_run_SOURCES = \
		$(top_srcdir)/src/moc_plugin.cpp

if RTAI3
run_SOURCES += \
		$(top_srcdir)/src/rt_os-rtai3.cpp
endif
if XENOMAI
run_SOURCES += \
		$(top_srcdir)/src/rt_os-xenomai.cpp
endif
if POSIX
run_SOURCES += \
		$(top_srcdir)/src/rt_os-posix.cpp
endif

test_plugin_module_la_LDFLAGS = -module -avoid-version

test_plugin_module_la_SOURCES = \
		test_plugin_module.h \
		test_plugin_module.cpp

test_plugin_module.so:
	$(CXXLINK) $(CXXFLAGS) $(LIBS) $(LDFLAGS) $(test_plugin_module_la_LDFLAGS) -rpath `readlink -f $(top_srcdir)/testsuite` -o test_plugin_module.la test_plugin_module.lo
	$(LIBTOOL) --mode=install cp test_plugin_module.la `readlink -f $(top_srcdir)/testsuite`

check-local: test_plugin_module.so
	$(top_srcdir)/testsuite/run

# MOC Rule - builds meta-object files as needed
moc_%.cpp: $(top_srcdir)/include/%.h
	$(MOC) -o $@ $<
