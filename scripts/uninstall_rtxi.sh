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

#!/bin/bash

# Directories
DIR=$PWD
ROOT=${DIR}/..

cd ${ROOT}

# Uninstall rtxi
sudo make uninstall
sudo make clean
sudo rm -rf /usr/local/lib/rtxi
sudo rm -rf /usr/local/lib/qwt
sudo rm -rf /usr/local/lib/rtxi_includes
sudo rm -rf /usr/local/include/rtxi
sudo rm -rf /etc/rtxi.conf

# Remove old qt/qwt installations
if [ $(dpkg-query -W -f='${Status}' qt4-dev-tools 2>/dev/null | grep -c "ok installed") -eq 1 ];
then
sudo apt-get purge libqt4-* qt4-*
fi

if [ $? -eq 0 ]; then
	echo "----->RTXI intallation successful. Reboot may be required."
else
	echo "----->RTXI installation failed."
	exit
fi
