
bin_PROGRAMS = comedi_test

noinst_HEADERS = comedi_test.h

comedi_test_SOURCES = \
	bufconfig.c cmd_1.c cmd_2.c cmd_3.c info.c insn_read.c \
	insn_read_time.c inttrig.c lib.c main.c mmap.c mode0_read.c \
	select.c
comedi_test_CFLAGS = $(COMEDILIB_CFLAGS)
comedi_test_LDADD = $(COMEDILIB_LIBS)

