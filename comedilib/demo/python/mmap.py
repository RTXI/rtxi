#!/usr/bin/env python
## A test-application to demonstrate using the Comedilib API
##  and streaming acquisition in particular, from Python. 

## It emulates the program "mmap" which is distributed with
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
import sys, os, string, struct, time, mmap, array
sys.path.append('./build/lib.linux-i586-2.1')

import comedi as c

#open a comedi device
dev=c.comedi_open('/dev/comedi0')
if not dev: raise "Error openning Comedi device"

#get a file-descriptor for use later
fd = c.comedi_fileno(dev)
if fd<=0: raise "Error obtaining Comedi device file descriptor"

#BUFSZ = 10000
freq=1000 # as defined in demo/common.c
subdevice=0 #as defined in demo/common.c
nscans=8000 #specify total number of scans
secs = 10 # used to stop scan after "secs" seconds

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

size = c.comedi_get_buffer_size(dev, subdevice)
print "buffer size is ", size
map = mmap.mmap(fd, size, mmap.MAP_SHARED, mmap.PROT_READ)
print "map = ", map

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

def prepare_cmd(dev, subdev, C):
    #global cmd

    C.subdev = subdev
    C.flags = 0
    C.start_src = c.TRIG_NOW
    C.start_arg = 0
    C.scan_begin_src = c.TRIG_TIMER
    C.scan_begin_arg = 1e9/freq
    C.convert_src = c.TRIG_TIMER
    C.convert_arg = 1
    C.scan_end_src = c.TRIG_COUNT
    C.scan_end_arg = nchans
    C.stop_src = c.TRIG_NONE
    #C.stop_src = c.TRIG_COUNT
    C.stop_arg = 0
    #C.stop_arg = 1000
    C.chanlist = mylist
    C.chanlist_len = nchans

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



cmd.chanlist = mylist # adjust for our particular context
cmd.chanlist_len = nchans
cmd.scan_end_arg = nchans

prepare_cmd(dev,subdevice,cmd)

print "command before testing"
dump_cmd(cmd)

#test our comedi command a few times. 
ret = c.comedi_command_test(dev,cmd)
print "first cmd test returns ", ret, cmdtest_messages[ret]
if ret<0:
	raise "comedi_command_test failed"
dump_cmd(cmd)

ret = c.comedi_command_test(dev,cmd)
print "second test returns ", ret, cmdtest_messages[ret]
if ret<0:
	raise "comedi_command_test failed"
if ret !=0:
	dump_cmd(cmd)
	raise "ERROR preparing command"
dump_cmd(cmd)

ret = c.comedi_command(dev,cmd)
if ret<0:
    raise "error executing comedi_command"

front = 0
back = 0

of = open("stream_log.bin","wb")

format = "H"

flag = 1

time_limit = nchans*freq*2*secs # stop scan after "secs" seconds
t0 = time.time()

while flag:
	front += c.comedi_get_buffer_contents(dev,subdevice)
## 	print "front = ", front
	if front > time_limit:
		flag = 0
		t1 = time.time() # reached "secs" seconds
	if (front<back):
		print "front<back"
		print "ERROR comedi_get_buffer_contents"
		break
	if (front==back):
		time.sleep(.01)
		continue
	DATA = array.array("H") # reset array to empty
## 	nbytes = front - back
## 	chunk_limit = (nbytes / size) * size # count for "size" sized chunks
## 	remainder = nbytes%size # bytes left after chunk_limit chunks done
## 	for i in range(0,chunk_limit,size):
## 		DATA.fromstring(map.read(size)) # read chunks
## 	for i in range(0,remainder,2):
## 		DATA.fromstring(map.read(2)) # read remaining bytes
	map.seek(back%size)
	for i in range(back,front,2):
		DATA.fromstring(map.read(2))
	DATA.tofile(of) # append data to log file
## 	time.sleep(.01)
	ret = c.comedi_mark_buffer_read(dev, subdevice, front-back)
	if ret<0:
		raise "error comedi_mark_buffer_read"
	back = front
print "bytes read = ", front
c.comedi_close(dev)
if ret<0:
	raise "ERROR executing comedi_close"
of.flush()
of.close()

print "Elapsed time = %d seconds" % (t1-t0)


