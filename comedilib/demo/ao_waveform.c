/*
 * Asynchronous Analog Output Example
 * Part of Comedilib
 *
 * Copyright (c) 1999,2000 David A. Schleef <ds@schleef.org>
 *
 * This file may be freely modified, distributed, and combined with
 * other software, as long as proper attribution is given in the
 * source code.
 */

/*
 * Requirements: Analog output device capable of
 *    asynchronous commands.
 *
 * This demo uses an analog output subdevice with an
 * asynchronous command to generate a waveform.  The
 * demo hijacks for -n option to select a waveform from
 * a predefined list.  The default waveform is a sine
 * wave (surprise!).  Other waveforms include sawtooth,
 * square, triangle and cycloid.
 *
 * The function generation algorithm is the same as
 * what is typically used in digital function generators.
 * A 32-bit accumulator is incremented by a phase factor,
 * which is the amount (in radians) that the generator
 * advances each time step.  The accumulator is then
 * shifted right by 20 bits, to get a 12 bit offset into
 * a lookup table.  The value in the lookup table at
 * that offset is then put into a buffer for output to
 * the DAC.
 *
 * [ Actually, the accumulator is only 26 bits, for some
 * reason.  I'll fix this sometime. ]
 *
 */

#include <stdio.h>
#include <comedilib.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include "examples.h"


/* frequency of the sine wave to output */
double waveform_frequency	= 10.0;

/* peak-to-peak amplitude, in DAC units (i.e., 0-4095) */
double amplitude		= 4000;

/* offset, in DAC units */
double offset			= 2048;

/* This is the size of chunks we deal with when creating and
   outputting data.  This *could* be 1, but that would be
   inefficient */
#define BUF_LEN 0x8000

int external_trigger_number = 0;

sampl_t data[BUF_LEN];

void dds_output(sampl_t *buf,int n);
void dds_init(double waveform_frequency, double update_frequency, int fn);

void dds_init_sine(void);
void dds_init_pseudocycloid(void);
void dds_init_cycloid(void);
void dds_init_ramp_up(void);
void dds_init_ramp_down(void);
void dds_init_triangle(void);
void dds_init_square(void);
void dds_init_blancmange(void);

static void (* const dds_init_function[])(void) = {
	dds_init_sine,
	dds_init_ramp_up,
	dds_init_ramp_down,
	dds_init_triangle,
	dds_init_square,
	dds_init_cycloid,
	dds_init_blancmange,
};

#define NUMFUNCS	(sizeof(dds_init_function)/sizeof(dds_init_function[0]))

int main(int argc, char *argv[])
{
	comedi_cmd cmd;
	int err;
	int n,m;
	int total=0;
	comedi_t *dev;
	unsigned int chanlist[16];
	unsigned int maxdata;
	comedi_range *rng;
	int ret;
	struct parsed_options options;
	int fn;

	init_parsed_options(&options);
	options.subdevice = -1;
	options.n_chan = 0;	/* default waveform */
	parse_options(&options, argc, argv);

	/* Use n_chan to select waveform (cheat!) */
	fn = options.n_chan;
	if(fn < 0 || fn >= NUMFUNCS){
		fn = 0;
	}

	/* Force n_chan to be 1 */
	options.n_chan = 1;

	if(options.value){
		waveform_frequency = options.value;
	}

	dev = comedi_open(options.filename);
	if(dev == NULL){
		fprintf(stderr, "error opening %s\n", options.filename);
		return -1;
	}
	if(options.subdevice < 0)
		options.subdevice = comedi_find_subdevice_by_type(dev, COMEDI_SUBD_AO, 0);

	maxdata = comedi_get_maxdata(dev, options.subdevice, options.channel);
	rng = comedi_get_range(dev, options.subdevice, options.channel, options.range);

	offset = (double)comedi_from_phys(0.0, rng, maxdata);
	amplitude = (double)comedi_from_phys(1.0, rng, maxdata) - offset;

	memset(&cmd,0,sizeof(cmd));
	cmd.subdev = options.subdevice;
	cmd.flags = CMDF_WRITE;
	cmd.start_src = TRIG_INT;
	cmd.start_arg = 0;
	cmd.scan_begin_src = TRIG_TIMER;
	cmd.scan_begin_arg = 1e9 / options.freq;
	cmd.convert_src = TRIG_NOW;
	cmd.convert_arg = 0;
	cmd.scan_end_src = TRIG_COUNT;
	cmd.scan_end_arg = options.n_chan;
	cmd.stop_src = TRIG_NONE;
	cmd.stop_arg = 0;

	cmd.chanlist = chanlist;
	cmd.chanlist_len = options.n_chan;

	chanlist[0] = CR_PACK(options.channel, options.range, options.aref);
	//chanlist[1] = CR_PACK(options.channel + 1, options.range, options.aref);

	dds_init(waveform_frequency, options.freq, fn);

	dump_cmd(stdout,&cmd);

	err = comedi_command_test(dev, &cmd);
	if (err < 0) {
		comedi_perror("comedi_command_test");
		exit(1);
	}

	err = comedi_command_test(dev, &cmd);
	if (err < 0) {
		comedi_perror("comedi_command_test");
		exit(1);
	}

	if ((err = comedi_command(dev, &cmd)) < 0) {
		comedi_perror("comedi_command");
		exit(1);
	}

	dds_output(data,BUF_LEN);
	n = BUF_LEN * sizeof(sampl_t);
	m = write(comedi_fileno(dev), (void *)data, n);
	if(m < 0){
		perror("write");
		exit(1);
	}else if(m < n)
	{
		fprintf(stderr, "failed to preload output buffer with %i bytes, is it too small?\n"
			"See the --write-buffer option of comedi_config\n", n);
		exit(1);
	}
	printf("m=%d\n",m);

	ret = comedi_internal_trigger(dev, options.subdevice, 0);
	if(ret < 0){
		perror("comedi_internal_trigger\n");
		exit(1);
	}

	while(1){
		dds_output(data,BUF_LEN);
		n=BUF_LEN*sizeof(sampl_t);
		while(n>0){
			m=write(comedi_fileno(dev),(void *)data+(BUF_LEN*sizeof(sampl_t)-n),n);
			if(m<0){
				perror("write");
				exit(0);
			}
			printf("m=%d\n",m);
			n-=m;
		}
		total+=BUF_LEN;
		//printf("%d\n",total);
	}

	return 0;
}



#define WAVEFORM_SHIFT 16
#define WAVEFORM_LEN (1<<WAVEFORM_SHIFT)
#define WAVEFORM_MASK (WAVEFORM_LEN-1)


sampl_t waveform[WAVEFORM_LEN];

unsigned int acc;
unsigned int adder;

void dds_init(double waveform_frequency, double update_frequency, int fn)
{
	adder = waveform_frequency / update_frequency * (1 << 16) * (1 << WAVEFORM_SHIFT);

	(*dds_init_function[fn])();
}

void dds_output(sampl_t *buf,int n)
{
	int i;
	sampl_t *p=buf;

	for(i=0;i<n;i++){
		*p=waveform[(acc>>16)&WAVEFORM_MASK];
		p++;
		acc+=adder;
	}
}

/* Defined for x in [0,1] */
static inline double triangle(double x)
{
	return (x > 0.5) ? 1.0 - x : x;
}

void dds_init_sine(void)
{
	int i;
	double ofs = offset;
	double amp = 0.5 * amplitude;

	if(ofs < amp){
		/* Probably a unipolar range.  Bump up the offset. */
		ofs = amp;
	}
	for(i=0;i<WAVEFORM_LEN;i++){
		waveform[i]=rint(ofs+amp*cos(i*2*M_PI/WAVEFORM_LEN));
	}
}

/* Yes, I know this is not the proper equation for a
   cycloid.  Fix it. */
void dds_init_pseudocycloid(void)
{
	int i;
	double t;

	for(i=0;i<WAVEFORM_LEN/2;i++){
		t=2*((double)i)/WAVEFORM_LEN;
		waveform[i]=rint(offset+amplitude*sqrt(1-4*t*t));
	}
	for(i=WAVEFORM_LEN/2;i<WAVEFORM_LEN;i++){
		t=2*(1-((double)i)/WAVEFORM_LEN);
		waveform[i]=rint(offset+amplitude*sqrt(1-t*t));
	}
}

void dds_init_cycloid(void)
{
	enum { SUBSCALE = 2 };	/* Needs to be >= 2. */
	int h, i, ni;
	double t, x, y;

	i = -1;
	for (h = 0; h < WAVEFORM_LEN * SUBSCALE; h++){
		t = (h * (2 * M_PI)) / (WAVEFORM_LEN * SUBSCALE);
		x = t - sin(t);
		ni = (int)floor((x * WAVEFORM_LEN) / (2 * M_PI));
		if (ni > i) {
			i = ni;
			y = 1 - cos(t);
			waveform[i] = rint(offset + (amplitude * y / 2));
		}
	}
}

void dds_init_ramp_up(void)
{
	int i;

	for(i=0;i<WAVEFORM_LEN;i++){
		waveform[i]=rint(offset+amplitude*((double)i)/WAVEFORM_LEN);
	}
}

void dds_init_ramp_down(void)
{
	int i;

	for(i=0;i<WAVEFORM_LEN;i++){
		waveform[i]=rint(offset+amplitude*((double)(WAVEFORM_LEN-1-i))/WAVEFORM_LEN);
	}
}

void dds_init_triangle(void)
{
	int i;

	for (i = 0; i < WAVEFORM_LEN; i++) {
		waveform[i] = rint(offset + amplitude * 2 * triangle((double)i / WAVEFORM_LEN));
	}
}

void dds_init_square(void)
{
	int i;

	for (i = 0; i < WAVEFORM_LEN / 2; i++) {
		waveform[i] = rint(offset);
	}
	for ( ; i < WAVEFORM_LEN; i++) {
		waveform[i] = rint(offset + amplitude);
	}
}

void dds_init_blancmange(void)
{
	int i, n;
	double b, x;

	for (i = 0; i < WAVEFORM_LEN; i++) {
		b = 0;
		for (n = 0; n < 16; n++) {
			x = (double)i / WAVEFORM_LEN;
			x *= (1 << n);
			x -= floor(x);
			b += triangle(x) / (1 << n);
		}
		waveform[i] = rint(offset + amplitude * 1.5 * b);
	}
}
