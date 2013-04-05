
#   A little input demo

use constant N_SAMPLES => 64;

use Comedi qw( :Functions :Constants :Ioctls ) ;

# allocate a data buffer

# create a channel list
$ch = pack('L*', CR_PACK(0, 0, AREF_GROUND));

# create a data buffer
$buf = pack('S' . N_SAMPLES);

# create a trigger object
#
$it = pack('L4 p2 L5 L3',
	   0,
	   0,
	   0,
	   1,
	   $ch,
	   $buf,
	   N_SAMPLES,
	   0,
	   0,
	   0
	   );

$fn = '/dev/comedi0';

sysopen(FILE, $fn, O_RDWR) || die "Can't open '$fn': $!";

ioctl(FILE, COMEDI_TRIG, $it) || die "ioctl failed: $!";

foreach (unpack('S*', $buf)) {
    printf("%d\n", $_);
}


