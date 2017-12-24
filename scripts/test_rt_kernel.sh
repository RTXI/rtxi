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

echo "----->Running latency test with a system frequency of 20 kHz under load. Please wait 10 minutes."
echo "----->Do not interrupt."
echo "----->If you do interrupt, stop stressing the system by running:"
echo "      $ pkill stress"
echo ""

#echo "----->Please enter the frequency (in Hz) that you would like to test, then press enter."
SysFreq=20000

# Get system information
DISTRO="$(lsb_release -is) $(lsb_release -rs)"
HOSTNAME=`uname -n`
RT_KERNEL=`uname -r`
PROCESSOR=$(cat /proc/cpuinfo | grep "model name" | uniq | cut -d":" -f2 | sed 's/ \+/ /g' | sed -e 's/^\  *//' -e 's/\ *$//')
GRAPHICS_CARD=$(lspci | grep VGA | uniq | cut -d":" -f3 | sed 's/ \+/ /g' | sed -e 's/^\  *//' -e 's/\ *$//')
GRAPHICS_DRIVER=$(sudo lshw -c display | grep "configuration: driver" | cut -d":" -f2 | cut -d"=" -f2 | cut -d" " -f1 | sed 's/ \+/ /g' | sed -e 's/^\  *//' -e 's/\ *$//')

# For nouveau, sometimes, lshw doesn't show that it's loaded but lsmod does. 
if [ "$GRAPHICS_DRIVER" == "" ]; then
	GRAPHICS_DRIVER=$(lsmod | grep video | sed 's/  */ /g' | cut -d" " -f4)
fi
DAQ=$(lspci | grep National | cut -d":" -f3 | sed 's/ \+/ /g' | sed -e 's/^\  *//' -e 's/\ *$//')

# Set up variables for run
TIME=600 # duration of run (s)
RT_PERIOD=$(awk "BEGIN {print 1 / $SysFreq * 1e6}") # period in us
RATE=$(expr 1000 / $RT_PERIOD) # Convert RT period to freq in kHz

# Run latency test under dynamic load
if [ -f test_rt_histdata.txt ]; then
	echo 'The test has been run already. Rename test_rt_histdata.txt or delete it. Then, run this script again.'
else
	# Calibrate Xenomai to not show negative latencies
	sudo bash -c "echo 0 > /proc/xenomai/latency"
	# Run stress
	stress --cpu 2 --vm 1 --hdd 1 --timeout $TIME & 
	# Start testing
	sudo /usr/xenomai/bin/./latency -s -h -p $RT_PERIOD -B 1 -H 500000 -T $TIME -g test_rt_histdata.txt | tee test_rt_kernel.log
fi

# Print out hardware config
sudo lshw > lswh.txt
lspci -vvv > lspci.txt
cat /proc/cpuinfo > cpuinfo.txt
mkdir test_results
mv *.txt test_results/.
mv *.log test_results/.

exit 0
