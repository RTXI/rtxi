 
functor MrciBackend (val compilerid: string
		     structure CBackend: DL_BACKEND
		     structure EqnGraph: DL_EQNGRAPH): DL_BACKEND = 

struct

open DLTypes

type name'        = DLDeps.name'
type type_info'   = DLDeps.type_info'
type depname      = DLDeps.depname
type depexp       = DLDeps.depexp
type indepexp     = DLDeps.indepexp
type depenv       = CBackend.depenv
type depenventry  = CBackend.depenventry
type depcondition = DLDeps.depcondition
type indepname      = DLInDeps.indepname

type eqngraph     = (depenventry, unit, unit) Graph.graph

type exp_info     = CBackend.exp_info
type depstorage   = CBackend.depstorage
type indepstorage = CBackend.indepstorage
type integration_method = CBackend.integration_method
datatype progline = datatype CBackend.progline
type external_state = CBackend.external_state

val integration_methods = CBackend.integration_methods

exception InvalidEntry
exception InvalidStorageEntry
exception InvalidCondition
exception InvalidConditionEntry
exception InvalidEquation
exception InvalidMethod
exception InvalidBinop
exception InvalidArgument
exception InvalidIdentifier
exception Unsupported



datatype order = Row_Major | Col_Major

datatype init_data = ScalarValue of real
		   | IntegerValue of int
		   | VectorValue of {size:int, 
				     data:real list}
		   | SetValue of symbol


type init_entry = {sym: Symbol.symbol, 
		   var_num: int, 
		   desc: string option,
		   init: init_data, 
		   dmode: bool, 
		   omode: bool}

type init_table = init_entry list


val indent_factor = 4

val say =
    (fn(outstream) => (fn (str) => TextIO.output(outstream, str)))
    
val sayln = 
    (fn(say) => (fn (str) => (say str; say "\n")))

val ind = (fn(say) => (fn(n) => let fun ind' (n) = if n <= 0 then ()
    else (say " "; ind' (n-1)) in ind' (n) end))
    
val indent = 
    (fn(ind) => (fn(indent_factor) => (fn(n) => ind(n * indent_factor))))

val vector_t        = "VECTOR_t"



fun outputvars (v::nil) = 
    (v ^ "")
  | outputvars (v::rest) =
    ((v ^ ", " ^ (outputvars rest)))
  | outputvars (nil) = ""

fun outputvecs (vecs) =
    map (fn(v) => CBackend.$(vector_t ^ "(" ^ v ^ "); ")) vecs

val output_depexp   = CBackend.output_depexp
val output_indepexp = CBackend.output_indepexp
val output_eq       = CBackend.output_eq
val printprog       = CBackend.printprog

fun symofindepname(name) = DLEnv'.symofname (DLInDeps.nameofindepname name)
fun symofdepname(name) = DLEnv'.symofname (DLDeps.nameofdepname name)
fun idofdepname (name) = DLEnv'.idofname (DLDeps.nameofdepname name)
fun vector2list v = Array.foldl (fn (i, l) => i::l) nil v


(* convert a name to a string *)
val id2str = PrettyPrint.id2str


fun arg2str ({name=sym, id=_}) = Symbol.name sym

fun name'2str {name, id} =
	Symbol.name (name)

fun fun2str (n) =
    (case n of 
	 DLInDeps.STATELESS (n) => name'2str n
       | DLInDeps.FUNARG (n) => name'2str n
       | DLInDeps.BUILTIN (n) => name'2str n
       | DLInDeps.TABLE (n) => name'2str n
       | DLInDeps.PARAM (n) => name'2str n)



fun depname2str (n) =
    (case n of
	 DLDeps.OLD (n) => name'2str n
       | DLDeps.NEW (n) => name'2str n
       | DLDeps.STATELESS (n) => name'2str n
       | DLDeps.BUILTIN (n) =>  name'2str n
       | DLDeps.CONDITION id => id2str id
       | DLDeps.TABLE (n) => name'2str n
       | DLDeps.PARAM (n) => name'2str n
       | DLDeps.EVENT (n) => name'2str n
       | DLDeps.EVENTARG (n,_) => name'2str n
       | DLDeps.SETELM n       => name'2str n)


fun int2str (i) =
    let
	fun minus (i) =
	    if i < 0 then
		"-"
	    else
		""
		
	val abs_i = Int.abs(i)
    in
	(minus i) ^ (Int.toString abs_i)
    end

fun real2str x = 
    let
	fun tilde2minus #"~" = #"-"
	  | tilde2minus c = c

    in
	String.implode (map tilde2minus (String.explode (Real.toString x)))
    end



fun make_init_entry (entry: depenventry, (varindex: int, init_table)) =
    (case entry of

	 DLDeps.EventEntry {name, arguments} =>
	 let
	     val (varindex,init_table,ee) = 
		 (varindex+1,
		  Symbol.enter(init_table, symofdepname name, varindex),
		  {sym=symofdepname name, 
		   var_num=varindex, 
		   desc=NONE, 
		   init=(ScalarValue 0.0), 
		   dmode=true, omode=false})

	     val (varindex,init_table,es) = 
		 foldl (fn (x,(i,env,es)) => 
			   let val e = {sym=Symbol.symbol((depname2str name) ^ "_" ^ (depname2str x)),
					var_num=i, 
					desc=NONE, 
					init=(ScalarValue 0.0), 
					dmode=true, omode=false}
			   in 
			       (i+1,Symbol.enter(env, symofdepname x, i),e :: es)
			   end)
		       (varindex,init_table,[]) arguments
				       
	 in
	     SOME (ee::es, (init_table, varindex))
	 end
	 
       | DLDeps.TimeEntry {quantity, initval, eq} => 
	 SOME ([{sym=symofdepname quantity, 
		 var_num=varindex, 
		 desc=NONE, 
		 init=(ScalarValue initval), 
		 dmode=true, omode=false}], 
	       (Symbol.enter(init_table, symofdepname quantity, varindex),	
		varindex+1))
	 
       | DLDeps.ScalarParEntry {quantity, value, description, ...} =>
	 SOME ([{sym=symofdepname quantity, 
		 var_num=varindex, 
		 desc=description, 
		 init=(ScalarValue value), 
		 dmode=true, omode=false}], 
	       (Symbol.enter(init_table, symofdepname quantity, varindex), 
		varindex+1))
	 
       | DLDeps.VectorParEntry {quantity, value, description, ...} =>
	 SOME ([{sym=symofdepname quantity, 
		 var_num=varindex, 
		 desc=description, 
		 init=(VectorValue {data=(vector2list value), 
				    size=length (vector2list value)}), 
		 dmode=true, omode=false}], 
	       (Symbol.enter(init_table, symofdepname quantity, varindex), 
		varindex+(Array.length value)+1))
	 
       | DLDeps.ScalarStateEntry {quantity, initval, method, 
				  eq, description, ...} =>
	 SOME ([{sym=symofdepname quantity, 
		 var_num=varindex, 
		 desc=description, 
		 init=(ScalarValue initval), 
		 dmode=true, omode=false}], 
	       (Symbol.enter(init_table, symofdepname quantity, varindex), 
		varindex+1))
	     
       | DLDeps.VectorStateEntry {quantity, initval, method, 
				  eq, description, ...} => 
	 SOME ([{sym=symofdepname quantity, 
		 var_num=varindex, 
		 desc=description, 
		 init=(VectorValue {data=(vector2list initval), 
				    size=(length (vector2list initval))}), 
		 dmode=true, omode=false}], 
	       (Symbol.enter(init_table, symofdepname quantity, varindex), 
		varindex+(Array.length initval)+1))

       | DLDeps.IntegerStateEntry {quantity, initval, 
				   eq, description, ...} =>
	 SOME ([{sym=symofdepname quantity, 
		 var_num=varindex, 
		 desc=description, 
		 init=(IntegerValue initval), 
		 dmode=true, omode=false}], 
	       (Symbol.enter(init_table, symofdepname quantity, varindex), 
		varindex+1))
	     
       | DLDeps.ExternalStateEntry {quantity, direction, 
				    eq, description, ...} =>

	 SOME ([{sym=symofdepname quantity, 
		 var_num=varindex, 
		 desc=description, 
		 init=(ScalarValue 0.0), 
		 dmode=true, omode=false}], 
	       (Symbol.enter(init_table, symofdepname quantity, varindex), 
		varindex+1))

       | DLDeps.DiscreteStateEntry {quantity, set, 
				    eq, description, ...} =>
	 SOME ([{sym=symofdepname quantity, 
		 var_num=varindex, 
		 desc=description, 
		 init=SetValue (#1 (hd set)), 
		 dmode=true, omode=false}], 
	       (Symbol.enter(init_table, symofdepname quantity, varindex), 
		varindex+1))

	     
       | DLDeps.VectorStateFunEntry {quantity, 
				     eq, size, description, ...} =>
	 SOME ([{sym=symofdepname quantity, 
		 var_num=varindex,
		 desc=description, 
		 init=(VectorValue {size=size, data=[]}), 
		 dmode=true, omode=false}], 
	       (Symbol.enter(init_table, symofdepname quantity, varindex), 
		varindex+1))
	     
       | DLDeps.ScalarStateFunEntry {quantity, description, ...} =>
	 SOME ([{sym=symofdepname quantity, 
		 var_num=varindex, 
		 desc=description, 
		 init=(ScalarValue 0.0), 
		 dmode=true, omode=false}], 
	       (Symbol.enter(init_table, symofdepname quantity, varindex), 
		varindex+1))
	 
       | DLDeps.TableFunEntry {quantity, body, low, high, step, argument,
			       description, ...} =>
	 SOME ([{sym=symofdepname quantity, 
		 var_num=varindex, 
		 desc=description, 
		 init=(ScalarValue 0.0), 
		 dmode=true, omode=false}], 
	       (Symbol.enter(init_table, symofdepname quantity, varindex), 
		varindex+1))
	 
       | _ => NONE)

fun make_init_table (env: depenv) = 

    let
	val (result, (v, t)) =   
	    foldl (fn (e, (l, (v, t))) => 
		      case make_init_entry (e, (v, t)) of
			  SOME (es, (t, v)) => (es @ l, (v, t))
			| NONE => (l, (v, t))) 
		  ([], (0, Symbol.empty)) (UniqueId.listItems env)
    in
	(result, v, t)
    end


(* takes in an init_table and returns a list of all the sizes needed
 * for vectors and matrices data 
 *)
fun type_sizes(init_table) =
    let
	fun obtain_sizeinfo({init=init, ...}:init_entry, table) =
	    case init of
		VectorValue {size=size, ...} => Symbol.enter(table, Symbol.symbol(int2str(size)), size)
	      | _ => table

	val table = foldl obtain_sizeinfo Symbol.empty init_table
    in
	Symbol.listItems(table)
    end

(* returns the constant name corresponding to a variable name (symbol) *)
(* in MRCI, e.g. sin1 maps to VAR_SIN1 *)
fun constname(sym) =
    let

	(* translates a string into an all uppercase collision free string *)
	(*   - lower case letters go to upper case *)
	(*   - upper case letters go to the same letter preceded by a _ *)
	(*   - _ characters map to __ *)
	(*   - everything else stays the same *)
	fun toUpper(char) = 
	    if Char.isUpper(char) then
		"_" ^ (Char.toString(char))
	    else if Char.isLower(char) then
		Char.toString(Char.toUpper(char))
	    else if char = #"_" then
		"__"
	    else
		Char.toString(char)

    in
	"VAR_"^ (String.translate toUpper (Symbol.name sym))
    end


fun depstorage (n, offset) =
    let
	fun addoffset (s) =
	    (case offset of 
		NONE => s
	      | SOME offset => s^"+"^"(int)"^offset)

	val index = (addoffset (constname (symofdepname n)))
    in
	case n of
	    DLDeps.EVENTARG (n,ev)  => let val ev = name'2str ev
					   val n  = name'2str n
					   val sym = Symbol.symbol (ev ^ "_" ^ n)
				       in
					   "state["  ^ "[" ^ (constname sym) ^ "]"
				       end
	  | DLDeps.OLD n            => "prev_state[" ^ index ^ "]"
	  | DLDeps.NEW n            => "state[" ^ index ^ "]"
	  | DLDeps.STATELESS n      => "state[" ^ index ^ "]"
	  | DLDeps.BUILTIN n        => Symbol.name (DLEnv'.symofname n)
	  | DLDeps.TABLE n          => "state[" ^ index ^ "]"
	  | DLDeps.PARAM n          => "state[" ^ index ^ "]"
	  | DLDeps.EVENT ev         => "state[" ^ index ^ "]"
	  | DLDeps.SETELM n         => "SET_" ^ (Symbol.name (DLEnv'.symofname n))
	  | DLDeps.CONDITION _      => raise InvalidStorageEntry
				      
    end


fun indepstorage (n, offset) =
    let
	fun addoffset (s) =
	    (case offset of 
		 NONE => s
	       | SOME offset => s^"+"^"(int)"^offset)
		    
	val index = (addoffset (constname (symofindepname n)))
    in
	case n of
	    DLInDeps.STATELESS n  => "state[" ^ index ^ "]"
	  | DLInDeps.FUNARG n     => Symbol.name (DLEnv'.symofname n)
	  | DLInDeps.BUILTIN n      => Symbol.name (DLEnv'.symofname n)
	  | DLInDeps.TABLE n      => "state[" ^ index ^ "]"
	  | DLInDeps.PARAM n      => "state[" ^ index ^ "]"
    end



(* create the constants header file *)
fun output_constants (outstream, num_vars, modelname, init_table, env) =
    let
	fun header() = 
	    [$"/*", 
	     SUB [$(modelname ^ "_constants.h"),
		  $("Generated by " ^ compilerid ^ " on " ^ (Date.toString (Date.fromTimeLocal (Time.now()))))], 
	     $"*/"]
		     
	val nvar = [$("#define NVAR " ^ (int2str num_vars))]
		     
	fun output_entry ({sym=sym, var_num=var_num,...}:init_entry) =
	     $("#define " ^ (constname sym) ^ " " ^ (int2str var_num))

	fun output_sets (DLDeps.SetEntry{name, value}) =
	     SOME ($("#define SET_" ^ (Symbol.name (symofdepname name)) ^ " " ^ (Int.toString value)))
	  | output_sets _ = NONE

	fun output_evargs (DLDeps.EventEntry {name, arguments}, ax) =
	    let
		val evname = constname (symofdepname name)
		val (_,evargs) = foldr (fn(x,(i,ax)) => (i+1,(i,depname2str x)::ax)) (0,[]) arguments
	    in 
		(map (fn((i,x)) => $("#define " ^ evname ^ "_" ^ x ^ " " ^ (Int.toString i))) evargs) @ ax
	    end
	  | output_evargs (_,ax) = ax

	val body = (header ()) @
		   (map output_entry init_table) @
		   nvar @
		   [$("")] @
		   (List.mapPartial output_sets (UniqueId.listItems env))

    in
	printprog (outstream, body, 0)
    end

(* create the var setup header file *)
fun output_varsetup (outstream, modelname, init_table) =
    let
		
	(* translate an initial value into a string representation *)
	fun init_str(init, sym) =
	    case init of
		ScalarValue (i) => real2str(i)
	      | VectorValue _   => "0.0"
	      | SetValue (n)    => "SET_" ^ (Symbol.name(n))
	      | IntegerValue(n) => int2str(n)
		

	fun output_init_entry ({sym, var_num, desc, init, dmode, omode}) =
	    let
		val constname  = constname(sym)
		val name = "\"" ^ (Symbol.name(sym)) ^ "\""
		val initval  = init_str(init, sym) 

		val dmode = if dmode then "1" else "0"

		val omode = if omode then "1" else "0"
	    in
		$(concat ["{", constname, ", ", name, ", ",  
			  initval, ", ", dmode, ",", omode,
			  "},"]) 
	    end
								       

	fun header() =
	     [$"/*",
	      SUB [$(modelname ^ "_varsetup.h"),
		   $("Generated by " ^ compilerid ^ " on " ^ (Date.toString (Date.fromTimeLocal (Time.now())))),
		   $"VAR_XXXX variable-name-string init-val display-group init-output-mode.",
		   $"The LAST term should have a negative number."],
	      $"*/"]
		
	(* create typedefs for vectors (1 for each size) *)
	fun typedefs() =
	    let
		fun print_typedef(n) =
		    $(concat ["#define VECTOR", int2str n, 
			      "_t(x)", " double x", 
			      "[", int2str (n+1),"]"])

	    in
		map print_typedef (type_sizes(init_table))
	    end

	(* creates the initial valued storage space for vectors and matrices *)
	fun init_values() =
	    let
		fun vals(nil) = ""
		  | vals(n::nil) = ", " ^ real2str(n)
		  | vals(n::rest) = ", " ^ (real2str(n)) ^ (vals(rest))

		fun init_val({init=init, sym=sym, ...}: init_entry) =
		    case init of 
			VectorValue {size=size, data=data} => 
			 SOME ($(concat["VECTOR", int2str(size), "_t(",
					constname(sym), "_INIT) = ",
					"{", ((int2str o length) data),
					(vals data), "};"]))
		      | _ => NONE
	    in
		List.mapPartial init_val init_table
	    end

	val struct_begin = [$"const VAR_INIT_ST varinit[] = {"]
	val struct_end   = [$"{-1, \"\", 0.0, 0, 0}\n};"]

	val body = (header ()) @
		   (typedefs ()) @
		   (init_values ()) @
		   struct_begin @
		   (map output_init_entry init_table) @
		   struct_end

    in
	printprog (outstream, body, 0)
    end




(* gives the shm_name used in the system for a table function *)
fun tablefun_shm_name(quantity) =
    "shm_table_" ^ (Symbol.name (symofdepname quantity))
    
fun tablefun_dimension(quantity) =
    "table_dimension_" ^ (Symbol.name (symofdepname quantity))
    
fun tablefun_offset(quantity) =
    "table_offset_" ^ (Symbol.name (symofdepname quantity))
    
fun tablefun_fname (quantity) = 
    (Symbol.name (symofdepname  quantity))


(* writes the table initialization code *)
fun output_module_init (outstream, depenv, init_table) =
    let

	fun output_tablefuns (depenventry: depenventry, l) =
	    (case depenventry of
		
		DLDeps.TableFunEntry{quantity, body, low, high, step,
				     argument, description} => 
		let
		    val q = tablefun_shm_name quantity
		in
		    (l @ 
		     [$("// memory allocation for lookup table used by function " 
			^ (Symbol.name (symofdepname quantity))),
		      $(q ^ " =  vmalloc(sizeof(*" ^ q ^ "));"),
		      $("if (" ^ q ^ " == NULL)"),
		      $"{",
		      SUB[$"printk(\"<KERN_WARN>vmalloc failed\\n\");",
			  $"return;"],
		      $"}",
		      $""])
		end

	      (* if its not a table fun entry, don't do anything *)
	      | _ => l)

	fun bool2str (b) = if b then "1" else "0"

	fun initialize_entry({sym, var_num, desc, init, dmode, omode}, l) =
	    (l @ 
	     [(case desc of
		   SOME desc => $("//" ^ desc)
		 | NONE      => $""),
	      $("j = " ^ (constname sym) ^ ";"),
	      $("strncpy (varmem->str[j], \"" ^ (Symbol.name sym) ^ "\", MAX_VAR_STR_SIZE);")] @
	     (case init of
		  VectorValue _  => [$("{"),
				     SUB [$"int k;",
					  $("for(k = 1; k <= " ^ (constname sym)^"_INIT[0]; k++) {"),
					  SUB [$("state[j+k] = " ^ (constname sym)^"_INIT[k];")],
					  $("}")],
				     $("}")]
		| ScalarValue x  => [$("state[j] = " ^ (real2str x) ^ ";")]
		| IntegerValue x => [$("state[j] = " ^ (int2str x) ^ ";")]
		| SetValue x     => [$("state[j] = SET_" ^ (Symbol.name(x)) ^ ";")]) @
	     [$("varmem->dmode[j] = " ^ (bool2str dmode) ^ ";"),
	      $("varmem->omode[j] = " ^ (bool2str omode) ^ ";"),
	      $("")])

	val body = 
	    (* static portion of function *)
	    [$"",
	     $"void model_module_init(void)",
	     $"{",
	     SUB ([$"int i, j;",
		   $"",
		   $"varmem->numvar = NVAR;",
		   $"",
		   $"/* default init */",
		   $"",
		   $"for(i = 0; i < NVAR; i++)",
		   $"{",
		   SUB [$"varmem->varval[i] = 0.0;",
			$"varmem->flag[i] = 0;",
			$"varmem->str[i][0] = '\\0';",
			$"varmem->dmode[i] = 0;",
			$"varmem->omode[i] = 0;"],
		   $"}",
		   $"",
		   $"/* now use varinit structure */",
		   $""] @
		  (foldl initialize_entry [] init_table) @
	    
		  (* setup table functions *)
		  ($""::(foldl output_tablefuns [] (UniqueId.listItems depenv))) @
	    
		  (* setup for numerical integration methods would go here *)
		  
		  (* rest of function *)
		  [$"",
		   $"START();"]),
	     $"}"]


    in
	printprog (outstream, body, 0)
    end

(* creates init_module function *)
fun output_init_module(outstream) =
    let
	val body  =
	    [$("int init_module (void)"),
	     $("{"),
	     SUB[$("return install_model (&model_loop, \n" ^
		   "                      &model_module_init, \n" ^
		   "                      &model_setrate_handler);")],
	     $("}"),
	     $("")]
    in
	printprog (outstream, body, 0)
    end

(* creates cleanup_module function *)
fun output_cleanup_module(outstream, depenv) =
    let
	fun tablefun (depenventry, l) =
	    (case depenventry of
		 
		 DLDeps.TableFunEntry{quantity, body, low, high, step,
				      argument, description} => 
		 let
		    val q = tablefun_shm_name quantity
		 in
		     l @
		     [$("// cleanup of lookup table used by function " ^ (Symbol.name (symofdepname quantity))),
		      $("vfree(" ^ q ^ ");"),
		      $""]
		 end
		     
	      (* if its not a table fun entry, don't do anything *)
	       | _ => l)

	val body = 
	    [$("void cleanup_module (void)"),
	     $("{"),
	     SUB ($"cleanup_model();"::(foldl tablefun [] (UniqueId.listItems depenv))),
	     $("}")]

    in
	printprog (outstream, body, 0)
    end

fun output_event_def (DLDeps.EventEntry {name,arguments},ax) =
    ($("double" ^ "[" ^ (Int.toString (length arguments)) ^ "]" ^ " " ^ (depname2str name) ^ ";")) :: ax
  | output_event_def (_,ax) = ax


(* outputs the preamble to the module code 
 *   (this means all the stuff at the top up to table functions)
 *)
fun output_preamble (outstream, outputname, depenv, inline_c) =
    let
			    
	val say = say (outstream)
	val sayln = sayln (say)
	val ind = ind (say)
	val indent = indent (ind) (indent_factor)

	val i = 0

	fun output_integration_method_def ({name=name, documentation=_, inputs=inputs,
					    invocation=_, body=body,
					    global_setup_code=_}:integration_method, l: progline list) =
	    let
		fun makeinputs (inputs, acc) = 
		    (if inputs > 1 
		     then makeinputs (inputs-1, ("y"^(Int.toString inputs))::acc)
		     else ("y1"::acc))

		val inputs = makeinputs (inputs, [])
		val inputvars = outputvars inputs

	    in
		[$"",
		 $("/* Definitions used by method " ^ name ^ " */"),
		 $("static inline double " ^ name ^ "(x, p, dt, "^inputvars^")"),
		 $"{",
		 SUB ((body ("x", "p", "dt", inputs)) @ [$"return x;"]),
		 $"}"] @ l
	    end

	val author = case Posix.ProcEnv.getenv("USER") of
			 SOME user => user
		       | NONE => "user"

	val time = Date.toString (Date.fromTimeLocal (Posix.ProcEnv.time()))

	val events = List.mapPartial (fn(x as DLDeps.EventEntry _ ) => SOME x
				      | _ => NONE) (UniqueId.listItems depenv)

	val body = 
	    [$"/*",
	     SUB [$("Generated by " ^ compilerid ^ " on " ^ (Date.toString (Date.fromTimeLocal (Time.now())))),
		  $("on " ^ time ^ " for " ^ author)],
	     $"*/",
	     $"",
	     $"",
	     $("#ifdef HAVE_CONFIG_H\n" ^ 
	       "#include <mrci_config.h>\n" ^ 
	       "#endif\n" ^ 
	       "\n" ^
	       "#include <linux/module.h>\n" ^
	       "#include <linux/kernel.h>\n" ^
	       "#include <linux/version.h>\n" ^
	       "#include <linux/slab.h>\n" ^
	       "#include <linux/string.h>\n" ^
	       "#include <linux/vmalloc.h>\n" ^
	       "\n" ^ 
	       "#include <mrci.h>\n" ^ 
	       "#include <rt_math.h>\n" ^ 
	       "#include <table.h>\n" ^ 
	       "\n" ^
	       "#include \"" ^ outputname ^ "_constants.h\"\n" ^
	       "#include \"" ^ outputname ^ "_varsetup.h\"\n"),
	     
	     $("MODULE_AUTHOR(\"" ^ author ^ "\");"),
	     $("#ifdef MODULE_LICENSE"),
	     $("MODULE_LICENSE(\"GPL\");"),
	     $("#endif"),

	     $(""),
	     $(""),
	     
	     $("extern double daqw_conv_input_to_phys (short);"),
	     $("extern short daqw_conv_phys_to_output (double);"),
	     $("extern int install_model (void(*loop)(short *in, short *out),\n" ^
               "                          void (*init)(void),\n" ^
               "                          void (*setrate_handler)(void));"),
	     $("extern int cleanup_model (void);"),
	     $(""),

	     $("/* Forward declarations */"),
	     $("void model_module_init (void);"),
	     $("void model_setrate_handler (void);"),
	     $("void model_loop (short *inval, short *outval);"),
	     $("void START (void);"),
	     $("")] @

	    (foldl output_integration_method_def [] integration_methods) @

	    [$(""),
	     $(""),

	     $("extern VAR_PASS_ST *varmem;"),
	     $("extern double state[];"),
	     $("extern double dt;"),
	     $("extern int ramp_var;"),
	     
	     $(""),
	     $("double prev_state[MAX_NUM_STATE_VARS];"),
	     $(""),
	     $("#define VECTOR_t(x) double x[128]")] @

	    [$(""),
	     $("#define sqr sqrt"),
	     $("#define cube(x) pow(x,3)"),
	     $(""),
	     $("/* Scalar/Vector product */"),
	     $("void sv_times (VECTOR_t(r), double s, VECTOR_t(v))"),
	     $("{"),
	     SUB [$("double *vp;"),
		  $("int len = v[0], i;"),
		  $(""),
		  $("r[0] = len;"),
		  $(""),
		  $("vp = &(v[0]);"),
		  $(""),
		  $("for (i = 1; i <= len; i++)"),
		  $("{"),
		  SUB [$("r[i] += *(vp + i) * s;")],
		  $("}"),
		  $("")],
	     $("}"),
	     $(""),
	     $(""),
	     $("/* Vector addition */"),
	     $("void v_plus (VECTOR_t(r), VECTOR_t(a), VECTOR_t(b))"),
	     $("{"),
	     SUB [$("double *ap, *bp;"),
		  $("int len = a[0], i;"),
		  $(""),
		  $("r[0] = len;"),
		  $(""),
		  $("ap = &(a[0]);"),
		  $("bp = &(b[0]);"),
		  $(""),
		  $("for (i = 1; i <= len; i++)"),
		  $("{"),
		  SUB [$("r[i] = *(ap + i) + *(bp + i);")],
		  $("}")],
	     $("}"),
	     $(""),
	     $("/* Vector subtraction */"),
	     $("void v_minus (VECTOR_t(r), VECTOR_t(a), VECTOR_t(b))"),
	     $("{"),
	     SUB[$("double *ap, *bp;"),
		 $("int len = a[0], i;"),
		 $(""),
		 $("r[0] = len;"),
		 $(""),
		 $("ap = &(a[0]);"),
		 $("bp = &(b[0]);"),
		 $(""),
		 $("for (i = 1; i <= len; i++)"),
		 $("{"),
		 SUB [$("r[i] = *(ap + i) - *(bp + i);")],
		 $("}")],
	     $("}"),
	     $(""),
	     $("/* Vector dot product */"),
	     $("double v_times (VECTOR_t(a), VECTOR_t(b))"),
	     $("{"),
	     SUB[$("double *ap, *bp;"),
		 $("int len = a[0], i;"),
		 $("double r;"),
		 $(""),
		 $("r = 0;"),
		 $(""),
		 $("ap = &(a[0]);"),
		 $("bp = &(b[0]);"),
		 $(""),
		 $("for (i = 1; i <= len; i++)"),
		 $("{"),
		 SUB[$("r += *(ap + i) * *(bp + i);")],
		 $("}"),
		 $(""),
		 $("return r;")],
	     $("}"),
	     $(""),
	     $(""),
	     $("/* Vector cross product */"),
	     $("void v_crossp (VECTOR_t(r), VECTOR_t(a), VECTOR_t(b))"),
	     $("{"),
	     SUB[$("int len = a[0];"),
		 $("r[0] = len;"),
		 $(""),
		 $("switch (len)"),
		 $("{"),
		 SUB[$("case 3:"),
		     SUB[$("r[1] = (a[2] * b[3]) - (a[3] * b[2]);"),
			 $("r[2] = (a[3] * b[1]) - (a[1] * b[3]);"),
			 $("r[3] = (a[1] * b[2]) - (a[2] * b[1]);"),
			 $("break;")],
		     $("")],
		 SUB[$("default: "),
		     $("break;")],
		 $("}")],
	     $("}"),
	     $(""),
	     $("double length (VECTOR_t(a))"),
	     $("{"),
	     SUB [$("return a[0];")],
	     $("}"),
	     $(""),
	     $("/* Vector right shift */"),
	     $("void rshift (VECTOR_t(r), VECTOR_t(a), double b)"),
	     $("{"),
	     SUB[$("int len = a[0];"),
		 $(""),
		 $("r[0] = a[0];"),
		 $(""),
		 $("memmove (&(r[2]), &(a[1]), (len-1) * sizeof (double));"),
		 $(""),
		 $("r[1] = b;")],
	     $("}"),
	     $(""),
	     $("/* Vector left shift */"),
	     $("void lshift (VECTOR_t(r), VECTOR_t(a), double b)"),
	     $("{"),
	     SUB[$("int len = a[0];"),
		 $(""),
		 $("r[0] = a[0];"),
		 $(""),
		 $("memmove (&(r[1]), &(a[2]), (len-1) * sizeof (double));"),
		 $(""),
		 $("r[len] = b;")],
	     $("}"),
	     $(""),
	     $(""),
	     $(""),
	     $("/* Vector sum */"),
	     $("double sum (VECTOR_t(a))"),
	     $("{"),
	     SUB[$("double *ap;"),
		 $("int len = a[0], i;"),
		 $("double r;"),
		 $(""),
		 $("r = 0;"),
		 $(""),
		 $("ap = &(a[0]);"),
		 $(""),
		 $("for (i = 1; i <= len; i++)"),
		 $("{"),
		 SUB [$("r += *(ap + i);")],
		 $("}"),
		 $(""),
		 $("return r;")],
	     $("}"),
	     $(""),

	     (* WRITE C DECLARATIONS HERE *)
	     $"/* C declarations: */"] @
	    (let
		 val lines = foldl (fn (s, l) => ($ s)::l) [] inline_c
	     in
		 rev lines
	     end) @
	    [$""]
    in
	printprog (outstream, body, 0)
    end


fun output_tablefun_defs (outstream, env: depenv) =
    let
	val items = DLDeps.query_env (env, TableFunEntryType)

	fun tablefun_fwd_def (DLDeps.TableFunEntry {quantity, body,
						    low, high, step,
						    argument,
						    description}, l) =  
	    l @ [$("// forward declaration of lookup table function " ^
		   (Symbol.name (symofdepname quantity))),
		 $("static inline double " ^ (tablefun_fname quantity) ^  
		   " (double " ^ (Symbol.name (symofdepname argument)) ^ ");")]

	  | tablefun_fwd_def (_) = raise InvalidEntry

	fun tablefun_def (DLDeps.TableFunEntry {quantity, body, low,
						high, step, argument, 
						description}, l) = 
	    let
		val range = (real2str low)  ^ "," ^
			    (real2str high) ^ "," ^ 
			    (real2str step)
 		val {body, vector_vars, 
		     scalar_vars, return_vars, 
		     return_type} =  (output_indepexp indepstorage) (body, env, NONE)
	    in
		l @ [$"",
		     $("// declarations for a lookup table " ^
		       "used by function " ^ (Symbol.name (symofdepname quantity))),
		     $("volatile double (*" ^ (tablefun_shm_name quantity) ^ ")" ^
		       "[" ^ (int2str 2) ^ "][LOOKUP_TABLE_DIMENSION(" ^ range ^ ")];"),
		     $("unsigned long int " ^ (tablefun_dimension quantity)  ^  " = " ^
		       "LOOKUP_TABLE_DIMENSION(" ^ range ^ ");"),
		     $("unsigned long int " ^ (tablefun_offset quantity)  ^  " = " ^
		       "LOOKUP_TABLE_OFFSET(" ^ range ^ ");"),
		     $("static inline double " ^ (tablefun_fname quantity) ^  
		       " (double " ^ (Symbol.name (symofdepname argument)) ^ ")"),
		     $"{",
		     SUB ((if not (scalar_vars = nil)
			   then
			       [$("double " ^ (outputvars scalar_vars) ^ ";")]
			   else []) @
			  (if not (vector_vars = nil)
			   then
			       outputvecs vector_vars
			   else []) @
			  body @
			  [$("return " ^ (hd return_vars) ^ ";")]),
		     $"}"]

	    end
	  | tablefun_def (_) = raise InvalidEntry

	val defs = foldl tablefun_fwd_def [] items
	val body = foldl tablefun_def [] items
    in
	printprog (outstream, defs, 0);
	printprog (outstream, body, 0)
    end


(* creates generic function declarations  *)
fun output_generic_functions (outstream, depenv) =
    let
	fun output_generic_fun (depenventry) =
	    case depenventry of
		DLDeps.FunEntry{name, body, typeinfo}
		=> 
		let
		    val {args, ret_type}: type_info' = typeinfo

		    fun rettype2str (typ) =
			case DLTypes.singletonToType(typ) of
			    DLTypes.Scalar => "double"
			  | (DLTypes.Vector _) => "void"
			  | _ => raise Unsupported

		    fun argtype2str (typ) =
			case DLTypes.singletonToType(typ) of
			    DLTypes.Scalar => "double"
			  | (DLTypes.Vector _) => "double *"
			  | _ => raise Unsupported
			
		    fun outputargs (nil) = ""
		      | outputargs ({name, typ}::nil) =   
			(argtype2str typ) ^ " " ^ (arg2str name)
		      | outputargs ({name, typ}::rest) = 
			((argtype2str typ) ^ " " ^ (arg2str name)) ^ ", " ^ (outputargs rest)
		       
		    val {body, vector_vars, scalar_vars,
			 return_vars, return_type} =
			(output_indepexp indepstorage) (body, depenv, SOME typeinfo)
					
		    val rv = hd return_vars

		    val ret_name = {name=Symbol.symbol((Symbol.name (symofdepname name)) ^ "_RET"), 
				    id=idofdepname name}

		    val ret_arg = {name=ret_name, typ=ret_type}

		    val args = case DLTypes.singletonToType(ret_type) of
				   DLTypes.Scalar => args
				 | (DLTypes.Vector _) => ret_arg::args
				 | _ => raise Unsupported

		    val i = "i" ^ (Int.toString (UniqueId.id2int (UniqueId.genid())))
		in
		    [$("// generic function named " ^ (Symbol.name(symofdepname name))),
		     $((rettype2str ret_type) ^ 
		       (" ") ^ (Symbol.name (symofdepname name)) ^
		       ("(") ^ (outputargs args) ^ (") {")),
		     SUB ([$(case DLTypes.singletonToType(ret_type) of
				 (DLTypes.Vector _) => "int " ^ i ^ ";"
			       | DLTypes.Scalar => ""
			       | _ => raise Unsupported)] @
			  (map (fn(s) => $("double " ^ s ^ ";")) scalar_vars) @
			  (map (fn(s) => $(vector_t ^ "(" ^ s ^ ");")) vector_vars) @
			  [$("")] @
			  body @
			  (case DLTypes.singletonToType ret_type of
			       DLTypes.Scalar => [$("return " ^ rv ^ ";")]
			     | (DLTypes.Vector _) => 
			       [$("for ("^i^" = 0; "^i^" <= " ^ rv ^ "[0]; "^i^"++)"),
				$"{",
				SUB[$((Symbol.name (symofdepname name)) ^ "_RET["^i^"] = " ^ rv ^ "["^i^"];")],
				$"}"]
			     | _ => raise Unsupported)),
		     $("}")]
		end
	      (* if its not a generic fun entry, don't do anything *)
	      | _ => []

    in 
	printprog (outstream, [$"", $"// generic functions", $""], 0);

	app 
	    (fn f => printprog (outstream, (output_generic_fun f), 0))
	    (UniqueId.listItems depenv)
    end


fun output_vector_functions (outstream) =
    let 
	val body = 
	    [$("void store_vec(int varnum, double *vector)"),
	     $"{",
	     SUB [$("int i;"),
		  $(""),
		  $("for (i = 0; i <= vector[0]; i++) "),
		  SUB [$("state[varnum+i] = vector[i];")]],
	     $("}"),
	     $(""),
	     $("void get_vec(int varnum, double *new_vec)"),
	     $("{"),
	     SUB [$("int i;"),
		  $("for (i = 0; i <= state[varnum]; i++) "),
		  SUB [$("new_vec[i] = state[varnum+i];")]],
	     $("}")]
    in
	printprog (outstream, body, 0)
    end


fun output_setrate_handler (outstream, env) = 
    let

	val items = DLDeps.query_env (env, TableFunEntryType)

	fun tablefun_setrate (DLDeps.TableFunEntry {quantity, body,
						    low, high, step,
						    argument,
						    description}, l) =  
	    let
		val range = (real2str low)  ^ "," ^
			    (real2str high) ^ "," ^ 
			    (real2str step)
	    in
		l @
		[$("// initialization of lookup table used by function " ^ (Symbol.name (symofdepname quantity))),
		 $("table_i = 0;"),
		 $(concat["INITIALIZE_LOOKUP_TABLE(", (tablefun_shm_name quantity),
			  ",", range,  ",", (Symbol.name (symofdepname argument)), 
			  ",", (Symbol.name (symofdepname quantity)), ");"])]
	    end
	  | tablefun_setrate (_) = raise InvalidEntry

	val body = 
	    [$"void model_setrate_handler (void)",
	     $"{",
	     SUB (if (length items)  > 0
		  then ($"int table_i;")::(foldl tablefun_setrate [] items)
		  else []),
	     $"}"]

    in
	printprog (outstream, body, 0)
    end


fun output_table_lookup (env, entry) =
    (case entry of 
	 DLDeps.TableFunEntry {quantity, body, low, high, step,
			       argument, description} =>
	 let
	     val name = Symbol.name (symofdepname quantity)
	 in
	     [$(concat[(depstorage (quantity, NONE)), " = ",
		       "(double)lookup(", (depstorage (argument, NONE)),
		       ", ",
		       "(double *)(*shm_table_", name, ")[0], ",
		       "(double *)(*shm_table_", name, ")[1], ",
		       "table_dimension_", name ^ ", ",
		       "table_offset_", name ^ ", ",
		       (real2str low), ", ",
		       (real2str high), ", ",
		       (real2str step),  ");"])]
	 end
       | _ => raise InvalidEntry)


fun output_external_input (assignments: depname list ref, quantity, channel) =
    (if (List.exists (fn(dn) => (idofdepname quantity) = (idofdepname dn)) (!assignments))
     then []
     else [$((depstorage (quantity, NONE)) ^ " = daqw_conv_input_to_phys (inval["^(Int.toString channel)^"]);")]
	  before (assignments :=  (quantity::(!assignments))))

fun output_external_output (assignments: depname list ref, quantity, channel) =
     [$("outval[" ^ (Int.toString channel) ^ "] = " ^ "daqw_conv_phys_to_output(" ^ 
	(depstorage (quantity, NONE)) ^ ");")]


(* Output the state updating function *)
fun output_time_block (g, env) =
    let
	val times   = DLDeps.query_env (env, TimeEntryType)
	val time    = DLDeps.nameofentry (hd times)
	val levels  = EqnGraph.color g

	val external_input_state = {func=output_external_input, assignments=ref []}
	val external_output_state = {func=output_external_output, assignments=ref []}
    in
	[$"{",
	 (* system equations *)
	 SUB (foldl (fn((ids), l) => 
		       ((foldl (fn (id, l) =>
				   (let
					val entry = valOf(UniqueId.look (env, id))
				    in
					(output_eq (depstorage, "dt", output_table_lookup,
						    external_input_state, external_output_state)) (env, entry)
				    end) @ l)
			       [] ids) @ l))
		    [] levels),
	 $"}"]
    end


fun output_runtime_fun (outstream, g, env) =
    printprog (outstream, 
	       ($"void model_loop( short *inval, short *outval )")::
	       (output_time_block (g, env)),  0)


fun output_start_fun (outstream, g, env) =
    printprog (outstream,
	       ($"void START( void )")::
	       (case (g, env) of 
		    (SOME g, SOME env) => output_time_block (g, env)
		  | _ => [$"{}"]), 0)



fun main (dir, modelname, outputname, inline_c, env, startenv, envgraph, startenvgraph) =

    let
	val (init_entries, num_vars,  init_entries_table) = make_init_table env

	val varsetup_stream  =TextIO.openOut(OS.Path.joinDirFile {dir=dir, file=outputname ^ "_varsetup.h"})
	val constants_stream =TextIO.openOut(OS.Path.joinDirFile {dir=dir, file=outputname ^ "_constants.h"})
	val module_stream    =TextIO.openOut(OS.Path.joinDirFile {dir=dir, file=outputname ^ "_module.c"})

	val sayln = sayln (say module_stream)

    in
	output_varsetup(varsetup_stream, modelname, init_entries);
	TextIO.closeOut(varsetup_stream);

	output_constants(constants_stream, num_vars, modelname, init_entries, env);
	TextIO.closeOut(constants_stream);
	
	output_preamble(module_stream, outputname, env, inline_c);

	output_tablefun_defs(module_stream, env);

	output_init_module(module_stream);

	output_cleanup_module(module_stream, env);

	output_module_init(module_stream, env, init_entries);

	output_setrate_handler(module_stream, env);

	output_vector_functions(module_stream);

	output_generic_functions(module_stream, env);

	output_start_fun(module_stream, startenvgraph, startenv);

	output_runtime_fun(module_stream, envgraph, env);

	TextIO.closeOut(module_stream)
    end

end

 
