# this is to include the Autoconf scripts in subdir m4/
ACLOCAL_AMFLAGS=-I m4

SUBDIRS = rtxi plugins models hdf scripts 

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
DISTCLEANFILES        = rtxi_comedi.conf rtxi.conf Makefile.modcompile config.log about.h rtos
MAINTAINERCLEANFILES  = aclocal.m4 configure config.guess config.sub Makefile.in
MAINTAINERCLEANDIRS   = autom4te.cache

pkgdata_DATA = rtxi_comedi.conf rtxi.conf Makefile.modcompile 

pkglib_SCRIPTS = libtool


include $(top_srcdir)/.buildvars

ChangeLog:
	touch ChangeLog
	-cvs2cl -rbt --fsf

rtxi_comedi.conf: force
	@echo "Generating rtxi_comedi.conf..."
	@echo '## Example comedi configuration file. Lines beginning with # are'   >  $@
	@echo '## comments and are ignored.  '                                     >> $@
	@echo '##'                                                                 >> $@
	@echo '## Syntax is:'                                                      >> $@
	@echo '## /dev/comediN:driver:opts'                                        >> $@
	@echo '##'                                                                 >> $@
	@echo '## Where: '                                                         >> $@
	@echo '##        /dev/comediN is a Comedi device to be configured, N=0,1,2...'   >> $@
	@echo '##        driver is the name of a Comedi device driver (see devices.txt)' >> $@
	@echo '##        opts is a comma-separated list of options for the driver'       >> $@
	@echo '##'                                                                       >> $@
	@echo '## Example:'                                                              >> $@
	@echo '## /dev/comedi0:ni_pcimio:'                                               >> $@
	@echo '##'                                                                       >> $@
	@echo '## Means that we want to load driver ni_pcimio (National Instruments PCI data' >> $@
	@echo '## acquisition boards) with no options, and configure /dev/comedi0 for'        >> $@
	@echo '## use with that driver. '                                                     >> $@
	@echo '##'                                                                            >> $@
	@echo '/dev/comedi0:ni_pcimio:'                                                       >> $@
	@echo ''                                                                              >> $@

rtxi.conf: 
	@echo "Generating rtxi.conf..."
	@echo '<RTXI class="settings" >'						> $@
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
if BUILD_NI
	@echo ' <OBJECT component="plugin" library="ni_driver.so" id="3" >'		>> $@
	@echo '  <PARAM name="0" >PCI::SCAN</PARAM>'					>> $@
	@echo '  <PARAM name="Num Devices" >1</PARAM>'					>> $@
	@echo '  <OBJECT id="14" name="0" />'						>> $@
	@echo ' </OBJECT>'								>> $@
endif
	@echo ' <OBJECT component="plugin" library="model_loader.so" id="4" />'		>> $@
	@echo ' <OBJECT component="plugin" library="dynamo_model_loader.so" id="5" />'	>> $@
	@echo ' <OBJECT component="plugin" library="system_control.so" id="6" />'	>> $@
	@echo ' <OBJECT component="plugin" library="oscilloscope.so" id="7" >'		>> $@
	@echo '  <PARAM name="Num Panels" >0</PARAM>'					>> $@
	@echo ' </OBJECT>'								>> $@
	@echo ' <OBJECT component="plugin" library="data_recorder.so" id="8" />'	>> $@
	@echo ' <OBJECT component="plugin" library="connector.so" id="9" />'		>> $@
	@echo ' <OBJECT component="plugin" library="performance_measurement.so" id="10" />'>> $@
	@echo '</RTXI>'									>> $@

Makefile.modcompile:  
	@echo "Generating Makefile.modcompile..."
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
	@echo 'INCLUDES := ' $(QT_INCLUDES)		>> $@
	@echo 'INCLUDES += -I'$(pkgincludedir)				>> $@
	@echo 'DEFS     += ' $(QT_DEFS)					>> $@
	@echo 'CXXFLAGS += $$(INCLUDES) $$(DEFS)' $(CXXFLAGS) $(RTOS_CPPFLAGS) >> $@
	@echo 'LIBS     += ' $(QT_LIBS) 				>> $@
	@echo 'LDFLAGS  += ' $(RTOS_LDFLAGS) -module -avoid-version	>> $@
	@echo ''                                                        >> $@
	@echo 'ifdef DEBUG '						>> $@
	@echo 'DEFS += -DDEBUG'						>> $@
	@echo 'endif'							>> $@
	@echo ''                                                        >> $@
	@echo '.dynamo.so:'					 	>> $@
	@echo '	$$(MOD_TRANSLATOR) -x -o`basename $$*` $$*.dynamo'      >> $@
	@echo '	@echo Translation complete'		         	>> $@
	@echo '	$$(CXXCOMPILE) $$(CXXFLAGS)  -c $$*.cpp -o $$*.lo'      >> $@
	@echo '	@echo Compilation complete'		         	>> $@
	@echo '	$$(CXXLINK) $$(CXXFLAGS) $$(LIBS) $$(LDFLAGS) 		\
		-rpath `realpath $$<`  -o $$*.la $$*.lo'	>> $@
	@echo '	@echo Module created'			         	>> $@
	@echo '	$$(LIBTOOL) --mode=install cp $$*.la `realpath $$<`'    >> $@
	@echo '	@echo Module installed'			         	>> $@
	@echo ''						 	>> $@

force:
