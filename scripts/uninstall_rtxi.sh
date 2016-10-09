#! /bin/bash
set -eu

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
# Created by Yogi Patel <yapatel@gatech.edu> 2014.1.31
#

if ! id | grep -q root; then
	echo "Must run script as root; try again with sudo ./uninstall_rtxi.sh"
	exit
fi

# Directories
DIR=$PWD
ROOT=${DIR}/..
DEPS=${ROOT}/deps

HDF_VERSION=1.8.4
QWT_VERSION=6.1.3

RTXI_LIB=/usr/local/lib/rtxi
RTXI_MOD=/usr/local/lib/rtxi_modules
RTXI_APP=/usr/share/applications/rtxi.desktop

QWT_LIB=/usr/local/lib/libqwt* # not too proud of this line...
QWT_DIR=/usr/local/qwt-${QWT_VERSION}

# Uninstall Qwt
cd ${DEPS}
if ! [ -d qwt-${QWT_VERSION} ]; then
	tar xf qwt-${QWT_VERSION}.tar.bz2
fi
cd qwt-${QWT_VERSION}
qmake qwt.pro
make uninstall
cd ${DEPS}
rm -rf ${QWT_LIB}
rm -rf ${QWT_DIR}
rm -rf qwt-${QWT_VERSION}

# Uninstall Hdf5
cd ${DEPS}
if ! [ -d hdf5-${HDF_VERSION} ]; then
	tar xf hdf5-${HDF_VERSION}.tar.bz2
fi
cd hdf5-${HDF_VERSION}
./configure --prefix=/usr
make uninstall
cd ${DEPS}
rm -rf hdf5-${HDF_VERSION}

# Uninstall RTXI
cd ${ROOT}
make uninstall
make clean

rm -rf ${RTXI_LIB}
rm -rf ${RTXI_MOD}
rm -rf ${RTXI_APP}

cd ${DIR}
echo "----->RTXI removed. Reboot may be required."
