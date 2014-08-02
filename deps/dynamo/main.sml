structure Main =
struct

open DLErrors

val VERSION  = "1.1.0"
val COMPILER = "Dynamo"

structure Ast      = DLAst
structure Parse    = DLParse (structure Ast=Ast)
structure Semant   = DLSemant 
structure Canon    = DLCanon
structure Deps     = DLDeps 

structure CBackend = CBackend (val compilerid=COMPILER^" "^VERSION
				structure EqnGraph=EqnGraph)

structure MrciBackend = MrciBackend(val compilerid=COMPILER^" " ^ VERSION
				    structure CBackend=CBackend
				    structure EqnGraph=EqnGraph)

structure RtxiBackend = RtxiBackend(val compilerid=COMPILER^" " ^ VERSION
				    structure CBackend=CBackend
				    structure EqnGraph=EqnGraph)

exception Exit of OS.Process.status 

	
fun printEnv(e, condmap) =
    PrettyPrint.print_env(TextIO.stdOut, e, condmap)
    
fun printEnv'(e, condmap) =
    PrettyPrint.print_env'(TextIO.stdOut, e, condmap)

(*    
fun printEqnGraph(g, outstream) =
    PrettyPrint.print_eqngraph(outstream, g)
*)



fun exitError (prog, msg) = 
    let 
	val _ = TextIO.output(TextIO.stdErr, prog ^ ": " ^ msg ^ "\n")
    in 
	raise Exit OS.Process.failure 
    end
	
fun exitHelp prog = 
    let 
	val _ = TextIO.output(TextIO.stdOut, (DLOptions.usage prog) ^ "\n")
    in 
	raise Exit OS.Process.success 
    end
	
fun exitVersion () = 
    let 
	val _ = TextIO.output (TextIO.stdOut, COMPILER ^ " version " ^ VERSION ^ "\n")
    in 
	raise Exit OS.Process.success 
    end

fun run (progname, args) =
    (let

 
	val optStatus = ref NONE
	val (opts, infiles) = (DLOptions.getopt optStatus) args

	val _ = (case !optStatus of 
		    SOME msg => exitError (progname, msg)
		  | NONE => ())

	val {help,version,output, eqdfg, eqdfgout, 
	     mrci, rtxi} =
	     DLOptions.getstate (opts)

	val _ = (if help = true then
		     exitHelp progname
		 else ())

	val _ = (if version = true then
		     exitVersion()
		 else ())

	(* parse the specified file and translate into an abstract
	 * syntax tree*)
	val (filearg, absyn) = Parse.parse (hd infiles)

	val systemname = 
	    case absyn of 
		Ast.SYSTEM text => Symbol.name (#name (#name text))

	(* type check the ast and convert into an environment table *)
	val (env, startenv, condmap, inline_c)  = Semant.type_check (filearg) (absyn)

	(* canonicalize the environment table *)
	val (condenvmap, eventenvmap: Canon.eventenvmap, env') = Canon.simplify_env env
								 
	(*val _ = print "ENV':\n"
	val _ = PrettyPrint.print_env' (TextIO.stdOut,env',condmap) *)

	val depenv = Deps.make_depenv (condmap, condenvmap, eventenvmap, env')

	val startenv' = 
	    case startenv of 
		SOME env => SOME (#3(Canon.simplify_env env))
	      | NONE => NONE

	val depstartenv = (case startenv' of
			       SOME startenv' => 
			       SOME (Deps.make_depenv (condmap, condenvmap, eventenvmap, startenv'))
			     | NONE => NONE)


	(* create an interference graph based upon the environment 
         * table, detect dependency cycles, and determine
   	 * ordering of computation *)
	val envgraph = EqnGraph.eqngraph (Symbol.symbol systemname, depenv)

	val _ = if !(#errors filearg) > 0 then
		    exitError (progname,
			       concat [Int.toString (!(#errors filearg)), 
				       " errors encountered."])
		else ()

	val startenvgraph = case depstartenv of 
				SOME env  => SOME (EqnGraph.eqngraph (Symbol.symbol systemname, env))
			      | NONE      => NONE

    in

	 
	if mrci then
	     case output of 
		 SOME prefix => 
		 MrciBackend.main(#filepath(filearg), 
				  systemname,
				  prefix, 
				  [], 
				  depenv, 
				  depstartenv, 
				  envgraph, 
				  startenvgraph)

	       | NONE =>  
		 MrciBackend.main(#filepath(filearg), 
				  systemname,
				  systemname, 
				  [], 
				  depenv, 
				  depstartenv, 
				  envgraph, 
				  startenvgraph)
	else ();

	if rtxi then
	     case output of 
		 SOME prefix => 
		 RtxiBackend.main(#filepath(filearg), 
				  systemname,
				  prefix, 
				  [], 
				  depenv, 
				  depstartenv, 
				  envgraph, 
				  startenvgraph)
		 
	       | NONE =>  
		 RtxiBackend.main(#filepath(filearg), 
				  systemname, 
				  systemname,
				  [], 
				  depenv, 
				  depstartenv, 
				  envgraph, 
				  startenvgraph)
	else ();

	 OS.Process.success 
    end)
    handle Exit status => status
	 | exn =>
	   let val _ = TextIO.output
			   (TextIO.stdErr,concat [progname,
						  ": Unexpected exception: ",
						  exnMessage exn, "\n"])
	   in OS.Process.failure
	   end




end


val _ = let val name = CommandLine.name()
	    val args = CommandLine.arguments()
	    val env = Posix.ProcEnv.environ()
	in
	    Main.run (name, args)
	end

