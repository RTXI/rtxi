def parse_options

    $filename = "/dev/comedi0"
    $value = 0
    $subdevice = 0
    $channel = 0
    $range = 0
    $aref = AREF_GROUND
    $n_chan = 4
    $n_scan = 1000
    $freq = 1000.0
    $verbose = false

    opts = GetoptLong.new(
	[ "--filename",		"-f",	GetoptLong::REQUIRED_ARGUMENT ],
	[ "--subdevice",	"-s",	GetoptLong::REQUIRED_ARGUMENT ],
	[ "--channel",		"-c",	GetoptLong::REQUIRED_ARGUMENT ],
	[ "--aref",		"-a",	GetoptLong::REQUIRED_ARGUMENT ],
	[ "--range",		"-r",	GetoptLong::REQUIRED_ARGUMENT ],
	[ "--n_chan",		"-n",	GetoptLong::REQUIRED_ARGUMENT ],
	[ "--n_scan",		"-N",	GetoptLong::REQUIRED_ARGUMENT ],
	[ "--freq",		"-F",	GetoptLong::REQUIRED_ARGUMENT ],
	[ "--verbose",		"-v",	GetoptLong::NO_ARGUMENT ],
	[ "--diff",		"-d",	GetoptLong::NO_ARGUMENT ],
	[ "--ground",		"-g",	GetoptLong::NO_ARGUMENT ],
	[ "--other",		"-o",	GetoptLong::NO_ARGUMENT ],
	[ "--common",		"-m",	GetoptLong::NO_ARGUMENT ]
	)

    opts.each do |opt, arg|
	case opt
	    when "--filename"
		$filename = arg.to_s
	    when "--subdevice"
		$subdevice = arg.to_i
	    when "--channel"
		$channel = arg.to_i
	    when "--range"
		$range = arg.to_i
	    when "--n_chan"
		$n_chan = arg.to_i
	    when "--n_scan"
		$n_scan = arg.to_i
	    when "--freq"
		$freq = arg.to_i
	    when "--verbose"
		$verbose = true
	    when "--diff"
		$aref = AREF_DIFF
	    when "--ground"
		$aref = AREF_GROUND
	    when "--other"
		$aref = AREF_OTHER
	    when "--common"
		$aref = AREF_COMMON
	    when "--help"
		usage
		exit 0
	end
    end

    if ARGV.length > 0
	$value = ARGV.shift.to_i
    end
end

def cmd_src(src)

    buf = ""
    buf << "none|" if src & TRIG_NONE > 0
    buf << "now|" if src & TRIG_NOW > 0
    buf << "follow|" if src & TRIG_FOLLOW > 0
    buf << "time|" if src & TRIG_TIME > 0
    buf << "timer|" if src & TRIG_TIMER > 0
    buf << "count|" if src & TRIG_COUNT > 0
    buf << "ext|" if src & TRIG_EXT > 0
    buf << "int|" if src & TRIG_INT > 0
    buf << "other|" if src & TRIG_OTHER > 0

    if buf == ""
	buf = "unknown(0x%08x)" % src
    else
	buf.sub!(/\|\z/, '')
    end

    return buf
end

def dump_cmd(out, cmd)

    out.printf("start:      %-8s %d\n",
	cmd_src(cmd.start_src),
	cmd.start_arg)

    out.printf("scan_begin: %-8s %d\n",
	cmd_src(cmd.scan_begin_src),
	cmd.scan_begin_arg)

    out.printf("convert:    %-8s %d\n",
	cmd_src(cmd.convert_src),
	cmd.convert_arg)

    out.printf("scan_end:   %-8s %d\n",
	cmd_src(cmd.scan_end_src),
	cmd.scan_end_arg)

    out.printf("stop:       %-8s %d\n",
	cmd_src(cmd.stop_src),
	cmd.stop_arg)
end
