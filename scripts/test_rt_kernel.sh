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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# Created by Yogi Patel <yapatel@gatech.edu> 2014.1.31
#

#!/bin/bash

echo "----->Running latency test under load. Please wait (approx 10 minutes)"
echo "----->Do not interrupt."

# Run latency test under dynamic load
stress --cpu 2 --io 1 --vm 1 --hdd 1 --timeout 600 & sudo /usr/xenomai/bin/./latency -T 600

exit 0
