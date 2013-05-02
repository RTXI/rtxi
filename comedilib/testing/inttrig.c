
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

#define BUFSZ 2000

int test_cmd_start_inttrig(void)
{
	comedi_cmd cmd;
	char buf[BUFSZ];
	unsigned int chanlist[1];
	int go;
	int total=0;
	int ret;
	unsigned int flags;

	flags = comedi_get_subdevice_flags(device,subdevice);
	if(!(flags&SDF_CMD) || (comedi_get_read_subdevice(device)!=subdevice)){
		printf("not applicable\n");
		return 0;
	}

	if(comedi_get_cmd_src_mask(device,subdevice,&cmd)<0){
		printf("  not supported\n");
		return 0;
	}

	if(!(cmd.start_src&TRIG_INT)){
		printf("  not supported\n");
		return 0;
	}

	if(comedi_get_cmd_generic_timed(device, subdevice, &cmd, 1, 1)<0){
		printf("  not supported\n");
		return 0;
	}

	if(realtime)cmd.flags |= TRIG_RT;
	cmd.chanlist = chanlist;
	cmd.start_src = TRIG_INT;
	cmd.start_arg = 0;
	cmd.scan_end_arg = 1;
	cmd.stop_arg = 100;
	cmd.chanlist_len = 1;
	chanlist[0] = CR_PACK(0,0,0);

	ret = comedi_command(device,&cmd);
	if(ret!=0){
		printf("E: comedi_command() returned %d, expecting 0\n",ret);
	}

	ret = comedi_internal_trigger(device,subdevice,0);
	if(ret<0){
		printf("E: comedi_internal_trigger(): %s\n",strerror(errno));
	}

	go=1;
	while(go){
		ret = read(comedi_fileno(device),buf,BUFSZ);
		if(ret<0){
			if(errno==EAGAIN){
				usleep(10000);
			}else{
				go = 0;
				printf("E: read: %s\n",strerror(errno));
			}
		}else if(ret==0){
			go = 0;
		}else{
			total += ret;
			if(verbose)
				printf("read %d %d\n",ret,total);
		}
	}

	return 0;
}


