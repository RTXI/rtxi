
(* Dependency analysis. *)

signature DL_DEPS =
sig
    type symbol       = Symbol.symbol
    type uniq_id      = UniqueId.uniq_id

    type condition    = DLEnv.condition
    type condmap      = DLEnv.condmap

    type env'
    type exp          = DLEnv'.exp
    type name'        = DLEnv'.name'
    type set_element' = DLEnv'.set_element'
    type discrete_exp = DLEnv'.discrete_exp
    type type_info'    = DLEnv'.type_info'
    type set_element  = DLEnv'.set_element
    type external_dir = DLEnv'.external_dir
    type differential_arguments = DLEnv'.differential_arguments
    type method       = DLEnv'.method
    type condenvmap   = DLEnv'.condenvmap
    type eventenvmap  = DLEnv'.eventenvmap


    type unaryexpop = DLAst.unaryexpop
    type binop      = DLAst.binop
    type relop      = DLAst.relop
    type indepexp   = DLInDeps.indepexp

    datatype depname  = OLD of name'
		      | NEW of name' 
		      | STATELESS of name' 
		      | BUILTIN of name' 
		      | CONDITION of uniq_id
		      | EVENT of name'
		      | EVENTARG of (name' * name')
		      | TABLE of name'
		      | PARAM of name'
		      | SETELM of name'

    type dependency = depname

    type depcondition = {relop: relop, 
			 quantity1: depname,   
			 quantity2: depname}

    datatype depdiscrete_exp = 
	     DISCRETE_ID of depname
	   | DISCRETE_CASE of depdiscrete_exp_clause list
		    
    and depdiscrete_exp_clause = 
	DISCRETE_COND of depexp * depname
      | DEFAULT_COND of depname
		
    and depexp = 
	CONSTINT of int
      | CONSTREAL of real
      | ID of depname
      | BINOP of {oper: binop, left: depexp, right: depexp}
      | RELOP of {oper: relop, left: depexp, right: depexp}
      | FUNCALL of {function: depname, arguments: depexp list}
      | UNARYEXPOP of {oper: unaryexpop, exp: depexp}
      | CONDITIONAL of {condition: depexp, iftrue: depexp, iffalse: depexp}

    datatype depdifferential_arguments = 
	     EULER_ARGS of depexp
	   | MAU_ARGS of depexp * depexp


datatype depequation = DifferenceEquation   of {line: int, exp: depexp, deps: dependency list}
		     | DifferentialEquation of {line: int, 
						arg: depdifferential_arguments,
						deps: dependency list}
		     | AlgebraicEquation    of {line: int, exp: depexp, 
						deps: dependency list}
		     | DiscreteEquation     of {line: int, exp: depdiscrete_exp, 
						deps: dependency list}
					       
datatype depenventry = 
	 EventEntry          of {name: depname, arguments: depname list}

       | ConditionEntry      of {id: uniq_id, 
				 condlist: depcondition list,
				 deps: dependency list}

       | TimeEntry           of {quantity: depname,
				 initval: real,
				 eq: depequation}
       | ScalarParEntry      of {quantity: depname, 
				 value: real,
				 description: string option}
       | VectorParEntry      of {quantity: depname, 
				 value: real Array.array,
				 description: string option}
       | ScalarStateEntry    of {quantity: depname, 
				 initval: real,
				 method: method option,
				 condition: uniq_id option,
				 event: depname option,
				 eq: depequation,
				 description: string option}
       | VectorStateEntry    of {quantity: depname, 
				 initval: real Array.array,
				 method: method option,
				 condition: uniq_id option,
				 event: depname option,
				 eq: depequation,
				 description: string option}
       | IntegerStateEntry   of {quantity: depname, 
				 initval: int,
				 condition: uniq_id option,
				 event: depname option,
				 eq: depequation,
				 description: string option}
       | ExternalStateEntry  of {quantity: depname,
				 direction: external_dir,
				 channel: int,
				 condition:  uniq_id option,
				 event: depname option,
				 eq: depequation option,
				 description: string option}
       | DiscreteStateEntry  of {quantity: depname,
				 set: set_element' list,
				 condition: uniq_id option,
				 event: depname option,
				 eq: depequation,
				 description: string option}
       | VectorStateFunEntry of {quantity: depname,
				 condition: uniq_id option,
				 event: depname option,
				 eq: depequation,
				 size: int,
				 description: string option}
       | ScalarStateFunEntry of {quantity: depname,
				 condition: uniq_id option,
				 event: depname option,
				 eq: depequation,
				 description: string option}
       | TableFunEntry       of {quantity: depname,
				 body: indepexp,
				 low: real,
				 high: real,
				 step: real,
				 argument: depname,
				 description: string option}
       | FunEntry            of {name: depname,
				 body: indepexp,
				 typeinfo: type_info'}
       | FunAlias            of {name: depname,
				 entryname: depname,
				 typeinfo: type_info'}
       | BuiltinFunEntry     of {name: depname,
				 typeinfo: type_info'}
       | BuiltinScalar       of {name: depname}
       | ForeignScalar       of {name: depname}
       | SetEntry            of {name: depname, value: int}

    type depenv = depenventry UniqueId.table

    val make_depenv:  condmap * condenvmap * eventenvmap * env' -> depenv 
    val query_env:    depenv * DLTypes.enventrytype ->  depenventry list

    val nameofentry:       depenventry       -> depname
    val eqsofentry:        depenventry       -> depequation option
    val depsofentry:       depenventry       -> dependency list
    val idofentry:         depenventry       -> uniq_id
    val symofentry:        depenventry       -> symbol
    val valofentry:        depenventry       -> real list option
    val nameofdepname:     depname           -> name'
    val idofdepname:       depname           -> uniq_id
    val symofdepname:      depname           -> symbol
    val depname2str:       depname           -> string
    val depnamecompare:   depname * depname  -> order
    val typeofentry:      depenventry        -> DLTypes.dl_type option

end

structure DLDeps: DL_DEPS =
struct

type symbol       = Symbol.symbol
type uniq_id      = UniqueId.uniq_id

type condition    = DLEnv.condition
type condmap      = DLEnv.condmap

type exp          = DLEnv'.exp
type env'         = DLEnv'.env'
type equation'    = DLEnv'.equation'
type enventry'    = DLEnv'.enventry'
type name'        = DLEnv'.name'
type set_element' = DLEnv'.set_element'
type condenvmap   = DLEnv'.condenvmap
type eventenvmap  = DLEnv'.eventenvmap

type discrete_exp = DLEnv'.discrete_exp
type type_info'    = DLEnv'.type_info'
type set_element  = DLEnv'.set_element
type external_dir = DLEnv'.external_dir
type differential_arguments = DLEnv'.differential_arguments
type method       = DLEnv'.method

type unaryexpop = DLAst.unaryexpop
type binop      = DLAst.binop
type relop      = DLAst.relop
type indepexp   = DLInDeps.indepexp


exception InvalidType of Symbol.symbol
exception InvalidIdentifier of Symbol.symbol
exception InvalidEntry

datatype depname  = OLD of name'
		  | NEW of name'
		  | STATELESS of name'
		  | BUILTIN of name'
		  | CONDITION of uniq_id
		  | EVENT of name'
		  | EVENTARG of (name' * name')
		  | TABLE of name'
		  | PARAM of name'
		  | SETELM of name'

type dependency = depname


type depcondition = {relop: relop, 
		     quantity1: depname,   
		     quantity2: depname}


datatype depdiscrete_exp = 
	 DISCRETE_ID of depname
       | DISCRETE_CASE of depdiscrete_exp_clause list
			   
and depdiscrete_exp_clause = 
    DISCRETE_COND of depexp * depname
  | DEFAULT_COND of depname
		    
and depexp = 
    CONSTINT of int
  | CONSTREAL of real
  | ID of depname
  | BINOP of {oper: binop, left: depexp, right: depexp}
  | RELOP of {oper: relop, left: depexp, right: depexp}
  | FUNCALL of {function: depname, arguments: depexp list}
  | UNARYEXPOP of {oper: unaryexpop, exp: depexp}
  | CONDITIONAL of {condition: depexp, iftrue: depexp, iffalse: depexp}

datatype depdifferential_arguments = 
	 EULER_ARGS of depexp
       | MAU_ARGS of depexp * depexp


datatype depequation = DifferenceEquation   of {line: int, exp: depexp, deps: dependency list}
		     | DifferentialEquation of {line: int, 
						arg: depdifferential_arguments,
						deps: dependency list}
		     | AlgebraicEquation    of {line: int, exp: depexp, 
						deps: dependency list}
		     | DiscreteEquation     of {line: int, exp: depdiscrete_exp, 
						deps: dependency list}
					       
datatype depenventry = 
	 EventEntry          of {name: depname, arguments: depname list}
				
       | ConditionEntry      of {id: uniq_id, 
				 condlist: depcondition list,
				 deps: dependency list}

       | TimeEntry           of {quantity: depname,
				 initval: real,
				 eq: depequation}
       | ScalarParEntry      of {quantity: depname, 
				 value: real,
				 description: string option}
       | VectorParEntry      of {quantity: depname, 
				 value: real Array.array,
				 description: string option}
       | ScalarStateEntry    of {quantity: depname, 
				 initval: real,
				 method: method option,
				 condition: uniq_id option,
				 event: depname option,
				 eq: depequation,
				 description: string option}
       | VectorStateEntry    of {quantity: depname, 
				 initval: real Array.array,
				 method: method option,
				 condition: uniq_id option,
				 event: depname option,
				 eq: depequation,
				 description: string option}
       | IntegerStateEntry   of {quantity: depname, 
				 initval: int,
				 condition: uniq_id option,
				 event: depname option,
				 eq: depequation,
				 description: string option}
       | ExternalStateEntry  of {quantity: depname,
				 direction: external_dir,
				 channel: int,
				 condition:  uniq_id option,
				 event: depname option,
				 eq: depequation option,
				 description: string option}
       | DiscreteStateEntry  of {quantity: depname,
				 set: set_element' list,
				 condition: uniq_id option,
				 event: depname option,
				 eq: depequation,
				 description: string option}

       | VectorStateFunEntry of {quantity: depname,
				 condition: uniq_id option,
				 event: depname option,
				 eq: depequation,
				 size: int,
				 description: string option}
       | ScalarStateFunEntry of {quantity: depname,
				 condition: uniq_id option,
				 event: depname option,
				 eq: depequation,
				 description: string option}
       | TableFunEntry       of {quantity: depname,
				 body: indepexp,
				 low: real,
				 high: real,
				 step: real,
				 argument: depname,
				 description: string option}
       | FunEntry            of {name: depname,
				 body: indepexp,
				 typeinfo: type_info'}
       | FunAlias            of {name: depname,
				 entryname: depname,
				 typeinfo: type_info'}
       | BuiltinFunEntry     of {name: depname,
				 typeinfo: type_info'}
       | BuiltinScalar       of {name: depname}
       | ForeignScalar       of {name: depname}
       | SetEntry            of {name: depname, value: int}

type depenv       = depenventry UniqueId.table

(* Return the initialization value of an entry, if one exists *)
fun vector2list v = Array.foldl (fn (i, l) => i::l) nil v
fun valofentry (entry) = 
    (case entry of 
	 ScalarParEntry    {value, ...}   => SOME [value]
       | VectorParEntry    {value, ...}   => SOME (vector2list value)
       | ScalarStateEntry  {initval, ...} => SOME [initval]
       | VectorStateEntry  {initval, ...} => SOME (vector2list initval)
       | IntegerStateEntry {initval, ...} => SOME [Real.fromInt initval]
       | TableFunEntry     {low, ...}     => SOME [low]
       | _ => NONE)

(* Return the equations of an environment entry *)

fun eqsofentry (ScalarStateEntry    {quantity, initval, method, eq, ...}) = SOME eq
  | eqsofentry (VectorStateEntry    {quantity, initval, method, eq, ...}) = SOME eq
  | eqsofentry (IntegerStateEntry   {quantity, initval, eq, ...})         = SOME eq
  | eqsofentry (ExternalStateEntry  {quantity, direction, eq, ...})       = eq
  | eqsofentry (DiscreteStateEntry  {quantity, set, eq, ...})             = SOME eq
  | eqsofentry (ScalarStateFunEntry {quantity, eq, ...})                  = SOME eq
  | eqsofentry (VectorStateFunEntry {quantity, eq, ...})                  = SOME eq
  | eqsofentry (_)       = NONE

(* Return the dependencies of an equation *)

fun depsofentry (entry) =
    (case entry of 
	ConditionEntry {id, condlist, deps} => deps
      | _ => 
	let
	    val eq = eqsofentry entry
	in
	    (case  eq of 
		SOME (DifferenceEquation   {line, exp, deps})    => deps
	      | SOME (DifferentialEquation {line, arg, deps})    => deps
	      | SOME (AlgebraicEquation    {line, exp, deps})    => deps 
	      | SOME (DiscreteEquation     {line, exp, deps})    => deps
	      | NONE => [])
	end)

(* Returns an optional condition list for an depenventry *)
fun condof (ScalarStateEntry    {quantity, initval, method, condition, ...})  = condition
  | condof (VectorStateEntry    {quantity, initval, method, condition, ...})  = condition
  | condof (IntegerStateEntry   {quantity, initval, condition, ...})          = condition
  | condof (ExternalStateEntry  {quantity, direction, condition, ...})        = condition
  | condof (DiscreteStateEntry  {quantity, set, condition, ...})              = condition
  | condof (ScalarStateFunEntry {quantity, condition, ...})                   = condition
  | condof (VectorStateFunEntry {quantity, condition, ...})                   = condition
  | condof (_)       = NONE


(* Returns an optional condition list for an depenventry *)
fun eventof (ScalarStateEntry    {quantity, event, ...})  = event
  | eventof (VectorStateEntry    {quantity, event, ...})  = event
  | eventof (IntegerStateEntry   {quantity, event, ...})  = event
  | eventof (ExternalStateEntry  {quantity, event, ...})  = event
  | eventof (DiscreteStateEntry  {quantity, event, ...})  = event
  | eventof (ScalarStateFunEntry {quantity, event, ...})  = event
  | eventof (VectorStateFunEntry {quantity, event, ...})  = event
  | eventof (_)       = NONE


(* Return the name of an environment entry *)
fun nameofentry (EventEntry  {name, ...})  = name
  | nameofentry (ConditionEntry  {id: uniq_id, ...})  = CONDITION id
  | nameofentry (TimeEntry {quantity, ...})           = quantity
  | nameofentry (ScalarParEntry      {quantity, ...}) = quantity
  | nameofentry (VectorParEntry      {quantity, ...}) = quantity
  | nameofentry (ScalarStateEntry    {quantity, ...}) = quantity
  | nameofentry (VectorStateEntry    {quantity, ...}) = quantity
  | nameofentry (IntegerStateEntry   {quantity, ...}) = quantity
  | nameofentry (ExternalStateEntry  {quantity, ...}) = quantity
  | nameofentry (DiscreteStateEntry  {quantity, ...}) = quantity
  | nameofentry (ScalarStateFunEntry {quantity, ...}) = quantity
  | nameofentry (VectorStateFunEntry {quantity, ...}) = quantity
  | nameofentry (TableFunEntry       {quantity, ...}) = quantity
  | nameofentry (FunEntry            {name, ...})     = name
  | nameofentry (FunAlias            {name, ...})     = name
  | nameofentry (BuiltinFunEntry     {name, ...})     = name
  | nameofentry (BuiltinScalar       {name})          = name
  | nameofentry (ForeignScalar       {name})          = name
  | nameofentry (SetEntry            {name, value})   = name

fun idofentry (entry) = 
    (case (nameofentry entry) of
	 OLD name => DLEnv'.idofname name
       | NEW name => DLEnv'.idofname name
       | STATELESS name => DLEnv'.idofname name
       | BUILTIN name => DLEnv'.idofname name
       | CONDITION id => id
       | EVENT name => DLEnv'.idofname name
       | EVENTARG (name,_) => DLEnv'.idofname name
       | TABLE name => DLEnv'.idofname name
       | PARAM name => DLEnv'.idofname name
       | SETELM name => DLEnv'.idofname name)

fun symofentry (entry) = 
    (case (nameofentry entry) of
	 OLD name => DLEnv'.symofname name
       | NEW name => DLEnv'.symofname name
       | STATELESS name => DLEnv'.symofname name
       | BUILTIN name => DLEnv'.symofname name
       | CONDITION id => Symbol.symbol ("#condition " ^ (Int.toString (UniqueId.id2int id)))
       | EVENT name => DLEnv'.symofname name
       | EVENTARG (name,_) => DLEnv'.symofname name
       | TABLE name => DLEnv'.symofname name
       | PARAM name => DLEnv'.symofname name
       | SETELM name => DLEnv'.symofname name)


fun nameofdepname(name) = 
    (case name of
	 OLD name => name
       | NEW name => name
       | STATELESS name => name
       | BUILTIN name => name
       | CONDITION id => {name=Symbol.symbol ("#condition"),id=id}
       | EVENT name => name
       | EVENTARG (name,_) => name
       | TABLE name => name
       | PARAM name => name
       | SETELM name => name)
	      

fun idofdepname (name) = 
    DLEnv'.idofname (nameofdepname name)

fun symofdepname (name) = 
    DLEnv'.symofname (nameofdepname name)

fun depname2str (name) =
    let 
	val (prefix, name) = 
	    case name of
		OLD (n) => ("OLD", DLEnv'.name2str n)
	      | NEW (n) => ("NEW", DLEnv'.name2str n)
	      | STATELESS (n) => ("STATELESS", DLEnv'.name2str n)
	      | BUILTIN (n) => ("BUILTIN", DLEnv'.name2str n)
	      | CONDITION id => ("CONDITION", UniqueId.id2str id)
	      | EVENTARG (_,name) => ("EVENTARG", DLEnv'.name2str name)
	      | EVENT n => ("EVENT", DLEnv'.name2str n)
	      | TABLE n => ("TABLE", DLEnv'.name2str n)
	      | PARAM n => ("PARAM", DLEnv'.name2str n)
	      | SETELM n => ("SETELM", DLEnv'.name2str n)
    in
	prefix ^ " " ^ name
    end

	      

(* compare 2 depnames *)
fun depnamecompare (x, y) =  
    let
	fun depname2key (OLD _)       = 1
	  | depname2key (NEW _)       = 2
	  | depname2key (STATELESS _) = 3
	  | depname2key (CONDITION _) = 4
	  | depname2key (BUILTIN _)   = 5
	  | depname2key (TABLE _)     = 6
	  | depname2key (PARAM _)     = 7
	  | depname2key (EVENT _)     = 8
	  | depname2key (EVENTARG _)  = 9
	  | depname2key (SETELM _)    = 10

	fun keyof x = (UniqueId.id2int (idofdepname x))
		
    in
	case Int.compare(depname2key x, depname2key y) of
	    EQUAL  => Int.compare(keyof x, keyof y)
	  | result => result 

    end

fun typeofentry (entry) =
    (case entry of
	 TimeEntry _            => SOME DLTypes.Scalar
       | ScalarParEntry _       => SOME DLTypes.Scalar
       | ScalarStateEntry _     => SOME DLTypes.Scalar
       | IntegerStateEntry _    => SOME DLTypes.Scalar
       | ExternalStateEntry _   => SOME DLTypes.Scalar
       | DiscreteStateEntry _   => SOME DLTypes.Discrete
       | ScalarStateFunEntry _  => SOME DLTypes.Scalar
       | TableFunEntry _        => SOME DLTypes.Scalar
       | BuiltinScalar _        => SOME DLTypes.Scalar
       | ForeignScalar _        => SOME DLTypes.Scalar
       | SetEntry _             => SOME DLTypes.Discrete
       | VectorStateEntry {initval, ...} =>
	 SOME (DLTypes.Vector (SOME (Array.length initval)))
       | VectorParEntry {value, ...} => 
	 SOME (DLTypes.Vector (SOME (Array.length value)))
       | VectorStateFunEntry {size, ...} =>
	 SOME (DLTypes.Vector (SOME size))
       | _                => NONE)

   
fun discrete_make_dep (make_depname, exp) =
    let
	fun name_make_dep(make_depname, name) =
	    let
		val (deps, ID name) = make_dep (make_depname, DLAst.ID name)
	    in
		(deps, name)
	    end
		
	fun addPairToList ((dep, exp), (deps, exps)) =
	    (dep @ deps, exp::exps)

	fun rewrite_clause clause = 
	    case clause of 
		DLAst.DISCRETE_COND (exp,name) =>
		let
		    val (deps1, name)  = name_make_dep (make_depname, name)
		    val (deps2, exp2)  = make_dep (make_depname, exp)
		in
		    (deps2, DISCRETE_COND (exp2, name)) 
		end
		    
	      | DLAst.DEFAULT_COND (name) => 
		let
		    val (deps, name) = name_make_dep (make_depname, name)
		in
		    (deps, DEFAULT_COND name)
		end

    in

	case exp of 
	    DLAst.DISCRETE_ID name => 
	    let
		val (deps:dependency list, name) =
		    name_make_dep (make_depname, name)
	    in
		(deps, DISCRETE_ID name)
	    end
	    
	  | DLAst.DISCRETE_CASE clauses =>
	    let
		val (deps, clauses) =
		    foldl addPairToList
			  (nil, nil)
			  (map rewrite_clause clauses)	
	    in
		(deps, DISCRETE_CASE clauses)
	    end
    end

and make_dep (make_depname, exp: DLAst.exp) =
    
    case exp of
	DLAst.CONSTINT (num) => (nil, CONSTINT (num))
	
      | DLAst.CONSTREAL (num) => (nil, CONSTREAL (num))

      |	DLAst.ID {name, pos} => (fn(dep) => ([dep], ID dep)) (make_depname name)
	 
      | DLAst.BINOP {oper=oper, left=left, right=right, pos=pos} =>
	
	let	
	    val (leftdep, leftexp)   = make_dep(make_depname, left)
	    val (rightdep, rightexp) = make_dep(make_depname, right)
	in
	    (leftdep @ rightdep, 
	     BINOP {oper=oper, left=leftexp, right=rightexp})
	end
	    
      | DLAst.RELOP {oper=oper, left=left, right=right, pos=pos} =>
	
	
	let	
	    val (leftdep, leftexp)   = make_dep(make_depname, left)
	    val (rightdep, rightexp) = make_dep(make_depname, right)
	in
	    (leftdep @ rightdep,
	     RELOP {oper=oper, left=leftexp, right=rightexp})
	end
	    
      | DLAst.FUNCALL {function=function, arguments=arguments, pos=pos} =>
	let
	    fun getDeps(e) = make_dep (make_depname, e)
	    val resultlist = map getDeps arguments
			     
	    val deplist = map (fn(x) => (#1 x)) resultlist
			      
	    val explist = map (fn(x) => (#2 x)) resultlist

	in
	    (foldl (op @) nil deplist,
	     FUNCALL {function=make_depname (#name function), 
		      arguments=explist})
	end
	    
      | DLAst.UNARYEXPOP {oper=oper, exp=exp, pos=pos} =>
	let
	    val (dep, exp) = make_dep (make_depname, exp)
	in
	    (dep, UNARYEXPOP {oper=oper, exp=exp})
	end
	    
	    
      | DLAst.CONDITIONAL {condition=cond, iftrue=iftrue, iffalse=iffalse, pos=pos} =>
	
	let
	    val (conddep, condexp)       = make_dep (make_depname, cond)
	    val (iftruedep, iftrueexp)   = make_dep (make_depname, iftrue)
	    val (iffalsedep, iffalseexp) = make_dep (make_depname, iffalse)
	in
	    (conddep @ iftruedep @ iffalsedep,
	     CONDITIONAL {condition=condexp, iftrue=iftrueexp, 
			  iffalse=iffalseexp})
	end
	    
	    
fun memoize_make_depname (c,g,l,ev) (name) =
    let 
	val entry = 
	    (case ev name of 
		 SOME entry => entry
	       | NONE => (case (l name) of 
			      NONE       => (g name)
			    | SOME entry => entry))
    in
	case entry of
	    DLEnv'.EventEntry ({name, ...})            => EVENT name
	  | DLEnv'.EventArgument {name,event}          => EVENTARG (name,event)
	  | DLEnv'.TimeEntry ({quantity, ...})         => OLD quantity
	  | DLEnv'.ScalarParEntry {quantity, ...}      => PARAM quantity
	  | DLEnv'.VectorParEntry {quantity, ...}      => PARAM quantity
	  | DLEnv'.ScalarStateEntry {quantity, ...}    => OLD quantity
	  | DLEnv'.VectorStateEntry {quantity, ...}    => OLD quantity
	  | DLEnv'.IntegerStateEntry {quantity, ...}   => OLD quantity
	  | DLEnv'.ExternalStateEntry {quantity, ...}  => NEW quantity
	  | DLEnv'.DiscreteStateEntry {quantity, ...}  => OLD quantity
	  | DLEnv'.ScalarStateFunEntry {quantity, ...} => STATELESS quantity
	  | DLEnv'.VectorStateFunEntry {quantity, ...} => STATELESS quantity
	  | DLEnv'.TableFunEntry {quantity, ...}       => TABLE quantity
	  | DLEnv'.FunEntry {name, ...}                => STATELESS name
	  | DLEnv'.FunAlias {name, ...}                => STATELESS name
	  | DLEnv'.BuiltinFunEntry {name, ...}         => BUILTIN name
	  | DLEnv'.BuiltinScalar {name}                => BUILTIN name
	  | DLEnv'.ForeignScalar {name}                => STATELESS name
	  | DLEnv'.SetEntry {name, value}              => SETELM name
	  | DLEnv'.Dummy                               => raise InvalidEntry
    
    end

	    

fun make_depequation (event, condition, condenv, env: env', eq) =
    let
	val event        = case event of 
			       SOME name => Symbol.look(env,DLEnv'.symofname name)
			     | NONE => NONE

	val event_args   = case event of 
			       SOME (DLEnv'.EventEntry {name,arguments}) => 
			       map (fn(x) => DLEnv'.EventArgument {name=x,event=name}) arguments
			     | _ => []
				       
	fun event_compare x = (fn(DLEnv'.EventArgument {name,...}) => (x=(DLEnv'.symofname name))
			       | _ => false)

	val make_depname = memoize_make_depname
			       (condition,
				(fn(sym) => 
				   (case (Symbol.look (env, sym)) of
						SOME entry => entry
					  | NONE => raise InvalidEntry 
							  before (print ("sym = " ^ (Symbol.name sym) ^ "\n")))),
				(fn(sym) => 
				   (case condenv of 
					SOME condenv =>
					(Symbol.look (condenv, sym))
				      | NONE => NONE)),
				fn(sym) => List.find (event_compare sym) event_args)

    in
	case eq of 
	    DLEnv'.DifferenceEquation {line, exp} =>
	    let
		val (deps, exp) = let val (deps,exp) = make_dep (make_depname, exp)
				  in 
				      (List.filter (fn(EVENTARG _) => false | _ => true) deps, exp)
				  end
		val deps = ListMergeSort.uniqueSort 
			       (fn(d1,d2) => Symbol.compare (symofdepname d1,
							     symofdepname d2))
			       deps
	    in
		DifferenceEquation {line=line, exp=exp, deps=deps}
	    end

	  | DLEnv'.DifferentialEquation {line=line, arg=arg} => 
	    (case arg of 
		 DLAst.EULER_ARGS exp =>
		 let
		     val (deps, exp) = let val (deps,exp) = make_dep (make_depname, exp)
				       in 
					   (List.filter (fn(EVENTARG _) => false | _ => true) deps, exp)
				       end
		     val deps = ListMergeSort.uniqueSort 
				    (fn(d1,d2) => Symbol.compare (symofdepname d1,
								  symofdepname d2))
				    deps

		 in
		     DifferentialEquation {line=line, arg=EULER_ARGS exp, deps=deps}
		 end
		     
	       | DLAst.MAU_ARGS (exp1, exp2) =>
		 let
		     val (deps1, exp1) = make_dep (make_depname, exp1)
		     val (deps2, exp2) = make_dep (make_depname, exp2)

		     val deps = List.filter (fn(EVENTARG _) => false | _ => true) (deps1 @ deps2)
		     val deps = ListMergeSort.uniqueSort 
				    (fn(d1,d2) => Symbol.compare (symofdepname d1,
								  symofdepname d2))
				    deps

		 in
		     DifferentialEquation {line=line, arg=MAU_ARGS(exp1, exp2), deps=deps}
		 end)
	    
	  | DLEnv'.AlgebraicEquation  {line, exp} =>
	    let
		val (deps, exp) = let val (deps,exp) = make_dep (make_depname, exp)
				  in 
				      (List.filter (fn(EVENTARG _) => false | _ => true) deps, exp)
				  end
		val deps = ListMergeSort.uniqueSort 
			       (fn(d1,d2) => Symbol.compare (symofdepname d1,
							     symofdepname d2))
			       deps
	    in
		AlgebraicEquation {line=line, exp=exp, deps=deps}
	    end

	  | DLEnv'.DiscreteEquation  {line, exp} =>
	    let
		val (deps, exp) = let val (deps,exp) = discrete_make_dep (make_depname, exp)
				  in 
				      (List.filter (fn(EVENTARG _) => false | _ => true) deps, exp)
				  end
		val deps = ListMergeSort.uniqueSort 
			       (fn(d1,d2) => Symbol.compare (symofdepname d1,
							     symofdepname d2))
			       deps
	    in
		DiscreteEquation {line=line, exp=exp, deps=deps}
	    end
    end

fun make_condentry (condmap, condenvmap, env) (condid, depenv) =
    let
	val condlist = valOf(UniqueId.look (condmap, condid))

	fun memoize_lookup (g, l) (sym) =
	    (case (l sym) of 
		 NONE       => (g sym)
	       | SOME entry => entry)


	val (deps, condlist') = 
	    foldl (fn ({relop, quantity1, quantity2, parentid}: condition, (deps,condlist)) =>
		    let
			val condenv = (case parentid of
					   SOME id => UniqueId.look (condenvmap, id)
					 | NONE => NONE)
				      
			val lookup = memoize_lookup 
					 ((fn(sym) => case (Symbol.look (env, sym)) of
									  SOME entry => entry
									| NONE => raise InvalidEntry),
					  (fn(sym) => (case condenv of 
							   SOME condenv =>
							   Symbol.look (condenv, sym)
							 | NONE => NONE)))

			val make_depname = memoize_make_depname
					       (parentid,
						(fn(sym) => 
						   (case (Symbol.look (env, sym)) of
							SOME entry => entry
							  | NONE => raise InvalidEntry)),
						(fn(sym) => 
						   (case condenv of 
							SOME condenv =>
							(Symbol.look (condenv, sym))
						      | NONE => NONE)),
						(fn(sym) => NONE))

					 
			val (q1, q2) = (make_depname quantity1,
					make_depname quantity2)
		    in
			(q1::q2::deps,
			 {relop=relop, quantity1=q1, quantity2=q2}::condlist)
		    end)
		  ([], [])
		condlist

	val deps = List.filter (fn(EVENTARG _) => false | _ => true) deps

	val entry = ConditionEntry {id=condid, condlist=condlist', deps=deps}

    in
	UniqueId.enter (depenv, condid, entry)
    end

(*
fun make_tdivname factor = Symbol.symbol ("dt" ^ (Int.toString factor))

fun make_tdiventry (factor,(tdivenv,depenv)) =
    let 
	val sym  =  make_tdivname factor
	val id   =  Symbol.look (tdivenv,sym)
    in
	case id of 
	    SOME id => (tdivenv,depenv)
	  | NONE => (let
			 val id    = UniqueId.genid()
			 val name  = {name=sym, id=id}
			 val entry = TimeDivEntry {name=STATELESS name, factor=factor}
		     in
			 (Symbol.enter(tdivenv,sym,id), UniqueId.enter (depenv, id, entry))
		     end)
    end
*)

fun make_depenventry (condenvmap: condenvmap, env: env', entry)  =

    let

    in
	case entry of
	    
	    DLEnv'.EventEntry {name, arguments} => 
	    EventEntry {name=EVENT name, 
			arguments = map (fn (x) => EVENTARG (x,name)) arguments}

	  | DLEnv'.TimeEntry ({quantity, initval, eq}) =>
	    
	    TimeEntry ({quantity=NEW quantity,
			initval=initval,
			eq = make_depequation (NONE,NONE,NONE,env,eq)})
	    
	  | DLEnv'.ScalarParEntry {quantity, value, description} =>
	    
	    ScalarParEntry {quantity=PARAM quantity, value=value, 
			    description=description}
	    
	  | DLEnv'.VectorParEntry {quantity, value, description} =>
	    
	    VectorParEntry {quantity=PARAM quantity, value=value, 
			    description=description}
	    
	  | DLEnv'.ScalarStateEntry {quantity, initval, method, 
				     condition, event,  
				     eq, description} =>
	    let
		val condenv = (case condition of 
				   SOME condition => SOME (valOf(UniqueId.look (condenvmap, condition)))
				 | NONE => NONE)
				
		val eventname = (case event of SOME event => SOME (EVENT event)
					     | NONE => NONE)
				
	    in
		ScalarStateEntry {quantity=NEW quantity, initval=initval, method=method, 
				  condition=condition, event=eventname, 
				  eq=make_depequation (event,condition,condenv,env,eq),
				  description=description}
	    end

	  | DLEnv'.VectorStateEntry {quantity, initval, method, 
				     condition, event,  
				     eq, description} =>
	    let
		val condenv = 
		    case condition of 
			SOME condition => SOME (valOf(UniqueId.look (condenvmap, condition)))
		      | NONE => NONE
				
		val eventname = case event of SOME event => SOME (EVENT event)
					    | NONE => NONE
		
	    in
		VectorStateEntry {quantity=NEW quantity, initval=initval,
				  method=method, condition=condition, event=eventname, 
				  eq=make_depequation (event,condition,condenv,env,eq),
				  description=description}
	    end

	  | DLEnv'.IntegerStateEntry {quantity, initval, 
				      condition, event,  
				      eq, description} =>
	    
	    let
		val condenv = 
		    case condition of 
			SOME condition => SOME (valOf(UniqueId.look (condenvmap, condition)))
		      | NONE => NONE

		val eventname = case event of SOME event => SOME (EVENT event)
					| NONE => NONE
	    in
		IntegerStateEntry {quantity=NEW quantity, initval=initval, 
				   condition=condition, event=eventname, 
				   eq=make_depequation (event,condition,condenv,env,eq), 
				   description=description}
	    end

	  | DLEnv'.ExternalStateEntry {quantity, direction, channel,
				       condition, event, eq, description} =>
	    let
		val condenv = 
		    case condition of 
			SOME condition => SOME (valOf(UniqueId.look (condenvmap, condition)))
		      | NONE => NONE

		val eventname = case event of SOME event => SOME (EVENT event)
					| NONE => NONE

	    in
		ExternalStateEntry {quantity=NEW quantity, 
				    direction=direction, channel=channel,  
				    condition=condition, event=eventname,
				    eq=(case eq of 
					    SOME eq => SOME (make_depequation (event,condition,condenv,env,eq))
					  | NONE => NONE),
				    description=description}
	    end

	  | DLEnv'.DiscreteStateEntry {quantity, set, 
				       condition, event,
				       eq, description} =>

	    let
		val condenv = 
		    case condition of 
			SOME condition => SOME (valOf(UniqueId.look (condenvmap, condition)))
		      | NONE => NONE
				
		val eventname = case event of SOME event => SOME (EVENT event)
					| NONE => NONE

	    in
		DiscreteStateEntry {quantity=NEW quantity, set=set,
				    condition=condition, event=eventname, 
				    eq=make_depequation (event,condition,condenv,env,eq), 
				    description=description}
	    end

	  | DLEnv'.VectorStateFunEntry {quantity, 
					condition, event, 
					eq, size, description} =>

	    let
		val condenv = 
		    case condition of 
			SOME condition => SOME (valOf(UniqueId.look (condenvmap, condition)))
		      | NONE => NONE

		val eventname = case event of SOME event => SOME (EVENT event)
					| NONE => NONE
	    in
		VectorStateFunEntry {quantity=STATELESS quantity, 
				     condition=condition, event=eventname, 
				     eq=make_depequation (event,condition,condenv,env,eq),
				     size=size, description=description}
	    end
		
	  | DLEnv'.ScalarStateFunEntry {quantity, 
					condition, event, 
					eq, description} =>
	    
	    let
		val condenv = 
		    case condition of 
			SOME condition => SOME (valOf (UniqueId.look (condenvmap, condition)))
		      | NONE => NONE
				
		val eventname = case event of SOME event => SOME (EVENT event)
					| NONE => NONE

	    in
		ScalarStateFunEntry {quantity=STATELESS quantity, 
				     condition=condition, event=eventname, 
				     eq=make_depequation(event,condition,condenv,env,eq),
				     description=description}
	    end

	  | DLEnv'.TableFunEntry {quantity, body, low, high, step, argument,
				  description} =>

	    let
		fun make_depname (name: name') =
		    (case valOf(Symbol.look (env, DLEnv'.symofname name)) of
			DLEnv'.EventEntry ({name, ...})            => EVENT name
		      | DLEnv'.EventArgument ({name, event})       => EVENTARG(name,event)
		      | DLEnv'.TimeEntry ({quantity, ...})         => OLD quantity
		      | DLEnv'.ScalarParEntry {quantity, ...}      => PARAM quantity
		      | DLEnv'.VectorParEntry {quantity, ...}      => PARAM quantity
		      | DLEnv'.ScalarStateEntry {quantity, ...}    => OLD quantity
		      | DLEnv'.VectorStateEntry {quantity, ...}    => OLD quantity
		      | DLEnv'.IntegerStateEntry {quantity, ...}   => OLD quantity
		      | DLEnv'.ExternalStateEntry {quantity, ...}  => NEW quantity
		      | DLEnv'.DiscreteStateEntry {quantity, ...}  => OLD quantity
		      | DLEnv'.ScalarStateFunEntry {quantity, ...} => STATELESS quantity
		      | DLEnv'.VectorStateFunEntry {quantity, ...} => STATELESS quantity
		      | DLEnv'.TableFunEntry {quantity, ...}       => TABLE quantity
		      | DLEnv'.FunEntry {name, ...}                => STATELESS name
		      | DLEnv'.FunAlias {name, ...}                => STATELESS name
		      | DLEnv'.BuiltinFunEntry {name, ...}         => BUILTIN name
		      | DLEnv'.BuiltinScalar {name}                => BUILTIN name
		      | DLEnv'.ForeignScalar {name}                => STATELESS name
		      | DLEnv'.SetEntry {name, value}              => SETELM name
		      | DLEnv'.Dummy                               => raise InvalidEntry)
			

	    in
		TableFunEntry {quantity=TABLE quantity, 
			       body=DLInDeps.make_indep (env, [argument], body),  
			       low=low, high=high, step=step, argument=make_depname argument, 
			       description=description}
	    end

	  | DLEnv'.FunEntry {name, body, typeinfo} =>
	    FunEntry {name=STATELESS name, 
		      body=DLInDeps.make_indep (env, map (fn (arg) => #name arg) (#args typeinfo), body), 
		      typeinfo=typeinfo}

	  | DLEnv'.FunAlias {name, entryname, typeinfo} =>
	    FunAlias {name=STATELESS name, 
		      entryname=STATELESS entryname, 
		      typeinfo=typeinfo}

	  | DLEnv'.BuiltinFunEntry {name, typeinfo} =>
	    
	    BuiltinFunEntry {name=BUILTIN name, typeinfo=typeinfo}

	  | DLEnv'.BuiltinScalar {name} =>
	    
	    BuiltinScalar {name=BUILTIN name}

	  | DLEnv'.ForeignScalar {name} =>
	    
	    ForeignScalar {name=STATELESS name}

	  | DLEnv'.SetEntry {name, value} =>
	    SetEntry {name=SETELM name, value=value}

	  | DLEnv'.Dummy => raise InvalidEntry

	  | DLEnv'.EventArgument _ => raise InvalidEntry
    end

(* Query an environment for all entries of a particular type *)
fun query_env (env, typ: DLTypes.enventrytype) =

    let
	open DLTypes

	fun query_env_lambda (typ)  = 
	    (case typ of 
		 TimeEntryType =>
		 (fn(e) => (case e of 
				TimeEntry _ => SOME e
			      | _ => NONE))

 	       | ScalarParEntryType =>
		 (fn(e) => (case e of 
				ScalarParEntry _ => SOME e
			      | _ => NONE))
	       | VectorParEntryType _ =>
		 (fn(e) => (case e of 
				VectorParEntry _ => SOME e
			      | _ => NONE))
	       | ScalarStateEntryType =>
		 (fn(e) => (case e of 
				ScalarStateEntry _ => SOME e
			      | _ => NONE))
	       | VectorStateEntryType _ =>
		 (fn(e) => (case e of 
				VectorStateEntry _ => SOME e
			      | _ => NONE))

	       | IntegerStateEntryType =>
		 (fn(e) => (case e of 
				IntegerStateEntry _ => SOME e
			      | _ => NONE))
	       | ExternalStateEntryType =>
		 (fn(e) => (case e of 
				ExternalStateEntry _ => SOME e
			      | _ => NONE))
	       | DiscreteStateEntryType =>
		 (fn(e) => (case e of 
				DiscreteStateEntry _ => SOME e
			      | _ => NONE))
	       | VectorStateFunEntryType _ =>
		 (fn(e) => (case e of 
				VectorStateFunEntry _ => SOME e
			      | _ => NONE))
	       | ScalarStateFunEntryType =>
		 (fn(e) => (case e of 
				ScalarStateFunEntry _ => SOME e
			      | _ => NONE))
	       | TableFunEntryType =>
		 (fn(e) => (case e of 
				TableFunEntry _ => SOME e
			      | _ => NONE))
	       | FunEntryType =>
		 (fn(e) => (case e of 
				FunEntry _ => SOME e
			      | _ => NONE))
	       | FunAliasType =>
		 (fn(e) => (case e of 
				FunAlias _ => SOME e
			      | _ => NONE))
	       | BuiltinFunEntryType =>
		 (fn(e) => (case e of 
				BuiltinFunEntry _ => SOME e
			      | _ => NONE))
	       | BuiltinScalarType  =>
		 (fn(e) => (case e of 
				BuiltinScalar _ => SOME e
			      | _ => NONE))
	       | SetEntryType =>
		 (fn(e) => (case e of
				SetEntry _ => SOME e
			      | _ => NONE))
	       | ForeignScalarType  =>
		 (fn(e) => (case e of 
				ForeignScalar _ => SOME e
			      | _ => NONE)))
	    
	val query_env' = query_env_lambda (typ)
    in
	List.mapPartial query_env' (UniqueId.listItems env)
    end



fun make_depenv (condmap: condmap, 
		 condenvmap: condenvmap, 
		 eventenvmap: eventenvmap, 
		 env: env') =
    let
	val make_condentry    = make_condentry (condmap, condenvmap, env)

	val depenv' = foldl make_condentry (UniqueId.empty) (UniqueId.listKeys condmap)

(*
	fun filter_tdiventry (entry) = 
	    (case entry of 
		 DLEnv'.ScalarStateEntry {timediv=SOME factor, ...} => SOME factor
	       | DLEnv'.VectorStateEntry {timediv=SOME factor, ...} => SOME factor
	       | DLEnv'.IntegerStateEntry  {timediv=SOME factor, ...} => SOME factor
	       | DLEnv'.VectorStateFunEntry {timediv=SOME factor, ...} => SOME factor
	       | DLEnv'.ScalarStateFunEntry {timediv=SOME factor, ...} => SOME factor
	       | _ => NONE)
	val (tdivenv,depenv'') = (foldl make_tdiventry (Symbol.empty, depenv') 
					(List.mapPartial filter_tdiventry (Symbol.listItems env)))
*)
						   
    in
	foldl (fn (entry,depenv) => 
		  let
		      val entry' = make_depenventry (condenvmap, env, entry)
		  in
		      UniqueId.enter (depenv, idofentry entry', entry')
		  end)

	      depenv'
 
	      ((foldl (fn (eventenv,ax) => (Symbol.listItems eventenv) @ ax) [] (Symbol.listItems eventenvmap)) @
	       ((Symbol.listItems env) @ 
		(foldl (fn (env, envlist) => (Symbol.listItems env) @ envlist)
		       nil (UniqueId.listItems condenvmap))))
	
    end

end
