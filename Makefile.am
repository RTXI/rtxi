# this is to include the Autoconf scripts in subdir m4/
ACLOCAL_AMFLAGS=-I m4

SUBDIRS = rtxi plugins scripts deps/hdf

ACLOCAL_DIST = m4/ac_pkg_swig.m4 \
	       m4/ac_python_devel.m4 \
	       m4/bnv_have_qt.m4

DEBIAN_DIST = debian/changelog  \
              debian/control    \
              debian/conffiles  \
              debian/copyright  \
              debian/rules      \
              debian/rules.rtxi-dynamo    \
              debian/rules.rtxi-base      \
              debian/rtxi-base.rtxi.init  

EXTRA_DIST  = autogen.sh TODO $(DEBIAN_DIST) $(ACLOCAL_DIST)

dist-hook: ChangeLog
	find $(srcdir)/dynamo -name "*.mlb" -or -name "*.sig" -or -name "*.sml" -or \
                              -name "*.grm" -or -name "*.lex" -or -name "*.dynamo" -or \
                              -name "*.texi" -or -name "*.1" -or \
                              -name "ChangeLog" -or -name README  | cpio -pvd $(distdir)


CLEANFILES            = *~ src/moc_*.cpp 
DISTCLEANFILES        = rtxi.conf Makefile.modcompile config.log about.h rtos
MAINTAINERCLEANFILES  = aclocal.m4 configure config.guess config.sub Makefile.in
MAINTAINERCLEANDIRS   = autom4te.cache

pkgdata_DATA = rtxi.conf Makefile.dynamo_compile Makefile.plugin_compile Makefile.plugin_example

pkgdata_SCRIPTS = libtool

include $(top_srcdir)/Makefile.buildvars

ChangeLog:
	touch ChangeLog
	-cvs2cl -rbt --fsf

rtxi.conf: 
	@echo "Generating rtxi.conf..."
	@echo '<RTXI class="settings" version="2.0" >'						> $@
	@echo ' <OBJECT component="rt" id="0" >'					>> $@
	@echo '  <PARAM name="Period" >1e+06</PARAM>'					>> $@
	@echo ' </OBJECT>'								>> $@
	@echo ' <OBJECT component="io" id="1" >'					>> $@
	@echo '  <PARAM name="Num Links" >0</PARAM>'					>> $@
	@echo ' </OBJECT>'								>> $@
if BUILD_COMEDI 
	@echo ' <OBJECT component="plugin" library="comedi_driver.so" id="2" >'		>> $@
	@echo '  <PARAM name="0" >/dev/comedi0</PARAM>'					>> $@
	@echo '  <PARAM name="Num Devices" >1</PARAM>'					>> $@
	@echo '  <OBJECT id="13" name="0" />'						>> $@
	@echo ' </OBJECT>'								>> $@
endif
if BUILD_COMEDILIB 
	@echo ' <OBJECT component="plugin" library="comedi_driver.so" id="2" >'		>> $@
	@echo '  <PARAM name="0" >/dev/comedi0</PARAM>'					>> $@
	@echo '  <PARAM name="Num Devices" >1</PARAM>'					>> $@
	@echo '  <OBJECT id="13" name="0" />'						>> $@
	@echo ' </OBJECT>'								>> $@
endif
if BUILD_ANALOGY 
	@echo ' <OBJECT component="plugin" library="analogy_driver.so" id="2" >'		>> $@
	@echo '  <PARAM name="0" >analogy0</PARAM>'					>> $@
	@echo '  <PARAM name="Num Devices" >1</PARAM>'					>> $@
	@echo '  <OBJECT id="13" name="0" />'						>> $@
	@echo ' </OBJECT>'								>> $@
endif
	@echo ' <OBJECT component="plugin" library="model_loader.so" id="3" />'		>> $@
	@echo ' <OBJECT component="plugin" library="dynamo_model_loader.so" id="4" />'	>> $@
	@echo ' <OBJECT component="plugin" library="system_control.so" id="5" />'	>> $@
	@echo ' <OBJECT component="plugin" library="oscilloscope.so" id="6" >'		>> $@
	@echo '  <PARAM name="Num Panels" >0</PARAM>'					>> $@
	@echo ' </OBJECT>'								>> $@
	@echo ' <OBJECT component="plugin" library="data_recorder.so" id="7" />'	>> $@
	@echo ' <OBJECT component="plugin" library="connector.so" id="8" />'		>> $@
	@echo ' <OBJECT component="plugin" library="performance_measurement.so" id="9" />'>> $@
	@echo ' <OBJECT component="plugin" library="userprefs.so" id="10" />'>> $@
	@echo '</RTXI>'									>> $@

Makefile.dynamo_compile:  
	@echo "Generating Makefile.dynamo_compile..."
	@echo ''						 	>  $@
	@echo 'exec_modeldir  = ' $(pkglibdir) 				>> $@
	@echo ''						 	>> $@
	@echo '.SUFFIXES:      .dynamo .o .so'			 	>> $@
	@echo ''						 	>> $@
	@echo 'MOD_TRANSLATOR = ' dynamo			 	>> $@
	@echo 'LIBTOOL        = ' $(pkglibdir)/libtool		 	>> $@
	@echo 'CXX            = ' $(CXX)			 	>> $@
	@echo 'CXXLD          = ' $(CXX)			 	>> $@
	@echo 'CXXCOMPILE     = $$(LIBTOOL) --mode=compile $$(CXX)'     >> $@
	@echo 'CXXLINK	      = $$(LIBTOOL) --mode=link $$(CXXLD)'	>> $@
	@echo ''                                                        >> $@
	@echo 'CXXFLAGS := -I. -I/usr/include/python2.6 -I'$(pkgincludedir) $(AM_CXXFLAGS) 	>> $@
	@echo 'LDFLAGS  := ' $(AM_LDFLAGS) -module -avoid-version	>> $@
	@echo ''                                                        >> $@
	@echo 'ifdef DEBUG '						>> $@
	@echo 'CXXFLAGS += -DDEBUG'					>> $@
	@echo 'endif'							>> $@
	@echo ''                                                        >> $@
	@echo '.dynamo.so:'					 	>> $@
	@echo '	$$(MOD_TRANSLATOR) -x -o`basename $$*` $$*.dynamo'      >> $@
	@echo '	@echo Translation complete'		         	>> $@
	@echo '	$$(CXXCOMPILE) $$(CXXFLAGS)  -c $$*.cpp -o $$*.lo'      >> $@
	@echo '	@echo Compilation complete'		         	>> $@
	@echo '	$$(CXXLINK) $$(CXXFLAGS) $$(LDFLAGS) \
		-rpath `readlink -f $$<`  -o $$*.la $$*.lo'		>> $@
	@echo '	@echo Module created'			         	>> $@
	@echo '	$$(LIBTOOL) --mode=install cp $$*.la `readlink -f $$<`'	>> $@
	@echo '	@echo Module installed'			         	>> $@
	@echo ''						 	>> $@

Makefile.plugin_compile: Makefile
	@echo "Generating Makefile.plugin_compile..."
	@echo ''						 	>  $@
	@echo 'exec_modeldir  = ' $(pkglibdir) 				>> $@
	@echo ''						 	>> $@
	@echo 'LIBTOOL        = ' $(pkglibdir)/libtool		 	>> $@
	@echo 'CXX            = ' $(CXX)			 	>> $@
	@echo 'CXXLD          = ' $(CXX)			 	>> $@
	@echo 'CXXCOMPILE     = $$(LIBTOOL) --mode=compile $$(CXX)'     >> $@
	@echo 'CXXLINK	      = $$(LIBTOOL) --mode=link $$(CXXLD)'	>> $@
	@echo 'MOC            = ' $(MOC)				>> $@
	@echo ''                                                        >> $@
	@echo 'CXXFLAGS := -I. -I'$(pkgincludedir) $(AM_CXXFLAGS) 	>> $@
	@echo 'LDFLAGS  := ' $(AM_LDFLAGS) -module -avoid-version	>> $@
	@echo ''                                                        >> $@
	@echo 'ifdef DEBUG '						>> $@
	@echo 'CXXFLAGS += -DDEBUG'					>> $@
	@echo 'endif'							>> $@
	@echo ''							>> $@
	@echo 'OBJECTS = $$(shell echo $$(SOURCES) | sed "s/\.cpp[ \t\n]*/\.lo /g")' >> $@
	@echo 'MOOBJECTS = $$(shell echo $$(HEADERS) | sed "s/\.h[ \t\n]*/\.lo /g")' >> $@
	@echo ''							>> $@
	@echo 'all: $$(PLUGIN_NAME).la'					>> $@
	@echo ''							>> $@
	@echo '%.lo: %.cpp'						>> $@
	@echo '	$$(CXXCOMPILE) $$(CXXFLAGS) -c $$< -o $$@'		>> $@
	@echo ''							>> $@
	@echo '$$(PLUGIN_NAME).la: $$(OBJECTS) $$(SOURCES) $$(HEADERS)' >> $@
	@echo '	$$(CXXLINK) $$(CXXFLAGS) $$(LIBS) $$(LDFLAGS) -rpath `readlink -f $$(exec_modeldir)` -o $$(PLUGIN_NAME).la $$(OBJECTS)' >> $@
	@echo ''							>> $@
	@echo 'install: $$(PLUGIN_NAME).la'				>> $@
	@echo '	$$(LIBTOOL) --mode=install cp $$(PLUGIN_NAME).la `readlink -f $$(exec_modeldir)`' >> $@
	@echo ''							>> $@
	@echo 'clean:'							>> $@
	@echo '	rm -f $$(OBJECTS)'					>> $@
	@echo '	rm -f $$(MOOBJECTS)'					>> $@
	@echo '	rm -f moc_*'						>> $@
	@echo '	rm -f *.o'						>> $@
	@echo '	rm -f $$(PLUGIN_NAME).la'				>> $@
	@echo '	rm -f $$(PLUGIN_NAME).o'				>> $@
	@echo '	rm -rf .libs'						>> $@
	@echo ''							>> $@
	@echo 'moc_%.cpp: %.h'						>> $@
	@echo '	$$(MOC) -o $$@ $$<'					>> $@

Makefile.plugin_example:
	@echo "Generating Makefile.plugin_example..."
	@echo ''							> $@
	@echo 'PLUGIN_NAME = my_plugin'					>> $@
	@echo ''							>> $@
	@echo 'HEADERS = my_plugin.h'					>> $@
	@echo ''							>> $@
	@echo 'SOURCES = my_plugin.cpp'					>> $@
	@echo ''							>> $@
	@echo '### Do not edit below this line ###'			>> $@
	@echo ''							>> $@
	@echo 'include $$(shell rtxi_plugin_config --pkgdata-dir)/Makefile.plugin_compile' >> $@

force:
