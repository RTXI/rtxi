#A test-application to demonstrate using the Comedilib API
# and streaming acquisition in particular, from Python. 
#This script is completely untested!
# author bryan.cole@teraview.co.uk

#set the paths so python can find the comedi module
import sys, os, string
sys.path.append('./build/lib.linux-i686-2.2')

import comedi as c

#open a comedi device
dev=c.comedi_open('/dev/comedi0')
if not dev: raise "Error openning Comedi device"

#get a file-descriptor for use later
fd = c.comedi_fileno(dev)

nscans=1000 #specify total number of scans

#three lists containing the chans, gains and referencing
#the lists must all have the same length
chans=[0,2,3]
gains=[1,1,1]
aref =[c.AREF_GROUND, c.AREF_GROUND, c.AREF_GROUND]

nchans = len(chans) #number of channels

#wrappers include a "chanlist" object (just an Unsigned Int array) for holding the chanlist information
mylist = c.chanlist(nchans) #create a chanlist of length nchans

#now pack the channel, gain and reference information into the chanlist object
#N.B. the CR_PACK and other comedi macros are now python functions
for index in range(nchans):
	mylist[index]=c.cr_pack(chans[index], gains[index], aref[index])

#construct a comedi command manually
cmd = c.comedi_cmd_struct()
cmd.subdev = 0
cmd.flags = 0
cmd.start_src = c.TRIG_NOW
cmd.sart_arg = 0
cmd.scan_begin_src = c.TRIG_TIMER
cmd.scan_begin_arg = int(1.0e9/100000)
cmd.convert_src = c.TRIG_TIMER
cmd.convert_arg = 1
cmd.scan_end_src = c.TRIG_COUNT
cmd.scan_end_arg = nchans
cmd.stop_src = c.TRIG_COUNT
cmd.stop_arg = nscans
cmd.chanlist = mylist
cmd.chanlist_len = nchans

#test our comedi command a few times. 
ret = c.comedi_command_test(dev,cmd)
print "first cmd test returns ", ret
ret = c.comedi_command_test(dev,cmd)
print "second test returns ", ret
ret = c.comedi_command_test(dev,cmd)
if not ret: raise "Error testing comedi command"

#execute the command!
ret = c.comedi_command(dev,cmd)

#I think this will work but it's completely untested!
datalist=[]
data=os.read(fd)
while len(data)>0:
	datalist.append(data)
	data=os.read(fd)

ret = c.comedi_close(dev)

datastr = string.join(datalist,'')

print datastr #heres your data, as a single string!

#if you've got Numeric installed you can convert data into a flat Numpy array
# with:
# dataarray = Numeric.fromstring(data, Int16)