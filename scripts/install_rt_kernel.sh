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
export linux_version=4.19.177
export linux_tree=/opt/linux-$linux_version
export xenomai_version=3.1
export xenomai_root=/opt/xenomai-$xenomai_version
export scripts_dir=`pwd`
export build_root=/opt/build
export opt=/opt
export ipipe_patch_digit=17
export ipipe_cip_str=-cip44

rm -rf $build_root
mkdir $build_root
echo  "-----> Environment configuration complete."

# Download essentials
echo  "-----> Downloading Linux kernel."
cd $opt
wget --no-clobber --no-check-certificate https://www.kernel.org/pub/linux/kernel/v${linux_version:0:1}.x/linux-$linux_version.tar.xz
tar xf linux-$linux_version.tar.xz

echo  "-----> Downloading Xenomai."
wget --no-clobber --no-check-certificate https://xenomai.org/downloads/xenomai/stable/xenomai-$xenomai_version.tar.bz2

echo "------> Downloading linux ipipe patch."
wget --no-clobber --no-check-certificate https://xenomai.org/downloads/ipipe/v${linux_version:0:1}.x/x86/ipipe-core-${linux_version}${ipipe_cip_str}-x86-${ipipe_patch_digit}.patch

tar xf xenomai-$xenomai_version.tar.bz2
echo  "-----> Downloads complete."

# Patch kernel
echo  "-----> Patching kernel."
cd $linux_tree
$xenomai_root/scripts/prepare-kernel.sh \
	--arch=x86 \
	--ipipe=$opt/ipipe-core-${linux_version}${ipipe_cip_str}-x86-${ipipe_patch_digit}.patch \
	--linux=$linux_tree \
	--verbose
yes "" | make oldconfig
make localmodconfig
make menuconfig
echo  "-----> Patching complete."

# Compile kernel
echo  "-----> Compiling kernel."
cd $linux_tree
export CONCURRENCY_LEVEL=$(grep -c ^processor /proc/cpuinfo)
fakeroot make-kpkg \
	--initrd \
	--append-to-version=-xenomai-$xenomai_version \
	--revision $(date +%Y%m%d) \
	kernel-image kernel-headers modules
echo  "-----> Kernel compilation complete."

# Install compiled kernel
echo  "-----> Installing compiled kernel"
cd $opt
dpkg -i linux-image-$linux_version-xenomai-$xenomai_version*.deb
dpkg -i linux-headers-$linux_version-xenomai-$xenomai_version*.deb
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
