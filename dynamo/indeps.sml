(* Independency expression translation for generic and table
 * functions. 
 *)

signature DL_INDEPS =
sig
    type symbol        = Symbol.symbol

    type exp          = DLEnv'.exp

    type env'         = DLEnv'.env'
    type name'        = DLEnv'.name'

    type unaryexpop = DLAst.unaryexpop
    type binop      = DLAst.binop
    type relop      = DLAst.relop

    datatype indepname  = STATELESS of name'
			| FUNARG    of name'
			| BUILTIN   of name'
			| TABLE     of name'
			| PARAM     of name'

    datatype indepexp = 
	CONSTINT of int
      | CONSTREAL of real
      | ID of indepname
      | BINOP of {oper: binop, left: indepexp, right: indepexp}
      | RELOP of {oper: relop, left: indepexp, right: indepexp}
      | FUNCALL of {function: indepname, arguments: indepexp list}
      | UNARYEXPOP of {oper: unaryexpop, exp: indepexp}
      | CONDITIONAL of {condition: indepexp, iftrue: indepexp, iffalse: indepexp}

    val make_indep: env' * name' list * exp -> indepexp
    val nameofindepname:  indepname      -> name'
    val symofindepname:     indepname      -> symbol

						
end

structure DLInDeps: DL_INDEPS =
struct

    type symbol        = Symbol.symbol

    type exp          = DLEnv'.exp

    type env'         = DLEnv'.env'
    type name'        = DLEnv'.name'

    type unaryexpop = DLAst.unaryexpop
    type binop      = DLAst.binop
    type relop      = DLAst.relop

    datatype indepname  = STATELESS of name'
			| FUNARG    of name'
			| BUILTIN   of name'
			| TABLE     of name'
			| PARAM     of name'

    datatype indepexp = 
	CONSTINT of int
      | CONSTREAL of real
      | ID of indepname
      | BINOP of {oper: binop, left: indepexp, right: indepexp}
      | RELOP of {oper: relop, left: indepexp, right: indepexp}
      | FUNCALL of {function: indepname, arguments: indepexp list}
      | UNARYEXPOP of {oper: unaryexpop, exp: indepexp}
      | CONDITIONAL of {condition: indepexp, iftrue: indepexp, iffalse: indepexp}

    exception InvalidName

	fun nameofindepname(name) = 
		(case name of
			 STATELESS name => name
		   | FUNARG name    => name
		   | BUILTIN name   => name
		   | TABLE name     => name
		   | PARAM name     => name)  

	fun symofindepname(name) = 
		DLEnv'.symofname (nameofindepname name)



    fun make_indepname (args, env, name: symbol) =
	let
	    fun find_arg [] = NONE
	      | find_arg ((h::r): name' list) = 
		(if (#name h) = name then
		     SOME h else NONE)

		fun lookup name =
			case valOf(Symbol.look (env, name)) of
				DLEnv'.ScalarParEntry {quantity, ...}      => PARAM quantity
			  | DLEnv'.VectorParEntry {quantity, ...}      => PARAM quantity
			  | DLEnv'.TableFunEntry {quantity, ...}       => TABLE quantity
			  | DLEnv'.FunEntry {name, ...}                => STATELESS name
			  | DLEnv'.BuiltinScalar {name, ...}           => BUILTIN name 
			  | DLEnv'.BuiltinFunEntry {name, ...}         => BUILTIN name
			  | DLEnv'.FunAlias {entryname, ...}           => lookup (DLEnv'.symofname entryname)
			  | _                                          => raise InvalidName
	in
	    case find_arg(args) of
		SOME arg => FUNARG arg
	  | NONE     => lookup name
		
	end


fun make_indep (env', args, exp: DLAst.exp) =
    
    case exp of
	DLAst.CONSTINT (num) =>
	CONSTINT (num)
	
      | DLAst.CONSTREAL (num) =>
	CONSTREAL (num)
	
      |	DLAst.ID {name=sym, pos=_} =>
	ID (make_indepname (args, env', sym))
	
	
      | DLAst.BINOP {oper=oper, left=left, right=right, pos=_} =>

    let	
	val leftexp  = make_indep(env', args, left)
	val rightexp = make_indep(env', args, right)
    in
	BINOP {oper=oper, left=leftexp, right=rightexp}
    end

  | DLAst.RELOP {oper=oper, left=left, right=right, pos=pos} =>

    let	
	val leftexp  = make_indep(env', args, left)
	val rightexp = make_indep(env', args, right)
    in
	RELOP {oper=oper, left=leftexp, right=rightexp}
    end

  | DLAst.FUNCALL {function=function, arguments=arguments, pos=pos} =>
    let
	val explist = map (fn(a) => make_indep(env', args, a)) 
			  arguments
    in
	 FUNCALL {function=make_indepname (args, env', #name function), arguments=explist}
    end

  | DLAst.UNARYEXPOP {oper=oper, exp=exp, pos=pos} =>
    let
	val exp = make_indep (env', args, exp)
    in
	UNARYEXPOP {oper=oper, exp=exp}
    end
		       

  | DLAst.CONDITIONAL {condition=cond, iftrue=iftrue, iffalse=iffalse, pos=pos} =>

    let
	val condexp    = make_indep (env', args, cond)
	val iftrueexp  = make_indep (env', args, iftrue)
	val iffalseexp = make_indep (env', args, iffalse)
    in
	CONDITIONAL {condition=condexp, 
		     iftrue=iftrueexp, 
		     iffalse=iffalseexp}
    end
end
