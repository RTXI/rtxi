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

red='\e[0;31m'
NC='\e[0m'

# Export environment variables
echo -e "${red}----->Setting up variables${NC}"
export linux_version=2.6.32.20
export linux_tree=`pwd`/linux-$linux_version

export xenomai_version=2.6.3
export xenomai_root=`pwd`/xenomai-$xenomai_version

export build_root=`pwd`/build
mkdir $build_root

if [ $? -eq 0 ]; then
	echo -e "${red}----->Environment configuration complete${NC}"
else
	echo -e "${red}----->Environment configuration failed${NC}"
	exit
fi

# Download essentials
echo -e "${red}----->Downloading Linux kernel${NC}"
wget http://www.kernel.org/pub/linux/kernel/v2.6/linux-$linux_version.tar.bz2
tar xf linux-$linux_version.tar.bz2

echo -e "${red}----->Downloading Xenomai${NC}"
wget http://download.gna.org/xenomai/stable/xenomai-$xenomai_version.tar.bz2
tar xf xenomai-$xenomai_version.tar.bz2

if [ $? -eq 0 ]; then
	echo -e "${red}----->Downloads complete${NC}"
else
	echo -e "${red}----->Downloads failed${NC}"
	exit
fi

# Patch kernel
echo -e "${red}----->Patching kernel${NC}"
cd $linux_tree
cp -vi /boot/config-`uname -r` $linux_tree/.config
cp ../patch/kernel_config .config
$xenomai_root/scripts/prepare-kernel.sh --arch=x86 --adeos=../patch/adeos-ipipe-2.6.32.20-x86-2.7-03.patch --linux=$linux_tree
make oldconfig
make menuconfig

if [ $? -eq 0 ]; then
	echo -e "${red}----->Patching complete${NC}"
else
	echo -e "${red}----->Patching failed${NC}"
	exit
fi

# Compile kernel
echo -e "${red}----->Compiling kernel${NC}"
cd $linux_tree
sed -i 4s/.*/EXTRAVERSION=.20-linux-"$linux_version"-xenomai-"$xenomai_version"/ Makefile
make dep
make bzImage
make modules

if [ $? -eq 0 ]; then
	echo -e "${red}----->Kernel compilation complete.${NC}"
else
	echo -e "${red}----->Kernel compilation failed.${NC}"
	exit
fi

# Install compiled kernel
echo -e "${red}----->Installing compiled kernel${NC}"
cd $linux_tree
su
cp arch/x86/boot/bzImage /boot/vmlinuz-linux-$linux_version-xenomai-$xenomai_version
#cp System.map /boot/System.map-linux-$linux_version-xenomai-$xenomai_version
make modules_install

# Update
echo -e "${red}----->Updating boot loader about the new kernel${NC}"
#sudo new-kernel-pkg -v --mkinitrd --depmod --install linux-$linux_version-xenomai-$xenomai_version

if [ $? -eq 0 ]; then
	echo -e "${red}----->Boot loader update complete${NC}"
else
	echo -e "${red}----->Boot loader update failed${NC}"
	exit
fi

# Install user libraries
echo -e "${red}----->Installing user libraries${NC}"
cd $build_root
$xenomai_root/configure --enable-shared --enable-smp --enable-posix-auto-mlockall --enable-dlopen-skins --enable-x86-sep
make
sudo make install

if [ $? -eq 0 ]; then
	echo -e "${red}----->User library installation complete${NC}"
else
	echo -e "${red}----->User library installation failed${NC}"
	exit
fi

# Setting up user permissions
echo -e "${red}----->Setting up user/group${NC}"
sudo usermod -a -G xenomai `whoami`

if [ $? -eq 0 ]; then
	echo -e "${red}----->Group setup complete${NC}"
else
	echo -e "${red}----->Group setup failed${NC}"
	exit
fi

# Restart
echo -e "${red}----->Restarting system${NC}"
sudo reboot now
