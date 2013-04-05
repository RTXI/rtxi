#!/usr/bin/env python
## It emulates the program "cmd" which is distributed with
## the comedilib software
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
import sys, os, string, struct, time
sys.path.append('./build/lib.linux-i586-2.1')

import comedi as c

#open a comedi device
dev=c.comedi_open('/dev/comedi0')
if not dev: raise "Error openning Comedi device"

#get a file-descriptor for use later
fd = c.comedi_fileno(dev)
if fd<=0: raise "Error obtaining Comedi device file descriptor"

BUFSZ = 10000
freq=1000 # as defined in demo/common.c
subdevice=0 #as defined in demo/common.c
nscans=8000 #specify total number of scans

#three lists containing the chans, gains and referencing
#the lists must all have the same length
chans=[0,1,2,3]
gains=[0,0,0,0]
aref =[c.AREF_GROUND, c.AREF_GROUND, c.AREF_GROUND, c.AREF_GROUND]

cmdtest_messages = [
	"success",
	"invalid source",
	"source conflict",
	"invalid argument",
	"argument conflict",
	"invalid chanlist"]

nchans = len(chans) #number of channels

#wrappers include a "chanlist" object (just an Unsigned Int array) for holding the chanlist information
mylist = c.chanlist(nchans) #create a chanlist of length nchans

#now pack the channel, gain and reference information into the chanlist object
#N.B. the CR_PACK and other comedi macros are now python functions
for index in range(nchans):
	mylist[index]=c.cr_pack(chans[index], gains[index], aref[index])

def dump_cmd(cmd):
	print "---------------------------"
	print "command structure contains:"
	print "cmd.subdev : ", cmd.subdev
	print "cmd.flags : ", cmd.flags
	print "cmd.start :\t", cmd.start_src, "\t", cmd.start_arg
	print "cmd.scan_beg :\t", cmd.scan_begin_src, "\t", cmd.scan_begin_arg
	print "cmd.convert :\t", cmd.convert_src, "\t", cmd.convert_arg
	print "cmd.scan_end :\t", cmd.scan_end_src, "\t", cmd.scan_end_arg
	print "cmd.stop :\t", cmd.stop_src, "\t", cmd.stop_arg
	print "cmd.chanlist : ", cmd.chanlist
	print "cmd.chanlist_len : ", cmd.chanlist_len
	print "cmd.data : ", cmd.data
	print "cmd.data_len : ", cmd.data_len
	print "---------------------------"

## ret = c.comedi_get_buffer_size(dev, subdevice)
## if ret==-1:
## 	raise "Error fetching comedi buffer size"
## else:
## 	print "buffer size = ", ret
## ret = c.comedi_get_max_buffer_size(dev, subdevice)
## if ret==-1:
## 	raise "Error fetching comedi max buff size"
## else:
## 	print "max buff size = ", ret
#construct a comedi command
cmd = c.comedi_cmd_struct()

ret = c.comedi_get_cmd_generic_timed(dev,subdevice,cmd,1.0e9/freq)
if ret: raise "Error comedi_get_cmd_generic failed"
	
cmd.chanlist = mylist # adjust for our particular context
cmd.chanlist_len = nchans
cmd.scan_end_arg = nchans
if cmd.stop_src==c.TRIG_COUNT: cmd.stop_arg=nscans

print "command before testing"
dump_cmd(cmd)

#test our comedi command a few times. 
ret = c.comedi_command_test(dev,cmd)
print "first cmd test returns ", ret, cmdtest_messages[ret]
if ret<0: raise "comedi_command_test failed"
dump_cmd(cmd)
ret = c.comedi_command_test(dev,cmd)
print "second test returns ", ret, cmdtest_messages[ret]
if ret<0: raise "comedi_command_test failed"
if ret !=0:
	dump_cmd(cmd)
	raise "Error preparing command"

#execute the command!
## ret = c.comedi_command(dev,cmd)
## if ret !=0: raise "comedi_command failed..."

datastr = ()
t0 = time.time()
ret = c.comedi_command(dev,cmd)
if ret !=0: raise "comedi_command failed..."
while (1):
	#ret = c.comedi_poll(dev,subdevice)
	#print "poll ret = ", ret
	data = os.read(fd,BUFSZ)
	#print "len(data) = ", len(data)
	if len(data)==0:
		break
	n = len(data)/2 # 2 bytes per 'H'
	format = `n`+'H'
	#print "format = ", format
	#bytes = struct.calcsize(format)
	#print "bytes = ", bytes
	#nbytes = c.comedi_get_buffer_contents(dev,subdevice)
	#print "n = ", n, " nbytes = ", nbytes
	datastr = datastr + struct.unpack(format,data)

t1 = time.time()
print "start time : ", t0
print "end time : ", t1
print "time : ", t1 - t0, " seconds"

count = 0
while count < len(datastr):
	for i in range(4):
		print "%d\t" % datastr[count+i],
	print "\n"
	count = count + 4
	
ret = c.comedi_close(dev)
if ret !=0: raise "comedi_close failed..."
