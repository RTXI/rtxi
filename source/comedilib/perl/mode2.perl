
#   A little input demo

use Comedi qw( :Functions :Constants :Ioctls ) ;
use Comedi::Trigger;
use Comedi::Lib;

use constant AI_DEV => 0;
use constant AI_SUB => 0;
use constant AO_DEV => 0;
use constant AO_SUB => 1;

use constant N_SAMPLES => 100;

$fn = '/dev/comedi0';

# create a channel list
#
@ch = ( CR_PACK(0, 0, AREF_GROUND),
	CR_PACK(1, 0, AREF_GROUND) );

$d = comedi_open($fn) || die "Can't open '$fn': " . comedi_error();

$fd = comedi_fileno($d);

open(COMDEV, "<&$fd") || die "Can't get filehandle from fd $fd: $!";

$freq = $opt_f || 1000;

comedi_loglevel(4);

# convert the requested frequency into a timer value
#
($ret = comedi_get_timer($d, AI_SUB, $freq, $ticks, $actual_freq)) == 0 ||
    die "Can't get timer: " . comedi_strerror($ret);

$buf = pack('S*', 100..(100+N_SAMPLES-1));

# create a trigger object
#
$it = new Comedi::Trigger(
   mode     => 2,
   chanlist => \@ch,
   n        => 20,
   data     => \$buf,
   major    => $ticks,
   minor    => 20);


die "Analog input trigger error ($ret): " . comedi_strerror(comedi_errno())
    if (($ret = comedi_trigger($d, $it)) < 0);

($ret = sysread(COMDEV, $buf, 2*N_SAMPLES)) || die "Read error ($ret): $!";
    
foreach (unpack('s*', $buf)) {
    printf("%d\n", $_);
}
