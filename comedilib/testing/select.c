
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


#define BUFSZ 10000

int test_read_select(void)
{
	comedi_cmd cmd;
	char buf[BUFSZ];
	unsigned int chanlist[1];
	int go;
	int total=0;
	int ret;
	int chunks=0;
	int length=100000;
	fd_set rdset;
	struct timeval timeout;
	unsigned int flags;

	flags = comedi_get_subdevice_flags(device,subdevice);
	if(!(flags&SDF_CMD) || (comedi_get_read_subdevice(device)!=subdevice)){
		printf("not applicable\n");
		return 0;
	}

	if(comedi_get_cmd_generic_timed(device, subdevice, &cmd, 1, 1)<0){
		printf("E: comedi_get_cmd_generic_timed failed\n");
		return 0;
	}

	if(realtime)cmd.flags |= TRIG_RT;

	cmd.chanlist = chanlist;
	cmd.scan_end_arg = 1;
	cmd.stop_arg = length;
	cmd.chanlist_len = 1;
	chanlist[0] = CR_PACK(0,0,0);

	//fcntl(comedi_fileno(device),F_SETFL,O_NONBLOCK);

	comedi_command(device,&cmd);

	go=1;
	while(go){
		FD_ZERO(&rdset);
		FD_SET(comedi_fileno(device),&rdset);
		timeout.tv_sec = 0;
		timeout.tv_usec = 10000;
		ret = select(comedi_fileno(device)+1,&rdset,NULL,NULL,&timeout);
		if(ret<0){
			perror("select");
		}else if(ret==0){
			if(verbose)printf("timeout\n");
		}else{
			ret = read(comedi_fileno(device),buf,BUFSZ);
			if(verbose)printf("read==%d\n",ret);
			if(ret<0){
				if(errno==EAGAIN){
					printf("E: got EAGAIN!\n");
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
	}

	return 0;
}

