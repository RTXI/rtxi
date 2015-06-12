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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# Created by Yogi Patel <yapatel@gatech.edu> 2014.1.31
#

#!/bin/bash

# Get the filenames
HDF_FILENAME=$1

# Check if an HDF file was provided. If so, filename as template to make other filenames
if [ -f "$HDF_FILENAME" ]; then
	if [ "${HDF_FILENAME##*.}" == "h5" ]; then 
		TXT_FILENAME=${HDF_FILENAME%.*}".txt"
		PLOT_FILENAME=${HDF_FILENAME%.*}".png"
	else
		echo "----->You need to provide an *.h5 file"
		exit 1
	fi
else 
	echo "----->Provided file not found."
	exit 1
fi

# Prompt user to pick a trials from the HDF file
ALL_TRIALS=$(h5ls $HDF_FILENAME | cut -d" " -f1 | sed "s/Trial//g")
if [ "$ALL_TRIALS" == "1" ]; then
	TRIAL_N=1
elif [ "$ALL_TRIALS" == "" ]; then
	echo "----->Check your filename."
	exit 1
else
	echo "----->Which trial do you want to plot? (Enter the number of the trial you want.)" 
	TRIAL_OPTIONS=""
	for trial in $ALL_TRIALS; do 
		TRIAL_OPTIONS=$TRIAL_OPTIONS$trial" "
	done
	echo "----->Options: $TRIAL_OPTIONS"
	read TRIAL_N
	if [[ "$(echo $ALL_TRIALS | sed "s/ //g")" =~ "$TRIAL_N" ]]&&[[ $ALL_TRIALS =~ "$TRIAL_N" ]]; then
		echo "Valid option"
	else
		echo "Invalid trial selected. (No whitespaces allowed.)"
		exit 1
	fi
fi

# Extract raw data from HDF file
h5dump -d "/Trial$TRIAL_N/Synchronous Data/Channel Data" -y -w 36 -o $TXT_FILENAME $HDF_FILENAME
sed -i "s/,//g" $TXT_FILENAME # take care of comma delimiting
sed -i "s/\[//g" $TXT_FILENAME # formatting file...
sed -i "s/\]//g" $TXT_FILENAME # 

# Get system information to record in the plot
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
RT_PERIOD=$(h5dump -d "/Trial$TRIAL_N/Period (ns)" $HDF_FILENAME |  grep "(0)" | cut -d":" -f2) # in ns
DOWNSAMPLE=$(h5dump -d "/Trial$TRIAL_N/Downsampling Rate" $HDF_FILENAME |  grep "(0)" | cut -d":" -f2)
COMPTIME_CHANNEL=$(h5ls -r $HDF_FILENAME | grep "Trial"$TRIAL_N | grep -- "Comp\\\ Time" | cut -d"/" -f4 | cut -d":" -f1)
RTPERIOD_CHANNEL=$(h5ls -r $HDF_FILENAME | grep "Trial"$TRIAL_N | grep -- "Real-time\\\ Period" | cut -d"/" -f4 | cut -d":" -f1)
RTJITTER_CHANNEL=$(h5ls -r $HDF_FILENAME | grep "Trial"$TRIAL_N | grep -- "RT\\\ Jitter" | cut -d"/" -f4 | cut -d":" -f1)

if [ "$COMPTIME_CHANNEL" == "" ]||[ "$RTPERIOD_CHANNEL" == "" ]||[ "$RTJITTER_CHANNEL" == "" ]; then
	echo "All the needed channels (Comp Time, Real-time Period, and RT Jitter), couldn't be found."
	exit 1
fi

Rscript makePerfPlot.r "$DISTRO" "$HOSTNAME" "$RT_KERNEL" "$PROCESSOR" "$GRAPHICS_CARD" \
                       "$GRAPHICS_DRIVER" "$RT_PERIOD" "$DOWNSAMPLE" "$COMPTIME_CHANNEL" \
                       "$RTPERIOD_CHANNEL" "$RTJITTER_CHANNEL" "$TXT_FILENAME" "$PLOT_FILENAME" "$DAQ"
exit 0
