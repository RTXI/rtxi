
#   A little input demo

use Comedi::Trigger;

use Comedilib qw( :DEFAULT :Constants :Ioctls ) ;

use constant (
  AI_DEV => 0,
  AI_SUB => 0,
  AO_DEV => 0,
  AO_SUB => 1,
);

$fn = '/dev/comedi0';

# create a channel list
#
@ch = ( CR_PACK(0, 0, AREF_GROUND),
	CR_PACK(1, 0, AREF_GROUND) );

$d = comedi_open($fn) || die "Can't open '$fn': " . comedi_error();

$freq = $opt_f || 1000;

comedi_loglevel(4);

# convert the requested frequency into a timer value
#
($ret = comedi_get_timer($d, AI_SUB, $freq, $ticks, $actual_freq)) == 0 ||
    die "Can't get timer: " . comedi_strerror($ret);

$buf = pack('S*', 100..119);

# create a trigger object
#
$it = new Comedi::Trigger(
   mode => 1,
   chanlist => \@ch,
   n => 20,
   data => \$buf,
   major => $ticks);


die "Mode 1 doesn't work yet...";

($ret = comedi_trigger($d, $it)) > 0 ||
    die "Analog input error: " . comedi_strerror(comedi_errno()) ;

foreach (unpack('S*', ${$it->data})) {
    printf("%d\n", $_);
}


