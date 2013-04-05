CLEANFILES = moc_*.cpp *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

pkglib_LTLIBRARIES = userprefs.la

userprefs_la_LDFLAGS = -module -avoid-version

userprefs_la_SOURCES = \
		userprefs.h \
		userprefs.cpp \
nodist_userprefs_la_SOURCES = \
		moc_userprefs.cpp

# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<
