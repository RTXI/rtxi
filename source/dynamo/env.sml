
signature DL_ENV =
sig

  structure Ast: DL_AST
  structure AliasMap: TABLE

  type exp        
  type relop
  type symbol
  type typeset
  type dl_type
  type type_info
  type uniq_id
  type discrete_exp
  type external_dir
  type set_element
  type method
  type env
  type arg
  type name
  type differential_arguments

  type aliasmap

  type condition = {relop: relop, 
		    quantity1: symbol,   
		    quantity2: symbol,
		    parentid: uniq_id option}
  type condmap      = condition list UniqueId.table


  datatype equation = AlgebraicEquation     of {line: int, exp: exp}
		    | DifferenceEquation    of {line: int, exp: exp}
		    | DifferentialEquation  of {line: int, arg: differential_arguments}
					      
		    | DiscreteEquation of {line: int, exp: discrete_exp}

		    | ConditionalBlock of {line: int, condition: uniq_id, 
					   eq: equation}
		    | EventBlock of {line: int, event: name, eq: equation}
				
  datatype argentry = ArgumentEntry of {name: name,
					typ: typeset}
	  
  datatype enventry = TimeEntry of {quantity:name,
				    initval:real,
				    eq: equation}
		    | ScalarParEntry of {quantity: name, 
					 value: real,
					 description: string option}
		    | VectorParEntry of {quantity: name, 
					 value: real Array.array,
					 description: string option}
		    | ScalarStateEntry of {quantity: name, 
					   initval: real,
					   method: method option,
					   eqs: equation list,
					   description: string option}
		    | VectorStateEntry of {quantity: name, 
					   initval: real Array.array,
					   method: method option,
					   eqs: equation list,
					   description: string option}
		    | IntegerStateEntry of {quantity: name, 
					    initval: int,
					    eqs: equation list,
					    description: string option}
		    | ExternalStateEntry of {quantity: name,
					     direction: external_dir,
					     channel: int option,
					     eqs: equation list option,
					     description: string option}
		    | DiscreteStateEntry of {quantity: name,
					     set: set_element list,
					     eqs: equation list,
					     description: string option}
		    | ScalarStateFunEntry of {quantity: name,
					      eqs: equation list,
					      description: string option}
		    | VectorStateFunEntry of {quantity: name,
					      eqs: equation list,
					      size: int option,
					      description: string option}

		    | TableFunEntry of {quantity: name,
					body: exp,
					low: real,
					high: real,
					step: real,
					argument: name,
					description: string option}
		    | FunEntry of {name: name,
				   body: exp,
				   typeinfo: type_info,
				   calls: int ref,
				   callhistory: aliasmap}
		    | BuiltinFunEntry of {name: name,
					  typeinfo: type_info,
					  callhistory: aliasmap}
		    | FunAlias of {name: name,
				   entryname: name,
				   typeinfo: type_info}

		    | BuiltinScalar of {name: name}
		    | ForeignScalar of {name: name}
		    | SetEntry of {name: name, value: int}

		    | EventEntry of {name: name, arguments: name list}
		    | EventArgument of {name: name, event: name}

		    | Dummy

  val sizeofentry: enventry -> int
  val nameofentry: enventry -> name
  val eqsofentry: enventry -> equation list
  val symofname: name -> symbol

  val base_env : env

end



structure DLEnv: DL_ENV =
struct

  structure Ast=DLAst
  structure TypeSet = DLTypes.TypeSet

  type typeset      = DLTypes.typeset

  type exp          = Ast.exp
  type relop        = Ast.relop
  type symbol       = Ast.symbol
  type name         = Ast.name
  type discrete_exp = Ast.discrete_exp
  type external_dir = Ast.external_dir
  type method       = Ast.method
  type set_element  = Ast.set_element

  type uniq_id      = UniqueId.uniq_id

  type condition    = {relop: relop, 
		       quantity1: symbol,   
		       quantity2: symbol,
		       parentid: uniq_id option}
  type condmap      = condition list UniqueId.table
	
  type dl_type      = DLTypes.dl_type
  type type_info    = DLTypes.type_info
  type differential_arguments = Ast.differential_arguments
		   
  val nopos = ~1


  structure AliasMap = IntMapTable (type key = int
				    fun getInt x = x)
  type aliasmap = name AliasMap.table

	   
  datatype equation = DifferenceEquation    of {line: int, exp: exp}
		    | DifferentialEquation  of {line: int, arg: differential_arguments}
		    | AlgebraicEquation     of {line: int, exp: exp}
		    | DiscreteEquation      of {line: int, exp: discrete_exp}
		    | ConditionalBlock      of {line: int, condition: uniq_id, eq: equation}
		    | EventBlock            of {line: int, event: name, eq: equation}

  datatype argentry = ArgumentEntry of {name: name,
					typ: typeset}
					  
  datatype enventry = TimeEntry of {quantity:name,
				    initval:real,
				    eq: equation}
		    | ScalarParEntry of {quantity: name, 
					 value: real,
					 description: string option}
		    | VectorParEntry of {quantity: name, 
					 value: real Array.array,
					 description: string option}
		    | ScalarStateEntry of {quantity: name, 
					   initval: real,
					   method: method option,
					   eqs: equation list,
					   description: string option}
		    | VectorStateEntry of {quantity: name, 
					   initval: real Array.array,
					   method: method option,
					   eqs: equation list,
					   description: string option}
		    | IntegerStateEntry of {quantity: name, 
					    initval: int,
					    eqs: equation list,
					    description: string option}
		    | ExternalStateEntry of {quantity: name,
					     direction: external_dir,
					     channel: int option,
					     eqs: equation list option,
					     description: string option}
		    | DiscreteStateEntry of {quantity: name,
					     set: set_element list,
					     eqs: equation list,
					     description: string option}

		    | VectorStateFunEntry of {quantity: name,
					      eqs: equation list,
					      size: int option,
					      description: string option}
		    | ScalarStateFunEntry of {quantity: name,
					      eqs: equation list,
					      description: string option}
		    | TableFunEntry of {quantity: name,
					body: exp,
					low: real,
					high: real,
					step: real,
					argument: name,
					description: string option}
		    | FunEntry of {name: name,
				   body: exp,
				   typeinfo: type_info,
				   calls: int ref,
				   callhistory: aliasmap}
		    | BuiltinFunEntry of {name: name,
					  typeinfo: type_info,
					  callhistory: aliasmap}
		    | FunAlias of {name: name,
				   entryname: name,
				   typeinfo: type_info}
		    | BuiltinScalar of {name: name}
		    | ForeignScalar of {name: name}
		    | SetEntry of {name: name, value: int}

		    | EventEntry of {name: name, arguments: name list}
		    | EventArgument of {name: name, event: name}

		    | Dummy

  type env          = enventry Symbol.table
  type arg          = argentry Symbol.table


(* Return the name of an environment entry *)

fun nameofentry(TimeEntry {quantity, ...})           = quantity
  | nameofentry(ScalarParEntry {quantity, ...} )     = quantity
  | nameofentry(VectorParEntry {quantity, ...})      = quantity
  | nameofentry(ScalarStateEntry {quantity, ...})    = quantity
  | nameofentry(VectorStateEntry {quantity, ...})    = quantity
  | nameofentry(IntegerStateEntry {quantity, ...})   = quantity
  | nameofentry(ExternalStateEntry {quantity, ...})  = quantity
  | nameofentry(DiscreteStateEntry {quantity, ...})  = quantity
  | nameofentry(ScalarStateFunEntry {quantity, ...}) = quantity
  | nameofentry(VectorStateFunEntry {quantity, ...}) = quantity
  | nameofentry(TableFunEntry {quantity, ...})       = quantity
  | nameofentry(FunEntry {name, ...})                = name
  | nameofentry(FunAlias {name, ...})                = name
  | nameofentry(BuiltinFunEntry {name, ...})         = name
  | nameofentry(BuiltinScalar {name})                = name
  | nameofentry(ForeignScalar {name})                = name
  | nameofentry(SetEntry {name, ...})                = name
  | nameofentry(EventEntry {name, ...})              = name
  | nameofentry(EventArgument {name, ...})           = name
  | nameofentry(Dummy)                               = {name=Symbol.symbol "#dummy", pos=nopos}

(* Return the equations of an environment entry *)

fun eqsofentry(ScalarStateEntry {quantity, initval, method, eqs, ...})    = eqs
  | eqsofentry(VectorStateEntry {quantity, initval, method, eqs, ...})    = eqs
  | eqsofentry(IntegerStateEntry {quantity, initval, eqs, ...})           = eqs
  | eqsofentry(ExternalStateEntry {quantity, direction, channel, eqs, ...})        = 
    (case eqs of NONE => [] | SOME eqs => eqs)
  | eqsofentry(DiscreteStateEntry {quantity, set, eqs, ...})              = eqs
  | eqsofentry(ScalarStateFunEntry {quantity, eqs, ...}) = eqs
  | eqsofentry(VectorStateFunEntry {quantity, eqs, ...}) = eqs
  | eqsofentry(_)       = []




(* take the symbol from a name *)   
fun symofname (n: name) =
    let
	val {name=name, pos=_} = n
    in
	name
    end


fun sizeofentry (e) =

    (case e of 
	 ScalarParEntry      {quantity, ...} => 1
       | VectorParEntry      {quantity, value, ...} => Array.length value
       | ScalarStateEntry    {quantity, ...} => 1
       | VectorStateEntry    {quantity, initval, ...} => Array.length initval
       | IntegerStateEntry   {quantity, ...} => 1
       | ExternalStateEntry  {quantity, ...} => 1
       | DiscreteStateEntry  {quantity, ...} => 1
       | ScalarStateFunEntry {quantity, ...} => 1
       | VectorStateFunEntry {quantity, eqs, size=(SOME size), ...} => size
       | TableFunEntry       {quantity, ...} => 1
       | ForeignScalar       {name}          => 1
       | _                                   => 0)

	
fun give_name (s) =
    {name=Symbol.symbol s, pos=nopos}

val base_env = Symbol.empty

val base_env = Symbol.enter(base_env,  Symbol.symbol "abs", 
			    BuiltinFunEntry {name= {name=Symbol.symbol "abs",pos=nopos}, 
					     typeinfo={
						       args=[{name=give_name "x", 
							      typ=TypeSet.singleton (DLTypes.Scalar)}],  
						       ret_type=TypeSet.singleton (DLTypes.Scalar),
						       pos=nopos},
					     callhistory=AliasMap.empty})
val base_env = Symbol.enter(base_env,  Symbol.symbol "dt",
			    BuiltinScalar {name={name=Symbol.symbol "dt",pos=nopos}})		

val base_env = Symbol.enter(base_env,  Symbol.symbol "atan", 
			    BuiltinFunEntry {name= {name=Symbol.symbol "atan",pos=nopos},
					     typeinfo={
						       args=[{name=give_name "x", 
							      typ=TypeSet.singleton (DLTypes.Scalar)}], 
						       ret_type=TypeSet.singleton (DLTypes.Scalar),
						       pos=nopos},
					     callhistory=AliasMap.empty})

val base_env = Symbol.enter(base_env, Symbol.symbol "asin",
			    BuiltinFunEntry {name= {name=Symbol.symbol "asin",pos=nopos},
					     typeinfo={
						       args=[{name=give_name "x", 
							      typ=TypeSet.singleton (DLTypes.Scalar)}], 
						       ret_type=TypeSet.singleton (DLTypes.Scalar),
						       pos=nopos},
					     callhistory=AliasMap.empty})

val base_env = Symbol.enter(base_env, Symbol.symbol "acos", 
			    BuiltinFunEntry {name= {name=Symbol.symbol "acos",pos=nopos},
					     typeinfo={
						       args=[{name=give_name "x", 
							      typ=TypeSet.singleton (DLTypes.Scalar)}], 
						       ret_type=TypeSet.singleton (DLTypes.Scalar),
						       pos=nopos},
					     callhistory=AliasMap.empty})

val base_env = Symbol.enter(base_env,  Symbol.symbol "sin",
			    BuiltinFunEntry {name= {name=Symbol.symbol "sin",pos=nopos},
					     typeinfo={
						       args=[{name=give_name "x", 
							      typ=TypeSet.singleton (DLTypes.Scalar)}], 
						       ret_type=TypeSet.singleton (DLTypes.Scalar),
						       pos=nopos},
					     callhistory=AliasMap.empty})

val base_env = Symbol.enter(base_env,  Symbol.symbol "cos",
			    BuiltinFunEntry {name= {name=Symbol.symbol "cos",pos=nopos},
					     typeinfo={
						       args=[{name=give_name "x", 
							      typ=TypeSet.singleton (DLTypes.Scalar)}], 
						       ret_type=TypeSet.singleton (DLTypes.Scalar),
						       pos=nopos},
					     callhistory=AliasMap.empty})

val base_env = Symbol.enter(base_env,  Symbol.symbol "cosh",
			    BuiltinFunEntry {name= {name=Symbol.symbol "cosh",pos=nopos},
					     typeinfo={
						       args=[{name=give_name "x", 
							      typ=TypeSet.singleton (DLTypes.Scalar)}], 
						       ret_type=TypeSet.singleton (DLTypes.Scalar),
						       pos=nopos},
					     callhistory=AliasMap.empty})

val base_env = Symbol.enter(base_env,  Symbol.symbol "ceil",
			    BuiltinFunEntry {name= {name=Symbol.symbol "ceil",pos=nopos},
					     typeinfo={
						       args=[{name=give_name "x", 
							      typ=TypeSet.singleton (DLTypes.Scalar)}], 
						       ret_type=TypeSet.singleton (DLTypes.Scalar),
						       pos=nopos},
					     callhistory=AliasMap.empty})

val base_env = Symbol.enter(base_env,  Symbol.symbol "cube",
			    BuiltinFunEntry {name= {name=Symbol.symbol "cube",pos=nopos},
					     typeinfo={
						       args=[{name=give_name "x", 
							      typ=TypeSet.singleton (DLTypes.Scalar)}], 
						       ret_type=TypeSet.singleton (DLTypes.Scalar),
						       pos=nopos},
					     callhistory=AliasMap.empty})

val base_env = Symbol.enter(base_env,  Symbol.symbol "exp",
			    BuiltinFunEntry {name= {name=Symbol.symbol "exp",pos=nopos},
					     typeinfo={
						       args=[{name=give_name "x", 
							      typ=TypeSet.singleton (DLTypes.Scalar)}], 
						       ret_type=TypeSet.singleton (DLTypes.Scalar),
						       pos=nopos},
					     callhistory=AliasMap.empty})

val base_env = Symbol.enter(base_env,  Symbol.symbol "floor",
			    BuiltinFunEntry {name= {name=Symbol.symbol "floor",pos=nopos},
					     typeinfo={
						       args=[{name=give_name "x", 
							      typ=TypeSet.singleton (DLTypes.Scalar)}], 
						       ret_type=TypeSet.singleton (DLTypes.Scalar),
						       pos=nopos},
					     callhistory=AliasMap.empty})

val base_env = Symbol.enter(base_env,  Symbol.symbol "fabs",
			    BuiltinFunEntry {name= {name=Symbol.symbol "fabs",pos=nopos},
					     typeinfo={
						       args=[{name=give_name "x", 
							      typ=TypeSet.singleton (DLTypes.Scalar)}], 
						       ret_type=TypeSet.singleton (DLTypes.Scalar),
						       pos=nopos},
					     callhistory=AliasMap.empty})

val base_env = Symbol.enter(base_env,  Symbol.symbol "heaviside",
			    BuiltinFunEntry {name= {name=Symbol.symbol "heaviside",pos=nopos},
					     typeinfo={
						       args=[{name=give_name "x", 
							      typ=TypeSet.singleton (DLTypes.Scalar)}], 
						       ret_type=TypeSet.singleton (DLTypes.Scalar),
						       pos=nopos},
					     callhistory=AliasMap.empty})

val base_env = Symbol.enter(base_env,  Symbol.symbol "log",
			    BuiltinFunEntry {name= {name=Symbol.symbol "log",pos=nopos},
					     typeinfo={
						       args=[{name=give_name "x", 
							      typ=TypeSet.singleton (DLTypes.Scalar)}], 
						       ret_type=TypeSet.singleton (DLTypes.Scalar),
						       pos=nopos},
					     callhistory=AliasMap.empty})

val base_env = Symbol.enter(base_env,  Symbol.symbol "log10",
			    BuiltinFunEntry {name= {name=Symbol.symbol "log10",pos=nopos},
					     typeinfo={
						       args=[{name=give_name "x", 
							      typ=TypeSet.singleton (DLTypes.Scalar)}], 
						       ret_type=TypeSet.singleton (DLTypes.Scalar),
						       pos=nopos},
					     callhistory=AliasMap.empty})

val base_env = Symbol.enter(base_env,  Symbol.symbol "sinh",
			    BuiltinFunEntry {name= {name=Symbol.symbol "sinh",pos=nopos},
					     typeinfo={
						       args=[{name=give_name "x", 
							      typ=TypeSet.singleton (DLTypes.Scalar)}], 
						       ret_type=TypeSet.singleton (DLTypes.Scalar),
						       pos=nopos},
					     callhistory=AliasMap.empty})

val base_env = Symbol.enter(base_env,  Symbol.symbol "sqrt",
			    BuiltinFunEntry {name= {name=Symbol.symbol "sqrt",pos=nopos},
					     typeinfo={
						       args=[{name=give_name "x", 
							      typ=TypeSet.singleton (DLTypes.Scalar)}], 
						       ret_type=TypeSet.singleton (DLTypes.Scalar),
						       pos=nopos},
					     callhistory=AliasMap.empty})

val base_env = Symbol.enter(base_env,  Symbol.symbol "sqr",
			    BuiltinFunEntry {name= {name=Symbol.symbol "sqr",pos=nopos},
					     typeinfo={
						       args=[{name=give_name "x", 
							      typ=TypeSet.singleton (DLTypes.Scalar)}], 
						       ret_type=TypeSet.singleton (DLTypes.Scalar),
						       pos=nopos},
					     callhistory=AliasMap.empty})

val base_env = Symbol.enter(base_env,  Symbol.symbol "tan",
			    BuiltinFunEntry {name= {name=Symbol.symbol "tan",pos=nopos},
					     typeinfo={
						       args=[{name=give_name "x", 
							      typ=TypeSet.singleton (DLTypes.Scalar)}], 
						       ret_type=TypeSet.singleton (DLTypes.Scalar),
						       pos=nopos},
					     callhistory=AliasMap.empty})

val base_env = Symbol.enter(base_env,  Symbol.symbol "tanh",
			    BuiltinFunEntry {name= {name=Symbol.symbol "tanh",pos=nopos},
					     typeinfo={
						       args=[{name=give_name "x", 
							      typ=TypeSet.singleton (DLTypes.Scalar)}], 
						       ret_type=TypeSet.singleton (DLTypes.Scalar),
						       pos=nopos},
					     callhistory=AliasMap.empty})

val base_env = Symbol.enter(base_env,  Symbol.symbol "atan2",
			    BuiltinFunEntry {name= {name=Symbol.symbol "atan2",pos=nopos},
					     typeinfo={
						       args=[{name=give_name "x", 
							      typ=TypeSet.singleton (DLTypes.Scalar)},
							     {name=give_name "y", 
							      typ=TypeSet.singleton (DLTypes.Scalar)}],
 						       ret_type=TypeSet.singleton (DLTypes.Scalar),
						       pos=nopos},
					     callhistory=AliasMap.empty})

val base_env = Symbol.enter(base_env,  Symbol.symbol "length",
			    BuiltinFunEntry {name= {name=Symbol.symbol "length",pos=nopos},
					     typeinfo={
						       args=[{name=give_name "v", 
							      typ=TypeSet.singleton (DLTypes.Vector NONE)}], 
						       ret_type=TypeSet.singleton (DLTypes.Scalar),
						       pos=nopos},
					     callhistory=AliasMap.empty})

val base_env = Symbol.enter(base_env,  Symbol.symbol "rshift",
			    BuiltinFunEntry {name= {name=Symbol.symbol "rshift",pos=nopos},
					     typeinfo={
						       args=[{name=give_name "v", 
							      typ=TypeSet.singleton (DLTypes.Vector NONE)},
							     {name=give_name "x", 
							      typ=TypeSet.singleton (DLTypes.Scalar)}], 
						       ret_type=TypeSet.singleton (DLTypes.Vector NONE),
						       pos=nopos},
					     callhistory=AliasMap.empty})

val base_env = Symbol.enter(base_env,  Symbol.symbol "lshift",
			    BuiltinFunEntry {name= {name=Symbol.symbol "lshift",pos=nopos},
					     typeinfo={
						       args=[{name=give_name "v", 
							      typ=TypeSet.singleton (DLTypes.Vector NONE)},
							     {name=give_name "x", 
							      typ=TypeSet.singleton (DLTypes.Scalar)}], 
						       ret_type=TypeSet.singleton (DLTypes.Vector NONE),
						       pos=nopos},
					     callhistory=AliasMap.empty})
    

val base_env = Symbol.enter(base_env,  Symbol.symbol "sum",
			    BuiltinFunEntry {name= {name=Symbol.symbol "sum",pos=nopos},
					     typeinfo={
						       args=[{name=give_name "x", 
							      typ=TypeSet.singleton (DLTypes.Vector NONE)}], 
						       ret_type=TypeSet.singleton (DLTypes.Scalar),
						       pos=nopos},
					     callhistory=AliasMap.empty})

end
