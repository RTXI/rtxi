include $(top_srcdir)/Makefile.buildvars

check_PROGRAMS = run

CXXFLAGS = $(CPPUNIT_CFLAGS)
LIBS = $(CPPUNIT_LIBS)

run_SOURCES = \
		main.cpp \
		test_rt_os.h \
		test_rt_os.cpp \
		test_fifo.h \
		test_fifo.cpp \
		test_no_copy_fifo.h \
		test_no_copy_fifo.cpp \
		$(top_srcdir)/include/rt.h \
		$(top_srcdir)/include/fifo.h \
		$(top_srcdir)/include/no_copy_fifo.h \
		$(top_srcdir)/src/fifo.cpp \
		$(top_srcdir)/src/no_copy_fifo.cpp

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
