# TESTS -- Programs run automatically by "make check"
# check_PROGRAMS -- Programs built by "make check" but not necessarily run

include $(top_srcdir)/Makefile.buildvars

#TESTS = rtxi_tests
#check_PROGRAMS = rtxi_tests

#bin_PROGRAMS = rtxi_tests

# pkginclude_HEADERS = rtxi_tests.h
# rtxi_tests_SOURCES = rtxi_tests.cpp
# pkginclude_HEADERS = rtxiTests.h


TESTS = rtxiTests 
check_PROGRAMS = rtxiTests

bin_PROGRAMS = rtxiTests 
pkginclude_HEADERS = rtxiTests.h
rtxiTests_SOURCES = rtxiTests.cpp 

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
       $(top_srcdir)/src/moc_*.o 

if XENOMAI
OBJS += $(top_srcdir)/src/rt_os-xenomai.o 
else
OBJS += $(top_srcdir)/src/rt_os-posix.o 
endif

#rtxi_tests_LDADD = ${BOOST_UNIT_TEST_FRAMEWORK_LIB} $(OBJS)
#rtxi_tests_LDADD = $(top_builddir)/src/func.o
rtxiTests_LDADD = -lgtest -lgmock $(OBJS)

