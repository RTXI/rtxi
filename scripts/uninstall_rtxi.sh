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

QWT_VERSION=6.1.3

# Directories
DIR=$PWD
ROOT=${DIR}/..
RTXI_LIB=/usr/local/lib/rtxi
QWT_LIB=/usr/local/lib/qwt
QWT_LIB2=/usr/local/qwt-${QWT_VERSION}
QWT_LIB3=/usr/local/lib/libqwt* # not too proud of this line...
RTXI_INC_MOD=/usr/local/lib/rtxi_modules
RTXI_INC=/usr/local/include/rtxi
RTXI_BIN=/usr/local/bin/rtxi*
RTXI_SHARE=/usr/local/share/rtxi
RTXI_APP=/usr/share/applications/rtxi.desktop
ETC=/etc/rtxi.conf

cd ${ROOT}

# Uninstall RTXI and QWT files. 
make uninstall
make clean
rm -rf ${RTXI_LIB}
rm -rf ${QWT_LIB}
rm -rf ${QWT_LIB2}
rm -rf ${QWT_LIB3}
rm -rf ${RTXI_INC_MOD}
rm -rf ${RTXI_INC}
rm -rf ${RTXI_BIN}
rm -rf ${RTXI_SHARE}
rm -rf ${RTXI_APP}
rm -rf ${ETC}

echo "----->RTXI removed. Reboot may be required."
