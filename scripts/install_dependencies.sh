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

#!/bin/bash

# Directories
DIR=$PWD
ROOT=${DIR}/../
DEPS=${ROOT}/deps
HDF=${DEPS}/hdf
QWT=${DEPS}/qwt
DYN=${DEPS}/dynamo

# Check for compilation dependencies
echo "Checking for dependencies..."

sudo apt-get update
sudo apt-get -y upgrade
sudo apt-get -y install autotools-dev automake libtool
sudo apt-get -y install kernel-package
sudo apt-get -y install fakeroot build-essential crash kexec-tools makedumpfile kernel-wedge
sudo apt-get -y build-dep linux
sudo apt-get -y install git-core libncurses5 libncurses5-dev libelf-dev binutils-dev libgsl0-dev vim stress libboost-all-dev
sudo apt-get -y install qt4-dev-tools libqt4-dev libqt4-opengl-dev

if [ $? -eq 0 ]; then
	echo "----->Dependencies installed."
else
	echo "----->Dependency installation failed."
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
	make -j2
	sudo make install
	if [ $? -eq 0 ]; then
			echo "----->HDF5 installed."
	else
		echo "----->HDF5 installation failed."
		exit
	fi
fi

# Installing Qwt
echo "----->Checking for Qwt"

if [ -f "/usr/local/lib/qwt/include/qwt.h" ]; then
	echo "----->Qwt already installed."
else
	echo "----->Installing Qwt..."
	cd ${QWT}
	tar xf qwt-6.1.0.tar.bz2
	cd qwt-6.1.0
	qmake qwt.pro
	make -j2
	sudo make install
	sudo cp /usr/local/lib/qwt/lib/libqwt.so.6.1.0 /usr/lib/.
	sudo ln -sf /usr/lib/libqwt.so.6.1.0 /usr/lib/libqwt.so
	sudo ldconfig
	if [ $? -eq 0 ]; then
		echo "----->Qwt installed."
	else
		echo "----->Qwt installation failed."
	exit
	fi
fi

# Install rtxi_includes
sudo rsync -a ${DEPS}/rtxi_includes /usr/local/lib/.
if [ $? -eq 0 ]; then
	echo "----->rtxi_includes synced."
else
	echo "----->rtxi_includes sync failed."
	exit
fi
find ../plugins/. -name "*.h" -exec cp -t /usr/local/lib/rtxi_includes/ {} +

# Install dynamo
echo "Installing DYNAMO utility..."

sudo apt-get -y install mlton
cd ${DYN}
mllex dl.lex
mlyacc dl.grm
mlton dynamo.mlb
sudo cp dynamo /usr/bin/
if [ $? -eq 0 ]; then
	echo "----->DYNAMO translation utility installed."
else
	echo "----->DYNAMO translation utility installation failed."
	exit
fi
