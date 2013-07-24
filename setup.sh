#
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

#!/bin/bash

# Check for compilation dependencies
echo "Checking for dependencies..."

sudo apt-get update
sudo apt-get upgrade
sudo apt-get install automake libtool autoconf autotools-dev build-essential qt3-dev-tools libboost-dev libboost-program-options-dev libgsl0-dev bison flex libncurses5-dev libqwt5-qt3-dev

if [ $? -eq 0 ]; then
	echo "----->Dependencies installed."
else
	echo "----->Dependency installation failed."
	exit
fi

sudo ln -s /usr/lib/libqwt-qt3.so.5 /usr/lib/libqwt.so

# Installing HDF5
echo "----->Installing HDF5..."

cd hdf
tar xf hdf5-1.8.4.tar.bz2
cd hdf5-1.8.4
./configure --prefix=/usr
sudo make
sudo make install

if [ $? -eq 0 ]; then
	echo "----->HDF5 installed."
else
echo "----->HDF5 installation failed."
	exit
fi

# Start configuring - by default configured to run on non-RT kernel
echo "----->Starting RTXI installation..."

cd ../../
./autogen.sh
./configure --enable-posix --disable-comedi --disable-analogy --disable-rtai --disable-xenomai
sudo make -C ./
sudo make install -C ./
sudo cp rtxi.conf /etc

if [ $? -eq 0 ]; then
	echo "----->RTXI intallation successful."
else
	echo "----->RTXI installation failed."
	exit
fi

echo "----->Happy Sciencing!"
