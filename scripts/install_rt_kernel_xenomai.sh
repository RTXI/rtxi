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
#export linux_version=$( uname -r | sed -r 's/([0-9]+\.[0-9]+).*/\1/' )
export linux_version=6.6
export xenomai_version="3.2"
export xenomai_root=/opt/xenomai-$xenomai_version
export xenomai_build_dir="$xenomai_root/build"
export scripts_dir=`pwd`
export build_root=/opt/build
export opt=/opt
export linux_tree=/opt/linux-dovetail
export linux_build="$linux_tree/build"

rm -rf $xenomai_build_dir
rm -rf $linux_build

echo  "-----> Environment configuration complete."

# Download essentials
cd $opt
echo "-----> Downloading main line kernel"
if [ ! -d $linux_tree ] ; then
  git clone --branch v$linux_version.y-dovetail-rebase https://source.denx.de/Xenomai/linux-dovetail.git
else
  cd $linux_tree
  git fetch
  git pull
  git checkout --branch v$linux_version.y-dovetail-rebase
fi 

echo  "-----> Downloading Xenomai."
if [ ! -d $xenomai_root ] ; then
  git clone --branch stable/v3.2.x https://source.denx.de/Xenomai/xenomai.git xenomai-$xenomai_version
else
  cd $xenomai_root
  git fetch
  git pull
  git checkout --branch stable/v3.2.x
fi
echo  "-----> Downloads complete."

# Install user libraries
echo  "-----> Installing Xenomai library."
mkdir $xenomai_build_dir
cd $xenomai_build_dir
$xenomai_root/configure --with-core=cobalt --enable-smp --enable-pshared --enable-dlopen-libs
make -j`nproc`
make install
echo  "-----> User library installation complete."


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

yes "" | make oldconfig
yes "" | make localmodconfig
make menuconfig
scripts/config --disable SYSTEM_TRUSTED_KEYS
scripts/config --disable SYSTEM_REVOCATION_KEYS
scripts/config --disable DEBUG_INFO
echo  "-----> Patching complete."

# Compile kernel
echo  "-----> Compiling kernel."
cd $linux_tree
make -j`nproc` bindeb-pkg LOCALVERSION=-xenomai-$xenomai_version-$linux_version
echo  "-----> Kernel compilation complete."

# Install compiled kernel
echo  "-----> Installing compiled kernel"
sudo dpkg -i ../linux-image*xenomai-$xenomai_version*.deb
sudo dpkg -i ../linux-headers*xenomai-$xenomai_version*.deb
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
