CLEANFILES = *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

if BUILD_NI
pkglib_LTLIBRARIES = ni_driver.la
endif

ni_driver_la_LDFLAGS = -module -avoid-version

ni_driver_la_SOURCES = \
			ai.h \
			ao.h \
			common.h \
			ni_device.h \
			ni_devices.h \
			ni_driver.h \
			osiBus.h \
			osiEnvirons.h \
			osiPlatform.h \
			osiTypes.h \
			scale.h \
			tMSeries.h \
			trace.h \
			tStatus2.h \
			tTIO.h \
			ai.cpp \
			ao.cpp \
			common.cpp \
			ni_device.cpp \
			ni_driver.cpp \
			osiBus.cpp \
			osiUserCode.cpp \
			scale.cpp \
			tMSeries_auto.cpp \
			tMSeries.cpp \
			tTIO_auto.cpp \
			tTIO.cpp
