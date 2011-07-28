
if HAVE_PYTHON
pyexec_LTLIBRARIES = _comedi.la
clean-local:
	$(RM) comedi_python_wrap.c comedi.py
else
pyexec_LTLIBRARIES =
clean-local:
endif

nodist__comedi_la_SOURCES = comedi_python_wrap.c
_comedi_la_CFLAGS = $(COMEDILIB_CFLAGS) $(PYTHON_INCLUDES) $(PYTHON_QUIET)
_comedi_la_LDFLAGS = -module -avoid-version $(COMEDILIB_LIBS)

pyexec_SCRIPTS = comedi.py

EXTRA_DIST = README.txt setup.py test_comedi.py

comedi_python_wrap.c comedi.py: $(srcdir)/../comedi.i
	$(SWIG) -python -o comedi_python_wrap.c -I$(top_srcdir)/include $(srcdir)/../comedi.i
