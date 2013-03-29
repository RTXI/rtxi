
#   A little input demo

use Comedi qw( :Functions :Constants );
use Comedi::Lib;

use constant AI_DEV => 0;
use constant AI_SUB => 0;
use constant AO_DEV => 0;
use constant AO_SUB => 1;
use constant N_SAMPLES => 100;

$fn = '/dev/comedi0';

# create a channel list
#
@ch = ( CR_PACK(0, 0, AREF_GROUND));

$d = comedi_open($fn) || die "Can't open '$fn': " . comedi_error();

$freq = $opt_f || 1000;

# convert the requested frequency into a timer value
#
($ret = comedi_get_timer($d, AI_SUB, $freq, $ticks, $actual_freq)) == 0 ||
    die "Can't get timer: " . comedi_strerror($ret);

$buf = pack('d*', 100..(100+N_SAMPLES-1));

($ret =
 comedi_timed_1chan($d, AI_SUB, 0, 0, AREF_GROUND, 1000, N_SAMPLES, $buf)) == 0 ||
    die "Analog input error ($ret): " . comedi_strerror(comedi_errno()) ;

foreach (unpack('d*', $buf)) {
    printf("%g\n", $_);
}


