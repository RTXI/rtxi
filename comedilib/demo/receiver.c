/*
 * Digital I/O example
 * Part of Comedilib
 *
 * Copyright (c) 1999,2000 David A. Schleef <ds@schleef.org>
 *
 * This file may be freely modified, distributed, and combined with
 * other software, as long as proper attribution is given in the
 * source code.
 */
/*
 * Requirements:  A board with a digital I/O subdevice.  Not just
 *    a 'digital input' or 'digital output' subdevice, but one in
 *    which the channels can be configured between input and output.
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

int pin_clk = 2;
int pin_data = 10;

comedi_t *device;

#define BUFSZ 1024
sampl_t buf[BUFSZ];

void prepare_cmd(comedi_t *dev, comedi_cmd *cmd, int subdevice);
void do_cmd(comedi_t *dev,comedi_cmd *cmd);

int main(int argc, char *argv[])
{
	comedi_t *dev;
	comedi_cmd cmd;
	int ret;
	struct parsed_options options;

	init_parsed_options(&options);
	options.channel = -1;
	parse_options(&options, argc, argv);

	dev = comedi_open(options.filename);
	if(!dev){
		perror(options.filename);
		exit(1);
	}
	device = dev;

	if(options.channel >= 0) pin_data = options.channel;

	ret = fcntl(comedi_fileno(dev),F_SETFL,O_NONBLOCK);
	if(ret<0)perror("fcntl");

	prepare_cmd(dev, &cmd, options.subdevice);

	do_cmd(dev,&cmd);

	return 0;
}

static int c=0;
static unsigned int bits =0;

void do_cmd(comedi_t *dev,comedi_cmd *cmd)
{
	unsigned int *chanlist;
	int n_chans;
	int total=0;
	int ret;
	int go;
	struct timeval timeout;
	fd_set rdset;

	chanlist = cmd->chanlist;
	n_chans = cmd->chanlist_len;

	ret=comedi_command_test(dev,cmd);

	//printf("test ret=%d\n",ret);
	if(ret<0){
		printf("errno=%d\n",errno);
		comedi_perror("comedi_command_test");
		return;
	}

	dump_cmd(stdout,cmd);

	cmd->chanlist =		chanlist;
	cmd->chanlist_len =	n_chans;

	ret=comedi_command_test(dev,cmd);

	printf("test ret=%d\n",ret);
	if(ret<0){
		printf("errno=%d\n",errno);
		comedi_perror("comedi_command_test");
		return;
	}

	dump_cmd(stdout,cmd);

	cmd->chanlist =		chanlist;
	cmd->chanlist_len =	n_chans;

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
		timeout.tv_sec=0;
		timeout.tv_usec=50000;
		ret = select(comedi_fileno(dev)+1,&rdset,NULL,NULL,&timeout);
		if(ret<0){
			perror("select");
		}else if(ret==0){
			if(c){
				fprintf(stderr,"\n");
				c=0;
				bits=0;
			}
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
				int i;

				total+=ret;
				for(i=0;i<ret/sizeof(sampl_t);i++){
					fprintf(stderr,"%d",buf[i]>0xa000);
					c++;
					if(c>=32){
						fprintf(stderr,"\n");
						c=0;
					}
#if 0
					//printf("%d %d\n",buf[i],buf[i]>0xa000);
					//printf("%d",buf[i]>0xa000);
					bits<<=1;
					bits|=(buf[i]>0xa000);
					c++;
					if(c>=33){
#if 0
						struct timeval now;

						gettimeofday(&now,NULL);
						printf(" %08x %ld.%06ld\n",bits,now.tv_sec,now.tv_usec);
						c=0;
						bits=0;
#else
						printf(" %08x\n",bits);
						c=0;
						bits=0;
#endif
					}
					if(!bits)c=0;
#endif
				}
				fflush(stdout);
				fflush(stderr);
			}
		}
	}
}

unsigned int chanlist[16];
/*
 * This part of the demo measures channels 1, 2, 3, 4 at a rate of
 * 10 khz, with the inter-sample time at 10 us (100 khz).  The number
 * of scans measured is 10.  This is analogous to the old mode2
 * acquisition.
 */
void prepare_cmd(comedi_t *dev,comedi_cmd *cmd, int subdevice)
{
	memset(cmd,0,sizeof(comedi_cmd));

	/* the subdevice that the command is sent to */
	cmd->subdev =		subdevice;

	/* flags */
	cmd->flags =		TRIG_WAKE_EOS;

	cmd->start_src =	TRIG_NOW;
	cmd->start_arg =	0;

	cmd->scan_begin_src =	TRIG_EXT;
	cmd->scan_begin_arg =	CR_EDGE | CR_INVERT | pin_clk;

#if 1
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

	chanlist[0]=CR_PACK(pin_data,0,0);
}

