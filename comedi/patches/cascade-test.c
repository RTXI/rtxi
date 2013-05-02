#include <comedi.h>
#include <math.h>
#include <stdio.h>
#include "../comedi/drivers/8253.h"

#if NOT
/* this function is meant to replace i8253_cascade_ns_to_timer_2div
 * (which is completely broken at the moment)
 */
static inline void cascade_new(int i8253_osc_base, unsigned int *d1, unsigned int *d2, unsigned int *nanosec, int round_mode)
{
	unsigned int divider;
	unsigned int div1, div2;
	unsigned int div1_glb, div2_glb, ns_glb;
	unsigned int div1_lub, div2_lub, ns_lub;
	unsigned int ns;
	unsigned int start;
	unsigned int ns_low, ns_high;

	divider = *nanosec / i8253_osc_base;

	div1_lub = div2_lub = 0;
	div1_glb = div2_glb = 0;

	ns_glb = 0;
	ns_lub = 0xffffffff;

	div2 = 0x10000;
	start = divider / div2;
	if(start < 2) start = 2;
	for (div1 = start; div1 <= divider / div1 + 1; div1++) {
		for(div2 = divider / div1; div1 * div2 <= divider + div1 + 1; div2++) {
			ns = i8253_osc_base * div1 * div2;
			if (ns <= *nanosec && ns > ns_glb) {
				ns_glb = ns;
				div1_glb = div1;
				div2_glb = div2;
			}
			if (div2 <= 0x10000) {
				ns = i8253_osc_base * div1 * div2;
				if (ns >= *nanosec && ns < ns_lub) {
					ns_lub = ns;
					div1_lub = div1;
					div2_lub = div2;
				}
			}
		}
	}

	switch (round_mode) {
	case TRIG_ROUND_NEAREST:
	default:
		ns_high = div1_lub * div2_lub * i8253_osc_base;
		ns_low = div1_glb * div2_glb * i8253_osc_base;
		if( ns_high - *nanosec < *nanosec - ns_low) {
			div1 = div1_lub;
			div2 = div2_lub;
		} else {
			div1 = div1_glb;
			div2 = div2_glb;
		}
		break;
	case TRIG_ROUND_UP:
		div1 = div1_lub;
		div2 = div2_lub;
		break;
	case TRIG_ROUND_DOWN:
		div1 = div1_glb;
		div2 = div2_glb;
		break;
	}

	*nanosec = div1 * div2 * i8253_osc_base;
	*d1 = div1;
	*d2 = div2;
	return;
}
#endif

#ifdef ROGI
int main(int argc,vhar *argv[])
{
	int osc_base = 1000;
	int round_mode = TRIG_ROUND_NEAREST;
	int i;
	int div1_old, div2_old, ns_old, err_old;
	int div1_new, div2_new, ns_new, err_new;
	int div1_pow, div2_pow, ns_pow, err_pow;
	int err = 0;

	/* loop over desired nanosecond timings to test */
	for(i = 10000; i < 100000; i++)
	{
		ns_old = ns_new = ns_pow = i;

		i8253_cascade_ns_to_timer_2div(osc_base, &div1_old, &div2_old, &ns_old, round_mode);
		err_old = ns_old - i;

		i8253_cascade_ns_to_timer_power(osc_base, &div1_pow, &div2_pow, &ns_pow, round_mode);
		err_pow = ns_pow - i;

		cascade_new(osc_base, &div1_new, &div2_new, &ns_new, round_mode);
		err_new = ns_new - i;

		/* print results on this condition */
		if(abs(err_new) > abs(err_pow))
			printf("nanosec %i\terr_new %i\tdiv1_new %i\tdiv2_new %i\n"
				"\t\terr_pow %i\tdiv1_pow %i\tdiv2_pow %i\n",
				i, err_new, div1_new, div2_new, err_pow, div1_pow, div2_pow);

	/* consistency checks */
		if(div1_old * div2_old * osc_base != ns_old) err = 1;
		if(div1_pow * div2_pow * osc_base != ns_pow) err = 2;
		if(div1_new * div2_new * osc_base != ns_new) err = 3;
		if(err)
		{
			printf("err %i\n", err);
			err=0;
		}
	}
	return 0;
}
#endif

int main(int argc,char *argv[])
{
	int osc_base = 1000;
	unsigned int ns;
	unsigned int div1,div2;
	int i=0;

	for(ns = 4*osc_base; ns<0x80000000;ns++){
		i8253_cascade_ns_to_timer_2div
			(osc_base, &div1, &div2, &ns, TRIG_ROUND_UP);

		i++;
		if(!(i&0xff))
			printf("ns=%d div1=%d div2=%d\n",ns,div1,div2);
		if(i==100000)exit(0);
	}

	return 0;
}

