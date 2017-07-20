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

if id | grep -q root; then
	echo "You do not need to run as root; try again without sudo"
	exit
fi

# Directories
ROOT=../
MODS=/usr/local/lib/rtxi_modules/

# Start at top
cd ${ROOT}

# Start configuring - by default configured to run on non-RT kernel
echo "-----> Starting RTXI installation..."
./autogen.sh

echo "-----> Kernel configuration..."
echo "1. Xenomai+Analogy (RT)"
echo "2. Xenomai+Analogy (RT) Debug"
echo "3. POSIX (Non-RT)"
echo "-----> Please select your configuration and then press enter:"
read kernel

if [ $kernel -eq "1" ]; then
	./configure --enable-xenomai --enable-analogy --disable-debug
elif [ $kernel -eq "2" ]; then
	./configure --enable-xenomai --enable-analogy --enable-debug
elif [ $kernel -eq "3" ]; then
	./configure --enable-posix --disable-debug
else
	echo "Invalid configuration."
	exit 1
fi

make -sj`nproc` -C ./
echo "-----> RTXI compilation successful."

sudo make install -C ./
echo "-----> RTXI intallation successful."

echo "-----> Putting things into place."
cp -f res/rtxi.desktop ~/Desktop/
chmod +x ~/Desktop/rtxi.desktop

# Install startup script to load analogy driver at boot
if [ $(lsb_release -sc) == "jessie" ] || [ $(lsb_release -sc) == "xenial" ]; then
	echo "-----> Load analogy driver with systemd"
	sudo cp -f ./scripts/services/rtxi_load_analogy.service /etc/systemd/system/
	sudo systemctl enable rtxi_load_analogy.service
else
	echo "-----> Load analogy driver with sysvinit/upstart"
	sudo cp -f ./scripts/services/rtxi_load_analogy /etc/init.d/
	sudo update-rc.d rtxi_load_analogy defaults
fi
sudo ldconfig
echo "-----> Successfully placed files.."

# TEMPORARY WORKAROUND
echo "-----> Installing basic modules."
sudo mkdir -p ${MODS}

# Allow all members of adm (administrator accounts) write access to the 
# rtxi_modules/ directory. 
sudo setfacl -Rm g:adm:rwX,d:g:adm:rwX ${MODS}

cd ${MODS}
sudo rm -rf ${MODS}/*
git clone https://github.com/RTXI/analysis-module.git
git clone https://github.com/RTXI/iir-filter.git
git clone https://github.com/RTXI/fir-window.git
git clone https://github.com/RTXI/sync.git
git clone https://github.com/RTXI/mimic-signal.git
git clone https://github.com/RTXI/signal-generator.git
git clone https://github.com/RTXI/ttl-pulses.git
git clone https://github.com/RTXI/wave-maker.git
git clone https://github.com/RTXI/noise-generator.git

for dir in ${MODS}/*; do
	if [ -d "$dir" ]; then
		make clean -C "$dir"
		git -C "$dir" pull
		make -j`nproc` -C "$dir"
		sudo make install -C "$dir"
	fi
done

echo ""
echo "-----> RTXI intallation successful. Reboot may be required."
echo "-----> Type '"sudo rtxi"' to start RTXI. Happy Sciencing!"
echo "-----> Please email help@rtxi.org with any questions/help requests."
echo "-----> Script developed/last modified by Yogi Patel <yapatel@gatech.edu> on May 2014."
