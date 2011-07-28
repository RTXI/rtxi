/*
 * poll.c - Example of using comedi_poll() with Comedi
 * Part of Comedilib
 *
 * Copyright (c) 1999,2000 David A. Schleef <ds@schleef.org>
 *
 * This file may be freely modified, distributed, and combined with
 * other software, as long as proper attribution is given in the
 * source code.
 */

/*
 * An example for using comedi_poll() in asynchronous input,
 * so you can ask the driver to pull samples that may be waiting
 * on the board into the buffer (so they can be read).
 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>
#include <sys/time.h>
#include "examples.h"

#define N_SCANS		10
#define N_CHANS		16

#define BUFSZ 1000
sampl_t buf[BUFSZ];

const int n_chans = 1;
const int n_scans = 10;

unsigned int chanlist[4];

comedi_t *device;

void prepare_cmd(comedi_t *dev, comedi_cmd *cmd, int subdevice);
void do_cmd(comedi_t *dev,comedi_cmd *cmd);

int main(int argc, char *argv[])
{
	comedi_cmd cmd;
	int i;
	struct parsed_options options;

	init_parsed_options(&options);
	parse_options(&options, argc, argv);

	device = comedi_open(options.filename);
	if(!device){
		perror(options.filename);
		exit(1);
	}

	fcntl(comedi_fileno(device), F_SETFL, O_NONBLOCK);

	for(i = 0; i < n_chans; i++){
		chanlist[i] = CR_PACK(options.channel + i, options.range, options.aref);
	}

	prepare_cmd(device, &cmd, options.subdevice);

	do_cmd(device, &cmd);

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
		FD_SET(comedi_fileno(device),&rdset);
		timeout.tv_sec = 0;
		timeout.tv_usec = 50000;
		ret = select(comedi_fileno(dev)+1,&rdset,NULL,NULL,&timeout);
		//printf("select returned %d\n",ret);
		if(ret<0){
			perror("select");
		}else if(ret==0){
			/* hit timeout */
			printf("timeout, polling...\n");
			ret = comedi_poll(device,0);
			printf("poll returned %d\n",ret);
		}else if(FD_ISSET(comedi_fileno(device),&rdset)){
			/* comedi file descriptor became ready */
			//printf("comedi file descriptor ready\n");
			ret=read(comedi_fileno(dev),buf,sizeof(buf));
			printf("read returned %d\n",ret);
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
				//printf("read %d %d\n",ret,total);
				for(i=0;i<ret/sizeof(sampl_t);i++){
					printf("%d\n",buf[i]);
				}
			}
		}else{
			/* unknown file descriptor became ready */
			printf("unknown file descriptor ready\n");
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
	cmd->subdev =	subdevice;

	/* flags */
	cmd->flags =	0;

	/* each event requires a trigger, which is specified
	   by a source and an argument.  For example, to specify
	   an external digital line 3 as a source, you would use
	   src=TRIG_EXT and arg=3. */

	/* In this case, we specify using TRIG_NOW to start
	 * acquisition immediately when the command is issued.
	 * The argument of TRIG_NOW is "number of nsec after
	 * NOW", but no driver supports it yet.  Also, no driver
	 * currently supports using a start_src other than
	 * TRIG_NOW.  */
	cmd->start_src =		TRIG_NOW;
	cmd->start_arg =		0;

	/* The timing of the beginning of each scan is controlled
	 * by scan_begin.  TRIG_TIMER specifies that scan_start
	 * events occur periodically at a rate of scan_begin_arg
	 * nanoseconds between scans. */
	cmd->scan_begin_src =	TRIG_TIMER;
	cmd->scan_begin_arg =	msec_to_nsec(100);

	/* The timing between each sample in a scan is controlled
	 * by convert.  Like above, TRIG_TIMER specifies that
	 * convert events occur periodically at a rate of convert_arg
	 * nanoseconds between scans. */
	cmd->convert_src =	TRIG_TIMER;
	cmd->convert_arg =	msec_to_nsec(1);

	/* The end of each scan is almost always specified using
	 * TRIG_COUNT, with the argument being the same as the
	 * number of channels in the chanlist.  You could probably
	 * find a device that allows something else, but it would
	 * be strange. */
	cmd->scan_end_src =	TRIG_COUNT;
	cmd->scan_end_arg =	n_chans;	/* number of channels */

	/* The end of acquisition is controlled by stop_src and
	 * stop_arg.  The src will typically be TRIG_COUNT or
	 * TRIG_NONE.  Specifying TRIG_COUNT will stop acquisition
	 * after stop_arg number of scans, or TRIG_NONE will
	 * cause acquisition to continue until stopped using
	 * comedi_cancel(). */
	cmd->stop_src =		TRIG_COUNT;
	cmd->stop_arg =		n_scans;

	/* the channel list determined which channels are sampled.
	   In general, chanlist_len is the same as scan_end_arg.  Most
	   boards require this.  */
	cmd->chanlist =		chanlist;
	cmd->chanlist_len =	n_chans;
}

