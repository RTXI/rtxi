#!/usr/bin/perl -w
# Copyright (c) 1999 Joseph E. Smith <jes@presto.med.upenn.edu>
#
# See the 'COPYRIGHT' section below for complete copyright information.

package Comedi;

use strict;
use Carp;
use vars qw($VERSION @ISA @EXPORT @EXPORT_OK %EXPORT_TAGS $AUTOLOAD);

require Exporter;
require DynaLoader;
require AutoLoader;

@ISA = qw(Exporter DynaLoader);

%EXPORT_TAGS = (

 Constants => [ qw(
	AREF_COMMON
	AREF_DIFF
	AREF_GROUND
	AREF_OTHER
	COMEDI_INPUT
	COMEDI_MAJOR
	COMEDI_NAMELEN
	COMEDI_NDEVCONFOPTS
	COMEDI_NDEVICES
	COMEDI_OUTPUT
 ) ],

 Ioctls => [ qw(
	COMEDI_CANCEL
	COMEDI_CHANINFO
	COMEDI_DEVCONFIG
	COMEDI_DEVINFO
	COMEDI_LOCK
	COMEDI_RANGEINFO
	COMEDI_SUBDINFO
	COMEDI_TRIG
	COMEDI_UNLOCK
 ) ],

 SubdeviceTypes => [ qw(
	COMEDI_SUBD_AI
	COMEDI_SUBD_AO
	COMEDI_SUBD_CALIB
	COMEDI_SUBD_COUNTER
	COMEDI_SUBD_DI
	COMEDI_SUBD_DIO
	COMEDI_SUBD_DO
	COMEDI_SUBD_MEMORY
	COMEDI_SUBD_PROC
	COMEDI_SUBD_TIMER
	COMEDI_SUBD_UNUSED
 ) ],

 SubdeviceFlags => [ qw(
	SDF_BUSY
	SDF_BUSY_OWNER
	SDF_COMMON
	SDF_DEGLITCH
	SDF_DIFF
	SDF_DITHER
	SDF_FLAGS
	SDF_GROUND
	SDF_INTERNAL
	SDF_LOCKED
	SDF_LOCK_OWNER
	SDF_LSAMPL
	SDF_MAXDATA
	SDF_MMAP
	SDF_MODE0
	SDF_MODE1
	SDF_MODE2
	SDF_MODE3
	SDF_MODE4
	SDF_OTHER
	SDF_RANGETYPE
	SDF_READABLE
	SDF_RT
	SDF_RUNNING
	SDF_WRITEABLE
 ) ],

 TriggerFlags => [ qw (
	TRIG_BOGUS
	TRIG_CONFIG
	TRIG_DEGLITCH
	TRIG_DITHER
	TRIG_RT
	TRIG_WAKE_EOS
	TRIG_WRITE
 ) ],

 Units => [ qw (
	UNIT_mA
	UNIT_none
	UNIT_volt
 ) ],

 Functions => [ qw(
        CR_PACK
 ) ],
);



# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.
@EXPORT = ();

@EXPORT_OK = (
    @{$EXPORT_TAGS{'Functions'}},
    @{$EXPORT_TAGS{'Constants'}},
    @{$EXPORT_TAGS{'Ioctls'}},
    @{$EXPORT_TAGS{'SubdeviceTypes'}},
    @{$EXPORT_TAGS{'SubdeviceFlags'}},
    @{$EXPORT_TAGS{'TriggerFlags'}},
    @{$EXPORT_TAGS{'Units'}},
);

$VERSION = '0.02';

sub AUTOLOAD {
    # This AUTOLOAD is used to 'autoload' constants from the constant()
    # XS function.  If a constant is not found then control is passed
    # to the AUTOLOAD in AutoLoader.

    my $constname;
    ($constname = $AUTOLOAD) =~ s/.*:://;
    my $val = constant($constname, @_ ? $_[0] : 0);
    if ($! != 0) {
	if ($! =~ /Invalid/) {
	    $AutoLoader::AUTOLOAD = $AUTOLOAD;
	    goto &AutoLoader::AUTOLOAD;
	}
	else {
		croak "Your vendor has not defined Comedi macro $constname";
	}
    }
    eval "sub $AUTOLOAD { $val }";
    goto &$AUTOLOAD;
}

bootstrap Comedi $VERSION;

# Preloaded methods go here.

sub CR_PACK {
    my ($chan, $rng, $aref) = @_;

    return ( ((($aref)&0x3)<<24) | ((($rng)&0xff)<<16) | (($chan)&0xffff) );
}


# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__

=head1 NAME

Comedi - Perl extension for data acquisition using comedi

=head1 SYNOPSIS

use Comedi qw( :Functions :Constants :Ioctls :SubdeviceTypes :SubdeviceFlags :TriggerFlags :Units );

=head1 DESCRIPTION

The B<Comedi> module provides constants and data types for using the comedi data acquisition module.

By itself, this module will allow you to access the B<comedi> module using standard I/O functions and ioctls.
For more robust access to B<comedi>, see the B<Comedi::Lib> module.

=head1 Exported constants

The large number of constants defined in C<comedi.h> are divided into
several functional categories for export.  No constants are exported
by default.

=item Constants

AREF_COMMON
AREF_DIFF
AREF_GROUND
AREF_OTHER
COMEDI_INPUT
COMEDI_MAJOR
COMEDI_NAMELEN
COMEDI_NDEVCONFOPTS
COMEDI_NDEVICES
COMEDI_OUTPUT

=item Ioctls

COMEDI_CANCEL
COMEDI_CHANINFO
COMEDI_DEVCONFIG
COMEDI_DEVINFO
COMEDI_LOCK
COMEDI_RANGEINFO
COMEDI_SUBDINFO
COMEDI_TRIG
COMEDI_UNLOCK

=item SubdeviceTypes

COMEDI_SUBD_AI
COMEDI_SUBD_AO
COMEDI_SUBD_CALIB
COMEDI_SUBD_COUNTER
COMEDI_SUBD_DI
COMEDI_SUBD_DIO
COMEDI_SUBD_DO
COMEDI_SUBD_MEMORY
COMEDI_SUBD_PROC
COMEDI_SUBD_TIMER
COMEDI_SUBD_UNUSED

=item  SubdeviceFlags

SDF_BUSY
SDF_BUSY_OWNER
SDF_COMMON
SDF_DEGLITCH
SDF_DIFF
SDF_DITHER
SDF_FLAGS
SDF_GROUND
SDF_INTERNAL
SDF_LOCKED
SDF_LOCK_OWNER
SDF_LSAMPL
SDF_MAXDATA
SDF_MMAP
SDF_MODE0
SDF_MODE1
SDF_MODE2
SDF_MODE3
SDF_MODE4
SDF_OTHER
SDF_RANGETYPE
SDF_READABLE
SDF_RT
SDF_RUNNING
SDF_WRITEABLE

=item TriggerFlags

TRIG_BOGUS
TRIG_CONFIG
TRIG_DEGLITCH
TRIG_DITHER
TRIG_RT
TRIG_WAKE_EOS
TRIG_WRITE

=item Units

UNIT_mA
UNIT_none
UNIT_volt

=item Functions

CR_PACK

=head1 VERSION

Version 0.01 09-Nov-1999

=head1 AUTHOR

Joe Smith <F<jes@presto.med.upenn.edu>>.

=head1 COPYRIGHT

Copyright (c) 1999 Joseph E. Smith.  All rights reserved.  This
program is free software.  You may redistribute it and/or modify it
under the same terms as Perl itself.

=head1 SEE ALSO

Comedi::Trigger(1), Comedi::Lib(1).

=cut
