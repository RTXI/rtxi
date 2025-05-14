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
SRCDIR=`pwd`
MODS=/tmp/rtxi_modules/

# Start configuring - by default configured to run on non-RT kernel
echo "-----> Starting RTXI installation..."

echo "-----> Kernel configuration..."
echo "evl -- Xenomai with EVL library (RT)"
echo "xenomai -- Legacy Xenomai with Alchemy skin (RT)"
echo "posix -- Linux based core (Non-RT)"
echo "preempt_rt -- Linux based core (RT)"
echo "-----> Please type your configuration and then press enter: "
read kernel

if [ -d "./build/local" ]; then
	rm -rf ./build/local
fi

cmake -S . -B ./build/local -D CMAKE_BUILD_TYPE=Release -DRTXI_RT_CORE=${kernel}

echo "-----> Configuration complete. Building..."
cmake --build ./build/local -j`nproc`
echo "-----> RTXI compilation successful. Creating Debian package..."
cd ./build/local
cpack -G DEB
echo "-----> RTXI package creation successful. Installing..."
sudo dpkg -i rtxi*.deb

# Delete modules directory if it exists from previous rtxi version
if [ -d "${MODS}" ]; then
	sudo rm -rf ${MODS}
fi

mkdir ${MODS}
cd ${MODS}

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
		git -C "$dir" pull
		git -C "$dir" clean -f -d 
		git -C "$dir" reset --hard
		cmake -S "$dir" -B "$dir/build" -D CMAKE_BUILD_TYPE=Release -D RTXI_PACKAGE_PATH=/usr
		cmake --build $dir/build -j`nproc` 
		sudo cmake --install "$dir/build"
	fi
done

cd ${SRCDIR}

echo ""
echo "-----> RTXI installation successful. Reboot may be required."
echo "-----> Type '"sudo rtxi"' to start RTXI. Happy Sciencing!"
echo "-----> Please email help@rtxi.org with any questions/help requests."
echo "-----> Script previously modified by Ivan F. Valerio <ivan@rtxi.org> on May 2023."
echo "-----> Script last modified by Sergio Hidalgo <sergiohg.dev@gmail.org> on March 2024."
