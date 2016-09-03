#! /bin/bash

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

if ! id | grep -q root; then
	echo "Must run script as root; try again with sudo ./install_teamviewer.sh"
	exit
fi

# Install gdebi for detecting and automatically installing dependencies
apt-get -y install gdebi 

# Teamviewer doesn't provide a 64-bit binary, so enable multiarch (i386) for 
# installing dependencies. 
if [ `uname -m` == "x86_64" ]; then
    dpkg --add-architecture i386 # if x86_64, enable i386 mirrors
    apt-get update
fi

wget http://download.teamviewer.com/download/teamviewer_i386.deb
gdebi teamviewer_i386.deb
apt-get -f -y install

# If you ever want to uninstall teamviewer and its i386 dependencies from your
# your x86_64 system, run:
#   $  apt-get purge teamviewer
#   $  apt-get purge `dpkg --get-selections | grep i386 | awk '{print $1}'`
#   $  dpkg --remove-architecture i386
#
# This will remove ALL i386 packages listed before removing them. DO NOT run 
# this script on a i386 system. If you don't know what architecture your 
# system is, run "uname -m" in the terminal to find out. 
