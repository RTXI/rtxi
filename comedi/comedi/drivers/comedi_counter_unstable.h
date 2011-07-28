/*
    comedi_counter_unstable.h

    This is for the development of comedi's counter API.  Any API
    described here should be considered under development and
    subject to change!  Eventually, this should get merged into
    comedi.h when consensus is reached on a good API.

    Copyright (C) 2003 Frank Mori Hess

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

************************************************************************/

/* ALPHA counter stuff */
/*

ID=INSN_CONFIG_COUNTER_ALPHA: counter configuration

  The subdevice has an accumulator which changes based on
  triggers.

  [0] - ID
  [1] - trigger0 (comedi_counter_trigger_bits() is helpful)
  [2] - action0 (action associated with trigger0, see comedi_counter_actions
  	for possibilities)
  [3] - trigger1
  [4] - action 1
  ....

  You can specify as many trigger/action pairs as you like.

  Example Modes:
    Up counter, counting falling edges on input 0

	data[1] = comedi_counter_trigger_bits(0, CR_EDGE | CR_INVERT);
	data[2] = COMEDI_INC_ACCUMULATOR;

    Down counter, counting rising edges on input 0, and using input 1 as a gate
    (input 1 must be high to count):

	data[1] = comedi_counter_trigger_bits(0, CR_EDGE) | comedi_counter_trigger_bits(1, 0);
	data[2] = COMEDI_DEC_ACCUMULATOR;

    Quadrature x1 encoding:
	data[1] = comedi_counter_trigger_bits(0, CR_EDGE) |
		comedi_counter_trigger_bits(1, CR_INVERT);
	data[2] = COMEDI_INC_ACCUMULATOR;
	data[3] = comedi_counter_trigger_bits(0, CR_EDGE | CR_INVERT) |
		comedi_counter_trigger_bits(1, CR_INVERT);
	data[4] = COMEDI_DEC_ACCUMULATOR;

  Notes:
	Could add fields for specifying what inputs are connected to (input 0 connected
	to external input pin, or internal oscillator?).

	Could make triggers/actions more involved, for example "trigger on accumulator
	reaches 0", or specify a value to reload the counter with for the
	COMEDI_RESET_ACCUMULATOR action.  Could add an action that sets the counter's
	output, like "pulse output when accumulator reaches 0".

	Using this instruction is a bit of a pain.  On the user-end, the user has
	to figure out exactly how to describe what her counter is doing in order to
	program it, even if the counter only supports a few different modes of
	operation.  This could be
	eased by providing helper functions that fill out an insn with the
	appropriate values for particular pieces of hardware.  On the driver end,
	it might make things easier if we provided some functions for operations
	on sets of trigger/action pairs.  For example, queries like "is A a subset
	of B" where A and B are sets of trigger/action pairs.
*/
#define INSN_CONFIG_COUNTER_ALPHA		0x1000

static inline int comedi_counter_trigger_bits(unsigned int input_num, int flags)
{
	static const int low_bit = 0x1;
	static const int edge_bit = 0x2;
	static const int valid_bit = 0x4;
	static const int bits_per_channel = 3;
	int bits = valid_bit;

	if (flags & CR_EDGE)
		bits |= edge_bit;
	if (flags & CR_INVERT)
		bits |= low_bit;
	return bits << (input_num * bits_per_channel);
}
enum comedi_counter_actions {
	COMEDI_INC_ACCUMULATOR,
	COMEDI_DEC_ACCUMULATOR,
	COMEDI_RESET_ACCUMULATOR,
};
