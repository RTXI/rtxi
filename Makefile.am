#
# The Real-Time eXperiment Interface (RTXI)
# 
# Copyright (C) 2011 Georgia Institute of Technology, University of Utah, Weill
# Cornell Medical College
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program. If not, see <http://www.gnu.org/licenses/>.
#

# Include the Autoconf scripts in subdir m4/
ACLOCAL_AMFLAGS = -I m4

SUBDIRS      = libs src plugins scripts res

ACLOCAL_DIST = 

EXTRA_DIST   = autogen.sh $(ACLOCAL_DIST)

CLEANFILES            = *~ src/moc_*.cpp 
DISTCLEANFILES        = rtxi.conf Makefile.modcompile config.log about.h rtos
MAINTAINERCLEANFILES  = aclocal.m4 configure config.guess config.sub Makefile.in
MAINTAINERCLEANDIRS   = autom4te.cache

pkgdata_DATA = rtxi.conf Makefile.plugin_compile Makefile.plugin_example

pkgdata_SCRIPTS = libtool

include $(top_srcdir)/Makefile.buildvars

rtxi.conf: 
	@echo "Generating rtxi.conf..."
	@echo '<RTXI class="settings" version="2.0" >' > $@
	@echo ' <OBJECT component="rt" id="0" >' >> $@
	@echo '  <PARAM name="Period" >1e+06</PARAM>' >> $@
	@echo ' </OBJECT>' >> $@
	@echo ' <OBJECT component="io" id="1" >' >> $@
	@echo '  <PARAM name="Num Links" >0</PARAM>' >> $@
	@echo ' </OBJECT>' >> $@
if BUILD_ANALOGY
	@echo ' <OBJECT component="plugin" library="analogy_driver.so" id="2" >' >> $@
	@echo '  <PARAM name="0" >analogy0</PARAM>' >> $@
	@echo '  <PARAM name="Num Devices" >1</PARAM>' >> $@
	@echo '  <OBJECT id="13" name="0" />' >> $@
	@echo ' </OBJECT>' >> $@
endif
	@echo ' <OBJECT component="plugin" library="model_loader.so" id="3" />'>> $@
	@echo ' <OBJECT component="plugin" library="module_installer.so" id="4" />'>> $@
	@echo ' <OBJECT component="plugin" library="system_control.so" id="5" />'>> $@
	@echo ' <OBJECT component="plugin" library="oscilloscope.so" id="6" >'>> $@
	@echo '  <PARAM name="Num Panels" >0</PARAM>'>> $@
	@echo ' </OBJECT>'>> $@
	@echo ' <OBJECT component="plugin" library="data_recorder.so" id="7" />'>> $@
	@echo ' <OBJECT component="plugin" library="connector.so" id="8" />'>> $@
	@echo ' <OBJECT component="plugin" library="performance_measurement.so" id="9" />'>> $@
	@echo ' <OBJECT component="plugin" library="userprefs.so" id="10" />'>> $@
	@echo '</RTXI>'>> $@

Makefile.plugin_compile: Makefile
	@echo "Generating Makefile.plugin_compile..."
	@echo 'exec_modeldir = ' $(pkglibdir) > $@
	@echo '' >> $@
	@echo 'LIBTOOL    = ' $(pkgdatadir)/libtool >> $@
	@echo 'CXX        = ' $(CXX) >> $@
	@echo 'CXXLD      = ' $(CXX) >> $@
	@echo 'CXXCOMPILE = $$(LIBTOOL) --mode=compile $$(CXX)'     >> $@
	@echo 'CXXLINK    = $$(LIBTOOL) --mode=link $$(CXXLD)'>> $@
	@echo 'MOC        = ' $(MOC)>> $@
	@echo ''                                                        >> $@
	@echo 'CXXFLAGS := $$(CXXFLAGS) -I. '$(RTDEPS_CXXFLAGS) $(AM_CXXFLAGS) >> $@
	@echo 'LDFLAGS  := $$(LDFLAGS) '$(RTDEPS_LIBS) $(AM_LDFLAGS) -module -avoid-version>> $@
	@echo ''                                                        >> $@
	@echo 'ifdef DEBUG '>> $@
	@echo 'CXXFLAGS += -DDEBUG'>> $@
	@echo 'endif'>> $@
	@echo ''>> $@
	@echo 'OBJECTS   = $$(shell echo $$(SOURCES) | sed "s/\.cpp[ \t\n]*/\.lo /g")' >> $@
	@echo 'MOOBJECTS = $$(shell echo $$(HEADERS) | sed "s/\.h[ \t\n]*/\.lo /g")' >> $@
	@echo ''>> $@
	@echo 'all: $$(PLUGIN_NAME).la'>> $@
	@echo ''>> $@
	@echo '%.lo: %.cpp'>> $@
	@echo '	$$(CXXCOMPILE) $$(CXXFLAGS) -c $$< -o $$@'>> $@
	@echo ''>> $@
	@echo '$$(PLUGIN_NAME).la: $$(OBJECTS) $$(SOURCES) $$(HEADERS)' >> $@
	@echo '	$$(CXXLINK) $$(CXXFLAGS) $$(LIBS) $$(LDFLAGS) -rpath `readlink -f $$(exec_modeldir)` -o $$(PLUGIN_NAME).la $$(OBJECTS)' >> $@
	@echo ''>> $@
	@echo 'install: $$(PLUGIN_NAME).la'>> $@
	@echo '	$$(LIBTOOL) --mode=install cp $$(PLUGIN_NAME).la `readlink -f $$(exec_modeldir)`' >> $@
	@echo ''>> $@
	@echo 'clean:'>> $@
	@echo '	rm -f $$(OBJECTS)'>> $@
	@echo '	rm -f $$(MOOBJECTS)'>> $@
	@echo '	rm -f moc_*'>> $@
	@echo '	rm -f *.o'>> $@
	@echo '	rm -f $$(PLUGIN_NAME).la'>> $@
	@echo '	rm -f $$(PLUGIN_NAME).o'>> $@
	@echo '	rm -rf .libs'>> $@
	@echo ''>> $@
	@echo 'moc_%.cpp: %.h'>> $@
	@echo '	$$(MOC) -o $$@ $$<'>> $@

Makefile.plugin_example:
	@echo "Generating Makefile.plugin_example..."
	@echo 'PLUGIN_NAME = my_plugin'> $@
	@echo ''>> $@
	@echo 'HEADERS = my_plugin.h'>> $@
	@echo ''>> $@
	@echo 'SOURCES = my_plugin.cpp'>> $@
	@echo ''>> $@
	@echo 'LIBS = '>> $@
	@echo ''>> $@
	@echo '### Do not edit below this line ###'>> $@
	@echo ''>> $@
	@echo 'include $$(shell rtxi_plugin_config --pkgdata-dir)/Makefile.plugin_compile' >> $@

uninstall-hook:
	if test -d $(pkglibdir); then rm -rf $(pkglibdir); fi
	if test -d $(pkgincludedir); then rm -rf $(pkgincludedir); fi
	if test -d $(pkgdatadir); then rm -rf $(pkgdatadir); fi
