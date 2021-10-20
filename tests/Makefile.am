# TESTS -- Programs run automatically by "make check"
# check_PROGRAMS -- Programs built by "make check" but not necessarily run
TESTS = rtxi_tests
check_PROGRAMS = rtxi_tests

bin_PROGRAMS = rtxi_tests

rtxi_tests_SOURCES = rtxi_tests.cpp
rtxi_tests_LDADD = ${BOOST_UNIT_TEST_FRAMEWORK_LIB}
#rtxi_tests_LDADD = $(top_builddir)/src/func.o
