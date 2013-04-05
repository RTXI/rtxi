
(* 
 *
 * Phases of type checking:
 *
 *
 * First, we process all the declarations of user defined functions to
 * resolve their return and argument types as much as possible.
 * Limiting down the range of possible types is done here (where
 * limiting it to one possible value is the goal).
 *
 * We then begin processing the equations, resolving any remaining
 * ambiguities in user-defined functions' types through knowledge of
 * the types of the quantities used as arguments, and the way the
 * function is used in the equations.
 *
 * After this point, we signal an error if there are any user-defined
 * functions or system quantities with unresolved types.  A type is
 * unresolved when the type set of a quantity is either empty or
 * contains more than one element. 
 *
 *)


signature DL_SEMANT =
sig

    structure Ast: DL_AST

    type filearg

    type env
    type enventry
    type condmap
    type type_info

	 
    val type_check: filearg -> Ast.system -> env * env option * condmap * string list

end

structure DLSemant =
struct

open DLEnv
open DLTypes
open DLErrors

exception UnusedState
exception InvalidIdentifier of string
exception InvalidArgumentList
exception InvalidFunction
exception InvalidEmptyList
exception InvalidOperator
exception InvalidTypeCombination
exception InvalidTimeBlock
exception InvalidTypeCombo
exception InvalidFuncall
exception InvalidEvent
exception InvalidRealConstExpr
exception InvalidIntConstExpr

structure AliasMap = DLEnv.AliasMap

type condmap = DLEnv.condmap


(* each time block evaluates to an environment
 * each of these corresponds to a start or runtime time block
 *)
datatype env_time_type = START of env | RUNTIME of (condmap * env)

(* checks if x is a state entry *)
fun isState(ScalarStateEntry _)    = true
  | isState(VectorStateEntry _)    = true
  | isState(IntegerStateEntry _)   = true
  | isState(ExternalStateEntry _)  = true
  | isState(DiscreteStateEntry _)  = true
  | isState(_)                     = false


(* Return the type of an environment entry *)
fun typeof(TimeEntry _)           = TypeSet.singleton(Scalar)
  | typeof(ScalarParEntry _)      = TypeSet.singleton(Scalar)
  | typeof(VectorParEntry {value, ...})      = TypeSet.singleton(Vector (SOME (Array.length value)))
  | typeof(ScalarStateEntry _)    = TypeSet.singleton(Scalar)
  | typeof(VectorStateEntry {initval, ...}) = TypeSet.singleton(Vector (SOME (Array.length initval)))
  | typeof(IntegerStateEntry _)   = TypeSet.singleton(Scalar)
  | typeof(ExternalStateEntry _)  = TypeSet.singleton(Scalar)
  | typeof(DiscreteStateEntry _)  = TypeSet.singleton(Discrete)
  | typeof(ScalarStateFunEntry _) = TypeSet.singleton(Scalar)
  | typeof(VectorStateFunEntry {size, ...}) = TypeSet.singleton(Vector size)
  | typeof(TableFunEntry _)       = TypeSet.singleton(Scalar)

  | typeof(FunEntry {name=_, body=_, typeinfo=typeinfo, calls=_, callhistory=_}) =
    let
	val {args=_, ret_type=ret_type, pos=_} = typeinfo 
    in
	ret_type
    end

  | typeof(BuiltinFunEntry {name=_, typeinfo=typeinfo, callhistory=_}) =
    let
	val {args=_, ret_type=ret_type, pos=_} = typeinfo 
    in
	ret_type
    end


  | typeof(FunAlias {name=_, entryname=_, typeinfo=typeinfo}) =
    let
	val {args=_, ret_type=ret_type, pos=_} = typeinfo 
    in
	ret_type
    end

  | typeof(BuiltinScalar _) =  TypeSet.singleton(Scalar)
  | typeof(ForeignScalar _) =  TypeSet.singleton(Scalar)
  | typeof(Dummy)           =  TypeSet.singleton(Scalar)
  | typeof(EventEntry _)    =  TypeSet.singleton(Scalar)
  | typeof(EventArgument _) =  TypeSet.singleton(Scalar)
  | typeof(SetEntry _)      =  TypeSet.singleton(Discrete)

val nopos = ~1

(* turn a string and position into a name  *)
val nameit = DLAst.createname

(* take the position of a name *)
val posof = DLAst.posofname

(* get the string representation of a name *)
val name2str = Symbol.name o DLAst.symofname
    
fun sym_to_tempname (s) = DLAst.createname (s, ~1)


fun eval_real_const_expr exp = 
    (case exp of (Ast.CONSTREAL r) => r
	       | (Ast.CONSTINT i) => Real.fromInt i
	       | (Ast.UNARYEXPOP {oper=Ast.UMINUS, exp=exp, pos}) =>
		 Real.~(eval_real_const_expr exp)
	       | (Ast.UNARYEXPOP {oper=Ast.UPLUS, exp=exp, pos}) =>
		 (eval_real_const_expr exp)
	       | (Ast.BINOP {oper=Ast.UNKNOWN (Ast.PLUS), left=exp1, right=exp2, pos}) =>
		 (eval_real_const_expr exp1) + (eval_real_const_expr exp2)
	       | (Ast.BINOP {oper=Ast.UNKNOWN (Ast.MINUS), left=exp1, right=exp2, pos}) =>
		 (eval_real_const_expr exp1) - (eval_real_const_expr exp2)
	       | (Ast.BINOP {oper=Ast.UNKNOWN (Ast.TIMES), left=exp1, right=exp2, pos}) =>
		 (eval_real_const_expr exp1) * (eval_real_const_expr exp2)
	       | (Ast.BINOP {oper=Ast.UNKNOWN (Ast.DIVIDE), left=exp1, right=exp2, pos}) =>
		 (eval_real_const_expr exp1) / (eval_real_const_expr exp2)
	       | (Ast.BINOP {oper=Ast.UNKNOWN (Ast.POWER), left=exp1, right=exp2, pos}) =>
		 Real.Math.pow (eval_real_const_expr exp1, eval_real_const_expr exp2)
	       | _ => raise InvalidRealConstExpr)

fun eval_int_const_expr exp = 
    (case exp of (Ast.CONSTINT i) => i
	       | (Ast.UNARYEXPOP {oper=Ast.UMINUS, exp=exp, pos}) =>
		 Int.~(eval_int_const_expr exp)
	       | (Ast.UNARYEXPOP {oper=Ast.UPLUS, exp=exp, pos}) =>
		 (eval_int_const_expr exp)
	       | (Ast.BINOP {oper=Ast.UNKNOWN (Ast.PLUS), left=exp1, right=exp2, pos}) =>
		 (eval_int_const_expr exp1) + (eval_int_const_expr exp2)
	       | (Ast.BINOP {oper=Ast.UNKNOWN (Ast.MINUS), left=exp1, right=exp2, pos}) =>
		 (eval_int_const_expr exp1) - (eval_int_const_expr exp2)
	       | (Ast.BINOP {oper=Ast.UNKNOWN (Ast.TIMES), left=exp1, right=exp2, pos}) =>
		 (eval_int_const_expr exp1) * (eval_int_const_expr exp2)
	       | (Ast.BINOP {oper=Ast.UNKNOWN (Ast.DIVIDE), left=exp1, right=exp2, pos}) =>
		 Int.div(eval_int_const_expr exp1, eval_int_const_expr exp2)
	       | _ => raise raise InvalidIntConstExpr)


(* Given a list of AST declarations, creates an environment 
   containing the quantities specified in the declaration list. *)
fun make_env (filearg: filearg) (env, decls): enventry Symbol.table =

    let
	fun make_env' (decl, env) =
	    let
		(* A function to check for duplicate 
		     declarations of the same name *)
		fun check_duplicate  (quantity) = 
		    (case Symbol.look (env, symofname quantity) of 
			 SOME _ => printError(filearg, posof quantity, 
					      ERR_MULTIPLE_DECLS{quantity=quantity})
		       | NONE  => ())

		fun add_entry_fn env (quantity, entry) =
		    let
			val _ = check_duplicate quantity
		    in 
			Symbol.enter (env, symofname quantity, entry)
		    end

		val add_entry = add_entry_fn env

	    in
		(case decl of 
		     Ast.COMPONENT_DECL decl => 
		     (case decl of
			  Ast.SCALAR_STATE_DECL{quantity, init, method, description, pos} =>
			  let val initval = eval_real_const_expr init 
				  handle InvalidRealConstExpr => 
					 (printError (filearg,pos,ERR_INVALID_REAL_CONST_EXPR); 0.0)
			  in
			      add_entry(quantity, 
					ScalarStateEntry {quantity=quantity, initval=initval, 
							  method=method, eqs=[], description=description})
			  end
			  
			| Ast.VECTOR_STATE_DECL{quantity, initval, method, description, pos} =>
			  add_entry(quantity, 
				    VectorStateEntry {quantity=quantity, initval=initval, 
						      method=method, eqs=[], description=description})
			  
			| Ast.INT_STATE_DECL{quantity, init, description, pos} =>
			  let val initval = eval_int_const_expr init
				  handle InvalidIntConstExpr => 
					 (printError (filearg,pos,ERR_INVALID_INT_CONST_EXPR); 0)
			  in
			      add_entry(quantity, 
					IntegerStateEntry {quantity=quantity, initval=initval, 
							   eqs=[], description=description})
			  end
			  
			| Ast.EXTERNAL_DECL{quantity, direction, channel, description, pos} =>
			  add_entry(quantity, 
				    ExternalStateEntry {quantity=quantity,
							direction=direction,
							channel=channel,
							eqs=NONE,
							description=description})
			  
			| Ast.DISCRETE_DECL{quantity, set, description, pos} =>
			  add_entry(quantity, 
				    DiscreteStateEntry {quantity=quantity,
							eqs=[],
							set=set,
							description=description})
			  
			  
			| Ast.SCALAR_PAR_DECL{quantity, init, description, pos } =>
			  let val initval = eval_real_const_expr init
				  handle InvalidRealConstExpr => 
					 (printError (filearg,pos,ERR_INVALID_REAL_CONST_EXPR); 0.0)
			  in
			      add_entry(quantity, 
					ScalarParEntry {quantity=quantity, value=initval, 
							description=description})
			  end
			  
			| Ast.VECTOR_PAR_DECL{quantity, initval, description, pos } =>

			  add_entry(quantity, 
 				    VectorParEntry{quantity=quantity,
						   value=initval,
						   description=description})
			  

			| Ast.SCALAR_STATE_FUNC_DECL {quantity, description,  pos} =>
			  add_entry(quantity, 
				    ScalarStateFunEntry {quantity=quantity,
							 eqs=[],
							 description=description})
			  
			  
			| Ast.VECTOR_STATE_FUNC_DECL {quantity, description, pos} =>
			  add_entry(quantity, 
				    VectorStateFunEntry {quantity=quantity,
							 eqs=[],
							 size=NONE,
							 description=description})
			  
			| Ast.TABLE_DECL{quantity, parameters, body, low, high, step,
					 argument, description, pos} =>
			  
			  let

			      (* check for a table declaration:
			       * 1) body of table_fun_entry to see if works with arguments 
			       *    and resolves to scalar
			       *) 
				      
			      val parameter = hd parameters
					      
			      val typeOfArgument = 
				  case (Symbol.look(env, symofname argument)) of
				      NONE => 
				      (printError (filearg, pos,
						   ERR_UNKNOWN_ID ({name=argument}));
				       Scalar)
				    | SOME (ScalarStateEntry{...}) => Scalar
				    | SOME (VectorStateEntry{initval,...}) => Vector (SOME (Array.length initval))
				    | SOME entry => 
				      (printError (filearg, pos,
						   ERR_INVALID_TABLE_ARG {name=nameofentry entry});
				       Scalar)
						 
			      val arg_table = Symbol.enter(Symbol.empty,
							   symofname argument,
							   ArgumentEntry{name=parameter,
									 typ=TypeSet.singleton(typeOfArgument)})
					      
			      val {typ=bodyTyp, env=env'} =
				  exp_check filearg (env, SOME arg_table, nil, body)

			      (* can body resolve to a scalar? *)
			      val _ = if containsOnly(bodyTyp, Scalar) 
				      then 
					  ()
				      else
					  printError (filearg, pos,
						      ERR_INVALID_EXP_TYPE  {quantity=quantity, 
									     expected=Scalar, 
									     actual=bodyTyp})

			  in	 
			      add_entry_fn env'
							   (quantity, 
								TableFunEntry
									{quantity=quantity, body=body, low=low, 
									 high=high, step=step, argument=argument, 
									 description=description})
			  end
			      
			| Ast.C_ID_DECL{quantity, pos} =>
			  add_entry(quantity, ForeignScalar {name=quantity}))
		     
		   | Ast.TIME_DECL (name) => 
		     add_entry(name, TimeEntry {quantity=name,
						initval=0.0,
						eq=AlgebraicEquation{line=nopos,
								     exp=DLAst.BINOP{oper=DLAst.SCALAR(DLAst.S_PLUS),
										     left=DLAst.ID(name),
										     right=DLAst.ID({name=Symbol.symbol "dt", 
												     pos=nopos}),
										     pos=nopos}}})


		   | Ast.EVENT_DECL (name) =>
		     add_entry (name, EventEntry {name=name, arguments=[]})
		     
		   | Ast.FUNCTION_DEF {name, args, body, pos} =>
		     add_entry(name, 
			       FunEntry {name=name, body=body,
					 typeinfo={ret_type=allTypes(),
						   args=(map (fn(id) => {name=id, typ=allTypes()}) args),
						   pos=pos},
					 calls = ref 0,
					 callhistory=AliasMap.empty})

		   | Ast.C_DECL _ => env)
	    end
    in
	foldr make_env' env decls
    end

(* looks up a chain of previous function calls
 * to see if the current function being called (id)
 * is in there *)
and calledFunctionPreviously(nil, id) =
    false 
  | calledFunctionPreviously(f::fun_history, id) =
    if (f = id) then
	true 
    else
	calledFunctionPreviously(fun_history, id) 

(* Type check the body of a function, figure out the 
   types of its arguments and its return type. *)
and fun_checker (filearg: filearg) (env, arg_table, fun_history, id:name, args: exp list option, pos) =
    let


	(* gets an optional body, and type information
	 *    for a function that is being called 
	 * function can be a generic function, built in function,
	 *    or table function
	 *)
	fun get_body_and_typeinfo(env, fun_entry) =
	    case fun_entry of 
		FunEntry {name=_, body=body,
			  typeinfo=typeinfo,
			  calls=_,
			  callhistory=_} => (SOME body, typeinfo)
						
	      | BuiltinFunEntry {name=_,
				 typeinfo=typeinfo,
				 callhistory=_} => (NONE, typeinfo)
						       
	      | TableFunEntry {argument=arg, body=body, ...} =>
		let

		    val typeOfArgument = case (Symbol.look(env, symofname arg)) of
					     NONE => (printError(filearg, pos,
								 ERR_UNKNOWN_ID ({name=arg}));
						      Scalar)
							   
					   | SOME (ScalarStateEntry{...}) => Scalar
					   | SOME (VectorStateEntry{initval,...}) => Vector (SOME (Array.length initval))
					   | SOME entry => 
					     (printError (filearg, pos,
							 ERR_INVALID_TABLE_ARG {name=nameofentry entry});
					      Scalar)
							
		    val argument = {name=arg, typ=TypeSet.singleton(typeOfArgument)}
		    val scalarSet = TypeSet.singleton(Scalar)
		in
		    (SOME body,
		     {args=argument::nil,
		      ret_type=scalarSet,
		      pos=pos}:type_info)
		end
		    
	      | entry => (printError (filearg, pos,
				      ERR_INVALID_FUN_ENTRY {name=nameofentry entry});
			  (NONE, {args=[], ret_type=TypeSet.singleton(Scalar), pos= ~1}))

    in
	    let
		val entry = case Symbol.look(env, symofname id) of
				SOME entry => entry 
			      | NONE => (printError (filearg, pos,
						     ERR_UNKNOWN_ID ({name=id}));
					 Dummy)
					      

		val (body, typeinfo) = get_body_and_typeinfo(env, entry)
				       
		val {args=arg_info, ret_type=ret_type,pos=_} = typeinfo 
		    
		(* blow up if the number of actual arguments != number of formal arguments  *)  
		val _ = case args of 
			    SOME args => 
			    if (length arg_info) = (length args)
			    then ()
			    else (printError (filearg, pos,
					      ERR_INVALID_NUM_ARGS ({expected=(length arg_info),
								     actual=(length args)})))
			  | NONE => ()
			
		(* repackage argument name and type information into 2 lists*)
		fun addArgInfo({name: name, typ: typeset}, 
			       (names, types)) =
		    ((name::names, typ::types))


		val (arg_names, arg_types) = foldr addArgInfo (nil, nil) arg_info


		(* checks the expression of the argument
		 * returns a tuple consisting of:
		 *   1) the argument's type information added to the list of argument
		 *      type information
		 *   2) the updated environment table
		 *)
		fun exp_check_arg(arg, (arg_types, env)) =
		    let
			val {typ=arg_type, env=env} = 
			    exp_check filearg (env, arg_table, fun_history, arg)
		    in
			(arg_type::arg_types, env)
		    end

		(* This recurses through all actual argument values, and uses the 
		 * information returned by exp_check to update the record for the 
		 * function being called, as well as any functions called as part of 
		 * the argument evaluation  
		 *)
		val (actual_arg_types, env') = 
		    case args of 
			 NONE           => 
			 ([], env)
		       | SOME arguments => 
			 foldl exp_check_arg ([], env) arguments 


		fun vec_size (typeset::typesetlist) =
		    let
			fun vec_size' (typ::typelist) =
			    (case typ of
				 Vector (SOME n) => SOME n
			       | _ => vec_size' typelist)
			  | vec_size' ([]) = NONE
		    in
			(case vec_size' (TypeSet.listItems typeset) of
			     SOME n => SOME n
			   | NONE   => vec_size (typesetlist))
		    end
		  | vec_size ([]) = NONE
					    
		    
		(* Intersect the type information about the actual and formal arguments. *)
		fun intersect_arg_types (result, arg_type'::rest', arg_type''::rest'') =
		    let
			val result' = TypeSet.intersection (arg_type'', arg_type')

		    in
			intersect_arg_types (result'::result, rest', rest'')
		    end
		  | intersect_arg_types (result, nil, nil) = result
		  | intersect_arg_types (_, _, _) = raise InvalidArgumentList
				
		val arg_types' = 
		    (case args of 
			 NONE => arg_types
		       | SOME _ => rev (intersect_arg_types ([], arg_types, rev actual_arg_types)))

		(* blows up if a set is empty *)
		fun check_typeset_not_empty(arg_name:name, typ, pos) =
		    if TypeSet.isEmpty(typ) then
			printError(filearg, pos, 
				   ERR_UNRESOLVED_TYPE_ARG{function=id, arg=arg_name})
		    else
			()

			
		fun make_arg_info (arg_names, arg_types) =
		    let
			
			fun make_arg_info' (result, 
					   arg_name::restn, 
					   arg_type::restt) = 
			    let
				(* check if type of argument is ok *)
				val _ = check_typeset_not_empty (arg_name, arg_type, pos) 
					
				val arg_entry = {name=arg_name, typ=arg_type}
				val result' = arg_entry::result
			    in
				make_arg_info' (result', restn, restt)
			    end

			  | make_arg_info' (result, nil, nil) = 
			    result
			    
			  | make_arg_info' (_, _, _) = 
			    raise InvalidArgumentList
		    in
			rev (make_arg_info' ([], arg_names, arg_types))
		    end
						    

		val arg_info' = 
		    (case args of 
			 NONE => arg_info
		       | SOME _ => make_arg_info (arg_names,
						  arg_types'))

		(* Now go through the function body and glean further type information. *) 

		(* function that enters argument information into the argument table *)
		fun enter_arg_info({name: name, typ: typeset},  table) =
		    Symbol.enter (table, symofname name, 
				  ArgumentEntry{name=name, typ=typ})
		val arg_table' = foldl enter_arg_info
				       Symbol.empty
				       arg_info'
				 
		(* typecheck the body expression *)
		val {typ=ret_type',  env=env''} = 
		    case body of 
			SOME body => exp_check filearg 
					       (env', SOME arg_table',
						id::fun_history, body)
		      | NONE => {typ=ret_type, env=env'}
		    
		(* Intersect the type information once again. *)
		val ret_type' = 
		    let


			val intersection = TypeSet.intersection (ret_type, ret_type')
			val types = TypeSet.listItems intersection

		    in
			if (length types) = 1
			then
			    case hd types of
				Vector NONE => TypeSet.singleton (Vector (vec_size arg_types'))
			      | typ         => intersection
			else
			    intersection
		    end

		(* blow up if the above set is empty *)
		val _ = if (TypeSet.isEmpty (ret_type')) 
			then 
			    printError(filearg, pos, 
				       ERR_UNRESOLVED_TYPE_FUN{function=id})
			else ()
			
		(* update the environment with function aliases and argument type information *)

			     
		val fun_alias = 
		    let
			val idstr = Int.toString (UniqueId.id2int (UniqueId.genid()))
			val name' = {name=Symbol.symbol ("_" ^ idstr ^ "_" ^ (name2str id)),
				     pos=(posof id)}
		    in
			FunAlias {name=name',
				  entryname=id,
				  typeinfo={args=make_arg_info (arg_names, arg_types'),
					    ret_type=ret_type',
					    pos=pos}}
		    end

		val env''' = Symbol.enter (env'', symofname (nameofentry fun_alias), fun_alias)


		fun typconv (typ) =
		    (case typ of 
			 Vector (SOME _) => Vector NONE
		       | _               => typ)

		fun typesetconv (typeset) =
		    (let
			 val types = TypeSet.listItems typeset
			 val types' = map typconv types
		       in
			 TypeSet.addList(TypeSet.empty, types')
		     end)

		val ret_type'' = typesetconv ret_type'

		val arg_types'' = map typesetconv arg_types'

		fun update_fun_entry (env, name, body, arg_info, ret_type) =
		    case Symbol.look(env, symofname name) of
			SOME (FunEntry{name=name, 
				       body=body, 
				       typeinfo=typeinfo,
				       calls=calls,
				       callhistory=callhistory}) => 
			Symbol.enter (env, symofname name, 
				      FunEntry {name=name,
						body=body,
						typeinfo={args=arg_info,
							  ret_type=ret_type,
							  pos=pos},
						calls=calls,
						callhistory=AliasMap.enter (callhistory, pos, 
									    nameofentry fun_alias)})
			
			 
		       | SOME (TableFunEntry{...}) => env
						      
		       | _ => raise InvalidIdentifier (name2str name)
		    
		fun update_builtin_fun_entry (env, name, arg_info, ret_type) =
		    
		    case Symbol.look(env, symofname name) of
			SOME (BuiltinFunEntry{name=name, 
					      typeinfo=typeinfo,
					      callhistory=callhistory}) => 
			Symbol.enter (env, symofname name, 
				      BuiltinFunEntry {name=name,
						       typeinfo={args=arg_info,
								 ret_type=ret_type,
								 pos=pos},
						       callhistory=AliasMap.enter (callhistory, pos, 
										   nameofentry fun_alias)})
			
			
		      | _ => raise InvalidIdentifier (name2str name)
				  


		val env'''' = 
		    case body of 
			SOME body => 
			update_fun_entry (env''', id, body,
					  make_arg_info(arg_names, arg_types''),  
					  ret_type'')
		      | NONE => 
			update_builtin_fun_entry (env''', id, 
						  make_arg_info(arg_names, arg_types''),  
						  ret_type'')

	    in
		{typ=ret_type', env=env''''} 
	    end
    end

(* Process equations.  Here we try to resolve any remaining
 * ambiguities in the user defined functions types by using 
 * the arguments passed to them as information.  *)

and
exp_check (filearg: filearg) (env, arg_table, fun_history, exp) =
(case exp of
     Ast.CONSTINT(i) =>
     {typ=TypeSet.singleton(Scalar), env=env}
     
   | Ast.CONSTREAL(r) =>
     {typ=TypeSet.singleton(Scalar), env=env}
     
   | Ast.ID {name, pos} =>
     let
	 fun isOutputState(entry) = 
	     case entry of
		ExternalStateEntry{quantity=_,
				   direction=Ast.OUTPUT,
				   channel=_,
				   description=_,
				   eqs=_} => true
	      | _ => false
		     
	fun findInEnv (name) = 
	    (case Symbol.look(env, name) of
		 SOME entry => if isOutputState(entry) then
				   raise InvalidIdentifier(Symbol.name name)
			       else
				   {typ=typeof(entry), 
				    env=env}
		| NONE       => (printError (filearg, pos,
					     ERR_UNKNOWN_ID ({name=sym_to_tempname(name)}));
				 {typ=TypeSet.singleton(Scalar), 
				  env=env}))
    in
	(case arg_table of
	     SOME arg_table => 

	     (case Symbol.look(arg_table, name) of
		  SOME (ArgumentEntry {name=name, typ=typ}) => {typ=typ, env=env}

 		| NONE => findInEnv (name))
	     
	   | NONE => 
	     findInEnv (name))

    end

	 
  | Ast.BINOP {oper=(Ast.UNKNOWN oper), left=left, right=right, pos=pos} => 

    let	
	val {typ=left_ty, env=env} = exp_check filearg (env, arg_table, fun_history, left)
	val {typ=right_ty, env=env} = exp_check filearg (env, arg_table, fun_history, right)


		  

	fun plusMinus(l,r) =
	    (case (l,r) of
		 (Vector NONE, Vector NONE) => SOME (Vector NONE)
					       
	       | (Vector (SOME size1), Vector (SOME size2)) => 
		 if (size1 = size2) then
		     SOME (Vector (SOME size1))
		 else
		     NONE						  
		     
	       | (Scalar, Scalar) => SOME Scalar
	       | (Vector (SOME size), Vector NONE) => raise InvalidTypeCombo
	       | (Vector NONE, Vector (SOME size)) => raise InvalidTypeCombo
	       | (_, _) => NONE)
	    
	fun times(l,r) = 
	    (case (l,r) of
		 (Vector NONE, Vector NONE) => SOME Scalar
					       
	       | (Vector (SOME size1), Vector (SOME size2)) => 
		 if (size1 = size2) then
		     SOME (Scalar)
		 else
		     NONE
		     
	       | (Vector size, Scalar) => SOME (Vector size)
	       | (Scalar, Vector size) => SOME (Vector size)
					  
	       | (Scalar, Scalar) => SOME Scalar
	       | (_, _) => NONE)

	fun divModPow(l,r) =
	    case (l,r) of
		(Scalar, Scalar) => SOME Scalar
	      | (_, _) => NONE

	fun crossp(l,r) =
	    case (l,r) of
		(Vector NONE, Vector NONE) => SOME (Vector NONE)
	      | (Vector NONE, Vector size) => SOME (Vector size)
	      | (Vector (SOME size), Vector NONE) => SOME (Vector (SOME size))
	      | (Vector (SOME size1), Vector (SOME size2)) => 
		if (size1 = size2) then
		    SOME (Vector (SOME size1))
		else
		    NONE
	      | (_, _) => NONE

	val types = possibleTypes(left_ty, right_ty)
				 (case oper of 
				      Ast.PLUS =>   plusMinus
				    | Ast.MINUS =>  plusMinus
				    | Ast.TIMES =>  times
				    | Ast.DIVIDE => divModPow
				    | Ast.MODULUS =>divModPow
				    | Ast.POWER   =>divModPow
				    | Ast.CROSSP  =>crossp)

    in
	{typ=types, env=env}
    end

  | Ast.BINOP {oper=_, left=left, right=right, pos=pos} => 

    raise InvalidOperator

  | Ast.RELOP {oper=oper, left=left, right=right, pos=pos} =>
    let
	val {typ=left_ty, env=env} = exp_check filearg (env, arg_table, fun_history, left)

	val {typ=right_ty, env=env} = exp_check filearg (env, arg_table, fun_history, right)

	val types = possibleTypes(left_ty, right_ty)
				 (fn(l,r) =>
				    case (l,r) of
					(Scalar, Scalar) => SOME Scalar
				      | (_, _) => NONE)
    in 
	{typ=types, env=env}
    end

  | Ast.FUNCALL {function=function, arguments=arguments, pos=pos} =>
    let
	val {typ, env} = fun_checker filearg 
				     (env, arg_table, fun_history, function, SOME arguments, pos)
	in
		{typ=typ, env=env}
	end

  | Ast.UNARYEXPOP {oper=oper, exp=exp, pos=pos} =>

    let val {typ=exp_type, env=env} = 
	    exp_check filearg (env, arg_table, fun_history, exp)

	(* blow up if the above type set is empty or of size greater than 1 *) 
	val _ = if contains(exp_type, Scalar) 
		   orelse (TypeSet.numItems exp_type) > 1
		then 
		     printError (filearg, pos,
				ERR_UNRESOLVED_EXP_TYPE  {exp=exp, typ=exp_type})
		else 
		    ()

    in
	{typ=exp_type, env=env}
    end
		       

  | Ast.CONDITIONAL {condition=cond, iftrue=iftrue, iffalse=iffalse, pos=pos} =>

    let
	val {typ=cond_type, env=env} = exp_check filearg (env, arg_table, fun_history, cond)
	val {typ=iftrue_type, env=env} = exp_check filearg (env, arg_table, fun_history, iftrue)
	val {typ=iffalse_type, env=env} = exp_check filearg (env, arg_table, fun_history, iffalse)
	val ret_type = TypeSet.intersection (iftrue_type, iffalse_type)
	
	(* blow up if the above type set is empty or of size greater than 1 *) 
	val _ = if TypeSet.isEmpty ret_type 
		   orelse (TypeSet.numItems ret_type) > 1
		then 
		    printError (filearg, pos,
				ERR_UNRESOLVED_IF_TYPE  {falsetype=iftrue_type,
							 truetype=iffalse_type})

		else ()

    in
	{typ=ret_type, env=env}
    end)

(* Process function declarations, and create an initial 
   function record that contains (some) type information. 
   Note that at this stage, it is possible that we are 
   not able to unambiguously resolve the function and 
   argument types. 
	    
   This is why fun_checker is also called by exp_check 
   while processing the equations in the time block, so 
   that functions with unresolved types can be resolved 
   by looking at the actual arguments passed, and the 
   way the function is used in equations. *)

fun process_defun (filearg: filearg) (Ast.FUNCTION_DEF {name, args, body, pos}, env) = 
    let
	val {typ=_, env=env} = fun_checker filearg (env, NONE, nil, name, NONE, pos) 
    in 
	env 
    end

  | process_defun (filearg: filearg) (_, env) = env

(* checks to see if a set (a list in implementation here)
 * contains a given symbol
 *)
fun setContainsSymbol(nil:set_element list, {name=name,pos=_}:name) =
    false

  | setContainsSymbol((sym,num)::set, n as {name=name,pos=_}:name) =
    if(symofname sym = name) then
	true
    else
	setContainsSymbol(set, n)

fun discrete_assign_check (filearg: filearg) (set, name, quantity) = 
    if setContainsSymbol(set, name) 
       orelse (symofname quantity) = (symofname name)
    then
	()
    else
	printError (filearg, posof quantity,
		    ERR_INVALID_SET_ELM {quantity=quantity, element=name})
    

(* check a discrete expression to make sure it contains
 * only valid identifiers for the corresponding discrete set
 *)
fun discrete_check (filearg: filearg) (quantity, set, env, exp) =
    let
	val discrete_assign_check = discrete_assign_check filearg
	val discrete_clause_check = discrete_clause_check filearg
    in
	(case exp of 
	     Ast.DISCRETE_ID(name) =>
	     discrete_assign_check(set, name, quantity)
	     
	   | Ast.DISCRETE_CASE(clauses) =>
	     app (fn(x)=>
		    discrete_clause_check (quantity, set, env,x))
		 clauses)
    end
	
(* check that a discrete clause only 
 * 	 1) performs comparisons that result in a scalar
 *       2) resolves to a valid set element or itself
 *)
and discrete_clause_check (filearg: filearg) (quantity, set, env, Ast.DISCRETE_COND(e,s)) =
    let
  	val {typ=exp_ty, env=env'} = exp_check (filearg: filearg) (env, NONE, nil, e)
	val scalarset = TypeSet.singleton(Scalar)
	val containsScalar = TypeSet.union(exp_ty, scalarset)
    in
	if isemptyset(containsScalar) then
	    printError (filearg, posof quantity,
			ERR_INVALID_EXP_TYPE  {quantity=quantity, 
					       expected=Scalar, 
					       actual=exp_ty})
	else
	    discrete_assign_check filearg (set, s, quantity)
    end
	    
  (* checks that the defualt condition resolves to a valid set element
   * or the discrete quantity itself 
   *)
  | discrete_clause_check (filearg: filearg) (quantity, set, env, Ast.DEFAULT_COND(s)) =
    discrete_assign_check filearg (set, s, quantity)

(* take a list of time entries (equations) and an environment table
 * and return an environment table updated with the information 
 * from the entries
 *)
fun process_time_entries (filearg: filearg) (eqs, condmap, env) =
    let

	(* process a specific entry *)
	fun process_time_entry (parent_event,parent_cond) (entry, (condmap, env)) =
	    (case entry of 
		(* difference equations *)
		Ast.EQN(Ast.DIFFERENCE_EQN(quantity, body, pos)) =>
		let
		    (* evaluate the expression body for types *)
  		    val {typ=exp_ty, env=env'} = exp_check filearg (env, NONE, nil, body)
			
		    (* check that the expression cannot be discrete *)
		    val _ = if contains (exp_ty, Discrete) then
				()
			    else
				printError (filearg, pos,
					    ERR_INVALID_EXP_TYPE  {quantity=quantity, 
								   expected=Discrete, 
								   actual=exp_ty})
				
				
		    (* pull the existing entry for this quantity from the table *)
		    val entry = case Symbol.look(env', symofname quantity) of
				    NONE => (printError(filearg, pos,
							ERR_UNKNOWN_ID ({name=quantity}));
					     Dummy)
				  | SOME entry => entry
						  
						  
		    (* an equation for the environment table representing this time entry *)
		    val base_eq = DifferenceEquation{line=pos, exp=body}

		    (* if this entry is inside a conditional block, nest it as is appropriate *)
		    val eq = case parent_cond of
				 NONE => base_eq
			       | SOME (condid, conds) => 
				 ConditionalBlock {line=pos, condition=condid, eq=base_eq}

		    (* if this entry is inside an event, nest it as is appropriate *)
		    val eq = case parent_event of 
				 NONE => eq
			       | SOME event => EventBlock {line=pos, event=event, eq=eq}

		    (* create a new entry to update the environment table with *)
		    val new_entry =
			case entry of
			    ScalarStateEntry{quantity=q,
					     initval=init,
					     method=method,
					     eqs=eqs,
					     description=description} =>  
			    if TypeSet.equal (exp_ty, TypeSet.singleton (Scalar))
			    then
				ScalarStateEntry{quantity=q,
						 initval=init,
						 method=method,
						 eqs=eq::eqs,
						 description=description}
			    else 
				(printError (filearg, pos,
					     ERR_INVALID_EXP_TYPE  {quantity=quantity, 
								    expected=Scalar, 
								    actual=exp_ty});
				 entry)
				
				
			  | VectorStateEntry{quantity=q,
					     initval=init,
					     method=method,
					     eqs=eqs,
					     description=description} => 
			    if TypeSet.equal (exp_ty, TypeSet.singleton (Vector NONE))
			    then
				VectorStateEntry{quantity=q,
						 initval=init,
						 method=method,
						 eqs=eq::eqs,
						 description=description}
			    else 
				(printError (filearg, pos,
					     ERR_INVALID_EXP_TYPE  {quantity=quantity, 
								    expected=Vector NONE, 
								    actual=exp_ty});
				 entry)
				
			  | _ => 
			    (printError (filearg, pos,
					 ERR_INVALID_DIFF_EQN_QUANTITY  {quantity=quantity});
			     entry)
			    
		    (* update the environment table *)
		    val env' = Symbol.enter(env', symofname quantity, new_entry) 
		in
		    (condmap, env')
		    
		end
		
	      (* discrete equations *)
		
	      | Ast.EQN(Ast.DISCRETE_EQN(quantity, disc_body, pos)) =>
		
		let
		    (* entry for this discrete eqn in env 
		     *)
		    val entry = Symbol.look(env, symofname quantity)
				
		    (* get the equations and the set associated with this quantity
		     *)
		    val (set, eqs) = 
			case entry of
			    SOME (DiscreteStateEntry {quantity=_, eqs=eqs, set=set,
						      description=_}) => (set, eqs)
			  | SOME _ => raise InvalidIdentifier (name2str quantity)
			  | NONE => (printError (filearg, pos,
						 ERR_UNKNOWN_ID ({name=quantity}));
				     ([],[]))
				    
				    
		    (* check now to make sure all the discrete identifiers
		     * are in the set associated with this quantity, or are
		     * the quantity itself 
		     *)
		    val _ = discrete_check filearg (quantity, set, env, disc_body)
			    
		    (* an equation for the environment table representing this time entry
		     *)
		    val base_eq = DiscreteEquation{line=pos, exp=disc_body}
				  
				  
		    (* if this entry is inside a conditional block, nest it as is appropriate
		     *)
		    val eq = case parent_cond of
				 NONE => base_eq
			       | SOME (condid, conds) => 
				 ConditionalBlock {line=pos, condition=condid, eq=base_eq}

		    (* if this entry is inside an event, nest it as is appropriate *)
		    val eq = case parent_event of 
				 NONE => eq
			       | SOME event => EventBlock {line=pos, event=event, eq=eq}
				 
		in
		    (* put all our new information back in the environment table *) 
		    (condmap, 
		     Symbol.enter(env, symofname quantity, DiscreteStateEntry
							       {quantity=quantity,
								eqs=eq::eqs, set=set,
								description=NONE}))
		    
		end
		
	      (* algebraic equation *)
		
	      | Ast.EQN(Ast.ALGEBRAIC_EQN(quantity, body, pos)) =>
		
		let
		    (* evaluate the expression body for types
		     *)
		    
		    val {typ=exp_ty, env=env'} = exp_check filearg (env, NONE, nil, body)
			
		    (* does the return type contain discrete as a possibility?
		     * if so, it is an invalid expression *)
		    val _ = if contains (exp_ty, Discrete) then
				()
			    else
				printError (filearg, pos,
					    ERR_INVALID_EXP_TYPE  {quantity=quantity, 
								   expected=Discrete, 
								   actual=exp_ty})
				
				
		    (* pull the existing entry for this quantity from the table *)
		    val entry = case Symbol.look(env', symofname quantity) of
				    NONE => (printError (filearg, pos,
							 ERR_UNKNOWN_ID ({name=quantity}));
					     Dummy)
				  | SOME entry => entry
						  
						  
						  
		    (* an equation for the environment table representing this time entry
		     *)
		    val base_eq = AlgebraicEquation{line=pos, exp=body}
				  
		    (* if this entry is inside a conditional block, nest it as is appropriate
		     *)
		    val eq = case parent_cond of
				 NONE => base_eq
			       | SOME (condid, conds) => 
				 ConditionalBlock {line=pos, condition=condid, eq=base_eq} 
				 
				 
		    (* if this entry is inside an event, nest it as is appropriate *)
		    val eq = case parent_event of 
				 NONE => eq
			       | SOME event => EventBlock {line=pos, event=event, eq=eq}

		    (* create a new entry to update the environment table with 
		     *)
		    val new_entry =
			case entry of
			    ExternalStateEntry{quantity=q,
					       direction=dir,
					       channel=channel,
					       eqs=eqs,
					       description=description} =>  
			    let
				(* external states must be output to be writable *)
				val _ = case dir of 
					    Ast.OUTPUT => ()
					  | Ast.INPUT  => 
					    printError(filearg, pos, 
						       ERR_EXT_INPUT_EQN{quantity=q})
					    
				(* only scalars can be written to external states *)
				val _ = if containsOnly(exp_ty, Scalar) then
					    ()
					else
					    printError (filearg, pos,
							ERR_INVALID_EXP_TYPE  {quantity=quantity, 
									       expected=Scalar, 
									       actual=exp_ty})
					

				(*add the equation to the list of equations *)
				val eqs = case eqs of 
					      SOME eqs => SOME (eq::eqs) 
					    | NONE => SOME (eq::nil)
			    in
				ExternalStateEntry{quantity=q,
						   direction=dir,
						   channel=channel,
						   eqs=eqs,
						   description=description}
			    end
			    
			    
			  | ScalarStateFunEntry{quantity=q,
						eqs=eqs,
						description=description} =>  
			    
			    if TypeSet.equal (exp_ty, 
					      TypeSet.singleton (Scalar))
			    then
				ScalarStateFunEntry{quantity=q,
						    eqs=eq::eqs,
						    description=description}
			    else 
				(printError (filearg, pos,
					     ERR_INVALID_EXP_TYPE  {quantity=q, 
								    expected=Scalar, 
								    actual=exp_ty});
				 entry)
				
			  | VectorStateFunEntry{quantity=q,
						eqs=eqs,
						size=size,
						description=description} => 
			    if TypeSet.equal (exp_ty, 
					      TypeSet.singleton (Vector NONE))
			    then
				VectorStateFunEntry{quantity=q,
						    eqs=eq::eqs,
						    size=size,
						    description=description}
			    else 			    
				(printError (filearg, pos,
					     ERR_INVALID_EXP_TYPE  {quantity=q, 
								    expected=Vector NONE, 
								    actual=exp_ty});
			     entry)
				
				
			  (*Integer State Entry?*)
			  | _ => (printError (filearg, pos,
					      ERR_INVALID_ALG_EQN_QUANTITY  {quantity=quantity});
				  entry)
				 
				 
		in
		    (* insert the updated entry back into the environment table *)
		    (condmap, 
		     Symbol.enter(env', symofname quantity, new_entry))
		    
		end
		
	  (* differential equation - euler *)
		

	      | Ast.EQN(Ast.DIFFERENTIAL_EQN(quantity,
					     Ast.EULER_ARGS(e), pos)) =>
		
		let
		    (* evaluate the expression body for types *)
		    val {typ=exp_ty, env=env'} = exp_check filearg (env, NONE, nil, e)
			
		    (* pull the existing entry for this quantity from the table *)
 		    val entry = (case Symbol.look(env', symofname quantity) of
				     NONE => (printError (filearg, pos,
							  ERR_UNKNOWN_ID ({name=quantity}));
					      Dummy)
					     
				   | SOME entry => entry)
			    
		    (* an equation for the environment table representing this time entry
		     *)
		    val base_eq = DifferentialEquation{line=pos, arg=Ast.EULER_ARGS(e)}
				  
		    (* if this entry is inside a conditional block, nest it as is appropriate
		     *)
		    val eq = case parent_cond of
				 NONE => base_eq
			       | SOME (condid, conds) => 
				 ConditionalBlock {line=pos, condition=condid, eq=base_eq}
				 

		    (* if this entry is inside an event, nest it as is appropriate *)
		    val eq = case parent_event of 
				 NONE => eq
			       | SOME event => EventBlock {line=pos, event=event, eq=eq}
				 
		    (* check that the return type is guaranteed to be a scalar *)
		    val _ = if containsOnly (exp_ty, Scalar) then
				()
			else
			    printError (filearg, pos,
					ERR_INVALID_EXP_TYPE  {quantity=quantity, 
							       expected=Scalar, 
							       actual=exp_ty})
			    
		    (* create a new entry to update the environment table with *)
		    val new_entry =
			case entry of
			    ScalarStateEntry{quantity=q,
					     initval=init,
					     method=method,
					     eqs=eqs,
					     description=description} =>  
			    ScalarStateEntry{quantity=q,
					     initval=init,
					     method=method,
					     eqs=eq::eqs,
					     description=description}
			  | _ => (printError (filearg, pos,
					      ERR_INVALID_DT_EQN_QUANTITY  {quantity=quantity});
				  entry)
				 
				 
		in
		    (* insert the updated entry back into the environment table *)
		    (condmap, 
		     Symbol.enter(env', symofname quantity, new_entry))
		end
		
	      (* differential equation - mau *)
		
	      | Ast.EQN(Ast.DIFFERENTIAL_EQN(quantity,
					     Ast.MAU_ARGS(arg1, arg2),
					     pos)) =>
		let
		    (* evaluate the expressions for types *)
		    val {typ=exp1_ty, env=env'} = exp_check filearg (env, NONE, nil, arg1)
		    val {typ=exp2_ty, env=env'} = exp_check filearg (env', NONE, nil, arg2)
			       
		    (* pull the existing entry for this quantity from the table *)
		    val entry =  (case Symbol.look(env', symofname quantity) of
				      NONE => (printError (filearg, pos,
							   ERR_UNKNOWN_ID ({name=quantity}));
					       Dummy)
				    | SOME entry => entry)
				 
				 
		    (* an equation for the environment table representing this time entry *)
		    val base_eq = DifferentialEquation{line=pos, arg=Ast.MAU_ARGS(arg1, arg2)}

		    (* check that the return type is guaranteed to be a scalar *)
		    val _ = if containsOnly (exp1_ty, Scalar) then ()
			    else
				printError (filearg, pos,
					    ERR_INVALID_EXP_TYPE  {quantity=quantity, 
								   expected=Scalar, 
								   actual=exp1_ty})
		    val _ = if containsOnly (exp2_ty, Scalar) then ()
			    else
				printError (filearg, pos,
					    ERR_INVALID_EXP_TYPE  {quantity=quantity, 
								   expected=Scalar, 
								   actual=exp2_ty})

				  
		    (* if this entry is inside a conditional block, nest it as is appropriate *)
		    val eq = case parent_cond of
				 NONE => base_eq
			       | SOME (condid, conds) => 
				 ConditionalBlock {line=pos, condition=condid, eq=base_eq}
				 

		    (* if this entry is inside an event, nest it as is appropriate *)
		    val eq = case parent_event of 
				 NONE => eq
			       | SOME event => EventBlock {line=pos, event=event, eq=eq}
				 
		    (* create a new entry to update the environment table with *)
		    val new_entry =
			case entry of
			    ScalarStateEntry{quantity=q,
					     initval=init,
					     method=method,
					     eqs=eqs,
					     description=description} =>  
			    ScalarStateEntry{quantity=q,
					     initval=init,
					     method=method,
					     eqs=eq::eqs,
					     description=description}
			  | _ => (printError (filearg, pos,
					      ERR_INVALID_DT_EQN_QUANTITY  {quantity=quantity});
				  entry)
				 
		in
		    (* insert the updated entry back into the environment table *)		
		    (condmap, 
		     Symbol.enter(env', symofname quantity, new_entry))
		end
		
	      (* conditional blocks *)
	      | Ast.COND({relop, quantity1, quantity2, body, pos}) =>
		(let
		     (* merge this condition with any previous ones 
		      * (ie this block is a nested conditional block)
		      *)
		     val (condmap', new_cond) = 
			 let
			     val condid = UniqueId.genid()
					  
			     val cond = {quantity1=symofname quantity1,
					 quantity2=symofname quantity2,
					 relop=relop,
					 parentid=(case parent_cond of
						       NONE => NONE
						     | SOME (condid,_) => SOME condid)}
					
			     val conds' = cond::(case parent_cond of
						     NONE => nil
						   | SOME (_, conds) => conds)
			 in
			     (UniqueId.enter (condmap, condid, conds'), 
			      SOME (condid, conds'))
			 end
			 
		     (* create a new environment for the equations 
		      * inside this conditional block *)
		     val (condmap'', env') = 
			 process_time_entry_list(parent_event, new_cond, condmap', env, body)
			 
		 in
		     (* return the updated environment table *)
		     (condmap'', env')
		 end)
		
	      (* events *)
	      | Ast.EVENT({name, arguments, equations, pos}) =>
		(case parent_cond of NONE => ()
				   | SOME _ => printError (filearg, pos,
							   ERR_EVENT_IN_COND {name=name});
		 case parent_event of NONE => ()
				   | SOME _ => printError (filearg, pos,
							   ERR_EVENT_IN_EVENT {name=name});
		 let
		     val env' = foldl (fn(x,env) => 
					 Symbol.enter(env,symofname x,EventArgument {name=x, event=name}))
				      env arguments

		     val (condmap', env') = 
			 process_time_entry_list(SOME name, NONE, condmap, env', equations)
			
		     val env' = foldl (fn(x,env) => #1 (Symbol.remove(env,symofname x)))
				      env' arguments


		     (* pull the existing entry for this quantity from the table *)
		     val entry = case Symbol.look(env', symofname name) of
				     NONE => (printError(filearg, pos,
							 ERR_UNKNOWN_ID ({name=name}));
					      Dummy)
				   | SOME entry => entry
 
		     val new_entry = (case entry of 
					  EventEntry {name, arguments=[]} =>
					  EventEntry {name=name, arguments=arguments}
					| EventEntry _ => (printError (filearg, pos,
								       ERR_DUP_EVENT {name=name});
							   Dummy)
					| _ => raise InvalidEvent)

		     val env'' = Symbol.enter(env', symofname name, new_entry)
		 in
		     (* return the updated environment table *)
		     (condmap', env'')
		 end))

		
	(* takes in a list of time entries, and handles each one, combining
	 * all information learned into the environment table 
	 *)
	and process_time_entry_list(parent_event, parent_cond, condmap, env, eqs) =
	    foldl (process_time_entry (parent_event,parent_cond)) (condmap, env) eqs

	(* the initial call to process the time entries at the
	 * topmost level of a time block 
	 *)
	val (condmap, env) = process_time_entry_list(NONE, NONE, condmap, env, eqs)
    in
	(condmap, env)
    end


(* check a type_info block to see if it has resolved
 * all the types
 *)
fun check_typeinfo_unresolved (filearg: filearg) (fname, {args=args, ret_type=ret_type, pos=pos}) =
    let
	(*check the args' types*)
	val _ = app (fn({name=name, typ=typ}) =>
		       if( sizeOfSet(typ) <> 1) then
			   printError(filearg, pos, 
				      ERR_UNRESOLVED_TYPE_ARG{function=fname, arg=name}) 
		       else
			   ())
		    args
	(* check the return type *)
	val _ = if( sizeOfSet(ret_type) <> 1) then
		    printError(filearg, pos, 
			       ERR_UNRESOLVED_TYPE_FUN{function=fname})
		else
		    ()
    in
	()
    end
    

(* check an enventry to see if it has any unresolved types
 *)
fun check_entrylist_unresolved (filearg: filearg) (env, entrylist) =
    let
	fun entryfn(entry) =
	    case entry of

		FunEntry{name, body=_, typeinfo, calls, callhistory} =>

		if (!calls) > 0
		then
		    check_typeinfo_unresolved filearg (name, typeinfo)
		else ()
		     
	      | _ => ()
    in
	app entryfn entrylist
    end

(* check the types structures of each env created 
 *)
fun unresolved_types_check (filearg: filearg) (envlist) =
    let
	val check_entrylist_unresolved = check_entrylist_unresolved filearg

	fun check_env(START env) = 
	    check_entrylist_unresolved(env, Symbol.listItems(env))
	  | check_env (RUNTIME (_,env)) =
	    check_entrylist_unresolved(env, Symbol.listItems(env))
    in
	app check_env envlist
    end
    

fun default_eq_check (ConditionalBlock _) =  NONE
  | default_eq_check (EventBlock _) =  NONE
  | default_eq_check (eq) =  SOME eq


fun default_state_check filearg (state) = 
    let
	(* only check quantities that can have equations
	 *)
	val eqs = case state of
		      ScalarStateEntry    {quantity=quantity, eqs=eqs, ...} => SOME (quantity, eqs)
		    | VectorStateEntry    {quantity=quantity, eqs=eqs, ...} => SOME (quantity, eqs)
		    | IntegerStateEntry   {quantity=quantity, eqs=eqs, ...} => SOME (quantity, eqs)
		    | DiscreteStateEntry  {quantity=quantity, eqs=eqs, ...} => SOME (quantity, eqs)
		    | ScalarStateFunEntry {quantity=quantity, eqs=eqs, ...} => SOME (quantity, eqs)
		    | VectorStateFunEntry {quantity=quantity, eqs=eqs, ...} => SOME (quantity, eqs)
		    | _ => NONE
    in
	case eqs of

	    (* Check that all quantities in a runtime environment
	     * are used if applicable
	     *)
	    SOME (quantity, nil) => (printWarning (filearg, posof quantity,
						 WARN_UNUSED_STATE{quantity=quantity}))
	    
	  | SOME (quantity, eqs) => if length(List.mapPartial default_eq_check eqs) < 1 then
					(printError (filearg, posof quantity,
						     ERR_NO_DEFAULT {quantity=quantity}))
				    else
					()
	  | _   => ()
    end


fun default_exists_check filearg (nil) = ()
  | default_exists_check filearg (env::rest) =
    case env of
	START env => ()
      | RUNTIME (_, env) =>
	let 
	    val _ = app (default_state_check filearg) (Symbol.listItems env)
	in
	    default_exists_check filearg (rest)
	end



fun transform_binop (Ast.UNKNOWN oper, Scalar, Scalar) =


    (case oper of 
	 Ast.PLUS     => Ast.SCALAR Ast.S_PLUS
       | Ast.MINUS    => Ast.SCALAR Ast.S_MINUS
       | Ast.TIMES    => Ast.SCALAR Ast.S_TIMES
       | Ast.DIVIDE   => Ast.SCALAR Ast.S_DIVIDE
       | Ast.MODULUS  => Ast.SCALAR Ast.S_MODULUS
       | Ast.POWER    => Ast.SCALAR Ast.S_POWER
       | Ast.CROSSP   => raise InvalidOperator)

  | transform_binop (Ast.UNKNOWN oper, Scalar, Vector (SOME n)) =

    (case oper of 
	 Ast.TIMES    => Ast.SCALAR_VECTOR (Ast.SV_TIMES n)
       | _   => raise InvalidOperator)

  | transform_binop (Ast.UNKNOWN oper, Vector (SOME n1), Vector (SOME n2)) =
    
    if not (n1 = n2)
    then
	raise InvalidOperator
    else
	(case oper of 
	     Ast.PLUS     => Ast.VECTOR (Ast.V_PLUS  n1)
	   | Ast.MINUS    => Ast.VECTOR (Ast.V_MINUS n1)
	   | Ast.TIMES    => Ast.VECTOR (Ast.V_TIMES n1)
	   | Ast.CROSSP   => Ast.VECTOR (Ast.V_CROSSP n1)
	   | _ => raise InvalidOperator)
    
  | transform_binop (a, b, c) =
    raise InvalidTypeCombination before (print ("a=" ^ (PrettyPrint.binop2str a) ^ " " ^ 
						"b = " ^ (DLTypes.typeToString b) ^ " " ^
						"c = " ^ (DLTypes.typeToString c) ^ "\n"))
	

fun rebuild_exp (filearg: filearg) (env, localenv, exp) =

    (case exp of

	 Ast.CONSTINT _ => (exp, Scalar)
   | Ast.CONSTREAL _ => (exp, Scalar)
						
   | Ast.ID id => 
	 (case Symbol.look (localenv, symofname id) of
	      SOME typ => (exp, hd (TypeSet.listItems typ))
	    | NONE => 
	      (case Symbol.look (env, symofname id) of
			   SOME entry => (exp, hd (TypeSet.listItems (typeof entry)))
			 | NONE =>
			   (printError (filearg, posof id,
							ERR_UNKNOWN_ID {name=id});
				(exp, Scalar))))
	 
   | Ast.BINOP {oper, left, right, pos} =>
	 let
	     val (lhs, lhst) = rebuild_exp filearg (env, localenv, left)
	     val (rhs, rhst) = rebuild_exp filearg (env, localenv, right)
	 in
	     case (lhst, rhst)  of

		 (Vector n, Scalar) => 
		 (Ast.BINOP {oper=transform_binop (oper, rhst, lhst),
			    left=rhs, right=lhs, pos=pos}, Vector n)

	       | (_, _) => 
		 (Ast.BINOP {oper=transform_binop (oper, lhst, rhst),
			     left=lhs, right=rhs, pos=pos}, rhst)
		 
	 end

       | Ast.RELOP {oper, left, right, pos} =>

	 let
	     val (lhs, lhst) = rebuild_exp filearg (env, localenv, left)
	     val (rhs, rhst) = rebuild_exp filearg (env, localenv, right)
	 in
	     if (lhst = rhst) 
	     then
		 (Ast.RELOP {oper=oper,
			     left=lhs, right=rhs, pos=pos}, Scalar)
	     else
		 raise InvalidOperator
	 end



       | Ast.FUNCALL {function, arguments, pos} =>
		 
		 (let
			  val fun_entry = case Symbol.look (env, symofname function) of
				 SOME entry => entry
			       | NONE => raise InvalidFuncall
	     val rebuild_exp = rebuild_exp filearg
	     val arguments = map (fn(a) => #1(rebuild_exp (env, localenv, a))) arguments

	     val (function, formals, rettype, body, calls: int ref option)   = 
		 (case fun_entry of 
		      FunEntry {name=_, body=body,
			       typeinfo={args,ret_type,pos=_},
			       calls=calls, callhistory=callhistory} =>  
		      let
			  val alias = case AliasMap.look (callhistory, pos) of
					  SOME name => name
					| NONE => raise InvalidFuncall

			  val ret_type' = case Symbol.look(env, symofname alias) of
								  SOME (DLEnv.FunAlias {typeinfo={ret_type,...}, ...}) => ret_type
								| _ => raise InvalidFuncall
		      in
			  (alias, args, ret_type', SOME body, SOME calls)
		      end
		    | BuiltinFunEntry {name=_,
				      typeinfo={args,ret_type,pos=_},
				      callhistory=callhistory} => 
		      let
			  val alias = case AliasMap.look (callhistory, pos) of
					  SOME name => name
					| NONE => raise InvalidFuncall

			  val ret_type' = case Symbol.look(env, symofname alias) of
								  SOME (DLEnv.FunAlias {typeinfo={ret_type,...}, ...}) => ret_type
								| _ => raise InvalidFuncall
		      in
			  (alias, args, ret_type', NONE, NONE)
		      end
		    | TableFunEntry {quantity=_, body=_, low=_,
				     high=_, step=_,
				     argument=argument,...} =>
		      (function, {name=argument, typ=TypeSet.singleton(Scalar)}::nil, 
		       TypeSet.singleton Scalar, NONE, NONE)
		    | _ => raise InvalidFunction)
		 
	     val formals = map (fn({name, typ}) => name) formals

	     val _ = (case calls of 
			  SOME calls => (calls := !calls + 1)
			| NONE => ())
	 in
	      (Ast.FUNCALL {function=function, arguments=arguments, pos=pos}, 
	       hd (TypeSet.listItems rettype)) 
	 end)
	 
       | Ast.UNARYEXPOP {oper, exp, pos} =>
	 let
	     val (exp, exptype) = rebuild_exp filearg (env, localenv, exp)
	 in
	     (Ast.UNARYEXPOP {oper=oper, exp=exp, pos=pos}, exptype)
	 end
	 
       | Ast.CONDITIONAL {condition, iftrue, iffalse, pos} =>
	 (let
	     val (condition, condtype) = rebuild_exp filearg (env, localenv, condition)
	     val (iftrue, truetype) = rebuild_exp filearg (env, localenv, iftrue)
	     val (iffalse, falsetype) = rebuild_exp filearg (env, localenv, iffalse)
	 in
	      (Ast.CONDITIONAL {condition=condition, iftrue=iftrue, iffalse=iffalse, pos=pos},
	       truetype)
	 end))

fun rebuild_eq (filearg: filearg) (env, eq) = 

      case eq of 
	  AlgebraicEquation {line, exp} => 
	  AlgebraicEquation{line=line, exp= (#1(rebuild_exp filearg (env, Symbol.empty, exp)))}
	  
	| DifferenceEquation {line, exp} =>
	  DifferenceEquation {line=line, exp=(#1(rebuild_exp filearg (env, Symbol.empty, exp)))}
	  
	| DifferentialEquation {line, arg=(Ast.EULER_ARGS exp)} =>
	  DifferentialEquation {line=line, arg=Ast.EULER_ARGS (#1(rebuild_exp filearg (env, Symbol.empty, exp)))}
	  
	| DifferentialEquation {line, arg=(Ast.MAU_ARGS (exp1,exp2))} =>
	  DifferentialEquation {line=line, arg=Ast.MAU_ARGS (#1(rebuild_exp filearg (env, Symbol.empty, exp1)),
							     #1(rebuild_exp filearg (env, Symbol.empty, exp2)))}
	  
	| ConditionalBlock {line, condition, eq}  =>
	  ConditionalBlock {line=line, condition=condition, eq=rebuild_eq filearg (env, eq)}

	| EventBlock {line, event, eq}  =>
	  let val arguments = (case Symbol.look (env,symofname event) of
				   SOME (EventEntry {name,arguments}) => arguments
				 | _ => raise InvalidEvent)

	      val env' = foldl (fn(x,env) => 
				  Symbol.enter(env,symofname x,EventArgument {name=x, event=event}))
			       env arguments
			 
	      val eq = rebuild_eq filearg (env', eq)
	  in
	      EventBlock {line=line, event=event, eq=eq}
	  end

	| _ => eq


fun rebuild_entry (filearg: filearg) (env, entry) = 
    case entry of 
		  

	ScalarStateEntry {quantity, initval, method, eqs, description} =>
	ScalarStateEntry {quantity=quantity, initval=initval, method=method, 
			  eqs=(map (fn(eq) => rebuild_eq filearg (env, eq)) eqs), 
			  description=description}

      | VectorStateEntry {quantity, initval, method, eqs, description}
			  =>
	VectorStateEntry {quantity=quantity, initval=initval, method=method, 
			  eqs=(map (fn(eq) => rebuild_eq filearg (env, eq)) eqs), 
			  description=description}

      | IntegerStateEntry {quantity, initval, eqs, description} =>
	IntegerStateEntry {quantity=quantity, initval=initval, 
			  eqs=(map (fn(eq) => rebuild_eq filearg (env, eq)) eqs), 
			  description=description}

      | ExternalStateEntry {quantity, direction, channel, eqs, description} =>
	ExternalStateEntry {quantity=quantity, direction=direction, channel=channel,
			    eqs=(case eqs of
				     SOME eqs => SOME (map (fn(eq) => rebuild_eq filearg (env, eq)) eqs)
				   | NONE => eqs), 
			    description=description}

      | ScalarStateFunEntry {quantity, eqs, description} =>
	ScalarStateFunEntry {quantity=quantity,  
			     eqs=(map (fn(eq) => rebuild_eq filearg (env, eq)) eqs), 
			     description=description}

      | VectorStateFunEntry {quantity, eqs, size, description} =>
	VectorStateFunEntry {quantity=quantity,  
			     eqs=(map (fn(eq) => rebuild_eq filearg (env, eq)) eqs), 
			     description=description, size=size}

      | TableFunEntry {quantity, body, low, high, step, argument,
			  description} =>
	TableFunEntry {quantity=quantity, body=(#1(rebuild_exp filearg (env, Symbol.empty, body))), 
		       low=low, high=high, step=step,
		       argument=argument, description=description}

      | FunEntry {name: name, body: exp,
		  typeinfo: type_info,
		  calls: int ref, callhistory}  =>
	let
	    val {args=formals, ...} = typeinfo
	    val localenv = 
		foldl (fn ({name,typ}, t) => 
			  Symbol.enter (t, symofname name, typ))
		      Symbol.empty formals
	in
	    FunEntry {name=name,
		      body=(#1(rebuild_exp  filearg (env, localenv, body))),
		      typeinfo=typeinfo, calls=calls, callhistory=callhistory}
	end
	
      | _ => entry



fun vector_exp_check (filearg: filearg) (env, localenv, exp) =

    (case exp of

	 Ast.CONSTINT _ => 1
       | Ast.CONSTREAL _ => 1

       | Ast.ID id => 
	 (case Symbol.look (localenv, symofname id) of
	      SOME size => size
	    | NONE => 
	      (case Symbol.look (env, symofname id) of
		   SOME entry => DLEnv.sizeofentry entry
		 | NONE =>
		   (printError (filearg, posof id,
				 ERR_UNKNOWN_ID {name=id});
		    1)))

       | Ast.BINOP {oper, left, right, pos} =>
	 let
	     val leftsize = vector_exp_check filearg (env, localenv, left)
	     val rightsize = vector_exp_check filearg (env, localenv, right)

	     val resultsize = 
		 (case (leftsize, rightsize) of
		     (1, 1) => 1
		   | (1, _) =>
		     (case oper of 
			  Ast.SCALAR_VECTOR(Ast.SV_TIMES _)   => rightsize
			| _ =>  (printError (filearg, pos,
					     ERR_INVALID_VECTOR_BINOP {oper=oper});
				 0))
		   | (_, 1) => raise InvalidOperator
		   | (_, _) =>
		     (if (leftsize = rightsize)
		      then
			  case oper of
			      (* crossp only defined in 3d (and 7d but we don't care) *)
			      Ast.VECTOR (Ast.V_CROSSP  _)
			      => if (leftsize <> 3) orelse (rightsize <> 3) then
				     (printError (filearg, pos,
						  ERR_INVALID_VECTOR_SIZE_BINOP {oper=oper}); 
				      leftsize)
				 else
				     leftsize
			      
			    | Ast.UNKNOWN (Ast.CROSSP)
			      => if (leftsize <> 3) orelse (rightsize <> 3) then
				    (printError (filearg, pos,
						 ERR_INVALID_VECTOR_SIZE_BINOP {oper=oper}); 
				     leftsize)
				else
				    leftsize

			    | Ast.VECTOR _ => leftsize

			    | Ast.UNKNOWN _ => leftsize

			    | _   => (printError (filearg, pos,
						  ERR_INVALID_VECTOR_BINOP {oper=oper}); 1)
		      else
			  (printError (filearg, pos,
				       ERR_INVALID_VECTOR_SIZE_BINOP {oper=oper});
			   1)))


	 in
	     resultsize
	 end
       | Ast.RELOP {oper, left, right, pos} =>
	 let
	     val leftsize = vector_exp_check filearg (env, localenv, left)
	     val rightsize = vector_exp_check filearg (env, localenv, left)
	 in
	     if (oper = Ast.AND) orelse (oper = Ast.OR)
	     then
		 (printError (filearg, pos,
			      ERR_INVALID_VECTOR_COMPARISON {oper=oper});
		  1)
	     else
		 if (leftsize <> rightsize)
		 then
		     (printError (filearg, pos,
				  ERR_INVALID_VECTOR_SIZE_RELOP {oper=oper});
		      1)
		 else 1
	 end

       | Ast.FUNCALL {function, arguments, pos} =>

	 (let
	     val fun_entry = case (Symbol.look (env, symofname function)) of
							 SOME fun_entry => fun_entry
						   | NONE => raise InvalidFuncall before print ("function name = " ^ (name2str function) ^ "\n")
	     val arguments = map (fn (a) => vector_exp_check filearg (env, localenv, a)) arguments
	     val (formals, rettype, body, calls)   = 
		 (case fun_entry of 
		      FunEntry {name=_, body=body,
			       typeinfo={args,ret_type,pos=_},
			       calls, callhistory} => 
		      (args, ret_type, SOME body, SOME calls)
		    | BuiltinFunEntry {name=_,
				      typeinfo={args,ret_type,pos=_},
				      callhistory} => 
		      (args, ret_type, NONE, NONE)
		    | TableFunEntry {quantity=_, body=_, low=_,
				     high=_, step=_,
				     argument=argument,...} =>
		      ({name=argument, typ=TypeSet.singleton(Scalar)}::nil, 
		       TypeSet.singleton Scalar, NONE, NONE)
		    | _ => raise InvalidFunction)
		 
	     val formals = map (fn({name, typ}) => name) formals

	 in
	     if  TypeSet.equal (rettype, TypeSet.singleton (Vector NONE)) 
	     then
		 (foldl 
		     (fn(a,l) =>  
			case a of 
			    1 => l
			  | n => if n = l then l 
				 else  if l = 1 then n else 
				 (printError (filearg, pos,
					      ERR_INVALID_VECTOR_ARG_SIZE ({function=function}));
				  1)) 
		     1 arguments)
	     else
		 if  TypeSet.equal (rettype, TypeSet.singleton (Scalar)) 
		 then
		     (case body of 
			  SOME body =>
			  vector_exp_check filearg
					   (env, 
					    (let
						 val (localenv, _) = 
						     foldl (fn (a, (t,n::r)) => 
							       (Symbol.enter (t, symofname a, n), r) 
							     | (a, (t, nil)) =>
							       raise InvalidEmptyList)
							   (Symbol.empty, arguments) formals
					     in
						 localenv
					     end), 
					    body)
			| NONE => 1)
		 else
		     1
	 end)
	 
       | Ast.UNARYEXPOP {oper, exp, pos} =>
	 
	  (if (vector_exp_check filearg (env, localenv, exp) > 1)
	   then
	       (printError (filearg, pos,
			    ERR_INVALID_VECTOR_UNARYOP {oper=oper});
		1)
	   else
	       1)
	 
       | Ast.CONDITIONAL {condition, iftrue, iffalse, pos} =>
	 (let
	     val condsize = vector_exp_check filearg (env, localenv, condition)
	     val truesize = vector_exp_check filearg (env, localenv, iftrue)
	     val falsesize = vector_exp_check filearg (env, localenv, iffalse)
	 in
	     if truesize <> falsesize
	     then 
		 (printError (filearg, pos,
			      ERR_INVALID_VECTOR_SIZE_COND);
		  1)
	     else truesize
	 end))
    
    
(* check vector usage in the environment and determine sizes *)
fun vector_check (filearg: filearg) (env, entry) =
    let
	(* get a list of all sizes of usage of the vector *)
	fun get_sizes(nil, sizes) = sizes
	  | get_sizes(eq::rest, sizes) = 
	    let
		fun clause_check(clause, sizes) =
		    case clause of
			Ast.DISCRETE_COND (e, n) => vector_exp_check filearg (env, Symbol.empty, e)::sizes
		      | Ast.DEFAULT_COND _ => sizes

		fun discrete_vector_check(disc_exp) =
		    case disc_exp of
			Ast.DISCRETE_ID _ => sizes
		      | Ast.DISCRETE_CASE clauses => foldl clause_check sizes clauses

		val sizes = 
		    case eq of
			DifferenceEquation {line, exp} => vector_exp_check filearg (env, Symbol.empty, exp)::sizes
		      | DifferentialEquation {line, arg} => sizes
		      | AlgebraicEquation {line, exp} => vector_exp_check filearg (env, Symbol.empty, exp)::sizes
		      | DiscreteEquation {line, exp} => discrete_vector_check exp
		      | ConditionalBlock {line, condition, eq} => get_sizes(eq::nil, sizes)
		      | EventBlock {line, event, eq} => get_sizes(eq::nil, sizes)
	    in
		get_sizes(rest, sizes)
	    end
	    
	(* get the size of the vector, and report errors if multiple sizes found *)
	fun get_size(q, eqs) =
	    let
		fun checksize(size, sizeopt) =
		    case sizeopt of
			SOME prevsize => if prevsize = size 
					 then
					     SOME size
					 else
					     (printError (filearg, posof q,
							  ERR_INVALID_VECTOR_STATE_FUN_SIZE {quantity=q});
					      SOME prevsize)
		      | NONE => SOME size
		    
		val sizes = get_sizes(eqs, [])
	    in
		foldl checksize NONE sizes
	    end
		
	    
	(* create a new entry with updated size info *)
	val entry' = 
	    case entry of
		VectorStateFunEntry {quantity, eqs, size, description}
		=> VectorStateFunEntry {quantity=quantity,
					eqs=eqs,
					size=get_size(quantity, eqs),
					description=description}
	      | _ => entry
		     
    in
	entry' 
    end    
	
fun comp (x, y) =
    if x < y then
	LESS
    else if x > y then
	GREATER
    else
	EQUAL

structure IntegerSet = RedBlackSetFn (struct
				      type ord_key = int
				      val compare = comp
				      end)

(* adds set_element names to the env as SetEntry's *)
fun build_set_entries (filearg: filearg) (env) =
    let

	fun processDiscreteStates(DiscreteStateEntry {set, ...}, env') =
	    let
		val usednums = ref IntegerSet.empty
			       
		fun findNum(num) =
		    let
			val collision = IntegerSet.intersection(!usednums, 
								IntegerSet.singleton(num))
		    in
			if IntegerSet.equal (collision, IntegerSet.empty) 
			then (usednums := IntegerSet.union(!usednums,
							   IntegerSet.singleton(num));
			      num)
			else
			    findNum(num + 1)
		    end

		fun addNums(nil) = ()
		  | addNums ((name, SOME num)::rest) =
		    let
			val collision = IntegerSet.intersection(!usednums, 
								IntegerSet.singleton(num))
		    in
			if IntegerSet.equal (collision, IntegerSet.empty) 
			then 
			    usednums := IntegerSet.union (!usednums, IntegerSet.singleton(num))
			else 
			    printError(filearg, posof name, 
				       ERR_SAME_SETNUM{quantity=name})
		    end
		  | addNums ((name, NONE)::rest) =
		    addNums (rest)

		fun addSet((name, SOME num), env') =
		    Symbol.enter(env', symofname name, SetEntry{name=name, value=num})

		  | addSet((name, NONE), env') =
		    Symbol.enter(env', symofname name, SetEntry{name=name, value=findNum(0)})
	     
	    in
		addNums(set);
		foldl addSet env set
	    end

	  | processDiscreteStates (_, env') = env'

	val envitems:enventry list = Symbol.listItems(env)

    in
	foldl processDiscreteStates env envitems
    end


(* assigns channel numbers for external states *)
fun assign_external_channels (filearg: filearg) (env) =
    let
	val inputnums = ref IntegerSet.empty
	val outputnums = ref IntegerSet.empty

	fun findNum(num, usednums) =
	    let
		val collision = IntegerSet.intersection(!usednums, 
							IntegerSet.singleton(num))
	    in
		if IntegerSet.equal (collision, IntegerSet.empty) 
		then (usednums := IntegerSet.union(!usednums,
						   IntegerSet.singleton(num));
		      num)
		else
		    findNum(num + 1, usednums)
	    end

	fun processExternalStates(entry: enventry, env) =
	    let
		
	    	fun addNum(num, usednums) =
		    let
			val collision = IntegerSet.intersection(!usednums, 
								IntegerSet.singleton(num))
			val quantity = nameofentry entry
		    in
			if not (IntegerSet.equal (collision, IntegerSet.empty))
			then printError(filearg, posof quantity, 
					ERR_CHANNEL_COLLISION{quantity=quantity, channel=num})
			else ()
		    end
	    in
		case entry of 
		    ExternalStateEntry {quantity,
					direction,
					channel, eqs,
					description} =>
		    (let 
			 val usednums = case direction of 
					    Ast.INPUT  => inputnums
					  | Ast.OUTPUT => outputnums
		     in
			 case channel of 
			     SOME n => (addNum (n, usednums); env)
			   | NONE => (let 
					  val num = findNum (0, usednums)
					  val entry = ExternalStateEntry {quantity=quantity,
									  direction=direction,
									  channel=SOME num, eqs=eqs,
									  description=description}
				      in
					  Symbol.enter(env, symofname quantity, entry)
				      end)
		     end)
		  | _ => env
	    end
		
	val envitems: enventry list = Symbol.listItems (env)
    in
	foldl processExternalStates env envitems
    end
    


(* Top-level type checking function *)
fun type_check filearg (x as Ast.SYSTEM {name, decl, time}) =
    let
	(*
	 val _ = PrettyPrint.print_ast (TextIO.stdOut,x)
	 *)

	(* make the basic environment *)
	val env = make_env filearg (base_env, decl)

	(* add all the declarations to the environment *)
	val process_defun = process_defun filearg
	val env = foldl process_defun env decl

	(* creates an empty conditional table *)
	val condmap = UniqueId.empty

	(* function which takes a time block, and returns
	 * an environment (start or runtime) for that time block
	 *)
	fun proc_time_entry_fn (time_entry) =
	    case time_entry of
		Ast.STARTTIME(time_entries) => 
		let
		    val (_,env) = process_time_entries filearg (time_entries, condmap, env)
		in
		    START (env)
		end
		
	      | Ast.RUNTIME(name, time_entries) => 
		let
		    val env = process_time_entries filearg (time_entries, condmap, env)
		    
		in
		    RUNTIME (env)
		end

	(* build an environment for each timeblock
	 * if timeblock is runtime, check for unused states
	 *)
	val envlist = map proc_time_entry_fn time

	(* check that there is always a default condition for an equation *)
	val _ = default_exists_check filearg (envlist)

	(* check for any unresolved types *)
	fun printenv(env) =
	    ( print ("NEW ENV ENTRY\n");
	      case env of 
		  START env => PrettyPrint.print_env(TextIO.stdOut, env, condmap)
		| RUNTIME (_,env) => PrettyPrint.print_env(TextIO.stdOut, env, condmap))

	(*val _ = app printenv envlist *)

	(* check scalar usage for size consistency, and determine 
	 * sizes where needed 
	 *)
	fun env_vec_check(env') =
	    Symbol.map (fn(entry) =>
			  vector_check filearg (env', entry))
		       env'
		       
	val envlist = map (fn(env) => 
			     case env of 
				 RUNTIME (condmap,env) => RUNTIME (condmap,env_vec_check env)
			       | START env => START (env_vec_check env)) envlist

	(* rebuild the environment with properly typed operations *)
	fun rebuild_env(env') =
	    Symbol.map (fn(e) => rebuild_entry filearg (env', e)) env'

	val envlist = map (fn(env) => 
			     case env of 
				 RUNTIME (condmap,env) => RUNTIME (condmap,rebuild_env env)
			       | START env => START (rebuild_env env)) envlist

	val envlist = map  
			  (fn(env) => 
			     case env of 
				 RUNTIME (condmap,env) => RUNTIME (condmap,build_set_entries filearg env)
			       | START env => START (build_set_entries filearg env))
			  envlist


	val envlist = map  
			  (fn(env) => 
			     case env of 
				 RUNTIME (condmap,env) => RUNTIME (condmap,assign_external_channels filearg env)
			       | START env => START (assign_external_channels filearg env))
			  envlist

	(* get inline c code 
	 *)
	fun get_inline_c(decl) =
	    case decl of
		Ast.C_DECL(str) => SOME str
	      | _ => NONE

	val inline_c = List.mapPartial get_inline_c decl
    in
	case envlist of 
	    (RUNTIME (condmap,env))::nil => (env, NONE, condmap, inline_c)
	  | (RUNTIME (condmap,env'))::(START env'')::nil => (env', SOME env'', condmap, inline_c)
	  | (START env')::(RUNTIME (condmap,env''))::nil => (env'', SOME env', condmap, inline_c)
	  | _ => raise InvalidTimeBlock
						  
    end
    
	


end
