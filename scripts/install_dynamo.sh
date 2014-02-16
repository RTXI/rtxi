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

# Install dynamo
echo "Installing DYNAMO utility..."

sudo yum install mlton
cd ../dynamo
mllex dl.lex
mlyacc dl.grm
mlton dynamo.mlb
sudo cp dynamo /usr/bin/

if [ $? -eq 0 ]; then
	echo "----->DYNAMO translation utility installed."
else
	echo "----->DYNAMO translation utility installation failed."
	exit
fi
