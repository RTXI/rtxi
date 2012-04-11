CLEANFILES = moc_*.cpp *~ */moc_*.cpp */*~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

if BUILD_CLAMPSUITE
pkglib_LTLIBRARIES = clamp_suite.la
endif

clamp_suite_la_LDFLAGS = -module -avoid-version -lqwt

clamp_suite_la_SOURCES = \
		clamp_suite.cpp\
		CS_protocol.cpp\
		CS_protocol_editor.cpp\
		ui/CS_main_windowUI.cpp\
		ui/CS_protocol_editorUI.cpp\
		CS_plot_window.cpp\
		ui/CS_plot_windowUI.cpp\
		plot/basicplot.cpp\
		plot/scrollzoomer.cpp\
		plot/scrollbar.cpp\
nodist_clamp_suite_la_SOURCES = \
		moc_clamp_suite.cpp\
		moc_CS_protocol_editor.cpp\
		ui/moc_CS_main_windowUI.cpp\
		ui/moc_CS_protocol_editorUI.cpp\
		moc_CS_plot_window.cpp\
		ui/moc_CS_plot_windowUI.cpp\
		plot/moc_basicplot.cpp\
		plot/moc_scrollzoomer.cpp\
		plot/moc_scrollbar.cpp

# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<