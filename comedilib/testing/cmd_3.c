
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

static int do_continuous(int multiplier);

#define BUFSZ 10000

int test_cmd_continuous(void)
{
	int mult;

	if(!(comedi_get_subdevice_flags(device,subdevice)&SDF_CMD)){
		printf("not applicable\n");
		return 0;
	}

	/* as if doing _one_ infinite loop wasn't slow enough,
	 * we loop through with higher and higher multipliers,
	 * in case the test fails because of latency problems */

	for(mult=1;mult<1024;mult*=2){
		do_continuous(mult);
	}

	return 0;
}

static int do_continuous(int multiplier)
{
	comedi_cmd cmd;
	char buf[BUFSZ];
	unsigned int chanlist[1];
	int go;
	int total=0;
	int ret;
	int chunks=0;
	unsigned long total_secs = 0;
	struct timeval tv,start_tv;

	if(comedi_get_cmd_generic_timed(device,subdevice, &cmd, 1, 1)<0){
		printf("  not supported\n");
		return 0;
	}

	if(realtime)cmd.flags |= TRIG_RT;

	cmd.chanlist = chanlist;
	cmd.scan_end_arg = 1;
	cmd.stop_src = TRIG_NONE;
	cmd.stop_arg = 0;
	cmd.chanlist_len = 1;
	chanlist[0] = CR_PACK(0,0,0);

	// slow down a bit
	cmd.scan_begin_arg *= multiplier;
	printf("multiplier=%d, scan_begin_arg=%d\n",
		multiplier,
		cmd.scan_begin_arg);

	ret=comedi_command(device,&cmd);
	if(ret<0){
		perror("comedi_command");
	}else{
		printf("ret==%d\n",ret);
	}

	gettimeofday(&start_tv,NULL);

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

			gettimeofday(&tv,NULL);
			tv.tv_sec-=start_tv.tv_sec;
			tv.tv_usec-=start_tv.tv_usec;
			if(tv.tv_usec<0){
				tv.tv_usec+=1000000;
				tv.tv_sec--;
			}
			if(tv.tv_sec>total_secs){
				double t;

				t=tv.tv_sec+1e-6*tv.tv_usec;
				printf("%0.3f %d (%g) %d (%g)\n",
					t,
					chunks,chunks/t,
					total,total/t);
				total_secs++;
			}
		}
	}
	{
		double t;

		t=tv.tv_sec+1e-6*tv.tv_usec;
		printf("end: %0.3f %d (%g) %d (%g)\n",
			t,
			chunks,chunks/t,
			total,total/t);
	}

	return 0;
}

