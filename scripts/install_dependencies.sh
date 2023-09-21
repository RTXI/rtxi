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
	build-essential qt5-default libmarkdown2-dev libgit2-dev libqwt-qt5-dev libhdf5-dev \
	libgsl0-dev libboost-dev libgtest-dev libgmock-dev vim lshw stress \
	autotools-dev automake libtool kernel-package binutils-dev zstd \
	crash kexec-tools makedumpfile kernel-wedge libncurses5-dev libelf-dev \
	flex bison libssl-dev pkgconf autoconf-archive python3-pip ninja-build dwarves
echo "-----> Package dependencies installed."
echo "-----> Installing python specific packages..."
pip3 install meson 
echo "-----> Done."
