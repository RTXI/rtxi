#! /bin/bash

sudo apt-get -y install gdebi # gdebi auto-installs dependencies

# Multiarch isn't supported for x86_64 systems, so manually add the
# i386 architecture. It's not pretty, but it should work. 
if [ `uname -m` == "x86_64" ]; then
   echo "Download and install teamviewer for x86_64"
   sudo dpkg --add-architecture i386
   sudo apt-get update
   wget http://download.teamviewer.com/download/teamviewer_linux.deb
#   wget http://download.teamviewer.com/download/teamviewer_amd64.deb
   sudo gdebi teamviewer_linux.deb
	sudo apt-get -f -y install
#   sudo dpkg --remove-architecture i386
   sudo apt-get update
elif [ `uname -m` == "x86" ]; then
   echo "Download and install teamviewer for x86"
   wget http://download.teamviewer.com/download/teamviewer_linux.deb
#   wget http://download.teamviewer.com/download/teamviewer_i386.deb
   sudo gdebi teamviewer_linux.deb
	sudo apt-get -f -y install
else 
   echo "What architecture is your computer?"
fi


# If you ever want to remove teamviewer and i386 packages this script installed
# on you x86_64 system, run:
#   $ sudo apt-get purge teamviewer
#   $ sudo apt-get purge `dpkg --get-selections | grep i386 | awk '{print $1}'
#   $ sudo dpkg --remove-architecture i386
#
# BE CAREFUL. This will remove ALL i386 packages listed before removing them.
# If you don't recognize a package (i.e. didn't manually name and install it)
# it should be safe to purge it. 
