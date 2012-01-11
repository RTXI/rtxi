
(* A backend for the RTXI system.
 *
 * rtxi.sourceforge.net
 *
 *)
 
functor RtxiBackend (val compilerid: string
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
type indepname    = DLInDeps.indepname


type eqngraph     = (depenventry, unit, unit) Graph.graph

type exp_info     = CBackend.exp_info
type depstorage   = CBackend.depstorage
type indepstorage = CBackend.indepstorage
type integration_method = CBackend.integration_method
datatype progline = datatype CBackend.progline
type external_state = CBackend.external_state

val integration_methods = CBackend.integration_methods

exception InvalidEntry
exception InvalidInitEntry
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
		   | VectorValue of {size:int, data:real list}
		   | SetValue of symbol

type entry_count = {state: int, parameter: int, stateless: int, table: int, function: int}

type init_entry = {name: depname, desc: string option, init: init_data}

type init_table = init_entry list

val vector_t_macro      = "VECTOR_t"
fun vector_t (n)        = "VECTOR" ^ (Int.toString n) ^ "_t"

fun event_struct_type(modelname) = modelname ^ "_event"
fun state_struct_type(modelname) = modelname ^ "_state"
fun stateless_struct_type(modelname) = modelname ^ "_stateless"
fun table_struct_type(modelname) = modelname ^ "_table"
fun parameter_struct_type(modelname) = modelname ^ "_parameter"

fun var_struct_type(modelname) = modelname ^ "_var"

val event_struct      = "event"
val prev_state_struct = "prev_state"
val next_state_struct = "next_state"
val stateless_struct  = "stateless"
val table_struct      = "table"
val parameter_struct  = "parameter"

	  
val indent_factor = 4

val say =
    (fn(outstream) => (fn (str) => TextIO.output(outstream, str)))
    
val sayln = 
    (fn(say) => (fn (str) => (say str; say "\n")))

val ind = (fn(say) => (fn(n) => let fun ind' (n) = if n <= 0 then ()
    else (say " "; ind' (n-1)) in ind' (n) end))
    
val indent = 
    (fn(ind) => (fn(indent_factor) => (fn(n) => ind(n * indent_factor))))


fun symofindepname(name)  = DLEnv'.symofname (DLInDeps.nameofindepname name)
fun symofdepname(name)    = DLEnv'.symofname (DLDeps.nameofdepname name)
fun idofdepname (name)    = DLEnv'.idofname (DLDeps.nameofdepname name)
fun vector2list v         = Array.foldl (fn (i, l) => i::l) nil v

val output_depexp     = CBackend.output_depexp
val output_indepexp   = CBackend.output_indepexp
val output_eq         = CBackend.output_eq
val printprog         = CBackend.printprog


val id2str = PrettyPrint.id2str
fun name'2str {name, id} = Symbol.name (name)

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
    let fun minus (i) = if i < 0 then "-" else ""
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

fun listrealvals(nil) = ""
  | listrealvals(n::nil) = ", " ^ (real2str n)
  | listrealvals(n::rest) = ", " ^ (real2str n) ^ (listrealvals rest)


fun outputvars (v::nil) = 
    (v ^ "")
  | outputvars (v::rest) =
    ((v ^ ", " ^ (outputvars rest)))
  | outputvars (nil) = ""

fun outputvecs (vecs) =
    map (fn(v) => $(vector_t_macro ^ "(" ^ v ^ "); ")) vecs



fun make_init_entry (entry: depenventry, 
		     count as {state=state_count, 
			       parameter=parameter_count, 
			       stateless=stateless_count, 
			       table=table_count, 
			       function=function_count}) = 
    (case entry of

	 DLDeps.EventEntry {name, arguments} => 
	 let val es = map (fn(x) => {name=x, desc=NONE, init=(ScalarValue 0.0)}) arguments
	 in 
	     (SOME ({name=name, desc=NONE, init=ScalarValue 0.0} :: es),
	      {state=state_count, parameter=parameter_count, stateless=stateless_count+1, 
	       table=table_count, function=function_count})
	 end

       | DLDeps.TimeEntry {quantity, initval, eq} => 
	 (SOME [{name=quantity, desc=NONE, init=(ScalarValue initval)}], 
	  {state=state_count+1, parameter=parameter_count, stateless=stateless_count, 
	   table=table_count, function=function_count})
	 
       | DLDeps.ScalarParEntry {quantity, value, description, ...} =>
	 (SOME [{name=quantity, desc=description, init=(ScalarValue value)}], 
	 {state=state_count, parameter=parameter_count+1, stateless=stateless_count, 
	  table=table_count, function=function_count})

       | DLDeps.VectorParEntry {quantity, value, description, ...} =>
	 (SOME [{name=quantity, desc=description, 
		 init=(VectorValue {data=(vector2list value), 
				    size=length (vector2list value)})}],
	  {state=state_count, parameter=parameter_count+1, stateless=stateless_count, 
	   table=table_count, function=function_count})
	 
       | DLDeps.ScalarStateEntry {quantity, initval, method, 
				  condition=NONE, event=NONE,
				  eq, description, ...} =>
	 (SOME [{name=quantity, desc=description, init=(ScalarValue initval)}], 
	  {state=state_count+1, parameter=parameter_count, stateless=stateless_count, 
	   table=table_count, function=function_count})

	     
       | DLDeps.VectorStateEntry {quantity, initval, method, 
				  condition=NONE, event=NONE,
				  eq, description, ...} => 
	 (SOME [{name=quantity, desc=description, 
		 init=(VectorValue {data=(vector2list initval), 
				    size=(length (vector2list initval))})}],
	  {state=state_count+1, parameter=parameter_count, stateless=stateless_count, 
	   table=table_count, function=function_count})


       | DLDeps.IntegerStateEntry {quantity, initval, 
				   condition=NONE, event=NONE,
				   eq, description, ...} =>
	 (SOME [{name=quantity, desc=description, init=(IntegerValue initval)}],
	  {state=state_count+1, parameter=parameter_count, stateless=stateless_count, 
	   table=table_count, function=function_count})
	     
       | DLDeps.ExternalStateEntry {quantity, direction, 
				    condition=NONE, event=NONE,
				    eq, description, ...} =>
	 (SOME [{name=quantity, desc=description, init=(ScalarValue 0.0)}],
	  {state=state_count+1, parameter=parameter_count, stateless=stateless_count, 
	   table=table_count, function=function_count})

       | DLDeps.DiscreteStateEntry {quantity, set, 
				    condition=NONE, event=NONE,
				    eq, description, ...} =>
	 (SOME [{name=quantity, desc=description, init=SetValue (#1 (hd set))}],
	  {state=state_count+1, parameter=parameter_count, stateless=stateless_count, 
	   table=table_count, function=function_count})
	     
       | DLDeps.VectorStateFunEntry {quantity, condition=NONE, event=NONE,
				     eq, size, description, ...} =>
	 (SOME [{name=quantity, desc=description, 
		 init=(VectorValue {size=size, data=[]})}],
	  {state=state_count, parameter=parameter_count, stateless=stateless_count+1, 
	   table=table_count, function=function_count})
	     
       | DLDeps.ScalarStateFunEntry {quantity, condition=NONE, event=NONE,
				     eq=_, description, ...} =>
	 (SOME [{name=quantity, desc=description, init=(ScalarValue 0.0)}],
	  {state=state_count, parameter=parameter_count, stateless=stateless_count+1, 
	   table=table_count, function=function_count})

	 
       | DLDeps.TableFunEntry {quantity, body, low, high, step, argument,
			       description} =>

	 (NONE,
	  {state=state_count, parameter=parameter_count, stateless=stateless_count+1, 
	   table=table_count, function=function_count})

	 
       | DLDeps.FunEntry {name, body, typeinfo} =>
	 (NONE, {state=state_count, parameter=parameter_count, stateless=stateless_count, 
		 table=table_count, function=function_count+1})


       | _ => (NONE, count))


fun make_init_table (env: depenv) = 
    let
	val (result, count) =   
	    foldl (fn (e, (l, count)) => 
		      case make_init_entry (e, count) of
			  (SOME es, count) => (es @ l, count)
			| (NONE, count) => (l, count))
		  ([], {state=0, parameter=0, stateless=0, 
			table=0, function=0}) (UniqueId.listItems env)
    in
	(result, count)
    end

fun sizeof(init) =
    (case init of
	 VectorValue {size, ...} => size
       | _ => 1)

(* takes in an init_table and returns a list of all the sizes needed
 * for vectors and matrices data 
 *)
fun type_sizes(init_table) = 
    let 
	fun sizeof({init, ...}: init_entry, l) =
	    (case init of
		 VectorValue {size, ...} => size::l
	       | _ => l)
    in foldl sizeof [] init_table end

fun depstorage (n, offset) =
    let
	val var = (case n of
		       DLDeps.OLD n        => prev_state_struct ^ "." ^ (name'2str n)
		     | DLDeps.NEW n        => next_state_struct ^ "." ^ (name'2str n)
		     | DLDeps.STATELESS n  => stateless_struct  ^ "."   ^ (name'2str n) 
		     | DLDeps.TABLE n      => table_struct ^ "." ^ (name'2str n)
		     | DLDeps.PARAM n      => parameter_struct ^ "." ^ (name'2str n)
		     | DLDeps.BUILTIN n    => (case name'2str n of
						  "dt" => "dt"
					          | n => n)
		     | DLDeps.EVENTARG (n,ev)  => event_struct ^ "." ^ (name'2str ev) ^ "." ^ (name'2str n) 
		     | DLDeps.EVENT n          => stateless_struct ^ "." ^ (name'2str n)
		     | DLDeps.SETELM n         => "SET_" ^ (name'2str n)
		     | DLDeps.CONDITION id => raise InvalidStorageEntry)

    in
	case offset of 
	    NONE => var
	  | SOME n => var ^ "[" ^ n ^ "]"
    end


fun indepstorage (n, offset) =
    let
	val var = (case n of
		       DLInDeps.STATELESS n  => stateless_struct ^ "." ^ (name'2str n)
		     | DLInDeps.TABLE n      => table_struct ^ "." ^ (name'2str n)
		     | DLInDeps.PARAM n      => parameter_struct ^ "." ^ (name'2str n)
		     | DLInDeps.FUNARG n     => name'2str n
		     | DLInDeps.BUILTIN n    => (case name'2str n of
						    "dt" => "dt"
					            | n => n))
    in
	case offset of 
	    NONE => var
	  | SOME n => var ^ "[" ^ n ^ "]"
    end

val comment_header = [$"*",
		      $("* Generated by " ^ compilerid ^ " on " ^ (Date.toString (Date.fromTimeLocal (Time.now())))),
		      $"*"]

val math_builtins = 
    [$("#define sqr sqrt"),
     $("#define cube(x) pow(x,3)")]


val vector_builtins = 
    
    [$("/* Scalar/Vector product */"),
     $("void sv_times (VECTOR_t(r), double s, VECTOR_t(v))"),
     $("{"),
     SUB [$("double *vp;"),
	  $("int len = (int)(v[0]), i;"),
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
	  $("int len = (int)(a[0]), i;"),
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
	 $("int len = (int)(a[0]), i;"),
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
	 $("int len = (int)(a[0]), i;"),
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
     SUB[$("int len = (int)(a[0]);"),
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
     SUB[$("int len = (int)(a[0]);"),
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
     SUB[$("int len = (int)(a[0]);"),
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
	 $("int len = (int)(a[0]), i;"),
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
     $("")]


(* create the constants header file *)
fun output_defs (outstream, modelname, init_table, env) =
    let
	val header = 
	    [$"/*"] @
	    comment_header @
	    [$"*/",
	     $"",
	     $"",
	     $"#include <string.h>",
	     $"#include <math.h>",
	     $"",
	     $"#include <default_gui_model.h>",
	     $"",
	     $""]

	val vector_t_macro_def = $"#define VECTOR_t(x) double x[128]"
		
	(* create typedefs for vectors (one for each size) *)
	val vector_typedefs =
	    let
		fun vector_typedef(n) =

		    $(concat ["typedef ", "double ", vector_t n, 
			      "[", int2str (n+1),"]", ";"])

	    in
		(map vector_typedef (type_sizes init_table)) 
	    end

	val var_constants = 
	    [$"#define MAX_VAR_NAME  32"]

	val shm_name = $("#define SHMNAM  \"" ^ (modelname) ^ "\"")

	fun event_def (DLDeps.EventEntry {name,arguments},ax) =
	    (SUB [$("struct "),
		  $("{"),
		  SUB (map (fn(x) => $("double " ^ (depname2str x) ^ ";")) arguments),
		  $("}" ^ (depname2str name) ^ ";")]) :: ax
	  | event_def (_,ax) = ax

	val event_struct_typedef = 
	    [$("typedef struct " ^ (event_struct_type modelname)),
	     $("{"), 
	     SUB (foldl event_def [] (UniqueId.listItems env)),
	     $("};")]
     

	fun state_entrydef (depentry) =
	    (case depentry of 
		DLDeps.TimeEntry {quantity, ...} => 
		SOME ($("double " ^ (depname2str quantity) ^ ";"))

	      | DLDeps.ScalarStateEntry {quantity, condition=NONE, event=NONE, 
					 description, ...} =>
		 SOME ($(concat ["double ", (depname2str quantity), ";",
				 (case description of 
				      SOME text => " /*" ^ text ^ "*/"
				    | NONE => "")]))
		
	      | DLDeps.VectorStateEntry {quantity, condition=NONE, event=NONE,
					 description, initval, ...} =>
		SOME ($(concat [(vector_t (Array.length initval)), " ", (depname2str quantity), ";", 
				(case description of 
				     SOME text => " /*" ^ text ^ "*/"
				   | NONE => "")]))
		
	      | DLDeps.IntegerStateEntry {quantity, condition=NONE, event=NONE, 
					  description, ...} =>
		SOME ($(concat ["int ", (depname2str quantity), ";",
				(case description of 
				     SOME text => " /*" ^ text ^ "*/"
				   | NONE => "")]))

	      | DLDeps.ExternalStateEntry {quantity, condition=NONE, event=NONE, 
					   description, ...} =>
		SOME ($(concat ["double ", (depname2str quantity), ";",
				(case description of 
				     SOME text => " /*" ^ text ^ "*/"
				   | NONE => "")]))

	      | DLDeps.DiscreteStateEntry {quantity, condition=NONE, event=NONE, 
					   description, ...}  =>
		SOME ($(concat ["int ", (depname2str quantity), ";",
				(case description of 
				     SOME text => " /*" ^ text ^ "*/"
				   | NONE => "")]))

	      | _ => NONE)

	val state_struct_typedef = [$("typedef struct " ^ (state_struct_type modelname)),
				    $"{",
				    SUB (List.mapPartial state_entrydef (UniqueId.listItems env)),
				    $"};"]

	fun stateless_entrydef (depentry) =
	    (case depentry of 
		 DLDeps.EventEntry {name,arguments} =>
		 SOME ($(concat ["double ", (depname2str name), ";"]))
		 
	       | DLDeps.ScalarStateFunEntry {quantity, condition=NONE, event=NONE, 
					     description, ...} =>
		  SOME ($(concat ["double ", (depname2str quantity), ";",
				  (case description of 
				       SOME text => " /*" ^ text ^ "*/"
				     | NONE => "")]))
		 
	       | DLDeps.VectorStateFunEntry {quantity, condition=NONE, event=NONE, 
					     size, description, ...} =>
		 SOME ($(concat [(vector_t size), " ", (depname2str quantity), ";",
				 (case description of 
				      SOME text => " /*" ^ text ^ "*/"
				    | NONE => "")]))
	       | _ => NONE)


	val stateless_struct_typedef = [$("typedef struct " ^ (stateless_struct_type modelname)),
					$"{",
					SUB (List.mapPartial stateless_entrydef (UniqueId.listItems env)),
					$"};"]

	fun table_entrydef (depentry, l) =
	    (case depentry of 
		 DLDeps.TableFunEntry {quantity, low, high, step,
				description, ...} =>
		 (let val size = Real.round ((high - low) / step)
		  in
		      ($("double " ^ (depname2str quantity) ^ ";"))::
		      ($(concat ["double ", ((depname2str quantity) ^ "_datapoints"), 
				 "[2]", "[", (Int.toString size), "];",
				 (case description of 
				      SOME text => " /*" ^ text ^ "*/"
				    | NONE => "")]))::l
		  end)
	      | _ => l)


	val table_struct_typedef = [$("typedef struct " ^ (table_struct_type modelname)),
				    $"{",
				    SUB (foldl table_entrydef [] (UniqueId.listItems env)),
				    $"};"]


	fun parameter_entrydef (depentry) = 
	    (case depentry of
		 DLDeps.ScalarParEntry {quantity, description, ...} =>
		 SOME ($(concat ["double ", (depname2str quantity), ";",
				 (case description of 
				      SOME text => " /*" ^ text ^ "*/"
				    | NONE => "")]))
		 
	       | DLDeps.VectorParEntry {quantity, value, description, ...} =>
		 SOME ($(concat [vector_t (Array.length value), " ", (depname2str quantity), ";",
				 (case description of 
				      SOME text => " /*" ^ text ^ "*/"
				    | NONE => "")]))
		       
	       | _ => NONE)

	val parameter_struct_typedef = [$("typedef struct " ^ (parameter_struct_type modelname)),
					$"{",
					SUB (List.mapPartial parameter_entrydef (UniqueId.listItems env)),
					$"};"]

	val var_struct_typedef       =  [$("typedef struct " ^ (var_struct_type modelname)),
					 $"{",
					 SUB [$"char name[MAX_VAR_NAME];",
					      $"unsigned char logged;",
					      $"unsigned int size;",
					      $"union", $"{",
					      SUB [$"int *ival;",
						   $"double *dval;"],
					      $"};"],
					 $"};"]
		
	val classdef = $("class " ^ modelname ^ " : public DefaultGUIModel")

	val publicdef = 
	    [$"public:",
	     SUB [$"",
		  $(modelname ^ "(void);"),
		  $("~"^modelname^"(void);"),
		  $"",
		  $"void execute(void);",
		  $"void receiveEvent(const ::Event::Object *);",
		  $""]]

	fun integration_method_sig ({name, inputs,...}:integration_method, l) =
	    (let
		 fun makeinputs (inputs, acc) = 
		     (if inputs > 1 
		      then makeinputs (inputs-1, ("double")::acc)
		      else ("double"::acc))
		     
		 val inputs = makeinputs (inputs, [])
		 val inputvars = outputvars inputs
	     in
		 ([$("static double " ^ name ^ "(double, double, double, "^inputvars^");")] 
		  @ l)
	     end)

	val protecteddef = 
	    [$"protected:",
	    SUB ([$"", 
		  $"void update(DefaultGUIModel::update_flags_t);", 
		  $"void start(void);", 
		  $"void initialize_states(void);",
		  $"void compute_interp_datapoints (void);"] @
		 (foldl integration_method_sig [] integration_methods))]

	val setdefs = 
	    (let 
		 fun setdef (DLDeps.SetEntry{name, value}) =
		     SOME ($("static const int SET_" ^ (depname2str name) ^ "=" ^ (Int.toString value) ^ ";"))
		   | setdef _ = NONE
	     in
		 (List.mapPartial setdef (UniqueId.listItems env))
	     end)

	val vector_init_const_defs = 
	    (let
		 fun vector_init_const_def({name, desc, init}) =
		     (case init of
			  VectorValue {size, data}   => 
			  SOME ($(concat ["static const ", vector_t size, " ", 
					  (depname2str name), "_INIT", " = ",
					  "{", (int2str o length) data,
					  (listrealvals data), "}",
					  ";"]))
			| _   => NONE)
	     in
		 List.mapPartial vector_init_const_def init_table
	     end)

		
	fun table_fun_sig (depenventry) =
	    (case depenventry of 
		 DLDeps.TableFunEntry {quantity, body,
				       low, high, step,
				       argument,
				       description} =>
		 SOME ($(concat ["double ", (depname2str quantity), " (double);", 
				 (case description of 
				      SOME text => " /* " ^ text ^ "*/"
				    | NONE => "")]))
	       | _ => NONE)
		     
	val table_fun_sigs =  List.mapPartial table_fun_sig (DLDeps.query_env (env, TableFunEntryType))

	fun table_fun_constdef (depenventry, l) =
	    (case depenventry of 
		 DLDeps.TableFunEntry {quantity, body,
				       low, high, step,
				       argument,
				       description} =>
		 [$(concat ["static const double ", (depname2str quantity), "_low",  " = ", (real2str low), ";"]),
		  $(concat ["static const double ", (depname2str quantity), "_high",  " = ", (real2str high), ";"]),
		  $(concat ["static const double ", (depname2str quantity), "_step",  " = ", (real2str step), ";"]),
		  $(concat ["static const unsigned int ", (depname2str quantity), "_offset",  " = ", 
			    (if low < 0.0
			     then (concat ["(unsigned long int)((", real2str (~1.0 * low), ") / (", real2str step, "))"])
			     else ("0")), ";"]),
		  $(concat ["static const unsigned long int ", (depname2str quantity), "_dimension",  " = ", 
			    "((unsigned long int)((", real2str high, " - ", real2str low, ") / (", real2str step, ")));"])] @ l
	       | _ => l)

	val table_fun_constdefs =  foldl table_fun_constdef [] (DLDeps.query_env (env, TableFunEntryType))

	fun generic_fun_sig (depenventry) =
	    (case depenventry of
		 DLDeps.FunEntry{name, body, typeinfo} =>
		 (let
		      val {args, ret_type}: type_info' = typeinfo
							 
		      fun rettype2str (typ) =
			  (case DLTypes.singletonToType(typ) of
			       DLTypes.Scalar => "double"
			     | DLTypes.Vector _ => "void"
			     | _ => raise Unsupported)

		      fun argtype2str (typ) =
			  (case DLTypes.singletonToType(typ) of
			       DLTypes.Scalar => "double"
			     | DLTypes.Vector _ => "double *"
			     | _ => raise Unsupported)

		      fun listargtypes (nil, acc) = concat acc
			| listargtypes ({name=_, typ}::nil, acc) = concat ((argtype2str typ)::acc)
			| listargtypes ({name=_, typ}::rest, acc) = listargtypes (rest, (argtype2str typ)::", "::acc)

		  in
		      SOME ($(concat [rettype2str ret_type, 
				      " ", (depname2str name),
				      "(", (listargtypes (args, [])), ");"]))
		  end)
	       | _ => NONE)
		

	val generic_fun_sigs = List.mapPartial generic_fun_sig (DLDeps.query_env (env, FunEntryType))


	val privatedef  = 
	    [$"private:",
	    SUB ([$"",
		  $"double dt;",
		  $"",
		  $("struct " ^ (event_struct_type modelname)  ^ " " ^ event_struct ^ ";"),
		  $("struct " ^ (state_struct_type modelname)  ^ " " ^ prev_state_struct ^ ";"),
		  $("struct " ^ (state_struct_type modelname)  ^ " " ^ next_state_struct ^ ";"),
		  $("struct " ^ (stateless_struct_type modelname)  ^ " " ^ stateless_struct ^ ";"),
		  $("struct " ^ (table_struct_type modelname)  ^ " " ^ table_struct ^ ";"),
		  $("struct " ^ (parameter_struct_type modelname)  ^ " " ^ parameter_struct ^ ";"),
		  $""] @
		 ($""::setdefs) @
		 ($""::table_fun_sigs) @
		 ($""::table_fun_constdefs) @
		 ($""::generic_fun_sigs))]
		 

	val body = header @
		   [$""] @
		   [vector_t_macro_def] @
		   vector_typedefs @
		   ($""::vector_init_const_defs) @
		   [$""] @
		   var_constants @
		   [$""] @
		   [shm_name] @
		   [$""] @
		   event_struct_typedef @
		   [$""] @
		   state_struct_typedef @
		   [$""] @
		   stateless_struct_typedef @
		   [$""] @
		   table_struct_typedef @
		   [$""] @
		   parameter_struct_typedef @
		   [$""] @
		   var_struct_typedef @
		   [$""] @
		   [$"", $""] @
		   [classdef,
		    $"{",
		    SUB (publicdef @
			 protecteddef @
			 privatedef),
		    $"};", $""] @
		   math_builtins @
		   [$""] @
		   vector_builtins

    in
	printprog (outstream, body, 0)
    end

fun varflags env ({name, desc, init}) = 
    let
	val input      = "DefaultGUIModel::INPUT"
	val output     = "DefaultGUIModel::OUTPUT"
	val parameter  = "DefaultGUIModel::PARAMETER"
	val state      = "DefaultGUIModel::STATE"
	val event      = "DefaultGUIModel::EVENT"
	val vector     = "DefaultGUIModel::VECTOR"
	val integer    = "DefaultGUIModel::INTEGER"
	val double     = "DefaultGUIModel::DOUBLE"

	val typ = (case init of VectorValue _  => vector
			      | ScalarValue _  => double
			      | IntegerValue _ => integer
			      | SetValue x     => integer)

	val var = (case name of
		       DLDeps.NEW {id=n,...} => 
		       (let val depenventry = valOf(UniqueId.look(env,n))
			in 
			    case depenventry of
				DLDeps.ExternalStateEntry {direction=DLAst.INPUT, ...}  => input  ^ " | " ^ typ	
			      | DLDeps.ExternalStateEntry {direction=DLAst.OUTPUT, ...} => output ^ " | " ^ typ
			      | _ => state ^ " | " ^ typ
		       end)
		     | DLDeps.STATELESS _  => state 
		     | DLDeps.TABLE _      => state 
		     | DLDeps.PARAM _      => parameter ^ " | " ^ typ
		     | DLDeps.EVENT _      => event 
		     | DLDeps.EVENTARG _   => state 
		     | _                   => raise InvalidInitEntry)
    in
	var
    end

fun output_table_constdefs (outstream, modelname, init_table, env) =
    let
	fun table_fun_constdef (depenventry, l) =
	    (case depenventry of 
		 DLDeps.TableFunEntry {quantity, body,
				       low, high, step,
				       argument,
				       description} =>
		 ([$(concat ["const double ", modelname, "::",
			     (depname2str quantity), "_low",    ";"]),
		   $(concat ["const double ", modelname, "::",
			     (depname2str quantity), "_high",   ";"]),
		   $(concat ["const double ", modelname, "::",
			     (depname2str quantity), "_step",   ";"]),
		   $(concat ["const unsigned int ", modelname, "::",
			     (depname2str quantity), "_offset", ";"]), 
		   $(concat ["const unsigned long int ", modelname, "::",
			     (depname2str quantity), "_dimension",  ";"])] @ l)
	       | _ => l)

	val table_fun_constdefs =  foldl table_fun_constdef [] (DLDeps.query_env (env, TableFunEntryType))
    in 
	printprog (outstream, table_fun_constdefs, 0)
    end


fun output_vars (outstream, modelname, init_table, env) = 
    let
	
	fun vardef (x as {name=DLDeps.EVENTARG _, desc, init}, ax) = ax
	  | vardef (x as {name, desc, init}, ax) = 
	    [$("{"),
	     SUB ([$("\"" ^ (depname2str name) ^ "\"" ^ ", "),
		   $("\"" ^ (case desc of SOME s => s | NONE => "") ^ "\"" ^ ", "),
		   $(varflags env x)]),
	     $("}, ")] @ ax

	val body = [$("static DefaultGUIModel::variable_t vars[] = "),
		    $("{"),
		    SUB(foldl vardef [] init_table),
		    $("};")]
    in 
	printprog (outstream, body, 0)

    end

fun output_num_vars (outstream, modelname, init_table) = 
    let
	val num_vars = length (List.filter (fn({name=DLDeps.EVENTARG _,...}) => false
					    | _ => true) init_table)
	val body = [$("static size_t num_vars = " ^ (Int.toString num_vars)  ^ ";")]
    in 
	printprog (outstream, body, 0)
    end

fun output_constructor (outstream, modelname, init_table) =
    let
	val fname = modelname

(*
 fun initdefs ({name, desc, init}::rest, acc) =
     let 
	 val acc = (if List.null(rest) then "" else ", ")::
		   ((case init of
			 VectorValue _  => (depname2str name) ^ "(" ^ 
		       | ScalarValue _  => (depname2str name) ^ "(" ^ 
		       | IntegerValue _ => (depname2str name) ^ "(" ^ 
		       | SetValue x     => (depname2str name) ^ "(" ^ 
     in
	 initdefs (rest, acc)
     end
   | initdefs (nil, acc) = concat (rev acc)
	*)

	val head = String.concat [modelname, "::", fname, " (void) ", 
				  " : DefaultGUIModel(", "\"", modelname,"\"", 
				  ", ", "::vars", ", ", "::num_vars", ")"]

	val body = [$ head,
		    $"{",
		    SUB (
			 [$("DEBUG_MSG(\""^modelname^"::"^fname^": starting\\n\");"),
			  $(""),
                          $("createGUI(vars, num_vars);"),
			  $("update(INIT);"),
			  $("refresh();"),
			  $("setActive(true);"),
			  $(""),
			  $("DEBUG_MSG(\""^modelname^"::"^fname^": complete\\n\");")]),
		    $"}"]

    in
	printprog (outstream, body, 0)
    end


fun output_destructor (outstream, modelname, init_table) =
    let
	val body = [$(modelname^"::"^"~"^modelname^"(void)"),
		    $"{", 
		    SUB[$"setActive(false);"],
		    $"}"]
    in
	printprog (outstream, body, 0)
    end


fun output_initialize_states (outstream, modelname, init_table) =
    let
	val header = 
	    [$"/* Initialize the states of the system. */",
	     $("inline void "^ modelname ^"::initialize_states (void)")]

	val rindex = "__rindex"

	fun init_entry({name, desc, init}, l) =
	    ([(case desc of
		   SOME text => $("/* " ^ text ^ " */")
		 | NONE      => $"")] @
	     (case init of
		  VectorValue _  => [$("{"),
				     SUB [$("int "^rindex^";"),
					  $(concat [depstorage (name, SOME "0"), " = ", (depname2str name), "_INIT[0];"]),
					  $(concat ["for(", rindex, " = 1; ", rindex, " <= " ,
						    (depname2str name), "_INIT[0]; ", rindex, "++)"]),
					  $"{",
					  SUB [$(concat [depstorage (name, SOME rindex), " = ", 
							 depname2str name, "_INIT[", rindex, "];"])],
					  $"}"],
				     $("}")]
		| ScalarValue x  => [$(concat [depstorage (name,NONE), " = ", real2str x, ";"])]
		| IntegerValue x => [$(concat [depstorage (name,NONE), " = ", int2str x, ";"])]
		| SetValue x     => [$(concat [depstorage (name,NONE), " = SET_", Symbol.name x, ";"])]) @
	      [$""] @ l)

	val body = header @ [$"{", SUB ([$("dt = RT::System::getInstance()->getPeriod() / 1e6;")] @
					(foldl init_entry [] init_table) @ 
					[$"", $"compute_interp_datapoints();", $"start();"]), $"}"]
    in
	printprog (outstream, body, 0)
    end


fun output_update (outstream, modelname, init_table) =
    let
	fun setstate ({name, desc, init},(i,ax)) =
	    (case name of 
		 DLDeps.NEW  _ => 
		 (case init of 
		      VectorValue _   => (i,ax)
		    | IntegerValue _  => (i,ax)
		    | SetValue _      => (i,ax)
		    | _  => 			 
		      (i+1,
		       ($(concat ["setState", "(", "\"", depname2str name, "\"", ", ",
				  "(", depstorage (name, NONE), ")", ");"])) :: ax))

	       | DLDeps.STATELESS  _ => 
		 (case init of 
		      VectorValue _   => (i,ax)
		    | IntegerValue _  => (i,ax)
		    | SetValue _      => (i,ax)
		    | _  => 			 
		      (i+1,
		       ($(concat ["setState", "(", "\"", depname2str name, "\"", ", ",
				  "(", depstorage (name, NONE), ")", ");"])) :: ax))

	       | _               => (i,ax))

	fun setevent ({name, desc, init},(i,ax)) =
	    (case name of 
		 DLDeps.EVENT  _ => 
		 (case init of 
		      VectorValue _   => (i,ax)
		    | IntegerValue _  => (i,ax)
		    | SetValue _      => (i,ax)
		    | _  => 			 
		      (i+1,
		       ($(concat ["setEvent", "(", "\"", depname2str name, "\"", ", ",
				  "(", depstorage (name, NONE), ")", ");"])) :: ax))

	       | _               => (i,ax))

	fun setparam ({name, desc, init}, l) =
	    (case name of 
		 DLDeps.PARAM _ => 
		 (($(concat ["setParameter", "(\"", (depname2str name), "\", ", 
			     depstorage (name, NONE), ");"])) :: l)
	       | _ => l)
	    
	fun getvalue ({name, desc, init},(i,ax)) =
	    (case name of 
		 DLDeps.PARAM _ => 
		 (i+1,
		  ($(concat [depstorage (name, NONE), " = ", 
			     "getValue", "(",
			     "Workspace::PARAMETER", ", ",
			     Int.toString i, ");"])) :: ax)

	       | DLDeps.NEW  _ => 
		 (case init of 
		      VectorValue _   => (i,ax)
		    | IntegerValue _  => (i,ax)
		    | SetValue _      => (i,ax)
		    | _  => 			 
		      (i+1,
		       ($(concat [depstorage (name, NONE), " = ", 
				  "getValue", "(",
				  "Workspace::STATE", ", ", 
				  Int.toString i, ");"])) :: ax))

	       | DLDeps.STATELESS  _ => 
		 (case init of 
		      VectorValue _   => (i,ax)
		    | IntegerValue _  => (i,ax)
		    | SetValue _      => (i,ax)
		    | _  => 			 
		      (i+1,
		       ($(concat [depstorage (name, NONE), " = ", 
				  "getValue", "(",
				  "Workspace::STATE", ", ",
				  Int.toString i, ");"])) :: ax))

	       | DLDeps.EVENT  _ => 
		 (case init of 
		      VectorValue _   => (i,ax)
		    | IntegerValue _  => (i,ax)
		    | SetValue _      => (i,ax)
		    | _  => 			 
		      (i+1,
		       ($(concat [depstorage (name, NONE), " = ", 
				  "getValue", "(",
				  "Workspace::EVENT", ", ",
				  Int.toString i, ");"])) :: ax))

	       | _               => (i,ax))

	fun getparam({name, desc, init}, l)= 
	    (case name of 
		 DLDeps.PARAM _ =>
		 (case init of 
		      VectorValue {size, data=_} =>
		      [$"{",
		       SUB [$"double *lst;",
			    $"",
			    $(concat ["lst = getParameter ", "(\"", (depname2str name), "\");"]),
			    $(concat ["memcpy (", (depstorage (name, NONE)), ", ", "lst", ", ", (Int.toString (size+1)), ");"]),
			    $"delete lst;"],
		       $"}"] @ l
		    | IntegerValue _  => 
		      ($(concat [depstorage (name, NONE), " = ", 
				 "getParameter", "(\"", (depname2str name), "\").toInt();"]))::l
		    | SetValue _      => 
		      ($(concat [depstorage (name, NONE), " = ", 
				 "getParameter", "(\"", (depname2str name), "\").toInt();"]))::l
		    | ScalarValue _   => 
		      ($(concat [depstorage (name, NONE), " = ", 
			    "getParameter", "(\"", (depname2str name), "\").toDouble();"]))::l)
	       | _ => l)

	val body = [$("void "^modelname ^"::"^"update(DefaultGUIModel::update_flags_t flag)"),
		    $"{",
		    SUB [$"switch (flag)",
			 $"{",
			 SUB [$"case INIT:",
			      SUB ([$"initialize_states ();",
				    $"compute_interp_datapoints ();"] @
				   (let val (_,ax) = foldr setstate (0,[]) init_table 
				    in ax end)  @
				   (let val (_,ax) = foldr setevent (0,[]) init_table 
				    in ax end)  @
				   (let val ax = foldr setparam [] init_table 
				    in ax end)  @
				   [$"break;"]),
			      $"case MODIFY:",
			      SUB ((let val (_,ax) = foldr getvalue (0,[]) init_table 
				    in ax end)  @
				   [$"break;"]),
			      $("case PERIOD:"),
			      SUB ([$("dt = RT::System::getInstance()->getPeriod() / 1e6;"),
				    $("compute_interp_datapoints ();"),
				    $("break;")]),
			      $"default: break;"],
			 $"}"],
		    $"}"]
    in
	printprog (outstream, body, 0)
    end



fun output_table_funs (outstream, modelname, env: depenv) =
    let
	val items = DLDeps.query_env (env, TableFunEntryType)

	fun tablefun (depenventry, l) =
	    (case depenventry of 
		 DLDeps.TableFunEntry {quantity, body, low,
				       high, step, argument, 
				       description} =>
		 let
 		     val {body, vector_vars, 
			  scalar_vars, return_vars, 
			  return_type} =  (output_indepexp indepstorage) (body, env, NONE)
		 in
		     [$"/* Table function */",
		      $(concat ["inline double ",
				modelname ^"::"^(depname2str quantity),
				" (double ", (depname2str argument), ")"]),
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
		      $"}",
		      $""] @ l
		     
		 end
	       | _ => l)

	val body = foldl tablefun [] items
    in
	printprog (outstream, body, 0)
    end


fun output_interp_datapoints (outstream, modelname, env) = 
    let
	val fname = "compute_interp_datapoints"
	val items = DLDeps.query_env (env, TableFunEntryType)

	fun table_fun_datapoints (depenventry, l) =
	    (case depenventry of
		 DLDeps.TableFunEntry {quantity, body,
				       low, high, step,
				       argument,
				       description} =>
		 [$"",
		  $("/* Interpolation datapoints for function " ^ (depname2str quantity) ^ " */"),
		  $("table_i = 0;"),
		  $"{",
		  SUB [$(concat ["double arg; ", "double low = " ^ (real2str low) ^ "; ",
				 "double high = " ^ (real2str high) ^ "; ", "double step = " ^ (real2str step) ^ "; "]),
		       $"for (arg = low; arg < high; arg += step)",
		       $"{",
		       SUB [$((depstorage (quantity, NONE)) ^"_datapoints" ^ "[0][table_i] = arg;"),
			    $((depstorage (quantity, NONE)) ^"_datapoints" ^ "[1][table_i] = " ^ (depname2str quantity) ^ "(arg);"),
			    $"table_i++;"],
		       $"}"],
		  $"}"] @ l
	       | _ => l)

	val body = 
	    [$("void " ^ modelname^"::"^ fname ^" (void)"),
	     $"{",
	     SUB ([$("DEBUG_MSG(\""^modelname^"::"^fname^": starting\\n\");")] @
		  (if (length items)  > 0
		   then ($"int table_i;")::(foldl table_fun_datapoints [] items)
		   else []) @
		  [$("DEBUG_MSG(\""^modelname^"::"^fname^": complete\\n\");")]),
	     $"}"]
    in
	printprog (outstream, body, 0)
    end



fun output_roundoff_interp (env, entry) =
    (case entry of 
	 DLDeps.TableFunEntry {quantity, body, low, high, step,
			       argument, description} =>
	 let
	     val name = depname2str quantity
	     val value = depstorage (quantity, NONE)
	     val x_data = (depstorage (quantity, NONE)) ^ "_datapoints[0]"
	     val y_data = (depstorage (quantity, NONE)) ^ "_datapoints[1]"
	 in
	     [$"{",
	      SUB [$(concat ["unsigned int p; ", 
			     "double x = " ^ (depstorage (argument, NONE)) ^ "; "]),
		   $(concat ["const double invstep = " ^ (real2str (1.0 / step)) ^ "; "]),
		   
		   $"",
		   $("if (x <= "^name^"_low) "^ value ^ " = " ^y_data^"[0]; else"),
		   SUB [$("if (x >= "^name^"_high) "^ value ^ " = " ^y_data^"["^name^"_dimension-1]; else"),
			$"{",
			SUB [$("p = (unsigned int)(round(x * invstep)) + " ^ name ^ "_offset;"),
			     $("if (p < "^name^"_dimension)"),
			     SUB [$(value ^ " = "^y_data^"[p];")],
			     $"else",
			     SUB [$(value ^ " = "^y_data^"["^name^"_dimension-1];")]],
			$"}"]],
	      $"}"]
	 end
       | _ => raise InvalidEntry)

fun output_generic_funs(outstream, modelname, depenv) =
    let
	val items = DLDeps.query_env (depenv, FunEntryType)

	val rindex = "__rindex"

	fun output_generic_fun (depenventry, l) =
	    case depenventry of
		DLDeps.FunEntry{name, body, typeinfo} => 
		let
		    val {args, ret_type}: type_info' = typeinfo
						       
		    fun rettype2str (typ) =
			case DLTypes.singletonToType(typ) of
			    DLTypes.Scalar => "double"
			  | DLTypes.Vector _ => "void"
			  | _ => raise Unsupported

		    fun argtype2str (typ) =
			case DLTypes.singletonToType(typ) of
			    DLTypes.Scalar => "double"
			  | DLTypes.Vector _ => "double *"
			  | _ => raise Unsupported
					
		    fun arg2str ({name, typ}) = (argtype2str typ) ^ " " ^ (name'2str name)

		    fun listargs (nil, acc) = concat acc
		      | listargs (arg::nil, acc) = concat ((arg2str arg)::acc)
		      | listargs (arg::rest, acc) = listargs (rest, (arg2str arg)::", "::acc)

		    val {body, vector_vars, scalar_vars,
			 return_vars, return_type} =
			(output_indepexp indepstorage) (body, depenv, SOME typeinfo)
					
		    val rv = hd return_vars

		    val ret_name = {name=Symbol.symbol ((depname2str name) ^ "_result"), 
				    id=idofdepname name}

		    val ret_arg = {name=ret_name, typ=ret_type}

		    val args = case DLTypes.singletonToType(ret_type) of
				   DLTypes.Scalar => args
				 | (DLTypes.Vector _) => ret_arg::args
				 | _ => raise Unsupported
		in
		    [$"/* Generic function */",
		     $(concat ["inline ", (rettype2str ret_type),
			       " ", modelname^"::"^(depname2str name), "(", listargs (args, []), ")"]), 
		       $"{",
		       SUB ([$(case DLTypes.singletonToType(ret_type) of
				   DLTypes.Vector _ => "int " ^ rindex ^ ";"
				 | DLTypes.Scalar => ""
				 | _ => raise Unsupported)] @
			    (map (fn(s) => $("double " ^ s ^ ";")) scalar_vars) @
			    (map (fn(s) => $(vector_t_macro ^ "(" ^ s ^ ");")) vector_vars) @
			    [$("")] @
			    body @
			    (case DLTypes.singletonToType ret_type of
				 DLTypes.Scalar => [$("return " ^ rv ^ ";")]
			       | (DLTypes.Vector _) => 
				 [$(concat ["for (", rindex, " = 0; ", rindex, " <= " ^ rv ^ "[0]; ", rindex, "++)"]),
				  $"{",
				  SUB[$(concat [depname2str name, "_result[", rindex, "] = ",
						rv, "[", rindex, "];"])],
				  $"}"]
			       | _ => raise Unsupported)),
		       $"}",
		       $""] @ l
		end
	      | _ => l

	val body = foldl output_generic_fun [] items
    in
	printprog (outstream, body, 0)
    end

fun output_integration_funs (outstream, modelname, methods: integration_method list) =
let
    fun integration_fun ({name=name, documentation=_, inputs=inputs,
				 invocation=_, body=body,
				 global_setup_code=_}:integration_method, l: progline list) =
	let
	    fun makeinputs (inputs, acc) = 
		(if inputs > 1 
		 then makeinputs (inputs-1, ("y"^(Int.toString inputs))::acc)
		 else ("y1"::acc))
		
	    val inputs = makeinputs (inputs, [])
	    val inputvars = outputvars inputs
	    val inputargs = outputvars (map (fn(s) => "double " ^ s) inputs)
	in
	    [$"",
	     $("/* Definitions used by integration method " ^ name ^ " */"),
	     $("inline double " ^ modelname ^ "::" ^ name ^ "(double x, double p, double dt, "^inputargs^")"),
	     $"{",
	     SUB ((body ("x", "p", "dt", inputs)) @ [$"return x;"]),
	     $"}"] @ l
	end

    val body = foldl integration_fun [] integration_methods
in
    printprog (outstream, body, 0)
end

fun output_external_input (assignments: depname list ref, quantity, channel) =
    (if (List.exists (fn(dn) => (idofdepname quantity) = (idofdepname dn)) (!assignments))
     then []
     else [$((depstorage (quantity, NONE)) ^ " = input ("^(Int.toString channel)^");")]
	  before (assignments :=  (quantity::(!assignments))))

fun output_external_output (assignments: depname list ref, quantity, channel) =
     ([$("output(" ^ (Int.toString channel) ^ ")" ^ " = " ^ (depstorage (quantity, NONE)) ^ ";")]
      before (assignments :=  (quantity::(!assignments))))


fun output_receiveEvent (outstream, modelname, env: depenv) =
    let
	fun make_event_handler (entry) = 
	    (case entry of
		 DLDeps.EventEntry {name, arguments} => 
		 SOME (SUB [$("if (x->getName() == \"" ^ (depname2str name) ^ "\")"),
			    $"{",
			    SUB (($(String.concat [depstorage(name,NONE), " = 1.0;"]))::
				 (map (fn(x) => 
					 $((String.concat [(depstorage(x,NONE))," = ", 
							   "*(double*)(x->getParam (\"", depname2str x, "\"));"])))
				      arguments)),
			    $"}"])
	       | _ => NONE)

	val handlers = List.mapPartial make_event_handler (UniqueId.listItems env)

	val body = [$("void " ^ modelname ^ "::receiveEvent(const ::Event::Object *x)"),
		    $"{",
		    SUB (($"Event::Handler::receiveEvent(x);") :: handlers),
		    $"}"]
    in
	printprog (outstream, body, 0)
    end
     

(* Output the state updating function *)
fun output_time_block (g, env) =
    let
	val times   = DLDeps.query_env (env, TimeEntryType)
	val time    = DLDeps.nameofentry (hd times)
	val levels  = EqnGraph.color g


	val external_input_state = {func=output_external_input, assignments=ref []}
	val external_output_state = {func=output_external_output, assignments=ref []}

	fun clear_event (entry) = 
	    (case entry of
		 DLDeps.EventEntry {name, arguments} => 
		 SOME ($(String.concat [depstorage(name,NONE), " = 0;"]))
	       | _ => NONE)

    in
	[$"{",
	 SUB ([$"",
	       (* copies the state vector into old_state *)
	       $(prev_state_struct ^ " = " ^ next_state_struct ^ ";")] @

	      (* system equations *)
	      (foldl (fn((ids), l) => 
			    ((foldl (fn (id, l) =>
					(let
					     val entry = valOf(UniqueId.look (env, id))
					 in
					     (output_eq (depstorage, "dt", output_roundoff_interp,
							 external_input_state, external_output_state)) (env, entry)
					 end) @ l)
				    [] ids) @ l))
		     [] levels) @
	      (List.mapPartial clear_event (UniqueId.listItems env))),
	 $"}"]
    end


fun output_execute (outstream, modelname, g, env) =
    let

	val body = ($("void "^modelname ^"::"^"execute(void)")) ::
		   (output_time_block (g, env))
    in
	printprog (outstream, body, 0)
    end


fun output_start (outstream, modelname, g, env) =
    let
	val body = $("void "^modelname ^"::"^"start(void)")::
		   (case (g, env) of 
			(SOME g, SOME env) => output_time_block (g, env)
		      | _ => [$"{}"])
    in
	printprog (outstream, body, 0)
    end


fun main(dir, modelname, outputname, inline_c, env, startenv, envgraph, startenvgraph) =

    let
	val (init_table, count) = make_init_table env

	val hpp_stream  = TextIO.openOut(OS.Path.joinDirFile {dir=dir, file=outputname ^ ".h"})
	val cpp_stream  = TextIO.openOut(OS.Path.joinDirFile {dir=dir, file=outputname ^ ".cpp"})

	val sayln = sayln (say cpp_stream)
	val preamble = 
	    [$"/*"] @
	    comment_header @
	    [$"*/",
	     $"",
	     $("#include \""^outputname^".h\""),
	     $"",
	     $"#include <unistd.h>",
	     $"#include <sys/types.h>",
	     $"#include <sys/mman.h>",
	     $"#include <sys/stat.h>",
	     $"#include <fcntl.h>",
	     $"#include <signal.h>",
	     $"#include <errno.h>",
	     $"//#include <rtai_shm.h>",
	     $"",
	     $"extern \"C\" Plugin::Object *createRTXIPlugin(void)",
	     $"{",
	     SUB [$("return (new "^modelname^"());")],
	     $"}",
	     $""]

    in
	output_defs(hpp_stream, modelname, init_table, env);
	TextIO.closeOut(hpp_stream);

	printprog (cpp_stream, preamble, 0);

	output_table_constdefs (cpp_stream, modelname, init_table, env);
	printprog (cpp_stream, [$"", $""], 0);

	output_vars (cpp_stream, modelname, init_table, env);
	printprog (cpp_stream, [$"", $""], 0);

	output_num_vars (cpp_stream, modelname, init_table);
	printprog (cpp_stream, [$"", $""], 0);

	output_constructor (cpp_stream, modelname, init_table);
	printprog (cpp_stream, [$"", $""], 0);

	output_destructor (cpp_stream, modelname, init_table);
	printprog (cpp_stream, [$"", $""], 0);

	output_initialize_states (cpp_stream, modelname, init_table);
	printprog (cpp_stream, [$"", $""], 0);

	output_update (cpp_stream, modelname, init_table);
	printprog (cpp_stream, [$"", $""], 0);

	output_table_funs (cpp_stream, modelname, env);
	printprog (cpp_stream, [$"", $""], 0);

	output_interp_datapoints (cpp_stream, modelname, env);
	printprog (cpp_stream, [$"", $""], 0);

	output_generic_funs(cpp_stream, modelname, env);
	printprog (cpp_stream, [$"", $""], 0);

	output_integration_funs(cpp_stream, modelname, integration_methods);
	printprog (cpp_stream, [$"", $""], 0);

	output_receiveEvent(cpp_stream, modelname, env);
	printprog (cpp_stream, [$"", $""], 0);

	output_start (cpp_stream, modelname, startenvgraph, startenv);
	printprog (cpp_stream, [$"", $""], 0);

	output_execute (cpp_stream, modelname, envgraph, env);
	printprog (cpp_stream, [$"", $""], 0);

	TextIO.closeOut(cpp_stream)
    end

end

 
