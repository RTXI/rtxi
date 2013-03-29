# Copyright (c) 1999 Joseph E. Smith <jes@presto.med.upenn.edu>
#
# See the 'COPYRIGHT' section below for complete copyright information.

package Comedi::Trigger;

use strict;
use vars qw($VERSION $AUTOLOAD @ISA @EXPORT @EXPORT_OK);

use Carp;

require Exporter;
require AutoLoader;

@ISA = qw(Exporter AutoLoader);

# Nothing exported by default; all access through Trigger object

@EXPORT;

$VERSION = '0.01';


# Preloaded methods go here.

sub new {
    my $class = shift;
    my $self = bless {};

    while (@_) {
	my ($member, $value) = (shift, shift);

	$self->$member($value);
    }

    return $self->pack;
}

sub pack {
    my $self = shift;
    my $struct = $self->_pack;

    $self->{'struct'} = $struct;

    return $struct ? $self : undef;
}

sub _pack {
    my $self = shift;

    my $chl = $self->{'chanlist'} || return undef;

    my $n_chan = (exists($self->{'n_chan'}) ?
	$self->{'n_chan'} : @$chl) || return undef;

    # keep these buffers in the object so they don't go away
    #
    $self->{'_chanlist'} = pack('L*', @{$self->{'chanlist'}});

    my ($n, $buf);

    # n can be zero
    #
    if (exists($self->{'n'}) && exists($self->{'data'})) {
	$n = $self->{'n'};
	$buf = $self->{'data'};
    }
    elsif (exists($self->{'n'})) {
	$n = $self->{'n'};
	$self->{'_data'} = "\0" x ($n * $n_chan * 2);
	$buf = $self->{'data'} = \$self->{'_data'};
    }
    elsif (exists($self->{'data'})) {
	$buf = $self->{'data'};
	$n = length($$buf)/(2*$n_chan);
    }
    else {
	return undef;
    }

    # TODO: more error-checking (return undef)
    #
    # minimal sanity checks
    #
    croak("Data buffer is not a scalar reference")
	unless ref($buf) eq 'SCALAR';
    croak("Sample count (2*$n) too large for buffer (" .
	  length($$buf) . ")")
	unless ((2*$n) <= length $$buf);

    return pack('L4 p2 L5 L3',
		 $self->{'subdev'} || 0,
		 $self->{'mode'} || 0,
		 $self->{'flags'} || 0,
		 $n_chan,
		 $self->{'_chanlist'},
		 $$buf,
		 $n,
		 $self->{'trigsrc'} || $self->{'source'},
		 $self->{'trigvar'} || $self->{'major'},
		 $self->{'trigvar1'} || $self->{'minor'},
		 length $$buf,
		 0, 0, 0);
}


sub AUTOLOAD {
    my $self = shift;
    ref($self) || croak "$self is not an object";

    (my $name = $AUTOLOAD) =~ s/.*://;  # strip leading class name(s)

    return @_ ? $self->{$name} = shift : $self->{$name};
}

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is the stub of documentation for your module. You better edit it!

=head1 NAME

Comedi::Trigger - Object oriented interface to comedi trigger struct

=head1 SYNOPSIS

  use Comedi::Trigger;

  $t = new Comedi::Trigger;

  $t->mode(2);
  $t->chanlist([ CR_PACK(0,0,AREF_GROUND), CR_PACK(1,0,AREF_GROUND) ]);
  $t->n(100);
  $t->major(1999);
  $t->minor(20);

  $ret = comedi_trigger($dev, $t);

=head1 DESCRIPTION

This module provides an object oriented interface to B<comedi> trigger
structures.  It provides some convenience over C<pack>ing one by hand:
you need only specify a minimum of information, the rest will be
calculated if possible.

=head1 METHODS

Each element of the B<comedi> trigger structure is available as a
method which either retrieves or sets the value of that element.  In
addition, some of the elements have aliases which may be used to make
the programmer's intention clear.

The following methods function analogously to the C struct elements:

=over 4

=item subdev

=item mode

=item flags

=item n_chan

=item n

=back

These elements function analogously, but have aliases:

=over 4

=item trigsrc or source

=item trigvar or major

=item trigvar1 or minor

=back

These elements function somewhat differently than the C implementation:

=over 4

=item chanlist

This should be a reference to a Perl list of channel descriptors
packed using B<Comedi::CR_PACK> (see the example above).

If the number of channels (B<n_chan>)is not specified, it will be set
to the number of elements in this list.

=item data

This should be a scalar, pre-allocated to a length large enough to
hold the requested data.  If no value is set, a scalar will be created
of the appropriate size.

=item data_len

This element should be set to the size of the data buffer (in bytes).
If not specified, it will be set automatically.

=back

This method is specific to the Perl module.

=item pack

This method packs the current state of the trigger elements into a
binary form compatible with the C C<comedi_trig_struct>.  This method
is normally called automatically by B<Comedi::Lib> functions that take
trigger arguments.

=head1 VERSION

Version 0.01 09-Nov-1999

=head1 AUTHOR

Joe Smith <F<jes@presto.med.upenn.edu>>.

=head1 COPYRIGHT

Copyright (c) 1999 Joseph E. Smith.  All rights reserved.  This
program is free software.  You may redistribute it and/or modify it
under the same terms as Perl itself.

=head1 SEE ALSO

Comedi(1), Comedi::Lib(1).

=cut
