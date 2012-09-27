CLEANFILES = moc_*.cpp *~ */moc_*.cpp */*~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.buildvars

if BUILD_CLAMPPROTOCOL
pkglib_LTLIBRARIES = clamp_protocol.la
endif

clamp_protocol_la_LDFLAGS = -module -avoid-version -lqwt

clamp_protocol_la_SOURCES = \
		clamp_protocol.cpp\
		CP_protocol.cpp\
		CP_protocol_editor.cpp\
		ui/CP_main_windowUI.cpp\
		ui/CP_protocol_editorUI.cpp\
		CP_plot_window.cpp\
		ui/CP_plot_windowUI.cpp\
		plot/basicplot.cpp\
		plot/scrollzoomer.cpp\
		plot/scrollbar.cpp\
nodist_clamp_protocol_la_SOURCES = \
		moc_clamp_protocol.cpp\
		moc_CP_protocol_editor.cpp\
		ui/moc_CP_main_windowUI.cpp\
		ui/moc_CP_protocol_editorUI.cpp\
		moc_CP_plot_window.cpp\
		ui/moc_CP_plot_windowUI.cpp\
		plot/moc_basicplot.cpp\
		plot/moc_scrollzoomer.cpp\
		plot/moc_scrollbar.cpp

# MOC Rule - builds meta-object files as needed
moc_%.cpp: %.h
	$(MOC) -o $@ $<