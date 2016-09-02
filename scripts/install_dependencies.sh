#!/bin/bash

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

if ! id | grep -q root; then
	echo "Must run script as root; try again with ./install_dependencies.sh"
	exit
fi

###############################################################################
# Set directory variable for compilation
###############################################################################
DIR=$PWD
ROOT=${DIR}/../
DEPS=${ROOT}/deps
HDF=${DEPS}/hdf
QWT=${DEPS}/qwt
PLG=${ROOT}/plugins

###############################################################################
# Some easy to use defines
###############################################################################
QWT_VERSION=6.1.2

###############################################################################
# Check for all RTXI *.deb dependencies and install them. Includes:
#  - Kernel tools
#  - C/C++ compiler and debugger
#  - Qt5, HDF, and Qwt6 libraries
###############################################################################
echo "Checking dependencies..."

apt-get update
apt-get -y upgrade
apt-get -y install autotools-dev automake libtool kernel-package gcc g++ \
                        gdb fakeroot crash kexec-tools makedumpfile \
                        kernel-wedge libncurses5-dev libelf-dev binutils-dev \
                        libgsl0-dev libboost-dev git vim emacs lshw stress \
                        libqt5svg5-dev libqt5opengl5 libqt5gui5 libqt5core5a \
                        libqt5xml5 qt5-default libgit2-dev libmarkdown2-dev
apt-get -y build-dep linux

if [ $? -eq 0 ]; then
	echo "----->Package dependencies installed."
else
	echo "----->Package dependency installation failed."
	exit
fi

# Start at top
cd ${DEPS}

# Installing HDF5
echo "----->Checking for HDF5"

if [ -f "/usr/include/hdf5.h" ]; then
	echo "----->HDF5 already installed."
else
	echo "----->Installing HDF5..."
	cd ${HDF}
	tar xf hdf5-1.8.4.tar.bz2
	cd hdf5-1.8.4
	./configure --prefix=/usr
	make -sj2
	make install
	if [ $? -eq 0 ]; then
			echo "----->HDF5 installed."
	else
		echo "----->HDF5 installation failed."
		exit
	fi
fi

# Installing Qwt
echo "----->Checking for Qwt"

if [ -f "/usr/local/qwt-${QWT_VERSION}/include/qwt.h" ]; then
	echo "----->Qwt already installed."
else
	echo "----->Installing Qwt..."
	cd ${QWT}
	tar xf qwt-${QWT_VERSION}.tar.bz2
	cd qwt-${QWT_VERSION}
	qmake qwt.pro
	make -sj2
	make install
	cp -vf /usr/local/qwt-${QWT_VERSION}/lib/libqwt.so.6.1.2 /usr/lib/.
	ln -sf /usr/lib/libqwt.so.${QWT_VERSION} /usr/lib/libqwt.so
	ldconfig
	if [ $? -eq 0 ]; then
		echo "----->Qwt installed."
	else
		echo "----->Qwt installation failed."
	exit
	fi
fi

# (Re)install rtxi_includes. Remove the moc files first. Failing to do so when 
# upgrading from Qt4 to Qt5 will cause compilation errors later on. 
[ -d /usr/local/lib/rtxi_includes ] && rm -r /usr/local/lib/rtxi_includes/moc_*
rsync -a ${DEPS}/rtxi_includes /usr/local/lib/.

# Allow all members of adm (administrator accounts) write access to the 
# rtxi_includes/ directory. 
setfacl -Rm g:adm:rwX,d:g:adm:rwX /usr/local/lib/rtxi_includes
if [ $? -eq 0 ]; then
	echo "----->rtxi_includes synced."
else
	echo "----->rtxi_includes sync failed."
	exit
fi
find ${PLG}/. -name "*.h" -exec cp -t /usr/local/lib/rtxi_includes/ {} +
