CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pkglib_LTLIBRARIES = membrane_test.la

membrane_test_la_LDFLAGS = -module -avoid-version -lgsl -lgslcblas

membrane_test_la_SOURCES = \
		membrane_test.h \
		membrane_test.cpp \
		membrane_testUI.cpp	
nodist_membrane_test_la_SOURCES = \
		moc_membrane_test.cpp \
		moc_membrane_testUI.cpp

# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<