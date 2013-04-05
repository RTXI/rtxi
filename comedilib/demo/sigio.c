/*
 * SIGIO example
 * Part of Comedilib
 *
 * Copyright (c) 1999,2000 David A. Schleef <ds@schleef.org>
 *
 * This file may be freely modified, distributed, and combined with
 * other software, as long as proper attribution is given in the
 * source code.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include "examples.h"

comedi_t *device;


void print_time(void);

void sigio_handler(int sig)
{
	print_time();
}

void print_time(void)
{
	struct timeval tv;
	static struct timeval oldtime={0};
	int dsec,dusec;

	gettimeofday(&tv,NULL);

	dsec=tv.tv_sec-oldtime.tv_sec;
	dusec=tv.tv_usec-oldtime.tv_usec;
	if(dusec<0){
		dsec--;
		dusec+=1000000;
	}
	printf("%d.%06d +%d.%06d\n",(int)tv.tv_sec,(int)tv.tv_usec,dsec,dusec);

	oldtime=tv;
}

int out_subd;

void config_output(void)
{
	int i;

	for(i=0;i<8;i++){
		comedi_dio_config(device,out_subd,i,COMEDI_OUTPUT);
	}
}

int count;

#define BUFSZ 1024
sampl_t buf[BUFSZ];

void do_cmd_1(comedi_t *dev, int subdevice);
void do_cmd_2(comedi_t *dev);
void do_cmd(comedi_t *dev,comedi_cmd *cmd);

int main(int argc, char *argv[])
{
	struct sigaction sa;
	int ret;
	sigset_t sigset;
	int flags;
	struct parsed_options options;

	init_parsed_options(&options);
	parse_options(&options, argc, argv);

	device = comedi_open(options.filename);
	if(!device){
		perror(options.filename);
		exit(1);
	}

	out_subd = 2;

	config_output();

	fcntl(comedi_fileno(device), F_SETOWN, getpid());
	flags = fcntl(comedi_fileno(device),F_GETFL);
	ret = fcntl(comedi_fileno(device),F_SETFL,flags|O_ASYNC);
	//ret = fcntl(comedi_fileno(device),F_SETFL,O_NONBLOCK|O_ASYNC);
	if(ret<0)perror("fcntl");

	memset(&sa,0,sizeof(sa));
	sa.sa_handler = &sigio_handler;
	ret = sigaction(SIGIO,&sa,NULL);
	if(ret<0)perror("sigaction");

	sigemptyset(&sigset);
	sigaddset(&sigset,SIGIO);
	ret = sigprocmask(SIG_UNBLOCK,&sigset,NULL);
	if(ret<0)perror("sigprocmask");

#if 0
	{
	struct sched_param p;
	memset(&p,0,sizeof(p));
	p.sched_priority = 1;
	ret = sched_setscheduler(0,SCHED_FIFO,&p);
	if(ret<0)perror("sched_setscheduler");
	}
#endif

	do_cmd_1(device, options.subdevice);

	return 0;
}

void do_cmd(comedi_t *dev,comedi_cmd *cmd)
{
	int total=0;
	int ret;
	int go;

	ret=comedi_command_test(dev,cmd);

	printf("test ret=%d\n",ret);
	if(ret<0){
		printf("errno=%d\n",errno);
		comedi_perror("comedi_command_test");
		return;
	}

	dump_cmd(stdout,cmd);

	ret=comedi_command_test(dev,cmd);

	printf("test ret=%d\n",ret);
	if(ret<0){
		printf("errno=%d\n",errno);
		comedi_perror("comedi_command_test");
		return;
	}

	dump_cmd(stdout,cmd);

	ret=comedi_command(dev,cmd);

	printf("ret=%d\n",ret);
	if(ret<0){
		printf("errno=%d\n",errno);
		comedi_perror("comedi_command");
		return;
	}

	go=1;
	while(go){
		ret=read(comedi_fileno(dev),buf,BUFSZ);
		if(ret<0){
			if(errno==EAGAIN){
				printf("EAGAIN\n");
				usleep(10000);
			}else{
				go = 0;
				perror("read");
			}
		}else if(ret==0){
			go = 0;
		}else{
			//int i;

			total+=ret;
			//printf("read %d %d\n",ret,total);
			//printf("count = %d\n",count);
			//print_time();
		}
	}
}

unsigned int chanlist[0];
/*
 * This part of the demo measures channels 1, 2, 3, 4 at a rate of
 * 10 khz, with the inter-sample time at 10 us (100 khz).  The number
 * of scans measured is 10.  This is analogous to the old mode2
 * acquisition.
 */
void do_cmd_1(comedi_t *dev, int subdevice)
{
	comedi_cmd cmd;

	memset(&cmd,0,sizeof(cmd));

	/* the subdevice that the command is sent to */
	cmd.subdev = subdevice;

	/* flags */
	cmd.flags =	TRIG_WAKE_EOS;

	cmd.start_src =		TRIG_NOW;
	cmd.start_arg =		0;

	cmd.scan_begin_src =	TRIG_TIMER;
	cmd.scan_begin_arg =	msec_to_nsec(100);

#if 1
	cmd.convert_src =	TRIG_TIMER;
	cmd.convert_arg =	1;
#else
	cmd.convert_src =	TRIG_ANY;
	cmd.convert_arg =	0;
#endif

	cmd.scan_end_src =	TRIG_COUNT;
	cmd.scan_end_arg =	1;

	cmd.stop_src =		TRIG_NONE;
	cmd.stop_arg =		0;

	cmd.chanlist =		chanlist;
	cmd.chanlist_len =	1;

	chanlist[0]=CR_PACK(0,0,0);

	do_cmd(dev,&cmd);
}

