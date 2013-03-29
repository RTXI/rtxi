#!/usr/bin/env python
## an adaptation of the info.c program bundled with the comedilib package
## the info.c file is found in the package demo directory
## Copyright (C) May 2003  Luc Lefebvre
## luc.lefebvre@mcgill.ca
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License
## as published by the Free Software Foundation; either version 2
## of the License, or (at your option) any later version.

## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.

#set the paths so python can find the comedi module
import sys, os, string
sys.path.append('./build/lib.linux-i586-2.1')

import comedi as c

subdevice_types = {c.COMEDI_SUBD_UNUSED:"unused",
		   c.COMEDI_SUBD_AI:"analog input",
		   c.COMEDI_SUBD_AO:"analog output",
		   c.COMEDI_SUBD_DI:"digital input",
		   c.COMEDI_SUBD_DO:"digital output",
		   c.COMEDI_SUBD_DIO:"digital I/O",
		   c.COMEDI_SUBD_COUNTER:"counter",
		   c.COMEDI_SUBD_TIMER:"timer",
		   c.COMEDI_SUBD_MEMORY:"memory",
		   c.COMEDI_SUBD_CALIB:"calibration",
		   c.COMEDI_SUBD_PROC:"processor"}
#open a comedi device
dev=c.comedi_open('/dev/comedi0')
if not dev: raise "Error openning Comedi device"

version_code = c.comedi_get_version_code(dev)
if not version_code: raise "Error reading version_code"
print "version code is: ", version_code

driver_name = c.comedi_get_driver_name(dev)
if not driver_name: raise "Error reading driver_name"
print "driver name is: ", driver_name

board_name = c.comedi_get_board_name(dev)
if not board_name: raise "Error reading board_name"
print "board name is: ", board_name

n_subdevices = c.comedi_get_n_subdevices(dev)
if not n_subdevices: raise "Error reading n_subdevices"
print "number of subdevices is: ", n_subdevices

def comedi_get_cmd_fast_1chan(dev,s,cmd):
	ret = c.comedi_get_cmd_src_mask(dev,s,cmd)
	if (ret<0): return ret;
	cmd.chanlist_len = 1
	cmd.scan_end_src = c.TRIG_COUNT
	cmd.scan_end_arg = 1
	if (cmd.convert_src & c.TRIG_TIMER):
		if (cmd.scan_begin_src & c.TRIG_FOLLOW):
			cmd.convert_src=c.TRIG_TIMER
			cmd.scan_begin=c.TRIG_FOLLOW
		else:
			cmd.convert_src=c.TRIG_TIMER
			cmd.scan_begin=c.TRIG_TIMER
	else:
		print "can't do timed!?!"
		return -1
	if (cmd.stop_src & c.TRIG_COUNT):
		cmd.stop_src=c.TRIG_COUNT
		cmd.stop_arg=2
	elif (cmd.stop_src & c.TRIG_NONE):
		cmd.stop_src=c.TRING_NONE
		cmd.stop_arg=0
	else:
		print "can't find a good stop_src"
		return -1
	ret = c.comedi_command_test(dev,cmd)
	if (ret==3):
		ret = c.comedi_command_test(dev,cmd)
	if ((ret==4) or (ret==0)):
		return 0
	return -1

def probe_max_1chan(dev,s):
	buf=""
	cmd=c.comedi_cmd_struct()
	print "\tcommand fast 1chan:"
	if(c.comedi_get_cmd_generic_timed(dev,s,cmd,1)<0):
		print "\t\tnot supported"
	else:
		print "\tstart: %s %d" % (cmd_src(cmd.start_src,buf),cmd.start_arg)
		print "\tscan_begin: %s %d" % (cmd_src(cmd.scan_begin_src,buf),cmd.scan_begin_arg)
		print "\tconvert begin: %s %d" % (cmd_src(cmd.convert_src,buf),cmd.convert_arg)
		print "\tscan_end: %s %d" % (cmd_src(cmd.scan_end_src,buf),cmd.scan_end_arg)
		print "\tstop: %s %d" % (cmd_src(cmd.stop_src,buf),cmd.stop_arg)

def cmd_src(src,buf):
	buf=""
	if(src & c.TRIG_NONE): buf=buf+"none|"
	if(src & c.TRIG_NOW): buf=buf+"now|"
	if(src & c.TRIG_FOLLOW): buf=buf+"follow|"
	if(src & c.TRIG_TIME): buf=buf+"time|"
	if(src & c.TRIG_TIMER): buf=buf+"timer|"
	if(src & c.TRIG_COUNT): buf=buf+"count|"
	if(src & c.TRIG_EXT): buf=buf+"ext|"
	if(src & c.TRIG_INT): buf=buf+"int|"
	if len(buf)==0:
		print "unknown"
	else:
		buf = buf[:-1] # trim trailing "|"
		return buf

def get_command_stuff(dev,s):
	buf = ""
	cmd = c.comedi_cmd_struct()
	if (c.comedi_get_cmd_src_mask(dev,s,cmd)<0):
		print "\tnot supported"
	else:
		print "\tstart: %s" % (cmd_src(cmd.start_src,buf))
		print "\tscan_begin: %s" % (cmd_src(cmd.scan_begin_src,buf))
		print "\tconvert begin: %s" % (cmd_src(cmd.convert_src,buf))
		print "\tscan_end: %s" % (cmd_src(cmd.scan_end_src,buf))
		print "\tstop: %s" % (cmd_src(cmd.stop_src,buf))
		probe_max_1chan(dev,s)

print "-----subdevice characteristics-----"
for i in range(n_subdevices):
	print "subdevice %d:" % (i)
	type = c.comedi_get_subdevice_type(dev,i)
	print "\ttype: %d (%s)" % (type,subdevice_types[type])
	if (type == c.COMEDI_SUBD_UNUSED):
		continue
	n_chans = c.comedi_get_n_channels(dev,i)
	print "\tnumber of channels: %d" % ( n_chans)
	if not(c.comedi_maxdata_is_chan_specific(dev,i)):
	    print "\tmax data value: %d" % (c.comedi_get_maxdata(dev,i,0))
	else:
		print "max data value is channel specific"
		for j in range(n_chans):
			print "\tchan: %d: %d" % (j,c.comedi_get_maxdata(dev,i,j))
	print "\tranges: "
	if not(c.comedi_range_is_chan_specific(dev,i)):
		n_ranges = c.comedi_get_n_ranges(dev,i,0)
		print "\t\tall chans:"
		for j in range(n_ranges):
			rng = c.comedi_get_range(dev,i,0,j)
			print "\t\t[%g,%g]" % (rng.min, rng.max)
	else:
		for chan in range(n_chans):
			n_ranges = c.comedi_get_n_ranges(dev,i,chan)
			print "\t\tchan: %d" % (chan)
			for j in range(n_ranges):
				rng = c.comedi_get_range(dev,i,chan,j)
				print "\t\t[%g,%g]" % (rng.min, rng.max)
	print "\tcommand:"
	get_command_stuff(dev,i)
		



