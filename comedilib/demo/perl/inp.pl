#!/usr/bin/perl
#
# same at inp.c, with the addition of -x/--convert option which will give output
# in volts
#

use strict;
use Comedi::Lib;
use Getopt::Long;

my $dev = 0;
my $subd = 0;
my $chan = 0;
my $range = 0;
my $aref = 0;
my $convert = 0;
my $verbose = 0;
my $help = 0;
my $data = pack ('i','0');

GetOptions ('dev|d=i' => \$dev, 'subd|s=i' => \$subd, 'chan|c=i' => 
\$chan, 'range|r=i' => \$range, 'aref|a=i' => \$aref, 'convert|x' => 
\$convert, 'verbose|v' => \$verbose, 'help|h' => \$help);

if ($help == 1) {
        print "usage: inp.pl [-d dev -s sudb -c chan -r range -a aref -v 
-h -x]\n";
        exit;
}
if ($verbose ==1) {
        print "measuring device=$dev subdevice=$subd channel=$chan 
range=$range analog reference=$aref\n";
}

my $it = comedi_open("/dev/comedi$dev") || die "cannot open 
/dev/comedi$dev: $!";

comedi_data_read($it, $subd, $chan, $range, $aref, $data);
my $result = unpack('i', $data);

if ($convert == "0") {
        print "$result\n";
} else {
        my $maxdata = comedi_get_maxdata($it, $subd, $chan);
        my $rng = comedi_get_range($it, $subd, $chan, $range);
        my $v = comedi_to_phys($result, $rng, $maxdata);
        print "$v\n";
}

comedi_close($it);


