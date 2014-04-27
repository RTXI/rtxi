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
#	Created by Yogi Patel <yapatel@gatech.edu> 2014.1.31
#

#!/bin/bash

cd ../

# Installing HDF5
echo "----->Checking for HDF5"

if [ -f "/usr/include/hdf5.h" ]; then
	echo "----->HDF5 already installed."
else
	echo "----->Installing HDF5..."
	cd hdf
	tar xf hdf5-1.8.4.tar.bz2
	cd hdf5-1.8.4
	./configure --prefix=/usr
	make
	sudo make install
	cd ../../
	if [ $? -eq 0 ]; then
			echo "----->HDF5 installed."
	else
		echo "----->HDF5 installation failed."
		exit
	fi
fi

# Start configuring - by default configured to run on non-RT kernel
echo "----->Starting RTXI installation..."
./autogen.sh

echo "----->Kernel configuration..."
echo "1. Xenomai+Analogy"
echo "2. RTAI+COMEDI"
echo "3. POSIX (Non-RT)"
echo "----->Please select your configuration and then press enter:"
read kernel

if [ $kernel -eq "1" ]; then
	./configure --enable-xenomai --enable-analogy
elif [ $kernel -eq "2" ]; then
	./configure --enable-rtai --enable-comedi
elif [ $kernel -eq "3" ]; then
	./configure --enable-posix --disable-analogy --disable-comedi
else
	echo "Invalid configuration."
	exit 1
fi

make -C ./

if [ $? -eq 0 ]; then
	echo "----->RTXI compilation successful."
else
	echo "----->RTXI compilation failed."
	exit
fi

sudo make install -C ./

if [ $? -eq 0 ]; then
	echo "----->RTXI intallation successful."
else
	echo "----->RTXI installation failed."
	exit
fi

echo "----->Putting things into place."
sudo cp libtool /usr/local/lib/rtxi/
sudo cp rtxi.conf /etc/
sudo cp /usr/xenomai/sbin/analogy_config /usr/sbin/
sudo cp ./scripts/rtxi_load_analogy /etc/init.d/
sudo update-rc.d rtxi_load_analogy defaults

if [ $? -eq 0 ]; then
	echo "----->RTXI intallation successful. Reboot may be required."
else
	echo "----->RTXI installation failed."
	exit
fi

echo "----->type '"sudo rtxi"' to begin"
echo "----->Happy Sciencing!"
echo "----->Please email help@rtxi.org with any questions/help requests."
echo "----->Script developed/last modified by Yogi Patel <yapatel@gatech.edu> on May 2014."
