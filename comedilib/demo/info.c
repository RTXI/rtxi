/*
   This demo reads information about a comedi device and
   displays the information in a human-readable form.
 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "examples.h"

void get_command_stuff(comedi_t *it,int s);

void help(void)
{
	fprintf(stderr,"info </dev/comediN>\n");
	exit(0);
}

char *tobinary(char *s,int bits,int n);

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
	"serial digital I/O",
	"pwm"
};

comedi_t *it;
extern char *filename;


int main(int argc,char *argv[])
{
	int i,j;
	int n_subdevices,type;
	const char *type_str;
	int chan,n_chans;
	int n_ranges;
	int subdev_flags;
	comedi_range *rng;
	struct parsed_options options;

	init_parsed_options(&options);
	parse_options(&options, argc, argv);

	it = comedi_open(options.filename);
	if(!it){
		comedi_perror(options.filename);
		exit(1);
	}

	printf("overall info:\n");
	printf("  version code: 0x%06x\n", comedi_get_version_code(it));
	printf("  driver name: %s\n", comedi_get_driver_name(it));
	printf("  board name: %s\n", comedi_get_board_name(it));
	printf("  number of subdevices: %d\n", n_subdevices = comedi_get_n_subdevices(it));

	for(i = 0; i < n_subdevices; i++){
		printf("subdevice %d:\n",i);
		type = comedi_get_subdevice_type(it, i);
		if(type < (int)(sizeof(subdevice_types) / sizeof(subdevice_types[0]))){
			type_str = subdevice_types[type];
		}else{
			type_str = "UNKNOWN";
		}
		printf("  type: %d (%s)\n",type,type_str);
		if(type==COMEDI_SUBD_UNUSED)
			continue;
		subdev_flags = comedi_get_subdevice_flags(it, i);
		printf("  flags: 0x%08x\n",subdev_flags);
		n_chans=comedi_get_n_channels(it,i);
		printf("  number of channels: %d\n",n_chans);
		if(!comedi_maxdata_is_chan_specific(it,i)){
			printf("  max data value: %lu\n", (unsigned long)comedi_get_maxdata(it,i,0));
		}else{
			printf("  max data value: (channel specific)\n");
			for(chan=0;chan<n_chans;chan++){
				printf("    chan%d: %lu\n",chan,
					(unsigned long)comedi_get_maxdata(it,i,chan));
			}
		}
		printf("  ranges:\n");
		if(!comedi_range_is_chan_specific(it,i)){
			n_ranges=comedi_get_n_ranges(it,i,0);
			printf("    all chans:");
			for(j=0;j<n_ranges;j++){
				rng=comedi_get_range(it,i,0,j);
				printf(" [%g,%g]",rng->min,rng->max);
			}
			printf("\n");
		}else{
			for(chan=0;chan<n_chans;chan++){
				n_ranges=comedi_get_n_ranges(it,i,chan);
				printf("    chan%d:",chan);
				for(j=0;j<n_ranges;j++){
					rng=comedi_get_range(it,i,chan,j);
					printf(" [%g,%g]",rng->min,rng->max);
				}
				printf("\n");
			}
		}
		printf("  command:\n");
		get_command_stuff(it,i);
	}

	return 0;
}

char *tobinary(char *s,int bits,int n)
{
	int bit=1<<n;
	char *t=s;

	for(;bit;bit>>=1)
		*t++=(bits&bit)?'1':'0';
	*t=0;

	return s;
}

void probe_max_1chan(comedi_t *it,int s);

void get_command_stuff(comedi_t *it,int s)
{
	comedi_cmd cmd;
	char buf[100];

	if(comedi_get_cmd_src_mask(it,s,&cmd)<0){
		printf("    not supported\n");
	}else{
		printf("    start: %s\n",cmd_src(cmd.start_src,buf));
		printf("    scan_begin: %s\n",cmd_src(cmd.scan_begin_src,buf));
		printf("    convert: %s\n",cmd_src(cmd.convert_src,buf));
		printf("    scan_end: %s\n",cmd_src(cmd.scan_end_src,buf));
		printf("    stop: %s\n",cmd_src(cmd.stop_src,buf));

		probe_max_1chan(it,s);
	}
}

void probe_max_1chan(comedi_t *it,int s)
{
	comedi_cmd cmd;
	char buf[100];

	printf("  command fast 1chan:\n");
	if(comedi_get_cmd_generic_timed(it, s, &cmd, 1, 1)<0){
		printf("    not supported\n");
	}else{
		printf("    start: %s %d\n",
			cmd_src(cmd.start_src,buf),cmd.start_arg);
		printf("    scan_begin: %s %d\n",
			cmd_src(cmd.scan_begin_src,buf),cmd.scan_begin_arg);
		printf("    convert: %s %d\n",
			cmd_src(cmd.convert_src,buf),cmd.convert_arg);
		printf("    scan_end: %s %d\n",
			cmd_src(cmd.scan_end_src,buf),cmd.scan_end_arg);
		printf("    stop: %s %d\n",
			cmd_src(cmd.stop_src,buf),cmd.stop_arg);
	}
}

