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

###############################################################################
# Set directory variable for compilation
###############################################################################
DIR=$PWD
ROOT=${DIR}/../
DEPS=${ROOT}/deps
HDF=${DEPS}/hdf
QWT=${DEPS}/qwt
DYN=${DEPS}/dynamo
PLG=${ROOT}/plugins

###############################################################################
# Check for all RTXI *.deb dependencies and install them. Includes:
#  - Kernel tools
#  - C/C++ compiler and debugger
#  - Qt5, HDF, and Qwt6 libraries
#  - R and some R packages
###############################################################################
echo "Checking dependencies..."

sudo apt-get update
sudo apt-get -y upgrade
sudo apt-get -y install autotools-dev automake libtool kernel-package gcc g++ \
                        gdb fakeroot crash kexec-tools makedumpfile \
                        kernel-wedge libncurses5-dev libelf-dev binutils-dev \
                        libgsl0-dev libboost-dev 
sudo apt-get -y install git vim emacs lshw stress
sudo apt-get -y install libqt5svg5-dev libqt5opengl5 libqt5gui5 libqt5core5a \
                        libqt5xml5 qt5-default
sudo apt-get -y install r-base r-cran-ggplot2 r-cran-reshape2 r-cran-hdf5 \
                        r-cran-plyr r-cran-scales

sudo apt-get -y build-dep linux

if [ $? -eq 0 ]; then
	echo "----->Package dependencies installed."
else
	echo "----->Package dependency installation failed."
	exit
fi

# Start at top
cd ${DEPS}

# gridExtra doesn't have a *.deb package yet, so install it manually.
echo "----->Install gridExtra R package"

if [ -d "/usr/local/lib/R/site-library" ]; then
	echo "----->gridExtra already installed"
else
	wget --no-check-certificate http://cran.r-project.org/src/contrib/gridExtra_0.9.1.tar.gz
	tar xf gridExtra_0.9.1.tar.gz
	sudo R CMD INSTALL gridExtra
	if [ $? -eq 0 ]; then
			echo "----->gridExtra installed."
	else
		echo "----->gridExtra installation failed."
		exit
	fi
fi

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
	make -sj2
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
find ${PLG}/. -name "*.h" -exec cp -t /usr/local/lib/rtxi_includes/ {} +

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
