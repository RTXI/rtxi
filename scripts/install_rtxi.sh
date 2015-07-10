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
ROOT=../
DEPS=../deps/
HDF=${DEPS}/hdf
QWT=${DEPS}/qwt
RTXI_LIB=/usr/local/lib/rtxi/

# Start at top
cd ${ROOT}

# Start configuring - by default configured to run on non-RT kernel
echo "----->Starting RTXI installation..."
./autogen.sh

echo "----->Kernel configuration..."
echo "1. Xenomai+Analogy (RT)"
echo "2. POSIX (Non-RT)"
echo "3. POSIX (Non-RT) Debug"
echo "----->Please select your configuration and then press enter:"
read kernel

if [ $kernel -eq "1" ]; then
	./configure --enable-xenomai --enable-analogy --disable-debug
elif [ $kernel -eq "2" ]; then
	./configure --enable-posix --disable-debug
elif [ $kernel -eq "3" ]; then
	./configure --enable-posix --enable-debug
else
	echo "Invalid configuration."
	exit 1
fi

make -sj`nproc` -C ./

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
sudo mkdir ${RTXI_LIB}
sudo cp -f libtool ${RTXI_LIB}
sudo cp -f scripts/icons/RTXI-icon.png ${RTXI_LIB}
sudo cp -f scripts/icons/RTXI-widget-icon.png ${RTXI_LIB}
sudo cp -f scripts/rtxi.desktop /usr/share/applications/
cp -f scripts/rtxi.desktop ~/Desktop/
chmod +x ~/Desktop/rtxi.desktop
sudo cp -f rtxi.conf /etc/
sudo cp -f /usr/xenomai/sbin/analogy_config /usr/sbin/

if [ $(lsb_release -sc) == "jessie" ]; then
	echo "Load analogy driver with systemd"
	sudo cp -f ./scripts/services/rtxi_load_analogy.service /etc/systemd/system/
	sudo systemctl enable rtxi_load_analogy.service
else
	echo "Load analogy driver with sysvinit/upstart"
	sudo cp -f ./scripts/services/rtxi_load_analogy /etc/init.d/
	sudo update-rc.d rtxi_load_analogy defaults
fi
sudo ldconfig

echo ""
if [ $? -eq 0 ]; then
	echo "----->RTXI intallation successful. Reboot may be required."
else
	echo "----->RTXI installation failed."
	exit
fi

echo "----->Type '"sudo rtxi"' to start RTXI. Happy Sciencing!"
echo "----->Please email help@rtxi.org with any questions/help requests."
echo "----->Script developed/last modified by Yogi Patel <yapatel@gatech.edu> on May 2014."
