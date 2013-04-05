CLEANFILES = *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

if BUILD_COMEDI
pkglib_LTLIBRARIES = comedi_driver.la
endif

comedi_driver_la_LDFLAGS = -module -avoid-version -lcomedi

comedi_driver_la_SOURCES = \
		comedi_device.h \
		comedi_driver.h \
		comedi_device.cpp \
		comedi_driver.cpp
