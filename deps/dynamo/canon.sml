(* Canonicalizer:
 * 
 *   Converts an DLEnv.env into a Canon.env'
 *
 *   This means it makes the enventries only have one equation each,
 *   breaking nesting based upon conditionals.  
 *)

signature DL_CANON = 
sig
    type name
    type equation
    type dependency
    type env

    type name'
    type enventry'
    type env'
    type condenvmap
    type eventenvmap

    val simplify_env: env -> condenvmap * eventenvmap * env' 

end


structure DLCanon =
struct


type name         = DLEnv.name
type env          = DLEnv.env
type equation     = DLEnv.equation
type condition    = DLEnv.condition

open DLEnv'


exception UnexpectedEquation
exception InvalidEntry

(* convert an equation into an equation' 
 *)
fun convert_eqn(eq: equation) =
    case eq of 
	DLEnv.DifferenceEquation{line, exp} => DifferenceEquation{line=line, exp=exp}

      | DLEnv.DifferentialEquation{line, arg} => DifferentialEquation{line=line, arg=arg}

      | DLEnv.AlgebraicEquation{line, exp} => AlgebraicEquation{line=line, exp=exp}

      | DLEnv.DiscreteEquation{line, exp} => DiscreteEquation{line= line, exp=exp}

      | DLEnv.ConditionalBlock _   => raise UnexpectedEquation

      | DLEnv.EventBlock _         => raise UnexpectedEquation

(* convert a name into name' *)
fun make_name'({name: symbol, pos:int}: name): name' =
    {name=name, id=UniqueId.genid()}

(* break equation blocks into their subequations *)
fun break_eq eq =
    case eq of
	DLEnv.ConditionalBlock {line=line, condition=condid, eq=eq} => 
	let
	    val (eqn, condid', _) = break_eq eq
	    val condid'' = case condid' of
			       SOME id => id
			     | NONE    => condid

	in
	    (eqn, SOME condid'', NONE)
	end

      | DLEnv.EventBlock {line, event, eq} => 
	let
	    val (eqn, condid, _) = break_eq eq
	in
	    (eqn, condid, SOME (make_name' event))
	end
	    
      | _ => (convert_eqn eq, NONE, NONE)
	
fun break_eqs eqs =  map break_eq eqs

(* convert set *)

fun convert_set env lst  =
    let 
	fun convert_set' (({name, pos}:name, num: int option): DLEnv.set_element, acc) =
	    (case num of 
		 SOME n => (name, n)::acc
	       | NONE => (let
			      val entry = valOf (Symbol.look(env, name))
			      val value = (case entry of 
					       DLEnv.SetEntry {name, value} => value
					     | _ => raise InvalidEntry)
			  in
			      (name, value)::acc
			  end))
    in
	foldl convert_set' [] lst 
    end
    


(* simplify a specific enventry into multiple enventry' *)
fun simplify_entry convert_set (entry, (condenvmap, eventenvmap, env)) =
    let

	fun update_condenv (condenvmap, condition, entry) =
	    (case UniqueId.look(condenvmap, condition) of
		 SOME condenv => UniqueId.enter(condenvmap, condition, 
						Symbol.enter (condenv, symofname (nameofentry entry), entry))
	       | NONE     => UniqueId.enter(condenvmap, condition, 
					    Symbol.enter (Symbol.empty, symofname (nameofentry entry), entry)))

	fun update_eventenv (eventenvmap, event, entry) =
	    (case Symbol.look (eventenvmap, event) of 
		 SOME eventenv => Symbol.enter(eventenvmap, event,
					       Symbol.enter(eventenv, symofname (nameofentry entry), entry))
	       | NONE => Symbol.enter(eventenvmap, event,
					       Symbol.enter(Symbol.empty, symofname (nameofentry entry), entry)))

	fun make_update (condenvmap, eventenvmap, env) (entry) =
	    let
		val condition = condofentry entry
		val event     = eventofentry entry
	    in
		case event of 
		    SOME event =>
		    (case condition of 
			 NONE => 
			 (condenvmap, update_eventenv (eventenvmap, symofname event, entry), env)
		       | SOME condition => 
			 (update_condenv (condenvmap, condition, entry), eventenvmap, env))
		  | NONE =>
		    (case condition of 
			 NONE => 
			 (condenvmap, eventenvmap, Symbol.enter (env, symofname (nameofentry entry), entry))
			 
		       | SOME condition => 
			 (update_condenv (condenvmap, condition, entry), eventenvmap, env))
	    end
		
	val update = make_update (condenvmap, eventenvmap, env)
		    
    in

	case entry of 
	    DLEnv.TimeEntry({quantity, initval, eq}) => 
	    update (TimeEntry {quantity=make_name' quantity,
			       initval=initval,
			       eq=convert_eqn eq})
		
	  | DLEnv.ScalarParEntry{quantity, value, description} => 

	    update (ScalarParEntry{quantity=make_name' quantity, 
				   value=value,
				   description=description})
		
	  | DLEnv.VectorParEntry{quantity, value, description} => 
	    update (VectorParEntry{quantity=make_name' quantity, 
				   value=value,
				   description=description})
		    
	  | DLEnv.ScalarStateEntry{quantity, initval, method, eqs, description} =>
	    let
		fun eqn_wrapper((eqn, condition, event), (condenvmap, eventenvmap, env)) = 
		    let
			val update = make_update (condenvmap, eventenvmap, env)
		    in
			update (ScalarStateEntry{quantity=make_name' quantity,
						 initval=initval,
						 method=method,
						 condition=condition,
						 event=event,
						 eq=eqn,
						 description=description})
		    end
	    in
		foldl eqn_wrapper (condenvmap, eventenvmap, env) (break_eqs eqs)
	    end


	  | DLEnv.VectorStateEntry{quantity, initval, method, eqs, description} =>
	    let

		fun eqn_wrapper((eqn, condition, event), (condenvmap, eventenvmap, env)) = 
		    let
			val update = make_update (condenvmap, eventenvmap, env)
		    in
			update (VectorStateEntry{quantity=make_name' quantity,
						 initval=initval,
						 method=method,
						 condition=condition,
						 event=event,
						 eq=eqn,
						 description=description})
		    end
	    in
		foldl eqn_wrapper (condenvmap, eventenvmap, env) (break_eqs eqs)
	    end

	  | DLEnv.IntegerStateEntry {quantity, initval, eqs, description} =>
	    let
		fun eqn_wrapper((eqn, condition, event), (condenvmap, eventenvmap, env)) = 
		    let
			val update = make_update (condenvmap, eventenvmap, env)
		    in
			update(IntegerStateEntry{quantity=make_name' quantity,
						 initval=initval,
						 condition=condition,
						 event=event,
						 eq=eqn,
						 description=description})
		    end
	    in
		foldl eqn_wrapper (condenvmap,eventenvmap,env) (break_eqs eqs)
	    end


	  | DLEnv.ExternalStateEntry{quantity, direction, channel, eqs, description} =>
	    let
		fun eqn_wrapper((eqn, condition, event), (condenvmap, eventenvmap, env)) = 
		    let
			val update = make_update (condenvmap, eventenvmap, env)
		    in
			update(ExternalStateEntry{quantity=make_name' quantity,
						  direction=direction,
						  channel=valOf channel,
						  condition=condition,
						  event=event,
						  eq=SOME eqn,
						  description=description})
		    end
	    in
		(case eqs of
		     SOME eqlist => 
		     foldl eqn_wrapper (condenvmap, eventenvmap, env) (break_eqs eqlist)
		   | NONE => 
		     update(ExternalStateEntry{quantity=make_name' quantity,
					       direction=direction,
					       channel=valOf channel,
					       condition=NONE,
					       event=NONE,
					       eq=NONE,
					       description=description}))
	    end


	  | DLEnv.DiscreteStateEntry{quantity, set, eqs, description} =>
	    let
		fun eqn_wrapper((eqn, condition, event), (condenvmap, eventenvmap, env)) =
		    let
			val update = make_update (condenvmap, eventenvmap, env)
		    in
			update(DiscreteStateEntry{quantity=make_name' quantity,
						  set=convert_set set,
						  condition=condition,
						  event=event,
						  eq=eqn,
						  description=description})
		    end
	    in
		foldl eqn_wrapper (condenvmap,eventenvmap,env) (break_eqs eqs)
	    end

	  | DLEnv.VectorStateFunEntry{quantity, eqs, size, description} =>
	    let
		fun eqn_wrapper((eqn, condition, event), (condenvmap, eventenvmap, env)) = 
		    let
			val update = make_update (condenvmap, eventenvmap, env)
		    in
			update(VectorStateFunEntry{quantity=make_name' quantity,
						   condition=condition,
						   event=event,
						   eq=eqn,
						   size=valOf size,
						   description=description})
		    end
	    in
		foldl eqn_wrapper (condenvmap,eventenvmap,env) (break_eqs eqs)
	    end

	  | DLEnv.ScalarStateFunEntry{quantity, eqs, description} =>
	    let
		fun eqn_wrapper((eqn, condition, event), (condenvmap, eventenvmap, env)) = 
		    let
			val update = make_update (condenvmap, eventenvmap, env)
		    in
			update(ScalarStateFunEntry{quantity=make_name' quantity,
						   condition=condition,
						   event=event,
						   eq=eqn,
						   description=description})
		    end
	    in
		foldl eqn_wrapper (condenvmap,eventenvmap,env) (break_eqs eqs)
	    end

	  | DLEnv.TableFunEntry{quantity, body, low, high, step, argument, description} =>

	    update(TableFunEntry{quantity=make_name' quantity, body=body, low=low,
				 high=high, step=step,
				 argument=make_name' argument,
				 description=description})

	  | DLEnv.FunEntry{name, body, typeinfo, calls, callhistory} =>
	    let
		val args = if (!calls) > 0 
			   then
			       map (fn ({name: DLAst.name, typ: typeset}) => 
				       {name=make_name' name, typ=typ})
				   (#args typeinfo)
			   else
			       []
	    in
		if (!calls) > 0
		then
		    update(FunEntry{name=make_name' name,
				    body=body,
				    typeinfo={args=args, ret_type=(#ret_type typeinfo)}})
		else
		    (condenvmap, eventenvmap, env)
	    end

	  | DLEnv.FunAlias{name, entryname, typeinfo} =>
	    let
		val args = map (fn ({name: DLAst.name, typ: typeset}) => 
				   {name=make_name' name, typ=typ})
			       (#args typeinfo)
	    in
		update(FunAlias{name=make_name' name,
				entryname=make_name' entryname,
				typeinfo={args=args, ret_type=(#ret_type typeinfo)}})
	    end

	  | DLEnv.BuiltinFunEntry{name, typeinfo, callhistory} =>
	    let
		val args = map (fn ({name: DLAst.name, typ: typeset}) => 
				   {name=make_name' name, typ=typ})
			       (#args typeinfo)

	    in
		update(BuiltinFunEntry{name=make_name' name, 
				       typeinfo={args=args, ret_type=(#ret_type typeinfo)}})
	    end

	  | DLEnv.BuiltinScalar{name} =>
	    update(BuiltinScalar{name=make_name' name})

	  | DLEnv.ForeignScalar{name} =>
	    update(ForeignScalar{name=make_name' name})

	  | DLEnv.SetEntry {name, value} => 
	    update(SetEntry {name=make_name' name, value=value})

	  | DLEnv.EventEntry {name, arguments} => 
	    update(EventEntry {name=make_name' name, arguments=map make_name' arguments})

	  | DLEnv.EventArgument _ => raise InvalidEntry

	  | DLEnv.Dummy => raise InvalidEntry

    end

fun update_funaliases env' (DLEnv'.FunAlias {name, entryname, typeinfo}, table) =
    Symbol.enter(table, 
		 symofname(name), 
		 DLEnv'.FunAlias {name=name, 
				  entryname=nameofentry(valOf(Symbol.look(env', symofname(entryname)))), 
				  typeinfo=typeinfo})
  | update_funaliases env' (entry, table) =
    Symbol.enter (table,
		  symofname(nameofentry(entry)), 
		  entry)

(* simplify an env into an env' *)
fun simplify_env(env) =
    let
	val (condenvmap, eventenvmap, env') = 
	    foldl (simplify_entry (convert_set env)) 
		  (UniqueId.empty, Symbol.empty, Symbol.empty) 
		  (Symbol.listItems env)

	val env' = foldl (update_funaliases env') Symbol.empty (Symbol.listItems env')
    in
	(condenvmap, eventenvmap, env')
    end

end
