/*
 * LED Clock demo
 * Part of Comedilib
 *
 * Copyright (c) 2001 David A. Schleef <ds@schleef.org>
 *
 * This file may be freely modified, distributed, and combined with
 * other software, as long as proper attribution is given in the
 * source code.
 */

/*
 * Requirements:
 *    - A board with a digital output subdevice and a subdevice that
 *      can trigger on an external digital line.  A parallel port
 *      satisfies these requirements.
 *    - A Fantazein LED Clock modified so that the individual LEDs
 *      can be controlled directly by the digital I/O lines.
 *
 * The Fantazein clock has 8 LEDs arranged in a row on a wand that
 * sweeps back and forth at about 15 Hz.  Unmodified, the firmware
 * of the clock lights the LEDs at the appropriate time to print
 * words and the time of day.  Since the wand moves quickly, it is
 * barely visible, so it looks like the image floats in the air.
 * Stuart Hughes modified a clock so that the LEDs could be controlled
 * directly by the parallel port of a computer, and wrote the
 * appropriate software using RTAI to create a stable image.  This
 * is an attempt to port the demo to Comedi.
 *
 * It needs much work.
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

int count;

int out_subd;

#define BUFSZ 1024
sampl_t buf[BUFSZ];

unsigned int chanlist[16];


void prepare_cmd(comedi_t *dev,comedi_cmd *cmd, int subdevice);
void do_cmd(comedi_t *dev,comedi_cmd *cmd);
void do_toggle(void);


void config_output(void)
{
	int i;

	for(i=0;i<8;i++){
		comedi_dio_config(device,out_subd,i,COMEDI_OUTPUT);
	}
}

void do_toggle(void)
{
#if 1
	comedi_insnlist il;
	comedi_insn insn[3];
	lsampl_t data[6];
	int mask = 0xff;

	count++;

	il.n_insns = 3;
	il.insns = insn;

	memset(insn,0,3*sizeof(comedi_insn));

	insn[0].insn = INSN_BITS;
	insn[0].n = 2;
	insn[0].data = data+0;
	insn[0].subdev = out_subd;

	data[0] = mask;
	//data[1] = count;
	data[1] = 0xfc;

	insn[1].insn = INSN_WAIT;
	insn[1].n = 1;
	insn[1].data = data+2;

	data[2] = 100000-1;

	insn[2].insn = INSN_BITS;
	insn[2].n = 2;
	insn[2].data = data+4;
	insn[2].subdev = out_subd;

	data[4] = mask;
	//data[5] = count;
	data[5] = 0xff;

	comedi_do_insnlist(device,&il);
#else
	unsigned int data;
	unsigned int mask = 0xff;

	count++;
	data = count;

	comedi_dio_bitfield(device,out_subd,mask,&data);
#endif
}

int main(int argc, char *argv[])
{
	int ret;
	comedi_cmd cmd;
	struct parsed_options options;

	init_parsed_options(&options);
	parse_options(&options, argc, argv);

	device = comedi_open(options.filename);
	if(!device){
		perror(options.filename);
		exit(1);
	}

	out_subd = 0;

	config_output();

	ret = fcntl(comedi_fileno(device),F_SETFL,O_NONBLOCK|O_ASYNC);
	if(ret<0)perror("fcntl");

#if 0
	{
	struct sched_param p;

	memset(&p,0,sizeof(p));
	p.sched_priority = 1;
	ret = sched_setscheduler(0,SCHED_FIFO,&p);
	if(ret<0)perror("sched_setscheduler");
	}
#endif

	prepare_cmd(device, &cmd, options.subdevice);

	do_cmd(device,&cmd);

	return 0;
}

void do_cmd(comedi_t *dev,comedi_cmd *cmd)
{
	int total=0;
	int ret;
	int go;
	fd_set rdset;
	struct timeval timeout;

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
		FD_ZERO(&rdset);
		FD_SET(comedi_fileno(dev),&rdset);
		timeout.tv_sec = 0;
		timeout.tv_usec = 50000;
		ret = select(comedi_fileno(dev)+1,&rdset,NULL,NULL,&timeout);
		if(ret<0){
			perror("select");
		}else if(ret==0){
			/* timeout */
		}else if(FD_ISSET(comedi_fileno(dev),&rdset)){
			ret=read(comedi_fileno(dev),buf,BUFSZ);
			if(ret<0){
				if(errno==EAGAIN){
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
				do_toggle();
#if 0
				for(i=0;i<ret;i+=sizeof(sampl_t)){
					do_toggle();
				}
#endif
			}
		}
	}
}

/*
 * This part of the demo measures channels 1, 2, 3, 4 at a rate of
 * 10 khz, with the inter-sample time at 10 us (100 khz).  The number
 * of scans measured is 10.  This is analogous to the old mode2
 * acquisition.
 */
void prepare_cmd(comedi_t *dev, comedi_cmd *cmd, int subdevice)
{
	memset(cmd,0,sizeof(*cmd));

	/* the subdevice that the command is sent to */
	cmd->subdev = subdevice;

	/* flags */
	cmd->flags =		TRIG_WAKE_EOS;

	cmd->start_src =	TRIG_NOW;
	cmd->start_arg =	0;

	cmd->scan_begin_src =	TRIG_EXT;
	cmd->scan_begin_arg =	0;

#if 0
	cmd->convert_src =	TRIG_TIMER;
	cmd->convert_arg =	1;
#else
	cmd->convert_src =	TRIG_ANY;
	cmd->convert_arg =	0;
#endif

	cmd->scan_end_src =	TRIG_COUNT;
	cmd->scan_end_arg =	1;

	cmd->stop_src =		TRIG_NONE;
	cmd->stop_arg =		0;

	cmd->chanlist =		chanlist;
	cmd->chanlist_len =	1;

	chanlist[0]=CR_PACK(0,0,0);
}

