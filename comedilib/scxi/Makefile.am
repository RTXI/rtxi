
EXTRA_DIST = README

if BUILD_SCXI
lib_LTLIBRARIES = libscxi.la
else
lib_LTLIBRARIES =
endif

libscxi_la_SOURCES = scxi.c
libscxi_la_CFLAGS = $(COMEDILIB_CFLAGS) -D_REENTRANT
libscxi_la_LDFLAGS = -version-info @SCXI_SO_VERSION@
