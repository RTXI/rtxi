
structure DLOptions = 
struct

structure G = GetOpt
	      
datatype flag = 
	 Help
       | Version 
       | Output    of string 
       | EQDFG     of string 
       | MRCI      of string 
       | RTXI      of string 

fun showflag (Help)      = "Help"
  | showflag (Version)   = "Version"
  | showflag (Output s)  = concat ["Output \"", s, "\""]
  | showflag (EQDFG s)   = concat ["EQDFG \"", s, "\""]
  | showflag (MRCI s)    = concat ["MRCI \"", s, "\""]
  | showflag (RTXI s)    = concat ["RTXI \"", s, "\""]

fun out NONE = Output ""
  | out (SOME o') = Output o'
		   
fun errout NONE = Output "/dev/stderr"
  | errout (SOME o') = Output o'

fun eqdfgout NONE = EQDFG ""
  | eqdfgout (SOME o') = EQDFG o'

fun mrci NONE = MRCI ""
  | mrci (SOME o') = MRCI o'

fun rtxi NONE = RTXI ""
  | rtxi (SOME o') = RTXI o'

val options = [
	       {short="h",
                long=["help"],
                desc=G.NoArg (fn() => Help),
                help="show help"},

	       {short="",
                long=["version","release"],
                desc=G.NoArg (fn() => Version),
                help="show version information"},

               {short="o",
                long=["output"],
                desc=G.OptArg (out, "FILE"),
                help="set the prefix of output file(s)"},

	       {short="e",
		long=["error-output"],
		desc=G.OptArg (errout, "FILE"),
		help="redirect error messages to file"},

	       {short="",
		long=["eqdfg"],
		desc=G.OptArg (eqdfgout, "FILE"),
		help="output the equation DFG to file"},

	       {short="r",
		long=["mrci"],
		desc=G.OptArg (mrci, "FILE"),
		help="output MRCI model description to file"},

	       {short="x",
		long=["rtxi"],
		desc=G.OptArg (rtxi, "FILE"),
		help="output RTXI model description to file"}
	       
	       ]


fun optError (status) (msg) = (status := SOME msg)

fun getopt status = (G.getOpt {argOrder=G.Permute, errFn=optError status,
			       options=options}) 

fun header (progname) = concat ["Usage: dynamo ", "[OPTION...] files..."]

fun usage (progname) = G.usageInfo {header=(header progname), options=options}

fun getstate (opts): {help: bool, version: bool, output: string
		      option, eqdfg: bool, eqdfgout: string, 
		      mrci: bool, rtxi: bool} =

    let
	val O_HELP        = ref false
	val O_VERSION     = ref false
	val O_VERBOSE     = ref false
	val O_OUTPUT_FILE = ref NONE
	val O_EQDFG        = ref false
	val O_EQDFGOUT     = ref ""
	val O_MRCI         = ref false
	val O_RTXI         = ref false

	fun getstate' (opt) = 
	    (case opt of 
		 Help          => O_HELP := true
	       | Version       => O_VERSION := true
	       | Output s      => O_OUTPUT_FILE := SOME s
	       | EQDFG s       => (O_EQDFG  := true;
				   O_EQDFGOUT := s)
	       | MRCI s        => O_MRCI  := true
	       | RTXI s        => O_RTXI  := true)

	val _ = app getstate' opts

    in {help=(!O_HELP), version=(!O_VERSION), output=(!O_OUTPUT_FILE),
	eqdfg=(!O_EQDFG), eqdfgout=(!O_EQDFGOUT), mrci=(!O_MRCI), rtxi=(!O_RTXI)}
    end

end
