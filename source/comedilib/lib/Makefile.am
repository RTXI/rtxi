
lib_LTLIBRARIES = libcomedi.la

noinst_HEADERS = calib_lex.h calib_yacc.h libinternal.h

CLEANFILES = grammar.output

EXTRA_DIST = calib_lex.l calib_yacc.y version_script

BUILT_SOURCES = calib_lex.c calib_lex.h calib_yacc.c calib_yacc.h
MAINTAINERCLEANFILES = calib_lex.c calib_lex.h calib_yacc.c calib_yacc.h

libcomedi_la_SOURCES = \
	buffer.c calib.c cmd.c comedi.c data.c dio.c error.c \
	filler.c get.c ioctl.c range.c sv.c timed.c timer.c \
	calib_lex.c calib_yacc.c insn_config_wrappers.c

libcomedi_la_CFLAGS = $(COMEDILIB_CFLAGS) -D_REENTRANT -DLOCALSTATEDIR=\"\$(localstatedir)\"

libcomedi_la_LDFLAGS = -soname libcomedi \
        $(COMEDILIB_LT_LDFLAGS) \
        -Wl,--version-script=$(srcdir)/version_script

$(srcdir)/calib_yacc.c $(srcdir)/calib_yacc.h: $(srcdir)/calib_yacc.y
	$(YACC) -d -p calib_yy -o $(srcdir)/calib_yacc.c $<

$(srcdir)/calib_lex.c $(srcdir)/calib_lex.h: $(srcdir)/calib_lex.l
	$(LEX) --header-file=$(srcdir)/calib_lex.h -Pcalib_yy -o$(srcdir)/calib_lex.c $<
