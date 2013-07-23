#-------------------------------------------------------------------------

#	This file is part of the BoneClamp Data Conversion and Processing System
#	Copyright (C) 2013 BoneClamp

#-------------------------------------------------------------------------

#	Filename: setup.sh simplifies the installation process by automatically
#             installing dependencies and xi-bone

#	Written in 2013 by:
#		Jon Newman <jnewman6@gatech.edu>
#		Yogi Patel <yapatel@gatech.edu>

#	To the extent possible under law, the author(s) have dedicated all copyright
#	and related and neighboring rights to this software to the public domain
#	worldwide. This software is distributed without any warranty.

#	You should have received a copy of the CC Public Domain Dedication along with
#	this software. If not, see <http://creativecommons.org/licenses/by-sa/3.0/legalcode>.

#!/bin/bash

# Check for compilation dependencies
echo "Checking for dependencies..."

sudo apt-get update
sudo apt-get upgrade
sudo apt-get install automake libtool autoconf autotools-dev build-essential qt3-dev-tools libboost-dev libboost-program-options-dev libgsl0-dev bison flex libncurses5-dev libqwt-dev

if [ $? -eq 0 ]; then
	echo "----->Dependencies installed."
else
	echo "----->Dependency installation failed."
	exit
fi

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
echo "----->Starting xi-bone installation..."

cd ../../
./autogen.sh
./configure 
sudo make -C ./
sudo make install -C ./

if [ $? -eq 0 ]; then
	echo "----->RTXI intallation successful."
else
	echo "----->RTXI installation failed."
	exit
fi

echo "----->Happy Sciencing!"
