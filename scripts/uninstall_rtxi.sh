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

RTXI_MOD=/usr/local/lib/rtxi_modules

QWT_LIB=/usr/local/lib/libqwt* # not too proud of this line...
QWT_DIR=/usr/local/qwt-${QWT_VERSION}

# Uninstall Qwt
cd ${DEPS}
if [ -d qwt-${QWT_VERSION} ]; then
	rm -rf qwt-${QWT_VERSION}
fi
tar xf qwt-${QWT_VERSION}.tar.bz2
cd qwt-${QWT_VERSION}
qmake qwt.pro
make uninstall
cd ${DEPS}
rm -rf ${QWT_LIB}
rm -rf ${QWT_DIR}
rm -rf qwt-${QWT_VERSION}

# Uninstall Hdf5
cd ${DEPS}
if [ -d hdf5-${HDF_VERSION} ]; then
	rm -rf hdf5-${HDF_VERSION}
fi
tar xf hdf5-${HDF_VERSION}.tar.bz2
cd hdf5-${HDF_VERSION}
./configure --prefix=/usr
make uninstall
cd ${DEPS}
rm -rf hdf5-${HDF_VERSION}

# Uninstall RTXI
cd ${ROOT}
make uninstall
make clean
sudo ldconfig
rm -rf ${RTXI_MOD}

# Remove startup scripts/services
if [ $(lsb_release -sc) == "jessie" ] || [ $(lsb_release -sc) == "xenial" ]; then
	echo "-----> Remove analogy driver systemd service"
	sudo systemctl stop rtxi_load_analogy.service
	sudo systemctl disable rtxi_load_analogy.service
	sudo rm -f /etc/systemd/system/rtxi_load_analogy.service
else
	echo "-----> Remove analogy driver sysvinit/upstart scripts"
	sudo update-rc.d rtxi_load_analogy stop
	sudo update-rc.d -f rtxi_load_analogy remove
	sudo rm -f /etc/init.d/rtxi_load_analogy
fi

cd ${DIR}
echo "-----> RTXI removed. Reboot may be required."
