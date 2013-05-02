# Copyright (c) 1999 Joseph E. Smith <jes@presto.med.upenn.edu>
#
# See the 'COPYRIGHT' section below for complete copyright information.

package Comedi::Lib;

use strict;
use Carp;
use vars qw($VERSION @ISA @EXPORT @EXPORT_OK %EXPORT_TAGS $AUTOLOAD);

require Exporter;
require DynaLoader;
require AutoLoader;

@ISA = qw(Exporter DynaLoader);

%EXPORT_TAGS = (

 Functions => [ qw(
        comedi_open
	comedi_close
	comedi_loglevel
	comedi_error
	comedi_perror
	comedi_strerror
	comedi_errno
	comedi_fileno
	comedi_get_n_subdevices
	comedi_get_version
	comedi_get_version_code
	comedi_get_driver_name
	comedi_get_board_name
	comedi_get_subdevice_type
	comedi_find_subdevice_by_type
	comedi_get_n_channels
	comedi_get_maxdata
	comedi_get_rangetype
	comedi_get_range
	comedi_find_range
	comedi_trigger
	comedi_to_phys
	comedi_from_phys
	comedi_data_read
	comedi_data_write
	comedi_sv_init
	comedi_sv_update
	comedi_sv_measure
	comedi_get_timer
	comedi_timed_1chan
 ) ],
);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

@EXPORT = ( @{$EXPORT_TAGS{'Functions'}});

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
		croak "Your vendor has not defined Comedilib macro $constname";
	}
    }
    eval "sub $AUTOLOAD { $val }";
    goto &$AUTOLOAD;
}

bootstrap Comedi::Lib $VERSION;

# Preloaded methods go here.

sub comedi_get_version {
    my $it = shift || return ();
    my $code = comedi_get_version_code($it);

    return (($code & 0xff0000) >> 16, 
	    ($code & 0x00ff00) >> 8,
	    ($code & 0x0000ff));
}

sub comedi_error {
    my $err = shift || comedi_errno();

    return comedi_strerror($err);
}

sub comedi_trigger {
    my ($d, $t) = @_;

    if (ref($t) eq 'Comedi::Trigger') {
	_comedi_trigger($d, $t->{struct});
    }
    else {
	_comedi_trigger($d, $t);
    }
}

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is the stub of documentation for your module. You better edit it!

=head1 NAME

Comedi::Lib - Perl API for B<comedilib>

=head1 SYNOPSIS

  use Comedi::Lib;

=head1 DESCRIPTION



=head2 Core functions

=item comedi_open

  $dev = comedi_open('/dev/comedi0');

=item comedi_close

  comedi_close($dev);

=item comedi_loglevel

  $l = comedi_loglevel(4);

=item comedi_perror

  comedi_perror('Oops!');

=item comedi_strerror

  $s = comedi_strerror($errno);

=item comedi_errno

  $errno = comedi_errno();

=item comedi_error

  $s = comedi_error();          # get current error string
  $s = comedi_error($errno);    # same as comedi_strerror

=item comedi_fileno

  $fd = comedi_fileno($dev);




=head2 Query functions

=item comedi_get_n_subdevices

  $n = comedi_get_n_subdevices($dev);

=item comedi_get_version_code

  $n = comedi_get_version_code($dev);

=item comedi_get_version

  ($major, $minor, $rev) = comedi_get_version();

=item comedi_get_driver_name

  $s = comedi_get_driver_name($dev);

=item comedi_get_board_name

  $s = comedi_get_board_name($dev);

=item comedi_get_subdevice_type

  $n = comedi_get_subdevice_type($dev, $subd);

=item comedi_find_subdevice_by_type

  $n = comedi_find_subdevice_by_type($dev, $type, $subd)

=item comedi_get_n_channels

  $n = comedi_get_n_channels($dev, $subd);

=item comedi_get_maxdata

  $n = comedi_get_maxdata($dev, $subd, $channel);

=item comedi_get_rangetype

  $n = comedi_get_rangetype($dev, $subd, $channel);

=item comedi_find_range

  $n = comedi_find_range($dev, $subd, $unit, $min, $max);





=head2 Trigger and command functions

=item comedi_cancel

  $n = comedi_cancel($dev, $subd)

=item comedi_trigger

=item _comedi_trigger

  $t_obj = new Comedi::Trigger(mode => 2, n => 100, ...);
  $ret = comedi_trigger($dev, $t_obj);

  # or equivalently...
  $ret = _comedi_trigger($dev, $t_obj->struct);

=item comedi_command

  $n = comedi_command($dev, $cmd)

This function is untested.
B<$cmd> is a scalar holding a packed C<comedi_cmd> structure.




=head2 Functions dealing with physical units

=item comedi_to_phys

  $v = comedi_to_phys($sample, $range, $maxdata);

=item comedi_from_phys

  $n = comedi_from_phys($v, $range, $maxdata);



=head2 Synchronous i/o functions

=item comedi_data_read

  $n = comedi_data_read($dev, $subd, $range, $aref, $data);

=item comedi_data_write

  $n = comedi_data_write($dev, $subd, $chan, $range, $aref, $data)




=head2 Slowly-varying i/o functions

=item comedi_sv_init

  $n = comedi_sv_init($svdev, $dev, $subd, $chan);

=item comedi_sv_update

  $n = comedi_sv_update($svdev);

=item comedi_sv_measure

  $n = comedi_sv_measure($svdev, $v);




=head2 Digital i/o functions

=item comedi_dio_config

  $n = comedi_dio_config($dev, $subd, $chan, $dir)

=item comedi_dio_read

  $n = comedi_dio_read($dev, $subd, $chan, $bit)

=item comedi_dio_write

  $n = comedi_dio_write($dev, $subd, $chan, $bit)

=item comedi_dio_bitfield

  $n = comedi_dio_bitfield($dev, $subd, $mask, $bits)





=head2 Timer functions

=item comedi_get_timer

  $n = comedi_get_timer($dev, $subd, $freq, $ticks, $actual_freq);

=item comedi_timed_1chan

  $n = comedi_timed_1chan($dev, $subd, $chan, $range, $aref, $freq, $n, $vdata);





=head2 Range functions

=item comedi_get_range

  $range = comedi_get_range($dev, $subd, $channel, $range_num);





=head1 VERSION

Version 0.01 09-Nov-1999

=head1 AUTHOR

Joe Smith <F<jes@presto.med.upenn.edu>>.

=head1 COPYRIGHT

Copyright (c) 1999 Joseph E. Smith.  All rights reserved.  This
program is free software.  You may redistribute it and/or modify it
under the same terms as Perl itself.

=head1 SEE ALSO

Comedi(1), Comedi::Trigger(1).

=cut
