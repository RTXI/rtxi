CLEANFILES = *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/.buildvars

if BUILD_COMEDI
pkglib_LTLIBRARIES = comedi_driver.la
endif

comedi_driver_la_LDFLAGS = -module -avoid-version

comedi_driver_la_SOURCES = \
		comedi_device.h \
		comedi_driver.h \
		comedi_device.cpp \
		comedi_driver.cpp
