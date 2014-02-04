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

# Export environment variables
echo "----->Setting up variables"
export linux_version=2.6.32.20
export linux_tree=`pwd`/linux-$linux_version

export xenomai_version=2.5.5.2
export xenomai_root=`pwd`/xenomai-$xenomai_version

export build_root=`pwd`/build
mkdir $build_root

# Download essentials
echo "----->Downloading essentials"
wget http://download.gna.org/xenomai/stable/xenomai-$xenomai_version.tar.bz2
tar xf xenomai-$xenomai_version.tar.bz2

wget http://www.kernel.org/pub/linux/kernel/v2.6/linux-$linux_version.tar.bz2
tar xf linux-$linux_version.tar.bz2

# Patch kernel
echo "----->Patching kernel"
cd linux-$linux_version
cp -vi /boot/config-`uname -r` $linux_tree/.config

$xenomai_root/scripts/prepare-kernel.sh --arch=x86 --adeos=$xenomai_root/ksrc/arch/x86/patches/adeos-ipipe-2.6.32.20-x86-2.7-03.patch --linux=$linux_tree

# Configure kernel
echo "----->Configuring kernel"
cd $linux_tree

# Compile kernel
echo "----->Compiling kernel"
export CONCURRENCY_LEVEL=7
fakeroot make-kpkg --bzimage --initrd --append-to-version=-xenomai-$xenomai_version kernel-image kernel-headers modules

cd ..
sudo dpkg -i linux-image-*.deb
sudo dpkg -i linux-headers-*.deb

sudo update-initramfs -c -k "$linux_version-xenomai-$xenomai_version"
sudo update-grub

# Install user libraries
echo "----->Installing user libraries"
cd $build_root
$xenomai_root/configure --enable-shared --enable-smp --enable-posix-auto-mlockall --enable-dlopen-skins --enable-x86-sep
make
sudo make install

# Setting up user permissions
echo "----->Setting up user groups"
sudo add user xenomai
sudo adduser `whoami` xenomai

sudo update-grub

# Restart
echo "----->Restarting system"
