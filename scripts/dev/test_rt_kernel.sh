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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# Created by Yogi Patel <yapatel@gatech.edu> 2014.1.31

#!/bin/bash

# Check to see if R and stress are installed
echo "----->Checking for dependencies needed to run stress test."
if ! $(dpkg-query -Wf'${db:Status-abbrev}' "stress" 2>/dev/null | grep -q '^i'); 
	then sudo apt-get -y install stress
fi
if ! $(dpkg-query -Wf'${db:Status-abbrev}' "lshw" 2>/dev/null | grep -q '^i'); 
	then sudo apt-get -y install lshw
fi
if ! $(dpkg-query -Wf'${db:Status-abbrev}' "r-base" 2>/dev/null | grep -q '^i'); 
	then sudo apt-get -y install r-base
fi
echo ""

echo "----->Running latency test under load. Please wait 30 minutes."
echo "----->Do not interrupt."
echo "----->If you do interrupt, stop stressing the system by running:"
echo "      $ pkill stress"
echo ""

# Get system information
DISTRO="$(lsb_release -is) $(lsb_release -rs)"
HOSTNAME=`uname -n`
RT_KERNEL=`uname -r`
PROCESSOR=$(cat /proc/cpuinfo | grep "model name" | uniq | cut -d":" -f2 | sed 's/ \+/ /g' | sed -e 's/^\  *//' -e 's/\ *$//')
GRAPHICS_CARD=$(lspci | grep VGA | uniq | cut -d":" -f3 | sed 's/ \+/ /g' | sed -e 's/^\  *//' -e 's/\ *$//')
GRAPHICS_DRIVER=$(lshw -c display | grep "configuration: driver" | cut -d":" -f2 | cut -d"=" -f2 | cut -d" " -f1 | sed 's/ \+/ /g' | sed -e 's/^\  *//' -e 's/\ *$//')

# Set up variables for run
TIME=1800 # duration of run (s)
RT_PERIOD=100
RATE=$(expr 1000 / $RT_PERIOD) # Convert RT period to freq in kHz

# Run latency test under dynamic load
stress --cpu 2 --vm 1 --hdd 1 --timeout $TIME & 
sudo /usr/xenomai/bin/./latency -s -h -p $RT_PERIOD -B 1 -H 500000 -T $TIME -g test_rt_histdata.txt | tee test_rt_kernel.log

Rscript makeHistPlot.r "$DISTRO" "$HOSTNAME" "$RT_KERNEL" "$PROCESSOR" "$GRAPHICS_CARD" "$GRAPHICS_DRIVER" "$RATE"

exit 0
