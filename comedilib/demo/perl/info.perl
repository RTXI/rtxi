#!/usr/bin/perl

use Comedi qw( :SubdeviceTypes );
use Comedi::Lib;

sub help {
    print STDERR "info </dev/comediN>\n";
    exit(0);
}

%subdevice_types = (
    COMEDI_SUBD_UNUSED,  'unused',
    COMEDI_SUBD_AI,      'analog input',
    COMEDI_SUBD_AO,      'analog output',
    COMEDI_SUBD_DI,      'digital input',
    COMEDI_SUBD_DO,      'digital output',
    COMEDI_SUBD_DIO,     'digital I/O',
    COMEDI_SUBD_COUNTER, 'counter',
    COMEDI_SUBD_TIMER,   'timer',
    COMEDI_SUBD_MEMORY,  'memory',
    COMEDI_SUBD_CALIB,   'calibration',
    COMEDI_SUBD_PROC,    'processor',
);

#
# Main Program
#

help() if (@ARGV != 1);

$it = comedi_open($ARGV[0]) || die "cannot open $ARGV[0]: $!";

printf("overall info:\n");
printf("  version code: %d.%d.%d (0x%06x)\n", comedi_get_version($it), comedi_get_version_code($it));
printf("  driver name: %s\n", comedi_get_driver_name($it));
printf("  board name: %s\n", comedi_get_board_name($it));
printf("  number of subdevices: %d\n", $n_subdevices = comedi_get_n_subdevices($it));

for ($i = 0; $i < $n_subdevices; $i++) {
    printf("subdevice %d:\n", $i);
    $type = comedi_get_subdevice_type($it, $i);
    printf("  type: %d (%s)\n", $type, $subdevice_types{$type});
    printf("  number of channels: %d\n", comedi_get_n_channels($it, $i));
    printf("  max data value: %d\n", comedi_get_maxdata($it, $i, 0));
}
