
#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <math.h>
#include <sys/time.h>
#include <string.h>
#include "comedi_test.h"


static const char * const subdevice_types[]={
	"unused",
	"analog input",
	"analog output",
	"digital input",
	"digital output",
	"digital I/O",
	"counter",
	"timer",
	"memory",
	"calibration",
	"processor",
	"serial",
	"pwm"
};


int test_info(void)
{
	int j;
	int type;
	const char *type_str;
	int chan,n_chans;
	int n_ranges;
	comedi_range *rng;

	printf("rev 1\n");

	type = comedi_get_subdevice_type(device,subdevice);
	if(type < (int)(sizeof(subdevice_types) / sizeof(subdevice_types[0]))) {
		type_str = subdevice_types[type];
	}else{
		type_str = "UNKNOWN";
	}
	printf("I: subdevice type: %d (%s)\n",type,type_str);
	if(type==COMEDI_SUBD_UNUSED)
		return 0;
	n_chans=comedi_get_n_channels(device,subdevice);
	printf("  number of channels: %d\n",n_chans);
	if(!comedi_maxdata_is_chan_specific(device,subdevice)){
		printf("  max data value: %d\n",comedi_get_maxdata(device,subdevice,0));
	}else{
		printf("  max data value: (channel specific)\n");
		for(chan=0;chan<n_chans;chan++){
			printf("    chan%d: %d\n",chan,
				comedi_get_maxdata(device,subdevice,chan));
		}
	}
	printf("  ranges:\n");
	if(!comedi_range_is_chan_specific(device,subdevice)){
		n_ranges=comedi_get_n_ranges(device,subdevice,0);
		printf("    all chans:");
		for(j=0;j<n_ranges;j++){
			rng=comedi_get_range(device,subdevice,0,j);
			printf(" [%g,%g]",rng->min,rng->max);
		}
		printf("\n");
	}else{
		for(chan=0;chan<n_chans;chan++){
			n_ranges=comedi_get_n_ranges(device,subdevice,chan);
			printf("    chan%d:",chan);
			for(j=0;j<n_ranges;j++){
				rng=comedi_get_range(device,subdevice,chan,j);
				printf(" [%g,%g]",rng->min,rng->max);
			}
			printf("\n");
		}
	}

	return 0;
}

