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

# Check for compilation dependencies
echo "Checking for dependencies..."

if [[ $(lsb_release --id) == *Ubuntu* ]]
then
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install autotools-dev automake libtool
sudo apt-get install kernel-package
sudo apt-get install fakeroot build-essential crash kexec-tools makedumpfile kernel-wedge
sudo apt-get build-dep linux
sudo apt-get install git-core libncurses5 libncurses5-dev libelf-dev asciidoc binutils-dev
sudo apt-get install qt4-dev-tools libqt4-dev  libqwt6 #libqwt5-qt4-dev
elif [[ $(lsb_release --id) == *Scientific* ]]
then
sudo yum update
sudo yum upgrade
sudo yum groupinstall “Development Tools”
sudo yum install automake libtool autoconf boost-devel bison flex ncurses-devel.x86_64 qwt-devel.x86_64 gsl-devel.x86_64 boost-program-options.x86_64 qt qt-devel fakeroot crash rpmdevtools ncurses-devel
fi

if [ $? -eq 0 ]; then
	echo "----->Dependencies installed."
else
	echo "----->Dependency installation failed."
	exit
fi

# Install qwt
echo "Installing widget library..."

#sudo ln -s /usr/lib/libqwt-qt4.so.5 /usr/lib/libqwt.so

if [ $? -eq 0 ]; then
	echo "----->Dependencies installed."
else
	echo "----->Dependency installation failed."
	exit
fi
