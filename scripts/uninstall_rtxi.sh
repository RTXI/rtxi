#! /bin/bash

#
# The Real-Time eXperiment Interface (RTXI)
# Copyright (C) 2011 Georgia Institute of Technology, University of Utah, Weill Cornell Medical College
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#	Created by Yogi Patel <yapatel@gatech.edu> 2014.1.31
#

# Directories
DIR=$PWD
ROOT=${DIR}/..
RTXI_LIB=/usr/local/lib/rtxi
QWT_LIB=/usr/local/lib/qwt
QWT_LIB2=/usr/local/qwt-6.1.2
QWT_LIB3=/usr/lib/libqwt-* # not too proud of this line...
RTXI_INC_LIB=/usr/local/lib/rtxi_includes
RTXI_INC_MOD=/usr/local/lib/rtxi_modules
RTXI_INC=/usr/local/include/rtxi
RTXI_BIN=/usr/local/bin/rtxi*
RTXI_SHARE=/usr/local/share/rtxi
RTXI_APP=/usr/share/applications/rtxi.desktop
ETC=/etc/rtxi.conf

cd ${ROOT}

# Uninstall RTXI and QWT files. 
sudo make uninstall
sudo make clean
sudo rm -rf ${RTXI_LIB}
sudo rm -rf ${QWT_LIB}
sudo rm -rf ${QWT_LIB2}
sudo rm -rf ${QWT_LIB3}
sudo rm -rf ${RTXI_INC_LIB}
sudo rm -rf ${RTXI_INC_MOD}
sudo rm -rf ${RTXI_INC}
sudo rm -rf ${RTXI_BIN}
sudo rm -rf ${RTXI_SHARE}
suro rm -rf ${RTXI_APP}
sudo rm -rf ${ETC}

if [ $? -eq 0 ]; then
	echo "----->RTXI removed. Reboot may be required."
else
	echo "----->RTXI uninstallation failed."
	exit
fi
