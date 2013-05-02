
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

static int get_chunks_per_length(int length);

#define BUFSZ 10000

int test_cmd_fifo_depth_check(void)
{
	int len;
	unsigned int flags = comedi_get_subdevice_flags(device,subdevice);

	if(!(flags&SDF_CMD) || (comedi_get_read_subdevice(device)!=subdevice)){
		printf("not applicable\n");
		return 0;
	}

	for(len=64;len<65536;len<<=1){
		printf("%d, %d\n",len,get_chunks_per_length(len));
	}

	return 0;
}

static int get_chunks_per_length(int length)
{
	comedi_cmd cmd;
	char buf[BUFSZ];
	unsigned int chanlist[1];
	int go;
	int total=0;
	int ret;
	int chunks=0;

	if(comedi_get_cmd_generic_timed(device, subdevice, &cmd, 1, 1)<0){
		printf("  not supported\n");
		return 0;
	}

	if(realtime)cmd.flags |= TRIG_RT;

	cmd.chanlist = chanlist;
	cmd.scan_end_arg = 1;
	cmd.stop_arg = length;
	cmd.chanlist_len = 1;
	chanlist[0] = CR_PACK(0,0,0);

	comedi_command(device,&cmd);

	go=1;
	while(go){
		ret = read(comedi_fileno(device),buf,BUFSZ);
		if(ret<0){
			if(errno==EAGAIN){
				usleep(10000);
			}else{
				go = 0;
				perror("read");
			}
		}else if(ret==0){
			go = 0;
		}else{
			total += ret;
			chunks++;
		}
	}

	return chunks;
}

