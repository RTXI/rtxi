include $(top_srcdir)/Makefile.buildvars

check_PROGRAMS = run

CXXFLAGS = $(CPPUNIT_CFLAGS)
LIBS = $(CPPUNIT_LIBS)

run_SOURCES = \
		$(top_srcdir)/include/rt.h \
		$(top_srcdir)/include/fifo.h \
		$(top_srcdir)/include/no_copy_fifo.h \
		$(top_srcdir)/include/rtfile.h \
		$(top_srcdir)/include/event.h \
		$(top_srcdir)/include/mutex.h \
		\
		$(top_srcdir)/src/fifo.cpp \
		$(top_srcdir)/src/no_copy_fifo.cpp \
		$(top_srcdir)/src/rtfile.cpp \
		$(top_srcdir)/src/rt.cpp \
		$(top_srcdir)/src/event.cpp \
		$(top_srcdir)/src/mutex.cpp \
		\
		main.cpp \
		test_rt_os.h \
		test_rt_os.cpp \
		test_rt.h \
		test_rt.cpp \
		test_fifo.h \
		test_fifo.cpp \
		test_no_copy_fifo.h \
		test_no_copy_fifo.cpp \
		test_rtfile.h \
		test_rtfile.cpp \
		test_event.h \
		test_event.cpp

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
