
signature DL_ENV' = 
sig
    type exp          = DLAst.exp
    type symbol
    type method
    type uniq_id
    type set_element
    type external_dir
    type discrete_exp
    type differential_arguments
    type type_info   = DLTypes.type_info
    type typeset     = DLTypes.typeset


    type name' = {name: symbol, id: uniq_id}
    type set_element' = symbol * int 

    type type_info' = {args: {name: name', typ: typeset} list,
		       ret_type: typeset}
		 
    datatype equation' = DifferenceEquation   of {line: int, exp: exp}
		       | DifferentialEquation of {line: int, 
						  arg: differential_arguments}
		       | AlgebraicEquation    of {line: int, exp: exp}
		       | DiscreteEquation     of {line: int, exp: discrete_exp}
						 
    datatype enventry' = TimeEntry         of {quantity:name',
					       initval: real,
					       eq: equation'}
 		       | ScalarParEntry    of {quantity: name', 
					       value: real,
					       description: string option}
		       | VectorParEntry     of {quantity: name', 
						value: real Array.array,
						description: string option}
		       | ScalarStateEntry   of {quantity: name', 
						initval: real,
						method: method option,
						condition: uniq_id option,
						event: name' option,
						eq: equation',
						description: string option}
		       | VectorStateEntry   of {quantity: name', 
						initval: real Array.array,
						method: method option,
						condition: uniq_id option,
						event: name' option,
						eq: equation',
						description: string option}
		       | IntegerStateEntry  of {quantity: name', 
						initval: int,
						condition: uniq_id option,
						event: name' option,
						eq: equation',
						description: string option}
		       | ExternalStateEntry of {quantity: name',
						direction: external_dir,
						channel: int,
						condition: uniq_id option,
						event: name' option,
						eq: equation' option,
						description: string option}
		       | DiscreteStateEntry  of {quantity: name',
						 set: set_element' list,
						 condition: uniq_id option,
						 event: name' option,
						 eq: equation',
						 description: string option}
		       | VectorStateFunEntry of {quantity: name',
						 condition: uniq_id option,
						 event: name' option,
						 size: int,
						 eq: equation',
						 description: string option}
		       | ScalarStateFunEntry of {quantity: name',
						 condition: uniq_id option,
						 event: name' option,
						 eq: equation',
						 description: string option}

		       | TableFunEntry   of {quantity: name',
					     body: exp,
					     low: real,
					     high: real,
					     step: real,
					     argument: name',
					     description: string option}
		       | FunEntry        of {name: name', body: exp,
					     typeinfo: type_info'}
		       | FunAlias of {name: name',
				      entryname: name',
				      typeinfo: type_info'}
		       | BuiltinFunEntry of {name: name', 
					     typeinfo: type_info'}
		       | BuiltinScalar   of {name: name'}
		       | ForeignScalar   of {name: name'}
		       | SetEntry        of {name: name', value: int}

		       | EventEntry of {name: name', arguments: name' list}
		       | EventArgument of {name: name', event: name'}

		       | Dummy



    type env'          = enventry' Symbol.table
    type condenvmap    = (enventry' Symbol.table) UniqueId.table
    type eventenvmap   = (enventry' Symbol.table) Symbol.table
    type idtable       = enventry' UniqueId.table

    val nameofentry:       enventry'  -> name'
    val eqsofentry:        enventry'  -> equation' option
    val condofentry:       enventry'  -> uniq_id option
    val eventofentry:      enventry'  -> name' option
    val idofname:          name'      -> uniq_id
    val symofname:         name'      -> Symbol.symbol
    val name2str:          name'      -> string
end

structure DLEnv': DL_ENV' =
struct


  structure Ast=DLAst

  type exp          = Ast.exp
  type symbol       = Ast.symbol
  type discrete_exp = Ast.discrete_exp
  type external_dir = Ast.external_dir
  type method       = Ast.method
  type set_element  = Ast.set_element
  type differential_arguments = Ast.differential_arguments

  type uniq_id      = UniqueId.uniq_id
	
  type type_info   = DLTypes.type_info
  type typeset     = DLTypes.typeset


  type name' = {name: symbol, id: uniq_id}

  type type_info' = {args: {name: name', typ: typeset} list,
		     ret_type: typeset}

  type set_element' = symbol * int
		 
	       
  datatype equation' = DifferenceEquation   of {line: int, exp: exp}
		     | DifferentialEquation of {line: int, 
						arg: differential_arguments}
		     | AlgebraicEquation    of {line: int, exp: exp}
		     | DiscreteEquation     of {line: int, exp: discrete_exp}
					       
  datatype enventry' = TimeEntry         of {quantity:name',
					     initval: real,
					     eq: equation'}
 		     | ScalarParEntry    of {quantity: name', 
					     value: real,
					     description: string option}
		     | VectorParEntry     of {quantity: name', 
					      value: real Array.array,
					      description: string option}
		     | ScalarStateEntry   of {quantity: name', 
					      initval: real,
					      method: method option,
					      condition: uniq_id option,
					      event: name' option,
					      eq: equation',
					      description: string option}
		     | VectorStateEntry   of {quantity: name', 
					      initval: real Array.array,
					      method: method option,
					      condition: uniq_id option,
					      event: name' option,
					      eq: equation',
					      description: string option}
		     | IntegerStateEntry  of {quantity: name', 
					      initval: int,
					      condition: uniq_id option,
					      event: name' option,
					      eq: equation',
					      description: string option}
		     | ExternalStateEntry of {quantity: name',
					      direction: external_dir,
					      channel: int,
					      condition: uniq_id option,
					      event: name' option,
					      eq: equation' option,
					      description: string option}
		     | DiscreteStateEntry  of {quantity: name',
					       set: set_element' list,
					       condition: uniq_id option,
					       event: name' option,
					       eq: equation',
					       description: string option}
		     | VectorStateFunEntry of {quantity: name',
					       condition: uniq_id option,
					       event: name' option,
					       size: int,
					       eq: equation',
					       description: string option}
		     | ScalarStateFunEntry of {quantity: name',
					       condition: uniq_id option,
					       event: name' option,
					       eq: equation',
					       description: string option}
		     | TableFunEntry   of {quantity: name',
					   body: exp,
					   low: real,
					   high: real,
					   step: real,
					   argument: name',
					   description: string option}
		     | FunEntry        of {name: name', body: exp,
					   typeinfo: type_info'}
		     | FunAlias of {name: name',
				    entryname: name',
				    typeinfo: type_info'}
		     | BuiltinFunEntry of {name: name', typeinfo: type_info'}
		     | BuiltinScalar   of {name: name'}
		     | ForeignScalar   of {name: name'}
		     | SetEntry        of {name: name', value: int}

		     | EventEntry of {name: name', 
				      arguments: name' list}
		     | EventArgument of {name: name', event: name'}

		     | Dummy



  type env'          = enventry' Symbol.table
  type condenvmap    = (enventry' Symbol.table) UniqueId.table
  type eventenvmap   = (enventry' Symbol.table) Symbol.table
  type idtable       = enventry' UniqueId.table


		       
  fun symofname (n) =
      let
	  val {name=name, id=_} = n
      in
	  name
      end

  (* Return the id of a dep name *)

  fun idofname({name=_, id=id}) = id

  (* Convert a depname to a string *)
  fun name2str (n:name'):string =
    let
	val name = Symbol.name (symofname n)
	val id = Int.toString(UniqueId.id2int(idofname n))
    in
	"(" ^ name ^ ":" ^ id ^ ")"
    end


  (* Return the name of an environment entry *)

  fun nameofentry(TimeEntry ({quantity, ...}))         = quantity
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
    | nameofentry(ForeignScalar {name})                = name
    | nameofentry(BuiltinScalar {name})                = name
    | nameofentry(SetEntry {name, value})              = name
    | nameofentry(EventEntry {name, ...})              = name
    | nameofentry(EventArgument {name, ...})           = name
    | nameofentry(Dummy)                               = {name=(Symbol.symbol "#dummy"), id=UniqueId.genid()}


  (* Return the equations of an environment entry *)

  fun eqsofentry(ScalarStateEntry {quantity, initval, method, eq, ...})    = SOME eq
    | eqsofentry(VectorStateEntry {quantity, initval, method, eq, ...})    = SOME eq
    | eqsofentry(IntegerStateEntry {quantity, initval, eq, ...})           = SOME eq
    | eqsofentry(ExternalStateEntry {quantity, direction, eq, ...})        = eq
    | eqsofentry(DiscreteStateEntry {quantity, set, eq, ...})              = SOME eq
    | eqsofentry(ScalarStateFunEntry {quantity, eq, ...}) = SOME eq
    | eqsofentry(VectorStateFunEntry {quantity, eq, ...}) = SOME eq
    | eqsofentry(_)       = NONE

(* returns an optional condition list for an enventry' *)

  fun condofentry(ScalarStateEntry    {quantity, initval, method, condition, ...})  = condition
    | condofentry(VectorStateEntry    {quantity, initval, method, condition, ...})  = condition
    | condofentry(IntegerStateEntry   {quantity, initval, condition, ...})          = condition
    | condofentry(ExternalStateEntry  {quantity, direction, condition, ...})        = condition
    | condofentry(DiscreteStateEntry  {quantity, set, condition, ...})              = condition
    | condofentry(ScalarStateFunEntry {quantity, condition, ...})                   = condition
    | condofentry(VectorStateFunEntry {quantity, condition, ...})                   = condition
    | condofentry(_)       = NONE

  fun eventofentry(ScalarStateEntry    {quantity, event, ...}) = event
    | eventofentry(VectorStateEntry    {quantity, event, ...})  = event
    | eventofentry(IntegerStateEntry   {quantity, event, ...})  = event
    | eventofentry(ExternalStateEntry  {quantity, event, ...}) = event
    | eventofentry(DiscreteStateEntry  {quantity, event, ...}) = event
    | eventofentry(ScalarStateFunEntry {quantity, event, ...}) = event
    | eventofentry(VectorStateFunEntry {quantity, event, ...}) = event
    | eventofentry(_)       = NONE

end
