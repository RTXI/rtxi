#!/bin/bash
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
# Modified by Ivan Valerio <valerioif@gmail.com> 2023.09.20
# Modified by Sergio Hidalgo <sergiohg.dev@gmail.com> 2025.03.19
#

if ! id | grep -q root; then
	echo "Must run script as root; try again with ./install_dependencies.sh."
	exit 1
fi

# Install RTXI dependencies
echo "-----> Installing dependencies..."
apt-get update
apt-get -y upgrade
apt-get -y install \
	build-essential qtbase5-dev qtbase5-dev-tools \
	libqwt-qt5-dev libqt5svg5-dev libhdf5-dev \
	libgsl-dev libgtest-dev libgmock-dev libfmt-dev vim lshw stress \
	binutils-dev zstd git cmake\
	crash kexec-tools makedumpfile kernel-wedge libncurses5-dev libelf-dev \
	flex bison pkgconf python3-pip ninja-build dwarves libboost-all-dev
echo "-----> Package dependencies installed."
echo "-----> Done."
