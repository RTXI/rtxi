 
(* Generic C and C++ backend *)

functor CBackend (val compilerid: string
		  structure EqnGraph: DL_EQNGRAPH): DL_BACKEND = 
struct

open DLTypes


type name'        = DLDeps.name'
type type_info'   = DLDeps.type_info'
type depname      = DLDeps.depname
type depenv       = DLDeps.depenv
type depexp       = DLDeps.depexp
type indepexp     = DLDeps.indepexp
type depenventry  = DLDeps.depenventry
type depcondition = DLDeps.depcondition
type indepname      = DLInDeps.indepname

type eqngraph     = (depenventry, unit, unit) Graph.graph

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


datatype progline = SUB of progline list | $ of string

type integration_method = {name: string,
			   documentation: string,
			   inputs: int,
			   invocation: string * string * string * string list -> progline,
			   body: string * string * string * string list -> progline list,
			   global_setup_code: progline list}

type depstorage = depname * string option -> string
type indepstorage = indepname * string option -> string

type exp_info = {body: progline list,
		 vector_vars: string list,
		 scalar_vars: string list,
		 return_vars: string list,
		 return_type: dl_type}


type external_state = {assignments: depname list ref, 
		       func: (depname list ref * depname * int -> progline list)}

val vector_t = "VECTOR_t"     


val indent_factor = 4

val say =
    (fn(outstream) => (fn (str) => TextIO.output(outstream, str)))
    
val sayln = 
    (fn(say) => (fn (str) => (say str; say "\n")))

val ind = (fn(say) => (fn(n) => let fun ind' (n) = if n <= 0 then () 
	     else (say " "; ind' (n-1)) in ind' (n) end)) 
    
val indent = 
    (fn(ind) => (fn(indent_factor) => (fn(n) => ind(n * indent_factor))))


fun symofindepname(name) = DLEnv'.symofname (DLInDeps.nameofindepname name)
fun symofdepname(name) = DLEnv'.symofname (DLDeps.nameofdepname name)
fun idofdepname (name) = DLEnv'.idofname (DLDeps.nameofdepname name)
fun vector2list v = Array.foldl (fn (i, l) => i::l) nil v

fun printprog (outstream, prog, i) =
    let
	val say = say (outstream)
	val sayln = sayln (say)
	val ind = ind (say)
	val indent = indent (ind) (indent_factor)
    in
	app (fn(p) => 
	       (case p of 
		    SUB prog => printprog (outstream, prog, i+1)
		  | $ line => (indent i; sayln line)))
	    prog
    end

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
       | DLDeps.EVENT n  => name'2str n
       | DLDeps.EVENTARG(n,_)  => name'2str n
       | DLDeps.SETELM n  => name'2str n)
    

fun int2str (i) = let fun minus (i) = if i < 0 then "-" else ""
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

fun printprog (outstream, prog, i) =
    let
	val say = say (outstream)
	val sayln = sayln (say)
	val ind = ind (say)
	val indent = indent (ind) (indent_factor)
    in
	app (fn(p) => 
	       (case p of 
		    SUB prog => printprog (outstream, prog, i+1)
		  | $ line => (indent i; sayln line)))
	    prog
    end


fun list (pelem,sep) l = 
    (let
	 fun pl' ax [x]     = rev ((pelem x) :: ax)
	   | pl' ax (x::r)  = pl' (sep :: (pelem x) :: ax) r
	   | pl' ax []      = rev ax
     in
	 (pl' [] l)
     end)

fun strlist sep (vars) = concat (list (fn(x)=>x,sep) vars)

fun binop (x,oper,y) = concat [x,oper,y]

fun fcall (f, args) = concat [f, "(", strlist "," args, ")"]

fun asgn (l, r) = concat [l, " = ", r]
		  
fun vec_asgn storage (i, v, name) =
    [$(concat ["for (", i, " = 0; ", i, " < ", (storage (name, SOME "0")), "; ", i, "++) "]),
     $"{",
     SUB [$(concat [v, "[(int)(", i, ")] = ", (storage (name, SOME ("(int)("^i^")"))), ";"])],
     $"}"]

fun output_vecdecls (vars) = map (fn(v) => $(vector_t ^ "(" ^ v ^ "); ")) vars


fun type_table (depenv: depenv) =
    let
	fun insert_type (depenventry, table) =
	    let 
		fun enter sym typ =
		    Symbol.enter (table, sym, typ)
		    
		val length = Array.length
		val length2 = Array2.dimensions
	    in
		case depenventry of
	            DLDeps.TimeEntry {quantity, ...} => 
		    enter (symofdepname quantity) TimeEntryType
	 
		  | DLDeps.ScalarParEntry {quantity, ...} =>
		    enter (symofdepname quantity) ScalarParEntryType
	 
		  | DLDeps.VectorParEntry {quantity, value, ...} =>
		    enter (symofdepname quantity) (VectorParEntryType (length value))
	 
		  | DLDeps.ScalarStateEntry {quantity, ...} =>
		    enter (symofdepname quantity) ScalarStateEntryType
		    
		  | DLDeps.VectorStateEntry {quantity, initval, ...} => 
		    enter (symofdepname quantity) (VectorStateEntryType (length initval))
	 
		  | DLDeps.IntegerStateEntry {quantity, ...} =>
		    enter (symofdepname quantity) IntegerStateEntryType
	 
		  | DLDeps.ExternalStateEntry {quantity,  ...} =>
		    enter (symofdepname quantity) ExternalStateEntryType

		  | DLDeps.DiscreteStateEntry {quantity, ...} =>
		    enter (symofdepname quantity) DiscreteStateEntryType

		  | DLDeps.VectorStateFunEntry {quantity, size, ...} =>
		    enter (symofdepname quantity) (VectorStateFunEntryType size)

	 	  | DLDeps.ScalarStateFunEntry {quantity,  ...} =>
		    enter (symofdepname quantity) ScalarStateFunEntryType

		  | DLDeps.TableFunEntry {quantity, ...} =>
		    enter (symofdepname quantity) TableFunEntryType
		    
		  | DLDeps.FunEntry {name, ...} =>
		    enter (symofdepname name) FunEntryType
		    
		  | DLDeps.FunAlias {name, ...} =>
			enter (symofdepname name) FunAliasType

		  | DLDeps.BuiltinFunEntry {name, ...} =>
		    enter (symofdepname name) BuiltinFunEntryType

		  | DLDeps.BuiltinScalar {name, ...} =>
		    enter (symofdepname name) BuiltinScalarType
		    
		  | DLDeps.ForeignScalar {name, ...} =>
		    enter (symofdepname name) ForeignScalarType
		    
		  | DLDeps.SetEntry {name, ...} =>
		    enter (symofdepname name) SetEntryType
		    
		  (* ignore DLDeps.ConditionEntry since we should never be 
		   * looking it up by name since it has none *)
		  | DLDeps.ConditionEntry _ => table

		  | DLDeps.EventEntry _ => table
	    end
    in
	foldl insert_type Symbol.empty (UniqueId.listItems depenv)
    end


fun scalar_binop2c (oper)    = 
    case oper of 
	DLAst.S_PLUS    => " + "
      | DLAst.S_MINUS   => " - "
      | DLAst.S_TIMES   => " * "
      | DLAst.S_DIVIDE  => " / "
      | DLAst.S_MODULUS => " % "
      | DLAst.S_POWER   => " pow "


fun relop2c (DLAst.EQ) = " == "
  | relop2c (DLAst.GT) = " > "
  | relop2c (DLAst.LT) = " < "
  | relop2c (DLAst.GE) = " >= "
  | relop2c (DLAst.LE) = " <= "
  | relop2c (DLAst.NE) = " != "
  | relop2c (DLAst.AND) = " && "
  | relop2c (DLAst.OR)  = " || "

fun unary2c (DLAst.UMINUS) = " - "
  | unary2c (DLAst.UPLUS)  = " + "
  | unary2c (DLAst.NOT)    = " ! "

val v_count = ref 0
val s_count = ref 0

fun v_str () = "v" ^ (int2str (!v_count)) before (v_count := (!s_count) + 1)
fun s_str () = "s" ^ (int2str (!s_count)) before (s_count := (!s_count) + 1)
fun fst (l) = (hd l)
fun snd (l) = (hd (tl l))


val null_method: integration_method =
    {name              =   "", 
     inputs            =   0,
     documentation     =   "", 
     invocation        =   (fn(x,p,dt,y)=> $""), 
     body              =   (fn(x,p,dt,y)=> []),
     global_setup_code =   []}

val euler_method: integration_method =
    {name              =   "euler", 
     inputs            =   1,
     documentation     =   "Standard Euler", 
     invocation        =   (fn(x,p,dt,y) => $((asgn(x,fcall("euler",[x,p,dt,fst y]))) ^ ";")),
     body              =   (fn(x,p,dt,y) => [$((asgn(x,binop(p,"+",binop(dt,"*",fst y)))) ^ ";")]),
     global_setup_code =   []}

val mau_method: integration_method = 
    {name              =   "mau",
     inputs            =   2,
     documentation     =   "Multiply-Add-Update",
     invocation        =   (fn(x,p,dt,y) => $((asgn(x,fcall("mau",[x,p,dt,fst y,snd y]))) ^ ";")),
     body              =   (fn(x,p,dt,y) => [$((asgn(x,binop(binop(p,"*",hd y),"+",snd y))) ^ ";")]),
     global_setup_code =   []}

     
val integration_methods = [euler_method, mau_method]


fun  output_indepexp indepstorage (e, depenv, funtypeinfo) =
    let
	val vec_asgn = vec_asgn indepstorage

	fun symofdepnameindep (DLInDeps.STATELESS {name=name, ...}) = name
	  | symofdepnameindep (DLInDeps.FUNARG {name=name,...}) = name
	  | symofdepnameindep (DLInDeps.BUILTIN {name=name,...}) = name
	  | symofdepnameindep (DLInDeps.TABLE {name=name, ...}) = name
	  | symofdepnameindep (DLInDeps.PARAM {name=name, ...}) = name
	
	val typetable = type_table depenv
		     

	fun args (h::nil) = exp(h) 
	  | args (h::r) = 
	    let
		val {body  = body1,
		     vector_vars = vv1,
		     scalar_vars = sv1,
		     return_vars = rv1,
		     return_type = rt1} = exp(h)

		val {body  = body2,
		     vector_vars = vv2,
		     scalar_vars = sv2,
		     return_vars = rv2,
		     return_type = rt2} = args (r)
	    in
		{body  = body1 @ body2, 
		 vector_vars = vv1 @ vv2, 
		 scalar_vars = sv1 @ sv2, 
		 return_vars = rv1 @ rv2,
		 return_type = rt1}
	    end

	  | args (nil) = 
	    {body = [],
	     vector_vars = [], 
	     scalar_vars = [], 
	     return_vars = [],
	     return_type = Scalar}

	and exp (DLInDeps.CONSTINT x) =
	    let
		val v = s_str()
	    in
		{body  = [$((asgn (v, int2str x)) ^ ";")],
		 vector_vars = [],
		 scalar_vars = [v],
		 return_vars = [v],
		 return_type = Scalar}
	    end
	    
	  | exp (DLInDeps.CONSTREAL x) =
	    let
		val v = s_str()
	    in
		{body  = [$((asgn (v, real2str x)) ^ ";")],
		 vector_vars = [],
		 scalar_vars = [v],
		 return_vars = [v],
		 return_type = Scalar}
	    end

	  | exp (DLInDeps.ID name) =
	    (case Symbol.look (typetable, symofdepnameindep name) of
		 SOME (VectorParEntryType n) => 
		 let
		     val i  = s_str()
		     val v  = v_str()
		 in
		     {body  = vec_asgn(i, v, name),
		      vector_vars = [v],
		      scalar_vars = [i],
		      return_vars = [v],
		      return_type = Vector (SOME n)}
		 end

	       | SOME _ => 
		 let
		     val x = s_str()
		 in
		     {body  = [$((asgn (x, (indepstorage (name, NONE)))) ^ ";")],
		      vector_vars = [],
		      scalar_vars = [x],
		      return_vars = [x],
		      return_type = Scalar}
		 end
		    
	       | _ => (* probably an argument *)
		 let
		     fun find_arg ({name={name=n,id=_}, typ=t}::rest) =
			 if (symofdepnameindep name) = n 
			 then t else find_arg rest

		       | find_arg (nil) = raise InvalidIdentifier
			       
		     (* verify it's an argument *)
		     val typ = (case funtypeinfo of
				   SOME {args=funargs, ret_type} => find_arg funargs
				 | NONE 
				   => raise InvalidIdentifier)
		     val typ = if sizeOfSet(typ) <> 1 then
				   raise InvalidIdentifier
			       else
				   hd(TypeSet.listItems(typ))
		 in
		     {body  = [],
		      vector_vars = [],
		      scalar_vars = [],
		      return_vars = [Symbol.name(symofdepnameindep name)],
		      return_type = typ}

		 end)

	     
	    
	  | exp (DLInDeps.BINOP{oper, left, right}) =
	    (case oper of 
		 DLAst.UNKNOWN _          => raise InvalidBinop
	       | DLAst.SCALAR oper        => 
		 let
		     val {body  = body1,
			  vector_vars = vv1,
			  scalar_vars = sv1,
			  return_vars = rv1,
			  return_type = rt1} = exp(left)

		     val {body  = body2,
			  vector_vars = vv2,
			  scalar_vars = sv2,
			  return_vars = rv2,
			  return_type = rt2} = exp(right)

		     val x = s_str()

		     val body = body1 @ body2 @
				[$((asgn (x, binop (hd rv1, scalar_binop2c oper, hd rv2))) ^ ";")]
		 in
		     {body  = body,
		      vector_vars = vv1 @ vv2,
		      scalar_vars = (x :: sv1) @ sv2,
		      return_vars = [x],
		      return_type = Scalar}
		 end

	       | DLAst.SCALAR_VECTOR oper =>
		 let
		     val {body  = body1,
			  vector_vars = vv1,
			  scalar_vars = sv1,
			  return_vars = rv1,
			  return_type = rt} = args ([left, right])

		     val v = v_str()

		     val body = body1 @
				[$(case oper of 
				       DLAst.SV_TIMES _ =>
				       (fcall ("sv_times", rv1)) ^ ";")]
		 in
		     {body  = body,
		      vector_vars = v :: vv1,
		      scalar_vars = sv1,
		      return_vars = [v],
		      return_type = (case rt of 
					 Vector (SOME n) => rt
				       | _ => raise InvalidBinop)}
		 end
		 
	       | DLAst.VECTOR oper        => 
		 let
		     val {body  = body1,
			  vector_vars = vv1,
			  scalar_vars = sv1,
			  return_vars = rv1,
			  return_type = rt} = args ([left, right])

		     val v = v_str()
		     val x = s_str()

		     val body = body1 @
				[$(case oper of 
				     DLAst.V_PLUS _   => fcall ("v_plus", rv1)
					
				   | DLAst.V_MINUS _  => fcall ("v_minus", rv1)
					
				   | DLAst.V_CROSSP _ => fcall ("v_crossp", rv1)

				   | DLAst.V_TIMES _ => asgn (x, fcall ("v_times", rv1)))]
				@ [$";"]
		 in
		     case oper of
			 DLAst.V_TIMES _ =>
			 {body  = body,
			  vector_vars = vv1,
			  scalar_vars = x :: sv1,
			  return_vars = [x],
			  return_type = Scalar}
		       | _ =>
			 {body  = body,
			  vector_vars = v :: vv1,
			  scalar_vars = sv1,
			  return_vars = [v],
			  return_type = (case rt of 
					     Vector (SOME n) => rt
					   | _ => raise InvalidBinop)}
		 end)

	  | exp (DLInDeps.RELOP {oper, left, right}) =
	    let
		
		val {body  = body1,
		     vector_vars = vv1,
		     scalar_vars = sv1,
		     return_vars = rv1,
		     return_type = rt} = args ([left, right])
		    
		val rvleft = fst rv1
		val rvright = snd rv1

		val x = s_str()

		fun scalar_relop () = ([x], [x], [$((asgn (x, binop(rvleft, relop2c oper, rvright))) ^ ";")])
				 
		fun vector_relop () = 
		    let
			val i = s_str()
		    in
			([x], [x,i], 
			      [$((asgn(x,"1")) ^ ";"),
			       $(concat ["for (", i, " = 0; ", i, " < ", rvleft, "[0]; ", i, "++)"]),
			       $"{",
			       SUB [$(concat ["if (!(", rvleft, "[", i, "] ", (relop2c oper), rvright, "[", i, "]))"]),
				    $"{",
				    SUB [$((asgn (x,"0")) ^ ";"),
					 $"break;"],
				    $"}"],
			       $"}"])
		    end
						
		val (rv2, sv2, body2) = case rt of
					   Vector _ => vector_relop()
					 | _        => scalar_relop()
	    in
		{body = body1 @ body2,
		 vector_vars = vv1,
		 scalar_vars = sv1 @ sv2,
		 return_vars = rv2,
		 return_type = Scalar}
	    end
		
	    
	  | exp (DLInDeps.FUNCALL{function, arguments}) =
	    let
		val typ = case UniqueId.look(depenv, DLEnv'.idofname (DLInDeps.nameofindepname function)) of
					  
					  SOME (DLDeps.FunEntry {name=n, typeinfo=typeinfo, ...})
					  => singletonToType (#ret_type typeinfo)
						 
					| SOME (DLDeps.BuiltinFunEntry {name=name, typeinfo=typeinfo, ...})
					  => singletonToType (#ret_type typeinfo)
						 
					| SOME (DLDeps.TableFunEntry {quantity=name, ...})
					  => Scalar

					| SOME (DLDeps.FunAlias {name=n, typeinfo=typeinfo, ...})
					  => singletonToType (#ret_type typeinfo)

					| _ => raise InvalidIdentifier

		val {body  = body1,
		     vector_vars = vv1,
		     scalar_vars = sv1,
		     return_vars = rv1,
		     return_type = rt1} = args (arguments)
		    
		fun funname'2str function = 
		    case UniqueId.look(depenv, DLEnv'.idofname (function)) of
			SOME (DLDeps.FunAlias {entryname, ...}) => 
			funname'2str (DLDeps.nameofdepname (entryname))
		      | _ => name'2str function

		fun funname2str function =
		    funname'2str (DLInDeps.nameofindepname function)

		fun scalar_funcall () = let val x = s_str()
					in
					    (x, [x], [], [$((asgn (x, fcall ((funname2str function), rv1))) ^ ";")])
					end

		fun vector_funcall () = let val v = v_str()
					in
					    (v, [], [v], [$((fcall ((funname2str function), v :: rv1)) ^ ";")])
					end

		val (rv, sv2, vv2, body2) = 
		    case typ of
			Scalar   => scalar_funcall()
		      | Vector _ => vector_funcall()
		      | _ => raise Unsupported

				 
	    in
		(case typ of
		     Scalar => {body  = body1 @ body2,
				vector_vars = vv1 @ vv2,
				scalar_vars = sv1 @ sv2,
				return_vars = [rv],
				return_type = Scalar}

		   | (Vector _) => {body  = body1 @ body2,
				    vector_vars = vv1 @ vv2,
				    scalar_vars = sv1 @ sv2,
				    return_vars = [rv],
				    return_type = (case rt1 of 
						       Vector (SOME n) => rt1
						     | _ => raise InvalidArgument)}
		   | _ => {body  = body1 @ body2,
			   vector_vars = vv1 @ vv2,
			   scalar_vars = sv1 @ sv2,
			   return_vars = [rv],
			   return_type = Scalar})
	    end

	  | exp (DLInDeps.UNARYEXPOP{oper, exp=e}) =
	    let
		val {body  = body1,
		     vector_vars = vv1,
		     scalar_vars = sv1,
		     return_vars = rv1,
		     return_type = rt} = exp (e)

		val x = s_str()
		    
		val body  = body1 @ [$((asgn (x, "-" ^ (hd rv1))) ^ ";")]
				 
	    in
		{body  = body,
		 vector_vars = vv1,
		 scalar_vars = x :: sv1,
		 return_vars = [x],
		 return_type = Scalar}
	    end
	
	  | exp (DLInDeps.CONDITIONAL {condition, iftrue, iffalse}) =
	    let
		val {body  = body1,
		     vector_vars = vv1,
		     scalar_vars = sv1,
		     return_vars = rv1,
		     return_type = rt1} = exp (condition)

		val {body  = body2,
		     vector_vars = vv2,
		     scalar_vars = sv2,
		     return_vars = rv2,
		     return_type = rt2} = exp (iftrue)

		val {body  = body3,
		     vector_vars = vv3,
		     scalar_vars = sv3,
		     return_vars = rv3,
		     return_type = rt3} = exp (iffalse)

		fun scalar_body () = 
		    let
			val x = s_str()
		    in
			([x], [x], [],
			 [$((asgn (x, concat ["(", (hd rv1), ") ? ", (hd rv2), " : ", (hd rv3)])) ^ ";")])
		    end
		    
		fun vector_body () = 
		    let
			val i = s_str()
			val v = v_str()
		    in
			([v], [i], [v],
			[$("if ("^(hd rv1)^")"),
			 $"{",
			 SUB [$(concat ["for (", i, " = 0; ", i, " <= ", (hd rv2), "[0]; ", i, "++)"]),
			      $"{",
			      SUB [$(concat [v, "[", i ,"] = ", (hd rv2), "[", i, "];"])],
			      $"}"],
			 $"}",
			 $"else",
			 $"{",
			 SUB [$(concat ["for (", i, " = 0; ", i, " <= ", (hd rv3), "[0]; ", i, "++)"]),
			      $"{",
			      $(concat [v, "[", i, "] = ", (hd rv3), "[", i, "];"]),
			      $"}"],
			 $"}"])
		    end
		
		val (rv4, sv4, vv4, body4) = 
		    (case rt2 of 
			 Scalar   => scalar_body()
		       | Vector _ => vector_body()
		       | _ => raise Unsupported) 

	    in
		{body  = body1 @ body2 @ body3 @ body4,
		 vector_vars = vv1 @ vv2 @ vv3 @ vv4,
		 scalar_vars = sv1 @ sv2 @ sv3 @ sv4,
		 return_vars = rv4,
		 return_type = rt2} 
	    end

	
			
    in
	exp(e)
    end

fun output_depexp depstorage (e, depenv:depenv, funtypeinfo: type_info' option) =
    let
	val typetable = type_table depenv
			
	fun findSetEntry(name, nil) =
	    raise InvalidIdentifier
		  
	  | findSetEntry(name, DLDeps.SetEntry{name=n, value=v, ...}::depenv) =
	    if name = (symofdepname n)
	    then
		v
	    else
		findSetEntry (name, depenv)
		
	  | findSetEntry(name, e::depenv) =
	    findSetEntry(name, depenv)
	    
	val vec_asgn = vec_asgn depstorage
	    
	fun args (h::nil) = exp(h) 
	  | args (h::r) = 
	    let
		val {body  = body1,
		     vector_vars = vv1,
		     scalar_vars = sv1,
		     return_vars = rv1,
		     return_type = rt1} = exp(h)

		val {body  = body2,
		     vector_vars = vv2,
		     scalar_vars = sv2,
		     return_vars = rv2,
		     return_type = rt2} = args (r)

	    in
		{body  = body1 @ body2, 
		 vector_vars = vv1 @ vv2, 
		 scalar_vars = sv1 @ sv2, 
		 return_vars = rv1 @ rv2,
		 return_type = rt1}
	    end

	  | args (nil) = 
	    {body = [],
	     vector_vars = [], 
	     scalar_vars = [], 
	     return_vars = [(s_str ())],
	     return_type = Scalar}

	and exp (DLDeps.CONSTINT x) =
	    let
		val v = s_str()
	    in
		{body  = [$((asgn (v, int2str x)) ^ ";")],
		 vector_vars = [],
		 scalar_vars = [v],
		 return_vars = [v],
		 return_type = Scalar}
	    end

	  | exp (DLDeps.CONSTREAL(x)) =
	    let
		val v = s_str()
	    in
		{body  = [$((asgn (v, real2str x)) ^ ";")],
		 vector_vars = [],
		 scalar_vars = [v],
		 return_vars = [v],
		 return_type = Scalar}
	    end

	  | exp (DLDeps.ID (x as DLDeps.EVENT name)) =
	    let
		val v = s_str()
	    in
		{body  = [$((asgn (v, depstorage (x, NONE))) ^ ";")],
		 vector_vars = [],
		 scalar_vars = [v],
		 return_vars = [v],
		 return_type = Scalar}
	    end

	  | exp (DLDeps.ID (x as DLDeps.EVENTARG (event,name))) =
	    let
		val v = s_str()
	    in
		{body  = [$((asgn (v, depstorage (x, NONE))) ^ ";")],
		 vector_vars = [],
		 scalar_vars = [v],
		 return_vars = [v],
		 return_type = Scalar}
	    end

	  | exp (DLDeps.ID name) =
	    (case Symbol.look (typetable, symofdepname name) of
		 SOME (VectorParEntryType n) => 
		 let
		     val  i = s_str()
		     val  v = v_str()
		 in
		     {body = vec_asgn(i,v,name),
		      vector_vars = [v],
		      scalar_vars = [i],
		      return_vars = [v],
		      return_type = Vector (SOME n)}
		 end

	       | SOME (VectorStateEntryType n) => 
		 let
		     val  i = s_str()
		     val  v = v_str()
		 in
		     {body  = vec_asgn(i, v, name),
		      vector_vars = [v],
		      scalar_vars = [i],
		      return_vars = [v],
		      return_type = Vector (SOME n)}
		 end
		    
	       | SOME (VectorStateFunEntryType n) => 
		 let
		     val  i = s_str()
		     val  v = v_str()
		 in
		     {body  = vec_asgn(i, v, name),
		      vector_vars = [v],
		      scalar_vars = [i],
		      return_vars = [v],
		      return_type = Vector (SOME n)}
		 end

	       | SOME _ => 
		 {body  = [],
		  vector_vars = [],
		  scalar_vars = [],
		  return_vars = [depstorage (name, NONE)],
		  return_type = Scalar}
		 
	       | _ => (* probably an argument *)
		 let
		     fun find_arg ({name={name=n,id=_}, typ=t}::rest) =
			 if (symofdepname name) = n then
			     t else find_arg rest

		       | find_arg (nil) =
			 raise InvalidIdentifier
			       
		     (* verify it's an argument *)
		     val typ = (case funtypeinfo of
				   SOME {args=funargs, ret_type} => find_arg funargs
				 | NONE => raise InvalidIdentifier)

		     val typ = if sizeOfSet(typ) <> 1 then
				   raise InvalidIdentifier
			       else
				   hd(TypeSet.listItems(typ))
		 in
		     
		     {body  = [],
		      vector_vars = [],
		      scalar_vars = [],
		      return_vars = [Symbol.name (symofdepname name)],
		      return_type = typ}
		 end)
	    
	  | exp (e as DLDeps.BINOP{oper, left, right}) =
	    (case oper of 
		 DLAst.UNKNOWN _          => 
		 (print "DLDeps.BINOP: unknown type: \n"; 
		  PrettyPrint.print_depexp (TextIO.stdOut, e, 0);
		  raise InvalidBinop)
	       | DLAst.SCALAR oper        => 
		 let
		     val {body  = body1,
			  vector_vars = vv1,
			  scalar_vars = sv1,
			  return_vars = rv1,
			  return_type = rt1} = exp left

		     val {body  = body2,
			  vector_vars = vv2,
			  scalar_vars = sv2,
			  return_vars = rv2,
			  return_type = rt2} = exp right

		     val (rv3,sv3,body3) = let val x = s_str()
					   in
					       ([x], [x],
						[$((asgn (x, binop(hd rv1, scalar_binop2c oper, hd rv2))) ^ ";")])
					   end
		 in
		     {body  = body1 @ body2 @ body3,
		      vector_vars = vv1 @ vv2,
		      scalar_vars = sv1 @ sv2 @ sv3,
		      return_vars = rv3,
		      return_type = Scalar}
		 end

	       | DLAst.SCALAR_VECTOR oper =>
		 let
		     val {body  = body1,
			  vector_vars = vv1,
			  scalar_vars = sv1,
			  return_vars = rv1,
			  return_type = rt1} = exp(left)

		     val {body  = body2,
			  vector_vars = vv2,
			  scalar_vars = sv2,
			  return_vars = rv2,
			  return_type = rt2} = exp(right)

		     val v = v_str()

		     val body3 = [$((case oper of 
					 DLAst.SV_TIMES _ => fcall ("sv_times", [fst rv1, fst rv2])) ^ ";")]

		     val rt = case oper of 
				  DLAst.SV_TIMES n => Vector (SOME n)

		 in
		     {body  = body1 @ body2 @ body3,
		      vector_vars = v :: (vv1 @ vv2),
		      scalar_vars = sv1 @ sv2,
		      return_vars = [v],
		      return_type = 
		      (case rt of 
			   Vector (SOME n) => rt
			 | _ => raise InvalidBinop 
				      before (app (fn (exp) => 
						      PrettyPrint.print_depexp (TextIO.stdOut, exp, 0)) [left, right]))}
		 end
		 
	       | DLAst.VECTOR oper        => 
		 let
		     val {body  = body1,
			  vector_vars = vv1,
			  scalar_vars = sv1,
			  return_vars = rv1,
			  return_type = rt} = args ([left, right])

		     val v = v_str()

		     val (rv2,sv2,vv2,body2) = 
			 case oper of 
			     DLAst.V_PLUS _   => let val v = v_str()
						 in
						     ([v],[],[v],[$((fcall ("v_plus", v :: rv1)) ^ ";")])
						 end
					   
			   | DLAst.V_MINUS _  => let val v = v_str()
						 in
						     ([v],[],[v],[$((fcall ("v_minus", v :: rv1)) ^ ";")])
						 end
						 
			   | DLAst.V_TIMES _  => let val s = s_str()
						 in
						     ([s],[s],[],[$((asgn(s,fcall ("v_times", rv1))) ^ ";")])
						 end

			   | DLAst.V_CROSSP _ => let val v = v_str()
						 in
						     ([v],[],[v],[$((fcall ("v_crossp", v :: rv1)) ^ ";")])
						 end
		 in
		     {body  = body1 @ body2,
		      vector_vars = vv1 @ vv2,
		      scalar_vars = sv1 @ sv2,
		      return_vars = rv2,
		      return_type = case oper of DLAst.V_TIMES _ => Scalar 
					       | _ => (case rt of 
							   Vector (SOME n) => rt
							 | _ => raise InvalidBinop)}
		 end)

	  | exp (DLDeps.RELOP{oper, left, right}) =
	    let
		val {body  = body1,
		     vector_vars = vv1,
		     scalar_vars = sv1,
		     return_vars = rv1,
		     return_type = rt} = args ([left, right])
		    
		val rvleft = fst rv1
		val rvright = snd rv1

		val x = s_str()

		fun scalar_relop () = ([x], [x], [$((asgn (x, binop(rvleft, relop2c oper, rvright))) ^ ";")])
				 
		fun vector_relop () = 
		    let
			val i = s_str()
		    in
			([x], [x,i], 
			      [$((asgn(x,"1")) ^ ";"),
			       $(concat ["for (", i, " = 0; ", i, " < ", rvleft, "[0]; ", i, "++)"]),
			       $"{",
			       SUB [$(concat ["if (!(", rvleft, "[", i, "] ", (relop2c oper), rvright, "[", i, "]))"]),
				    $"{",
				    SUB [$((asgn (x,"0")) ^ ";"),
					 $"break;"],
				    $"}"],
			       $"}"])
		    end
						
		val (rv2, sv2, body2) = case rt of
					   Vector _ => vector_relop()
					 | _        => scalar_relop()
	    in
		{body = body1 @ body2,
		 vector_vars = vv1,
		 scalar_vars = sv1 @ sv2,
		 return_vars = rv2,
		 return_type = Scalar}
	    end

	    
	  | exp (DLDeps.FUNCALL{function, arguments}) =
	    let
						 
		val typ = case UniqueId.look(depenv, DLDeps.idofdepname function) of
			      
			      SOME (DLDeps.FunEntry {name=n, typeinfo=typeinfo, ...}) => 
			      singletonToType (#ret_type typeinfo)
				 
			    | SOME (DLDeps.BuiltinFunEntry {name=name, typeinfo=typeinfo, ...}) => 
			      singletonToType (#ret_type typeinfo)
				 
			    | SOME (DLDeps.TableFunEntry {quantity=name, ...}) => 
			      Scalar
				 
			    | SOME (DLDeps.FunAlias {name=n, typeinfo=typeinfo, ...}) => 
			      singletonToType (#ret_type typeinfo)
				 
			    | _ => raise InvalidIdentifier
									 
		val {body  = body1,
		     vector_vars = vv1,
		     scalar_vars = sv1,
		     return_vars = rv1,
		     return_type = rt1} = args arguments

		fun funname2str function = 
		    case UniqueId.look(depenv, DLDeps.idofdepname function) of
			SOME (DLDeps.FunAlias {entryname, ...}) => funname2str entryname
		      | _ => depname2str function

		fun scalar_funcall () = let val x = s_str()
					in
					    (x, [x], [], [$((asgn (x, fcall ((funname2str function), rv1))) ^ ";")])
					end

		fun vector_funcall () = let val v = v_str()
					in
					    (v, [], [v], [$((fcall ((funname2str function), v :: rv1)) ^ ";")])
					end

		val (rv, sv2, vv2, body2) = 
		    case typ of
			Scalar   => scalar_funcall()
		      | Vector _ => vector_funcall()
		      | _ => raise Unsupported

	    in
		(case typ of
		     Scalar => {body  = body1 @ body2,
				vector_vars = vv1 @ vv2,
				scalar_vars = sv1 @ sv2,
				return_vars = [rv],
				return_type = Scalar}

		   | (Vector _) => {body  = body1 @ body2,
				    vector_vars = vv1 @ vv2,
				    scalar_vars = sv1 @ sv2,
				    return_vars = [rv],
				    return_type = (case rt1 of 
						       Vector (SOME n) => rt1
						     | _ => raise InvalidArgument)}
		   | _ => {body  = body1 @ body2,
			   vector_vars = vv1 @ vv2,
			   scalar_vars = sv1 @ sv2,
			   return_vars = [rv],
			   return_type = Scalar})
			  
	    end

	  | exp (DLDeps.UNARYEXPOP{oper, exp=e}) =
	    let
		val {body  = body1,
		     vector_vars = vv1,
		     scalar_vars = sv1,
		     return_vars = rv1,
		     return_type = rt} = exp (e)

		val x = s_str()
		    
		val body  = body1 @ [$((asgn (x, "-" ^ (hd rv1))) ^ ";")]
				 
	    in
		{body  = body,
		 vector_vars = vv1,
		 scalar_vars = x :: sv1,
		 return_vars = [x],
		 return_type = Scalar}
	    end
	
	  | exp (DLDeps.CONDITIONAL{condition, iftrue, iffalse}) =
	    let
		val {body  = body1,
		     vector_vars = vv1,
		     scalar_vars = sv1,
		     return_vars = rv1,
		     return_type = rt1} = exp (condition)

		val {body  = body2,
		     vector_vars = vv2,
		     scalar_vars = sv2,
		     return_vars = rv2,
		     return_type = rt2} = exp (iftrue)

		val {body  = body3,
		     vector_vars = vv3,
		     scalar_vars = sv3,
		     return_vars = rv3,
		     return_type = rt3} = exp (iffalse)

		fun scalar_body () = 
		    let
			val x = s_str()
		    in
			([x], [x], [],
			 [$((asgn (x, concat ["(", (hd rv1), ") ? ", (hd rv2), " : ", (hd rv3)])) ^ ";")])
		    end
		    
		fun vector_body () = 
		    let
			val i = s_str()
			val v = v_str()
		    in
			([v], [i], [v],
			[$("if ("^(hd rv1)^")"),
			 $"{",
			 SUB [$(concat ["for (", i, " = 0; ", i, " <= ", (hd rv2), "[0]; ", i, "++)"]),
			      $"{",
			      SUB [$(concat [v, "[", i ,"] = ", (hd rv2), "[", i, "];"])],
			      $"}"],
			 $"}",
			 $"else",
			 $"{",
			 SUB [$(concat ["for (", i, " = 0; ", i, " <= ", (hd rv3), "[0]; ", i, "++)"]),
			      $"{",
			      $(concat [v, "[", i, "] = ", (hd rv3), "[", i, "];"]),
			      $"}"],
			 $"}"])
		    end
		
		val (rv4, sv4, vv4, body4) = 
		    (case rt2 of 
			 Scalar   => scalar_body()
		       | Vector _ => vector_body()
		       | _ => raise Unsupported) 

	    in
		{body  = body1 @ body2 @ body3 @ body4,
		 vector_vars = vv1 @ vv2 @ vv3 @ vv4,
		 scalar_vars = sv1 @ sv2 @ sv3 @ sv4,
		 return_vars = rv4,
		 return_type = rt2} 

	    end

	
			
    in
	exp(e)
    end

fun output_asgn depstorage (quantity, e: DLDeps.depexp, typ:enventrytype, depenv:depenv) =
    let

	val output_depexp = output_depexp depstorage

 	val {body=body1, vector_vars, scalar_vars, return_vars, return_type} = 
	    output_depexp(e, depenv, NONE)

	fun scalar_body () = [$((asgn(depstorage (quantity,NONE),hd return_vars)) ^ ";")]

	val idx = "s" ^ Int.toString(!s_count)

	fun vector_body () = 
	    [$(concat ["for(", idx, " = 0; ", 
		       idx, " <= ", depstorage (quantity, SOME "0"), "; ", 
		       idx, "++)"]),
	     $"{", 
	     SUB [$(concat [depstorage (quantity, SOME ("(int)("^idx^")")), " = ", 
			    hd return_vars, "[", "(int)", idx, "];"])],
	     $"}"]

	val _ = s_count := !s_count + 1

	val body2 = case typ of
			     VectorParEntryType _ => vector_body()
			   | VectorStateEntryType _ => vector_body()
			   | VectorStateFunEntryType _ => vector_body()
			   | _ => scalar_body()

	val scalar_vars = case return_type of
			      Vector _ => idx :: scalar_vars
			    | _ => scalar_vars
			      
    in
	{body=body1 @ body2,
	 vector_vars=vector_vars,
	 scalar_vars=scalar_vars,
	 return_vars=[],
	 return_type=return_type}

    end

fun output_disc_exp depstorage (e, depenv:depenv) =
    let
	val output_depexp = output_depexp depstorage
			    
	val retval = s_str()

	fun findSetEntry(name, nil) =
	    raise InvalidIdentifier

	  | findSetEntry(name, DLDeps.SetEntry{name=n, value=v, ...}::depenv) =
	    if name = n then v
	    else findSetEntry (name, depenv)

	  | findSetEntry(name, e::depenv) =
	    findSetEntry(name, depenv)

	fun output_discrete_conds (elseflag, nil, default) = 
	    (case default of 
		 SOME default => 
		 {body = (if elseflag then [$"else"] else []) @ 
			 [$"{",
			  SUB [$"/* default clause */",
			       $((asgn(retval,depstorage(default,NONE))) ^ ";")], 
			  $"}"],
		  vector_vars= [],
		  scalar_vars= [retval],
		  return_vars= [retval],
		  return_type= Scalar}
	       | NONE =>
		 {body = [],
		  vector_vars = [], 
		  scalar_vars = [retval], 
		  return_vars = [retval],
		  return_type = Scalar})

	  | output_discrete_conds (elseflag, clause::clauses, default) =
	    case clause of
		DLDeps.DISCRETE_COND (exp, n) => 
		let
		    val {body=out1,
			 vector_vars=vv1,
			 scalar_vars=sv1,
			 return_vars=rv1,
			 return_type=rt1} = output_discrete_conds (true, clauses, default)

		    val {body=out2,
			 vector_vars=vv2,
			 scalar_vars=sv2,
			 return_vars=rv2,
			 return_type=rt2} = 
			output_depexp(exp, depenv, NONE)

		    val cond = (hd rv2)

		in 
		    {body = (if elseflag then [$"else {"] else []) @
			    [SUB (out2 @
				  [$("if (" ^ cond ^ ") "), 
				   $"{",
				   SUB [$((asgn(retval,"SET_" ^ (Symbol.name (symofdepname n)))) ^ ";")],
				   $"}"] @ 
				  out1)] @ 
			    (if elseflag then [$"}"] else []),
		     vector_vars = vv1 @ vv2, 
		     scalar_vars = sv1 @ sv2, 
		     return_vars = rv1,
		     return_type = Scalar}
		    
		end

	      | DLDeps.DEFAULT_COND n => output_discrete_conds(elseflag, clauses, SOME n)
		   
    in
	(case e of 
	     DLDeps.DISCRETE_ID n => output_depexp (DLDeps.ID n, depenv, NONE)
	   | DLDeps.DISCRETE_CASE clauses => output_discrete_conds (false, clauses, NONE))

    end
	

(* creates the string representing an assignment of a quantity to a discrete expression *)
fun output_disc_asgn depstorage (quantity, e, depenv:depenv) =
    let	

 	val {body,
	     vector_vars,
	     scalar_vars,
	     return_vars,
	     return_type} = (output_disc_exp depstorage) (e, depenv)

    in
	{body = body @ [$((asgn(depstorage (quantity, NONE),"(int)" ^ (hd return_vars))) ^ ";")],
	 vector_vars=vector_vars,
	 scalar_vars=scalar_vars,
	 return_vars=return_vars,
	 return_type=return_type}	
    end




fun output_cond depstorage (env, body, (cond::next): depcondition list) =
    let
	val {relop, quantity1, quantity2} = cond
					    
	val {body=body1, vector_vars, scalar_vars, return_vars, return_type} =
	    output_depexp depstorage (DLDeps.RELOP {oper=relop, 
						    left=DLDeps.ID quantity1, 
						    right=DLDeps.ID quantity2}, 
				      env, NONE)

    in
	(if not (vector_vars  = nil)
	 then
	     (output_vecdecls vector_vars)
	 else []) @
	(if not (scalar_vars  = nil)
	 then
	     [$("double " ^ (strlist "," scalar_vars) ^ ";")]
	 else []) @
	body1 @
	[$("if (" ^ (hd return_vars) ^ ")"),
	 $"{",
	 SUB (case next of 
		  nil => body
		| _   => output_cond depstorage (env, body, next)),
	 $"}"]
    end
  | output_cond depstorage (env, body, nil) =
    raise InvalidCondition

(*
fun output_timediv depstorage (quantity, startfun, endfun, bodyfun, timediv, timestep) =
    let
	val timediv = valOf (UniqueId.look (env,idofdepname timediv))
	val t       = s_str()
	val i       = s_str()

	val {body, vector_vars, scalar_vars,
	     return_vars, return_type} =   (bodyfun (SOME i))

    in
	{body=([$(concat ["int ", i, ";"]),
		$(concat [i, "= 1;"])] @ (case startfun of SOME startfun => startfun i | _ => []) @
	       [$(concat ["for (", t, " = 0.0; ", 
			  t, " < ", timestep, "; ", 
			  t, "+=", depstorage (DLDeps.nameofentry timediv,NONE,NONE), ", ", i ,"++)"]),
		$("{"),
		SUB body,
		$("}")] @
	       (case endfun of SOME endfun => endfun i | _ => [])),
	 vector_vars=vector_vars,
	 scalar_vars=t :: scalar_vars,
	 return_vars=return_vars,
	 return_type=return_type} 
    end
*)

fun output_ev depstorage (env, event, body) =
    let
	val {body=body1, vector_vars, scalar_vars, return_vars, return_type} =
	    output_depexp depstorage (DLDeps.RELOP {oper=DLAst.GT, 
						    left=DLDeps.ID event, 
						    right=DLDeps.CONSTREAL 0.0}, 
				      env, NONE)

    in
	(if not (vector_vars  = nil)
	 then
	     (output_vecdecls vector_vars)
	 else []) @
	(if not (scalar_vars  = nil)
	 then
	     [$("double " ^ (strlist "," scalar_vars) ^ ";")]
	 else []) @
	body1 @
	[$("if (" ^ (hd return_vars) ^ ")"),
	 $"{",
	 SUB body,
	 $"}"]
    end

fun output_euler_eq (depstorage,timestep) (quantity, exp: depexp, env) = 
    let
	val output_depexp = output_depexp depstorage
	val {body, vector_vars, scalar_vars, 
	     return_vars, return_type=rt} =  output_depexp(exp, env, NONE)
    in
	{body=(body @
	       [(#invocation euler_method) 
		    (depstorage (quantity, NONE),
		     depstorage (DLDeps.OLD (DLDeps.nameofdepname quantity), NONE), 
		     timestep,
		     return_vars)]),
	 vector_vars=vector_vars,
	 scalar_vars=scalar_vars,
	 return_vars=[],
	 return_type=rt}
    end
	

fun output_mau_eq (depstorage,timestep) (quantity, arg1: depexp, arg2: depexp, env) = 
    let
	val output_depexp = output_depexp depstorage
	val {body=body1, vector_vars=vector_vars1, scalar_vars=scalar_vars1, 
	     return_vars=return_vars1, return_type=_} =  output_depexp(arg1, env, NONE)
	val {body=body2, vector_vars=vector_vars2, scalar_vars=scalar_vars2, 
	     return_vars=return_vars2, return_type=rt} =  output_depexp(arg2, env, NONE)
    in
	{body=(body1 @ body2 @
	       [(#invocation mau_method) 
		    (depstorage (quantity, NONE),
		     depstorage (DLDeps.OLD (DLDeps.nameofdepname quantity), NONE), 
		     timestep,
		     return_vars1 @ return_vars2)]),
	 vector_vars=vector_vars1 @ vector_vars2,
	 scalar_vars=scalar_vars1 @ scalar_vars2,
	 return_vars=[],
	 return_type=rt}
    end
	
(* given an entry, print out its equation *)
fun output_eq (depstorage, timestep, output_table_lookup, 
	       external_input: external_state, external_output: external_state) (env, entry) =
    let
	val output_depexp    = output_depexp depstorage
	val output_asgn      = output_asgn depstorage
	val output_disc_asgn = output_disc_asgn depstorage
	val output_cond      = output_cond depstorage
	val output_ev        = output_ev depstorage
	val output_mau_eq    = output_mau_eq (depstorage,timestep)
	val output_euler_eq  = output_euler_eq (depstorage,timestep)

	fun cond (condition, body) =
	    (case condition of 
		 SOME condition =>
		 let
		     val entry = valOf(UniqueId.look (env, condition))
		     val condlist = 
			 (case entry of
			      DLDeps.ConditionEntry {condlist,...} => condlist
			    | _ => raise InvalidConditionEntry)
		 in
		     output_cond(env, body, condlist)
		 end
	       | NONE => body)

	fun ev (SOME event, body) = output_ev (env, event, body)
	  | ev (NONE, body) = body

	fun decls (bodyfun) = 
	    let
		val {body, vector_vars, scalar_vars, ...} =  bodyfun()
	    in
		(if not (scalar_vars = nil)
		 then
		     [$("double " ^ (strlist "," scalar_vars) ^ ";")]
		 else []) @ 
		(if not (vector_vars = nil)
		 then
		     (output_vecdecls vector_vars)
		 else []) @ body
	    end
    in
	(case entry of
	     DLDeps.TimeEntry {quantity, initval, eq} =>
	     (case eq of
		  (DLDeps.AlgebraicEquation {line, exp, deps}) =>
		  let
 		      fun bodyfun () = output_asgn(quantity, exp, TimeEntryType, env)
		  in
		      decls bodyfun
		  end
		| _ => raise InvalidEquation)
	   | DLDeps.ScalarStateEntry {quantity, initval, method,
				      condition, event, 
				      eq, description} =>
	     (case eq of 
		  DLDeps.DifferenceEquation {line, exp, deps} =>
		  let
		      fun bodyfun () = output_asgn(quantity, exp, ScalarStateEntryType, env) 
		  in
		      ev (event, cond(condition, decls bodyfun))
		  end

		| DLDeps.DifferentialEquation {line, arg, deps} =>
		  let
		      fun eulerfun (arg) = 
			  (case arg of 
			       DLDeps.EULER_ARGS exp  => 
			       output_euler_eq (quantity, exp, env)
			     | _ => raise InvalidMethod)

		      fun maufun (arg) = 
			  (case arg of
			       DLDeps.MAU_ARGS (arg1, arg2) => 
			       output_mau_eq (quantity, arg1, arg2, env)
			     | _ => raise InvalidMethod)
			  
		      val bodyfun =
			  (case method of
			       SOME DLAst.EULER => (fn() => eulerfun arg)
			       
			     | SOME DLAst.MAU => (fn () => maufun arg)
			       
			     | NONE => (case arg of 
					    DLDeps.EULER_ARGS argexp  => (fn() => eulerfun arg)
					  | DLDeps.MAU_ARGS (arg1, arg2) => (fn() => maufun arg)))

		  in
		      ev (event, cond (condition, decls bodyfun))
		  end
				  
		      
		| _ => raise InvalidEquation)
	     
	   | DLDeps.VectorStateEntry {quantity, initval, method,
				      condition, event,
				      eq, description} =>

	     (case eq of 
		  DLDeps.DifferenceEquation {line, exp, deps} =>
		  let
		      fun bodyfun () = output_asgn(quantity, exp, VectorStateEntryType (Array.length initval), env)
		  in
		      ev (event, cond(condition, decls bodyfun))
		  end
		| _ => raise InvalidEquation)
	     
	   | DLDeps.IntegerStateEntry {quantity, initval, 
				       condition, event, 
				       eq, description} => 

	     (case eq of 
		  DLDeps.DifferenceEquation {line, exp, deps} =>
		  let
 		      fun bodyfun () = output_asgn(quantity, exp, ScalarStateEntryType, env)
		  in
		      ev (event, cond(condition, decls bodyfun))
		  end
	     
		| _ => raise InvalidEquation)

	   | DLDeps.ExternalStateEntry {quantity, direction, channel,
					condition, event, 
					eq, description} =>

	     (case direction of 
		  DLAst.INPUT  => (#func external_input)(#assignments external_input, quantity, channel)
		| DLAst.OUTPUT =>
		  (case eq of 
		       SOME (DLDeps.AlgebraicEquation {line, exp, deps}) =>
		       let
			   fun bodyfun () = output_asgn(quantity, exp, ScalarStateEntryType, env)
		       in
			   ev (event, 
			       cond(condition, 
				    (decls bodyfun)
				    @ ((#func external_output)(#assignments external_output, quantity, channel))))
		       end
		     | SOME _ => raise InvalidEquation
		     | NONE => []))
			     
	     
	   | DLDeps.DiscreteStateEntry {quantity, set, 
					condition, event,
					eq, description} =>
	     (case eq of 
		  DLDeps.DiscreteEquation {line, exp, deps} =>
		  let
 		      fun bodyfun () = output_disc_asgn(quantity, exp, env)
		  in
		      ev (event, cond(condition, decls bodyfun))
		  end
		| _ => raise InvalidEquation)
	     
	   | DLDeps.VectorStateFunEntry {quantity, 
					 condition, event, 
					 eq, size, description} =>

	     (case eq of 
		  DLDeps.AlgebraicEquation {line, exp, deps} =>
		  let
		      fun bodyfun () = output_asgn(quantity, exp, VectorStateEntryType size, env)
		  in
		      ev (event, cond(condition, decls bodyfun))

		  end
		| _ => raise InvalidEquation)
	     
	   | DLDeps.ScalarStateFunEntry {quantity, 
					 condition, event,
					 eq, description} =>
	     (case eq of 
		  DLDeps.AlgebraicEquation {line, exp, deps} =>
		  let
		      fun bodyfun () = output_asgn(quantity, exp, ScalarStateEntryType, env)
		  in
		      ev (event, cond(condition, decls bodyfun))
		  end
	     	| _ => raise InvalidEquation)
	 
	   | DLDeps.TableFunEntry {quantity, body, low, high, step, argument,
				   description} =>
	     output_table_lookup (env, entry)

	   | _ => [])
    end




fun main(dir, modelname, outputname, inline_c, env, startenv, envgraph, startenvgraph) =

    let
	val module_streamname = outputname ^ ".c"
	val module_stream     = TextIO.openOut(OS.Path.joinDirFile {dir=dir, file=module_streamname})

	val sayln = sayln (say module_stream)
    in
	sayln("")
    end




end

 
