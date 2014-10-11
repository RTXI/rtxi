#! /bin/bash

sudo apt-get -y install gdebi # gdebi auto-installs dependencies

# I've had issues with debian. Adding a foreign arch fixes the issue,
#  but it isn't the most elegant solution. 
if [ `lsb_release -sc` == "wheezy" ]; then
   sudo dpkg --add-architecture i386
   sudo apt-get update
   wget http://download.teamviewer.com/download/teamviewer_linux.deb
   sudo gdebi teamviewer_linux.deb
else
   wget http://download.teamviewer.com/download/teamviewer_linux.deb
   sudo gdebi teamviewer_linux.deb
fi
