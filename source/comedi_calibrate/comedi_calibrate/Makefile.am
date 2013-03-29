
bin_PROGRAMS = comedi_calibrate

noinst_HEADERS = calib.h

comedi_calibrate_SOURCES = \
	comedi_calibrate.c ni.c cb.c cb64.c other.c cal_common.c ni_labpc.c
#comedi_calibrate_CFLAGS = $(COMEDILIB_CFLAGS)
comedi_calibrate_LDADD = ../libcomedi_calibrate/libcomedi_calibrate.a -lcomedi -lm

install-data-local:
	$(INSTALL) -d $(DESTDIR)$(localstatedir)/lib/comedi/calibrations

EXTRA_DIST = results

