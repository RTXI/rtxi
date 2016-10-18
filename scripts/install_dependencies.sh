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
	echo "Must run script as root; try again with ./install_dependencies.sh."
	exit 1
fi

#
# Check for all RTXI *.deb dependencies and install them. Includes:
#  - Kernel tools
#  - C/C++ compiler and debugger
#  - Qt5, HDF, and Qwt6 libraries
#
echo "-----> Checking dependencies..."
apt-get update
apt-get -y upgrade
apt-get -y install \
	autotools-dev automake libtool kernel-package gcc g++ gdb fakeroot \
	crash kexec-tools makedumpfile kernel-wedge libncurses5-dev libelf-dev \
	binutils-dev libgsl0-dev libboost-dev git vim emacs lshw stress gksu \
	libqt5svg5-dev libqt5opengl5 libqt5gui5 libqt5core5a libqt5xml5 \
	qt5-default qttools5-dev-tools qttools5-dev libqwt-qt5-dev \
	libhdf5-dev libgit2-dev libmarkdown2-dev
apt-get -y build-dep linux
echo "-----> Package dependencies installed."
echo "-----> Done."
