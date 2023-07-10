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
	echo "Must run script as root; try again with sudo ./install_rt_kernel.sh."
	exit
fi

# Export environment variables
echo  "-----> Setting up variables."
export linux_version=5.10
export xenomai_version=3.2
export xenomai_root=~/git/xenomai-$xenomai_version
export scripts_dir=`pwd`
export build_root=/opt/build
export opt=/opt
export linux_tree=/opt/linux-dovetail

rm -rf $build_root
mkdir $build_root
echo  "-----> Environment configuration complete."

# Download essentials
cd $opt
echo "-----> Downloading main line kernel"
if [ ! -d linux-dovetail ] ; then
  git clone https://source.denx.de/Xenomai/linux-dovetail.git
  git checkout -b v5.15.y-dovetail-rebase
fi 

echo  "-----> Downloading Xenomai."
#wget --no-clobber --no-check-certificate https://xenomai.org/downloads/xenomai/stable/xenomai-$xenomai_version.tar.bz2
if [ ! -d /opt/xenomai-$xenomai_version ] ; then
  git clone --branch stable/v3.2.x https://source.denx.de/Xenomai/xenomai.git xenomai-$xenomai_version
fi
echo  "-----> Downloads complete."

# Patch kernel
echo  "-----> Patching kernel."
cd $linux_tree
git clean -f -d
git reset --hard
make clean
make mrproper
make distclean
$xenomai_root/scripts/prepare-kernel.sh \
	--arch=x86_64 \
	--linux=$linux_tree \
	--verbose

#yes "" | make oldconfig
yes "" | make localmodconfig
make menuconfig
scripts/config --disable SYSTEM_TRUSTED_KEYS
scripts/config --disable SYSTEM_REVOCATION_KEYS
scripts/config --disable DEBUG_INFO
echo  "-----> Patching complete."

# Compile kernel
echo  "-----> Compiling kernel."
cd $linux_tree
#export CONCURRENCY_LEVEL=$(grep -c ^processor /proc/cpuinfo)
make -j`nproc` bindeb-pkg LOCALVERSION=xenomai-$xenomai_version
echo  "-----> Kernel compilation complete."

# Install compiled kernel
echo  "-----> Installing compiled kernel"
sudo dpkg -i ../linux-image*.deb
sudo dpkg -i ../linux-headers*.deb
#sudo dpkg -i ../linux-glibc*.deb
echo  "-----> Kernel installation complete."

# Update
echo  "-----> Updating boot loader about the new kernel."
cd $linux_tree
update-initramfs -ck all

# Modify grub file to enable verbose boot
sed -i '7,8 s/^/#/' /etc/default/grub
sed -i -e 's/quiet//g' /etc/default/grub
sed -i -e 's/splash//g' /etc/default/grub
update-grub
echo  "-----> Boot loader update complete."

# Install user libraries
echo  "-----> Installing user libraries."
cd $xenomai_root
$xenomai_root/scripts/bootstrap
cd $build_root
$xenomai_root/configure --with-core=cobalt --enable-pshared --enable-smp --enable-dlopen-libs
make -sj`nproc`
make install
echo  "-----> User library installation complete."

# Add analogy_config to root path
cp -f /usr/xenomai/sbin/analogy_config /usr/sbin/

# Setting up user permissions
echo  "-----> Setting up user/group."
if grep -q xenomai /etc/group; then
    echo "xenomai group already exists"
else
    groupadd xenomai
fi
usermod -a -G xenomai "$SUDO_USER"
echo  "-----> Group setup complete."

# Restart
echo  "-----> Kernel patch complete."
echo  "-----> Reboot to boot into RT kernel."
