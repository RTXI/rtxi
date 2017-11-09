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

QWT_VERSION=6.1.3

RTXI_MOD=/usr/local/lib/rtxi_modules

# Uninstall Qwt
if ! [[ $(dpkg -s libqwt-qt5-dev) > /dev/null ]]; then
	cd ${DEPS}
	if [ -d qwt-${QWT_VERSION} ]; then
		rm -rf qwt-${QWT_VERSION}
	fi
	tar xf qwt-${QWT_VERSION}.tar.bz2
	cd qwt-${QWT_VERSION}
	qmake qwt.pro
	make uninstall
	cd ${DEPS}
	rm -rf qwt-${QWT_VERSION}
fi

# Uninstall RTXI
cd ${ROOT}
make uninstall
make clean
ldconfig
rm -rf ${RTXI_MOD}

# Remove startup scripts/services
if [ $(lsb_release -sc) == "jessie" ] || [ $(lsb_release -sc) == "xenial" ]; then
	echo "-----> Remove analogy driver systemd service"
	systemctl stop rtxi_load_analogy.service
	systemctl disable rtxi_load_analogy.service
	rm -f /etc/systemd/system/rtxi_load_analogy.service
	systemctl daemon-reload
	systemctl reset-failed
else
	echo "-----> Remove analogy driver sysvinit/upstart scripts"
	update-rc.d rtxi_load_analogy disable
	update-rc.d -f rtxi_load_analogy remove
	rm -f /etc/init.d/rtxi_load_analogy
fi

cd ${DIR}
echo "-----> RTXI removed. Reboot may be required."
