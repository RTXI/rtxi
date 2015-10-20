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

#! /bin/bash

sudo apt-get -y install gdebi # gdebi auto-installs dependencies

# Multiarch isn't supported for x86_64 systems, so manually add the
# i386 architecture. It's not pretty, but it should work. 
if [ `uname -m` == "x86_64" ]; then
   echo "Download and install teamviewer for x86_64"
   sudo dpkg --add-architecture i386
   sudo apt-get update
   wget http://download.teamviewer.com/download/teamviewer_linux.deb
#   wget http://download.teamviewer.com/download/teamviewer_amd64.deb
   sudo gdebi teamviewer_linux.deb
	sudo apt-get -f -y install
#   sudo dpkg --remove-architecture i386
   sudo apt-get update
elif [ `uname -m` == "x86" ]; then
   echo "Download and install teamviewer for x86"
   wget http://download.teamviewer.com/download/teamviewer_linux.deb
#   wget http://download.teamviewer.com/download/teamviewer_i386.deb
   sudo gdebi teamviewer_linux.deb
	sudo apt-get -f -y install
else 
   echo "What architecture is your computer?"
fi


# If you ever want to remove teamviewer and i386 packages this script installed
# on you x86_64 system, run:
#   $ sudo apt-get purge teamviewer
#   $ sudo apt-get purge `dpkg --get-selections | grep i386 | awk '{print $1}'
#   $ sudo dpkg --remove-architecture i386
#
# BE CAREFUL. This will remove ALL i386 packages listed before removing them.
# If you don't recognize a package (i.e. didn't manually name and install it)
# it should be safe to purge it. 
