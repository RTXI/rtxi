CLEANFILES = *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

if BUILD_ANALOGY
pkglib_LTLIBRARIES = analogy_driver.la
endif

analogy_driver_la_LDFLAGS =-module -avoid-version \
                           $(RTOS_LDFLAGS) -lanalogy -lrtdm

analogy_driver_la_SOURCES = \
		analogy_device.h \
		analogy_driver.h \
		analogy_device.cpp \
		analogy_driver.cpp
