
structure PrettyPrint = 
struct

structure Ast = DLAst

type model = Ast.system
type component_decl = Ast.component_decl
type equation = Ast.equation
type exp = Ast.exp

open DLEnv

val say =
    (fn(outstream) => (fn (str) => TextIO.output(outstream, str)))
    
val sayln = 
    (fn(say) => (fn (str) => (say str; say "\n")))

val ind = 
    (fn(say) => (fn(n) => 
		   let 
		       fun ind' (n) = 
			   if n <= 0 then () else (say " "; ind' (n-1))
		   in
		       ind' (n)
		   end)) 
    
val indent = 
    (fn(ind) => (fn(indent_factor) => (fn(n) => ind(n * indent_factor))))

fun unaryop2str (oper) = 
    (case oper of 
	 Ast.UMINUS => "UMINUS"
       | Ast.UPLUS  => "UPLUS"
       | Ast.NOT    => "NOT")

fun binop2str (Ast.UNKNOWN(Ast.PLUS))   = "PLUS"
  | binop2str (Ast.UNKNOWN(Ast.MINUS))  = "MINUS"
  | binop2str (Ast.UNKNOWN(Ast.TIMES))  = "TIMES"
  | binop2str (Ast.UNKNOWN(Ast.DIVIDE)) = "DIVIDE"
  | binop2str (Ast.UNKNOWN(Ast.MODULUS))= "MODULUS"
  | binop2str (Ast.UNKNOWN(Ast.POWER))  = "POWER"
  | binop2str (Ast.UNKNOWN(Ast.CROSSP)) = "CROSSP"

  | binop2str (Ast.SCALAR(Ast.S_PLUS))    = "S_PLUS"
  | binop2str (Ast.SCALAR(Ast.S_MINUS))   = "S_MINUS"
  | binop2str (Ast.SCALAR(Ast.S_TIMES))   = "S_TIMES"
  | binop2str (Ast.SCALAR(Ast.S_DIVIDE))  = "S_DIVIDE"
  | binop2str (Ast.SCALAR(Ast.S_MODULUS)) = "S_MODULUS"
  | binop2str (Ast.SCALAR(Ast.S_POWER))   = "S_POWER"

  | binop2str (Ast.SCALAR_VECTOR(Ast.SV_TIMES n)) = "SV_TIMES(" ^ (Int.toString n) ^ ")"

  | binop2str (Ast.VECTOR(Ast.V_PLUS n))   = "V_PLUS("^ (Int.toString n) ^ ")"
  | binop2str (Ast.VECTOR(Ast.V_MINUS n))  = "V_MINUS("^ (Int.toString n) ^ ")"
  | binop2str (Ast.VECTOR(Ast.V_TIMES n))  = "V_TIMES("^ (Int.toString n) ^ ")"
  | binop2str (Ast.VECTOR(Ast.V_CROSSP n)) = "V_CROSSP("^ (Int.toString n) ^ ")"

fun relop2str (Ast.EQ) = "EQ"
  | relop2str (Ast.GT) = "GT"
  | relop2str (Ast.LT) = "LT"
  | relop2str (Ast.GE) = "GE"
  | relop2str (Ast.LE) = "LE"
  | relop2str (Ast.NE) = "NE"
  | relop2str (Ast.AND) = "AND"
  | relop2str (Ast.OR) = "OR"
	
fun id2str (id) =
    (Int.toString o UniqueId.id2int) id

fun name2str (n) =
    let
	val {name=name, pos=_} = n
    in
	Symbol.name name
    end

fun name'2str (n) =
    let
	val {name=name, id=id} = n
	val name = Symbol.name name
	val id = Int.toString(UniqueId.id2int(id))
    in
	"(" ^ name ^ ":" ^ id ^ ")"
    end

fun name'2idstr (n) =
    let
	val {name=name, id=id} = n
	val name = Symbol.name name
	val id = Int.toString(UniqueId.id2int(id))
    in
	name ^ id 
    end


fun depname2str (n) =
    let 
	val (prefix, name) = case n of
				 DLDeps.OLD (n) => ("OLD", name'2str n)
			       | DLDeps.NEW (n) => ("NEW", name'2str n)
			       | DLDeps.STATELESS (n) => ("STATELESS", name'2str n)
			       | DLDeps.BUILTIN (n) => ("BUILTIN", name'2str n)
			       | DLDeps.CONDITION id => ("CONDITION", id2str id)
			       | DLDeps.TABLE id => ("TABLE", name'2str id)
			       | DLDeps.PARAM n => ("PARAM", name'2str n)
			       | DLDeps.EVENT n => ("EVENT", name'2str n)
			       | DLDeps.EVENTARG (n,_) => ("EVENTARG", name'2str n)
			       | DLDeps.SETELM n => ("SETELM", name'2str n)
    in
	prefix ^ " " ^ name
    end

fun indepname2str (n) =
    let 
	val (prefix, name) = case n of
				 DLInDeps.STATELESS (n) => ("STATELESS", name'2str n)
			       | DLInDeps.FUNARG(s) => ("FUNARG", name'2str s)
			       | DLInDeps.BUILTIN(s) => ("BUILTIN", name'2str s)
			       | DLInDeps.TABLE(s) => ("TABLE", name'2str s)
			       | DLInDeps.PARAM(s) => ("PARAM", name'2str s)
    in
	prefix ^ " " ^ name
    end

fun depname2idstr (n) =
    let 
	val (prefix, name) = case n of
				 DLDeps.OLD (n) => ("OLD", name'2idstr n)
			       | DLDeps.NEW (n) => ("NEW", name'2idstr n)
			       | DLDeps.STATELESS (n) => ("STATELESS", name'2idstr n)
			       | DLDeps.BUILTIN (n) => ("BUILTIN", name'2idstr n)
			       | DLDeps.CONDITION id => ("CONDITION", id2str id)
			       | DLDeps.TABLE id => ("TABLE", name'2idstr id)
			       | DLDeps.PARAM n => ("PARAM", name'2idstr n)
			       | DLDeps.EVENT n => ("EVENT", name'2idstr n)
			       | DLDeps.EVENTARG (n,_) => ("EVENTARG", name'2idstr n)
			       | DLDeps.SETELM n => ("SETELM", name'2idstr n)
    in
	prefix ^ "_" ^ name
    end


fun print_discrete_exp_clauses (outstream, expr, i) =
    let
	val indent_factor = 2
			    
	val say = say (outstream)
	val sayln = sayln (say)
	val ind = ind (say)
	val indent = indent (ind) (indent_factor)
    in

	case expr of
	    Ast.DISCRETE_COND(e,sym) =>
	    (indent(i);
	     sayln(name2str(sym)))
	    
	  | Ast.DEFAULT_COND(sym) =>
	    (indent(i);
	     sayln(name2str(sym)))
    end

fun print_discrete_exp (outstream, expr, i) =
    let
	val indent_factor = 2
			    
	val say = say (outstream)
	val sayln = sayln (say)
	val ind = ind (say)
	val indent = indent (ind) (indent_factor)
    in
	case expr of 
	    Ast.DISCRETE_ID(sym) =>
	    (indent(i);
	     sayln((name2str(sym))))
	    
	  | Ast.DISCRETE_CASE(disc_exp_clause_list) =>
	    (indent(i);
	     app (fn(x) =>
		    print_discrete_exp_clauses(outstream, x, i+1))
		 disc_exp_clause_list)
    end

fun printset (outstream, set, i) =
    let
	val indent_factor = 2
			    
	val say = say (outstream)
	val sayln = sayln (say)
	val ind = ind (say)
	val indent = indent (ind) (indent_factor)
	
    in
	sayln("{" 
	      ^ (foldl (fn((sym, intopt), s) =>
			  case intopt of
			      SOME i => s ^ " " ^ (name2str(sym) ^
						   Int.toString(i))
			    | NONE   => s ^ " " ^ (name2str(sym)))
		       ""
		       set) 
	      ^ "}")
    end

fun printset' (outstream, set, i) =
    let
	val indent_factor = 2
			    
	val say = say (outstream)
	val sayln = sayln (say)
	val ind = ind (say)
	val indent = indent (ind) (indent_factor)
	
    in
	sayln("{" 
	      ^ (concat (map (fn(sym, n) =>  (Symbol.name(sym) ^  ": " ^ Int.toString(n) ^ "  ")) set))
	      ^ "}")
    end


fun print_exp (outstream, top_exp:Ast.exp) =
    let
	val indent_factor = 2

	val say = say (outstream)
	val sayln = sayln (say)
	val ind = ind (say)
	val indent = indent (ind) (indent_factor)

	fun exp (Ast.CONSTINT(x), i) =
	    (indent(i);
	     sayln("CONSTINT " ^ (Int.toString(x))))
	    
	  | exp (Ast.CONSTREAL(x), i) =
	    (indent(i);
	     sayln("CONSTREAL " ^ (Real.toString(x))))
	    

	  | exp (Ast.ID({name, ...}), i) =
	    (indent(i);
	     sayln("ID " ^ Symbol.name(name)))
	    
	  | exp (Ast.BINOP{oper,
		       left,
		       right,
		       pos}, i) =
	    (indent (i);
	     sayln("BINOP " ^ binop2str(oper) ^ ": ");
	     indent (i+1);
	     sayln("LEFT ");
	     exp(left, i+2);
	     indent (i+1);
	     sayln("RIGHT ");
	     exp(right,i+2))

	  | exp (Ast.RELOP{oper,
			   left,
			   right,
			   pos}, i) = 
	    (indent (i);
	     sayln("RELOP " ^ relop2str(oper) ^ ": ");
	     indent (i+1);
	     sayln("LEFT ");
	     exp(left, i+2);
	     indent (i+1);
	     sayln("RIGHT ");
	     exp(right,i+2))
	    
	  | exp (Ast.FUNCALL{function,
			 arguments,
			 pos}, i) =
	    (indent (i);
	     sayln("FUNCTION CALL " ^ (name2str(function)));
	     indent(i+1);
	     sayln("ARGUMENTS ");
	     app (fn(x) => 
		    exp(x,i+2))
		 arguments)
	     
	  | exp (Ast.UNARYEXPOP{oper,
				exp=e,
				pos}, i) =
	    (indent (i);
	     sayln("UNARY EXP OP " 
		   ^ (case oper of 
			  Ast.UMINUS => "UMINUS"
			| Ast.UPLUS => "UPLUS"
			| Ast.NOT => "NOT"));
	     indent(i+1);
	     sayln("EXP ");
	     exp(e,i+2))

	
	  | exp (Ast.CONDITIONAL{condition,
				 iftrue,
				 iffalse,
				 pos}, i) =
	    (indent(i);
	     sayln("CONDITIONAL EXP:");

	     indent(i+1);
	     sayln("CONDITION ");
	     exp(condition, i+2);

	     indent(i+1);
	     sayln("IFTRUE ");
	     exp(iftrue, i+2);

	     indent(i+1);
	     sayln("IFFALSE ");
	     exp(iffalse, i+2))
	    
			     


    in
	exp(top_exp, 1); 
	sayln("");
	TextIO.flushOut outstream
    end


fun print_diff_args(outstream, args,  i) =
    let
	val indent_factor = 2
			    
	val say = say (outstream)
	val sayln = sayln (say)
	val ind = ind (say)
	val indent = indent (ind) (indent_factor)
    in
	case args of 
	    Ast.EULER_ARGS(e) =>
	    (indent(i);
	     sayln("Euler: ");
	     print_exp(outstream, e))
	    
      | Ast.MAU_ARGS(exp1, exp2) =>
	(indent(i);
	 sayln "MAU: ";
	 print_exp(outstream, exp1);
	 print_exp(outstream, exp2))

    end

fun print_ast (outstream, m:Ast.system) =
    let
	val indent_factor = 2

	val say = say (outstream)
	val sayln = sayln (say)
	val ind = ind (say)
	val indent = indent (ind) (indent_factor)

	fun model (Ast.SYSTEM{name, decl, time}) =
	    let
		val {name=n, pos=_} = name
	    in
		(sayln ("SYSTEM = " ^ (Symbol.name(n)));
		 app declaration decl;
		 app system_time time)
	    end

	and declaration (Ast.COMPONENT_DECL(comp_decl)) =
	    (sayln "COMPONENT_DECL(";
	     component_decl(comp_decl,1);
	     sayln ")"
	     )


	  | declaration (Ast.TIME_DECL(name)) =
	    let
		val {name=sym, pos=_} = name
	    in
		sayln ("TIME_DECL (" ^ (Symbol.name(sym)) ^ ")")
	    end

	  | declaration (Ast.EVENT_DECL name) =
	    let
		val {name=sym, pos=_} = name
	    in
		sayln ("EVENT_DECL (" ^ (Symbol.name(sym)) ^ ")")
	    end

	  | declaration (Ast.FUNCTION_DEF{name,
					 args,
					 body,
					 pos}) =
	    let
		val {name=n, pos=_} = name

		(* strips out the pos information *)
		fun arg_to_sym ({name=n,pos=_}) =
		    n
	    in
		(say (Symbol.name(n));
		 say (" (");
		 map (say o Symbol.name o arg_to_sym) args;
		 sayln (") =");
		 exp(body, 1)
		 )
	    end

	  | declaration (Ast.C_DECL(s)) =
	    (sayln("C_DECL = [[[");
	     sayln(s);
	     sayln("]]]"))
	     
	and component_decl (Ast.SCALAR_STATE_DECL{quantity,
						  init,
						  method,
						  description,
						  pos
						 }, i) =
	    (indent(i);
	     sayln("SCALAR_STATE_DECL(");
	     indent(i+1);
	     sayln("quantity    = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln "init     = "; exp(init,i);
	     indent(i+1);
	     sayln("method      = " ^ (case method of
					   NONE           => "NONE"
					 | SOME Ast.MAU   => "MAU"
					 | SOME Ast.EULER => "EULER"));
	     indent(i+1);
	     sayln("description = " ^ (case description of
					   SOME s => s
					 | NONE   => ""));
	     indent(i);
	     sayln(")"))

	  | component_decl (Ast.VECTOR_STATE_DECL{quantity,
					      initval,
					      method,
					      description,
					      pos
					      }, i) =
	    (indent(i);
	     sayln("VECTOR_STATE_DECL(");
	     indent(i+1);
	     sayln("quantity    = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln("initval     = [" ^ (Array.foldl 
					   (fn (x,str) =>
					       str ^ (Real.toString(x)))
					   ""
					   initval)
		   ^ "]");
	     						       
	     indent(i+1);
	     sayln("method      = " ^ (case method of
					   NONE           => "NONE"
					 | SOME Ast.MAU   => "MAU"
					 | SOME Ast.EULER => "EULER"));
	     indent(i+1);
	     sayln("description = " ^ (case description of
					   SOME s => s
					 | NONE   => ""));
	     indent(i);
	     sayln(")"))


	  | component_decl(Ast.INT_STATE_DECL{quantity,
					      init,
					      description,
					      pos
					     },i) =
	    (indent(i);
	     sayln("INT_STATE_DECL(");
	     indent(i+1);
	     sayln("quantity = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln "init     = "; exp(init,i);
	     indent(i+1);
	     sayln("description = " ^ (case description of
					   SOME s => s
					| NONE   => ""));
	     indent(i);
	     sayln(")"))

	  | component_decl (Ast.SCALAR_PAR_DECL{quantity,
						init,
						description,
						pos
					       }, i) =
	    (indent(i);
	     sayln("SCALAR_PAR_DECL(");
	     indent(i+1);
	     sayln("quantity    = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln "init        = "; exp(init,i);
	     indent(i+1);
	     sayln("description = " ^ (case description of
					   SOME s => s
					 | NONE   => ""));
	     indent(i);
	     sayln(")"))

	  | component_decl (Ast.VECTOR_PAR_DECL{quantity,
						initval,
						description,
						pos
						}, i) =
	    (indent(i);
	     sayln("VECTOR_PAR_DECL(");
	     indent(i+1);
	     sayln("quantity    = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln("initval     = [" ^ (Array.foldl 
					   (fn (x,str) =>
					       str ^ (Real.toString(x)))
					   ""
					   initval)
		   ^ "]");
	     						       
	     indent(i+1);
	     sayln("description = " ^ (case description of
					   SOME s => s
					 | NONE   => ""));
	     indent(i);
	     sayln(")"))

	  | component_decl(Ast.SCALAR_STATE_FUNC_DECL{quantity,
					   description,
					   pos}, i) =
	    (indent(i);
	     sayln("SCALAR_STATE_FUNC_DECL (");
	     indent(i+1);
	     sayln("quantity    = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln("description = " ^ (case description of 
					   SOME s => s
					 | NONE   => ""));
	     indent(i);
	     sayln(")"))
	  | component_decl(Ast.VECTOR_STATE_FUNC_DECL{quantity,
					   description,
					   pos}, i) =
	    (indent(i);
	     sayln("VECTOR_STATE_FUNC_DECL (");
	     indent(i+1);
	     sayln("quantity    = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln("description = " ^ (case description of 
					   SOME s => s
					 | NONE   => ""));
	     indent(i);
	     sayln(")"))

	  | component_decl(Ast.TABLE_DECL{quantity,
					  parameters,
					  body,
					  low,
					  high,
					  step,
					  argument,
					  description,
					  pos}, i) =
	    (indent(i);
	     sayln("TABLE_DECL (");
	     indent(i+1);
	     sayln("quantity    = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln("argument   = " ^ (name2str(argument)));
	     indent(i+1);
	     sayln("low         = " ^ (Real.toString(low)));
	     indent(i+1);
	     sayln("high        = " ^ (Real.toString(high)));
	     indent(i+1);
	     sayln("step        = " ^ (Real.toString(step)));
	     indent(i+1);
	     sayln("description = " ^ (case description of 
					   SOME s => s
					 | NONE => ""));
	     indent(i+1);
	     sayln("body =");
	     exp(body, i+2))

	  | component_decl(Ast.C_ID_DECL{quantity,
					 pos}, i) =
	    (indent(i);
	     sayln("C_ID_DECL = " ^ (name2str(quantity))))

	  | component_decl(Ast.EXTERNAL_DECL{quantity,
					     direction,
					     channel,
					     description,
					     pos}, i) =
	    (indent(i);
	     sayln("EXTERNAL_DECL (");
	     indent(i+1);
	     sayln("quantity    = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln("direction   = " ^ (case direction of 
					   Ast.INPUT => "INPUT"
					 | Ast.OUTPUT => "OUTPUT"));
	     indent(i+1);
	     sayln("channel   = " ^ (case channel of 
					 NONE => "NONE"
				       | SOME n => (Int.toString n)));

	     indent(i+1);
	     sayln("description = " ^ (case description of 
					   SOME s => s
					 | NONE   => ""));
	     indent(i);
	     sayln(")"))
	    
	  | component_decl(Ast.DISCRETE_DECL{quantity,
					 set,
					 description,
					 pos}, i) =
	    (indent(i);
	     sayln("DISCRETE_DECL (");
	     indent(i+1);
	     sayln("quantity    = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln("set         = {" 
		   ^ (foldl (fn((sym, intopt):Ast.set_element, s) =>
			      case intopt of
				  SOME i => s ^ " " ^ (name2str(sym) ^
						       Int.toString(i))
				| NONE   => s ^ " " ^ (name2str(sym)))
			 ""
			 set) 
		   ^ "}");
	     indent(i+1);
	     sayln("description = " ^ (case description of 
					   SOME s => s
					 | NONE   => ""));
	     indent(i);
	     sayln(")"))

	and exp (Ast.CONSTINT(x), i) =
	    (indent(i);
	     sayln("CONSTINT = " ^ (Int.toString(x))))
	    
	  | exp (Ast.CONSTREAL(x), i) =
	    (indent(i);
	     sayln("CONSTREAL = " ^ (Real.toString(x))))
	    

	  | exp (Ast.ID({name, ...}), i) =
	    (indent(i);
	     sayln("ID = " ^ Symbol.name(name)))
	    
	  | exp (Ast.BINOP{oper,
		       left,
		       right,
		       pos}, i) =
	    (indent (i);
	     sayln("BINOP of " ^ binop2str(oper) ^ ": ");
	     indent (i+1);
	     sayln("LEFT = ");
	     exp(left, i+2);
	     indent (i+1);
	     sayln("RIGHT = ");
	     exp(right,i+2))

	  | exp (Ast.RELOP{oper,
			   left,
			   right,
			   pos}, i) = 
	    (indent (i);
	     sayln("RELOP of " ^ relop2str(oper) ^ ": ");
	     indent (i+1);
	     sayln("LEFT = ");
	     exp(left, i+2);
	     indent (i+1);
	     sayln("RIGHT = ");
	     exp(right,i+2))
	    
	  | exp (Ast.FUNCALL{function,
			 arguments,
			 pos}, i) =
	    (indent (i);
	     sayln("FUNCTION CALL " ^ (name2str(function)));
	     indent(i+1);
	     sayln("ARGUMENTS =");
	     app (fn(x) => 
		    exp(x,i+2))
		 arguments)
	     
	  | exp (Ast.UNARYEXPOP{oper,
				exp=e,
				pos}, i) =
	    (indent (i);
	     sayln("UNARY EXP OP = " 
		   ^ (case oper of 
			  Ast.UMINUS => "UMINUS"
			| Ast.UPLUS => "UPLUS"
			| Ast.NOT => "NOT"));
	     indent(i+1);
	     sayln("EXP = ");
	     exp(e,i+2))

	
	  | exp (Ast.CONDITIONAL{condition,
				 iftrue,
				 iffalse,
				 pos}, i) =
	    (indent(i);
	     sayln("CONDITIONAL EXP:");

	     indent(i+1);
	     sayln("CONDITION = ");
	     exp(condition, i+2);

	     indent(i+1);
	     sayln("IFTRUE = ");
	     exp(iftrue, i+2);

	     indent(i+1);
	     sayln("IFFALSE = ");
	     exp(iffalse, i+2))

	and system_time (Ast.STARTTIME(st)) =
	    starttime(st)

	  | system_time (Ast.RUNTIME(rt)) =
	    runtime(rt)

	and starttime(tel) =
	    (sayln("STARTTIME = ");
	     app (fn(x) =>
		    time_entry(x,1))
		 tel)

	and runtime({name=n,pos=_}, tel) =
	    (sayln("RUNTIME (" ^ (Symbol.name(n)) ^ ") =");
	     app (fn(x) =>
		    time_entry(x,1))
		 tel)

	and time_entry(Ast.COND({relop,
				 quantity1,
				 quantity2,
				 body,
				 pos}), i) =
	    (indent(i);
	     sayln("TIME_ENTRY CONDITIONAL BLOCK:");
	     indent(i+1);
	     sayln("relop     = " ^ relop2str(relop));
	     indent(i+1);
	     sayln("quantity1 = " ^ (name2str(quantity1)));
	     indent(i+1);
	     sayln("quantity2 = " ^ (name2str(quantity2)));
	     indent(i+1);
	     app (fn(x) =>
		    time_entry(x,i+2))
		 body)
	    
	  | time_entry(Ast.EQN(eqn), i) =
	    (indent(i);
	     sayln("TIME_ENTRY EQUATION BLOCK: ");
	     equation(eqn, i+1))

	  | time_entry (Ast.EVENT {name,arguments,equations,pos},i) =
	    (indent i;
	     sayln("TIME_ENTRY EVENT BLOCK: ");
	     sayln("name = " ^ (name2str name));
	     indent(i+1);
	     app (fn(x) =>
		    time_entry(x,i+2))
		 equations)
	     

	and equation(Ast.DIFFERENCE_EQN(sym,e,p), i) =
	    (indent(i);
	     sayln("DIFFERENCE_EQN " ^ (name2str(sym)) ^ " = ");
	     exp(e,i+1))
	    
	  | equation(Ast.DISCRETE_EQN(sym, discexp, p), i) =
	    (indent(i);
	     sayln("DISCRETE_EQN " ^ (name2str(sym)) ^ " = ");
	     print_discrete_exp(outstream, discexp,i+1))

	  | equation(Ast.ALGEBRAIC_EQN(sym, e, pos), i) =
	    (indent(i);
	     sayln("ALGEBRAIC_EQN" ^ (name2str(sym)) ^ " = ");
	     exp(e, i+1))

	  | equation(Ast.DIFFERENTIAL_EQN(sym, diff_args, p), i) =
	    (indent(i);
	     sayln("DIFFERENTIAL_EQN " ^ (name2str(sym)) ^ " with ARGS:");
	     print_diff_args(outstream, diff_args, i+1))


    in
	model(m); 
	sayln("");
	TextIO.flushOut outstream
    end

fun print_decl (outstream, decl:Ast.component_decl) =
    let
	val indent_factor = 1;

	val indent_factor = 2

	val say = say (outstream)
	val sayln = sayln (say)
	val ind = ind (say)
	val indent = indent (ind) (indent_factor)

	fun component_decl (Ast.SCALAR_STATE_DECL{quantity, init, 
						  method, description, pos }, i) =  

	    (indent(i);
	     sayln("SCALAR_STATE_DECL(");
	     indent(i+1);
	     sayln("quantity    = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln "init        = "; exp(init,i);
	     indent(i+1);
	     sayln("method      = " ^ (case method of
					   NONE           => "NONE"
					 | SOME Ast.MAU   => "MAU"
					 | SOME Ast.EULER => "EULER"));
	     indent(i+1);
	     sayln("description = " ^ (case description of
					   SOME s => s
					 | NONE   => ""));
	     indent(i);
	     sayln(")"))

	  | component_decl (Ast.VECTOR_STATE_DECL{quantity,
					      initval,
					      method,
					      description,
					      pos
					      }, i) =
	    (indent(i);
	     sayln("VECTOR_STATE_DECL(");
	     indent(i+1);
	     sayln("quantity    = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln("initval     = [" ^ (Array.foldl 
					   (fn (x,str) =>
					       str ^ (Real.toString(x)))
					   ""
					   initval)
		   ^ "]");
	     						       
	     indent(i+1);
	     sayln("method      = " ^ (case method of
					   NONE           => "NONE"
					 | SOME Ast.MAU   => "MAU"
					 | SOME Ast.EULER => "EULER"));
	     indent(i+1);
	     sayln("description = " ^ (case description of
					   SOME s => s
					 | NONE   => ""));
	     indent(i);
	     sayln(")"))


	  | component_decl(Ast.INT_STATE_DECL{quantity,
					      init,
					      description,
					      pos
					     },i) =
	    (indent(i);
	     sayln("INT_STATE_DECL(");
	     indent(i+1);
	     sayln("quantity = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln "init     = "; exp(init,i);
	     indent(i+1);
	     sayln("description = " ^ (case description of
					   SOME s => s
					| NONE   => ""));
	     indent(i);
	     sayln(")"))

	  | component_decl (Ast.SCALAR_PAR_DECL{quantity,
						init,
						description,
						pos
					       }, i) =
	    (indent(i);
	     sayln("SCALAR_PAR_DECL(");
	     indent(i+1);
	     sayln("quantity    = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln "init        = "; exp(init,i);
	     indent(i+1);
	     sayln("description = " ^ (case description of
					   SOME s => s
					 | NONE   => ""));
	     indent(i);
	     sayln(")"))

	  | component_decl (Ast.VECTOR_PAR_DECL{quantity,
						initval,
						description,
						pos
						}, i) =
	    (indent(i);
	     sayln("VECTOR_PAR_DECL(");
	     indent(i+1);
	     sayln("quantity    = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln("initval     = [" ^ (Array.foldl 
					   (fn (x,str) =>
					       str ^ (Real.toString(x)))
					   ""
					   initval)
		   ^ "]");
	     						       
	     indent(i+1);
	     sayln("description = " ^ (case description of
					   SOME s => s
					 | NONE   => ""));
	     indent(i);
	     sayln(")"))

	  | component_decl(Ast.SCALAR_STATE_FUNC_DECL{quantity,
					   description,
					   pos}, i) =
	    (indent(i);
	     sayln("SCALAR_STATE_FUNC_DECL (");
	     indent(i+1);
	     sayln("quantity    = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln("description = " ^ (case description of 
					   SOME s => s
					 | NONE   => ""));
	     indent(i);
	     sayln(")"))
	  | component_decl(Ast.VECTOR_STATE_FUNC_DECL{quantity,
					   description,
					   pos}, i) =
	    (indent(i);
	     sayln("VECTOR_STATE_FUNC_DECL (");
	     indent(i+1);
	     sayln("quantity    = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln("description = " ^ (case description of 
					   SOME s => s
					 | NONE   => ""));
	     indent(i);
	     sayln(")"))

	  | component_decl(Ast.TABLE_DECL{quantity,
					  parameters,
					  body,
					  low,
					  high,
					  step,
					  argument,
					  description,
					  pos}, i) =
	    (indent(i);
	     sayln("TABLE_DECL (");
	     indent(i+1);
	     sayln("quantity    = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln("argument   = " ^ (name2str(argument)));
	     indent(i+1);
	     sayln("low         = " ^ (Real.toString(low)));
	     indent(i+1);
	     sayln("high        = " ^ (Real.toString(high)));
	     indent(i+1);
	     sayln("step        = " ^ (Real.toString(step)));
	     indent(i+1);
	     sayln("description = " ^ (case description of 
					   SOME s => s
					 | NONE => ""));
	     indent(i+1);
	     sayln("body =");
	     exp(body, i+2))

	  | component_decl(Ast.C_ID_DECL{quantity,
					 pos}, i) =
	    (indent(i);
	     sayln("C_ID_DECL = " ^ (name2str(quantity))))

	  | component_decl(Ast.EXTERNAL_DECL{quantity,
					     direction,
					     channel,
					     description,
					     pos}, i) =
	    (indent(i);
	     sayln("EXTERNAL_DECL (");
	     indent(i+1);
	     sayln("quantity    = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln("direction   = " ^ (case direction of 
					   Ast.INPUT => "INPUT"
					 | Ast.OUTPUT => "OUTPUT"));
	     indent(i+1);
	     sayln("channel     = " ^ (case channel of 
					   NONE => "NONE"
					 | SOME n => (Int.toString n)));

	     indent(i+1);
	     sayln("description = " ^ (case description of 
					   SOME s => s
					 | NONE   => ""));
	     indent(i);
	     sayln(")"))
	    
	  | component_decl(Ast.DISCRETE_DECL{quantity,
					 set,
					 description,
					 pos}, i) =
	    (indent(i);
	     sayln("DISCRETE_DECL (");
	     indent(i+1);
	     sayln("quantity    = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln("set         = {" 
		   ^ (foldl (fn((sym, intopt):Ast.set_element, s) =>
			      case intopt of
				  SOME i => s ^ " " ^ (name2str(sym) ^
						       Int.toString(i))
				| NONE   => s ^ " " ^ (name2str(sym)))
			 ""
			 set) 
		   ^ "}");
	     indent(i+1);
	     sayln("description = " ^ (case description of 
					   SOME s => s
					 | NONE   => ""));
	     indent(i);
	     sayln(")"))

	and exp (Ast.CONSTINT(x), i) =
	    (indent(i);
	     sayln("CONSTINT = " ^ (Int.toString(x))))
	    
	  | exp (Ast.CONSTREAL(x), i) =
	    (indent(i);
	     sayln("CONSTREAL = " ^ (Real.toString(x))))
	    

	  | exp (Ast.ID({name, ...}), i) =
	    (indent(i);
	     sayln("ID = " ^ Symbol.name(name)))
	    
	  | exp (Ast.BINOP{oper,
		       left,
		       right,
		       pos}, i) =
	    (indent (i);
	     sayln("BINOP of " ^ binop2str(oper) ^ ": ");
	     indent (i+1);
	     sayln("LEFT = ");
	     exp(left, i+2);
	     indent (i+1);
	     sayln("RIGHT = ");
	     exp(right,i+2))

	  | exp (Ast.RELOP{oper,
			   left,
			   right,
			   pos}, i) = 
	    (indent (i);
	     sayln("RELOP of " ^ relop2str(oper) ^ ": ");
	     indent (i+1);
	     sayln("LEFT = ");
	     exp(left, i+2);
	     indent (i+1);
	     sayln("RIGHT = ");
	     exp(right,i+2))
	    
	  | exp (Ast.FUNCALL{function,
			 arguments,
			 pos}, i) =
	    (indent (i);
	     sayln("FUNCTION CALL " ^ (name2str(function)));
	     indent(i+1);
	     sayln("ARGUMENTS =");
	     app (fn(x) => 
		    exp(x,i+2))
		 arguments)
	     
	  | exp (Ast.UNARYEXPOP{oper,
				exp=e,
				pos}, i) =
	    (indent (i);
	     sayln("UNARY EXP OP = " 
		   ^ (case oper of 
			  Ast.UMINUS => "UMINUS"
			| Ast.UPLUS => "UPLUS"
			| Ast.NOT => "NOT"));
	     indent(i+1);
	     sayln("EXP = ");
	     exp(e,i+2))

	
	  | exp (Ast.CONDITIONAL{condition,
				 iftrue,
				 iffalse,
				 pos}, i) =
	    (indent(i);
	     sayln("CONDITIONAL EXP:");

	     indent(i+1);
	     sayln("CONDITION = ");
	     exp(condition, i+2);

	     indent(i+1);
	     sayln("IFTRUE = ");
	     exp(iftrue, i+2);

	     indent(i+1);
	     sayln("IFFALSE = ");
	     exp(iffalse, i+2))
	    

    in
	component_decl(decl, 1); 
	sayln("");
	TextIO.flushOut outstream
    end

fun print_ast_equation (outstream, top_equation:Ast.equation) =
    let
	val indent_factor = 2

	val say = say (outstream)
	val sayln = sayln (say)
	val ind = ind (say)
	val indent = indent (ind) (indent_factor)

	fun exp (Ast.CONSTINT(x), i) =
	    (indent(i);
	     sayln("CONSTINT = " ^ (Int.toString(x))))
	    
	  | exp (Ast.CONSTREAL(x), i) =
	    (indent(i);
	     sayln("CONSTREAL = " ^ (Real.toString(x))))
	    

	  | exp (Ast.ID({name, ...}), i) =
	    (indent(i);
	     sayln("ID = " ^ Symbol.name(name)))
	    
	  | exp (Ast.BINOP{oper,
		       left,
		       right,
		       pos}, i) =
	    (indent (i);
	     sayln("BINOP of " ^ binop2str(oper) ^ ": ");
	     indent (i+1);
	     sayln("LEFT = ");
	     exp(left, i+2);
	     indent (i+1);
	     sayln("RIGHT = ");
	     exp(right,i+2))

	  | exp (Ast.RELOP{oper,
			   left,
			   right,
			   pos}, i) = 
	    (indent (i);
	     sayln("RELOP of " ^ relop2str(oper) ^ ": ");
	     indent (i+1);
	     sayln("LEFT = ");
	     exp(left, i+2);
	     indent (i+1);
	     sayln("RIGHT = ");
	     exp(right,i+2))
	    
	  | exp (Ast.FUNCALL{function,
			 arguments,
			 pos}, i) =
	    (indent (i);
	     sayln("FUNCTION CALL " ^ (name2str(function)));
	     indent(i+1);
	     sayln("ARGUMENTS =");
	     app (fn(x) => 
		    exp(x,i+2))
		 arguments)
	     
	  | exp (Ast.UNARYEXPOP{oper,
				exp=e,
				pos}, i) =
	    (indent (i);
	     sayln("UNARY EXP OP = " 
		   ^ (case oper of 
			  Ast.UMINUS => "UMINUS"
			| Ast.UPLUS => "UPLUS"
			| Ast.NOT => "NOT"));
	     indent(i+1);
	     sayln("EXP = ");
	     exp(e,i+2))

	
	  | exp (Ast.CONDITIONAL{condition,
				 iftrue,
				 iffalse,
				 pos}, i) =
	    (indent(i);
	     sayln("CONDITIONAL EXP:");

	     indent(i+1);
	     sayln("CONDITION = ");
	     exp(condition, i+2);

	     indent(i+1);
	     sayln("IFTRUE = ");
	     exp(iftrue, i+2);

	     indent(i+1);
	     sayln("IFFALSE = ");
	     exp(iffalse, i+2))
	    

	and equation(Ast.DIFFERENCE_EQN(sym,e,p), i) =
	    (indent(i);
	     sayln("DIFFERENCE_EQN " ^ (name2str(sym)) ^ " = ");
	     exp(e,i+1))
	    
	  | equation(Ast.DISCRETE_EQN(sym, discexp, p), i) =
	    (indent(i);
	     sayln("DISCRETE_EQN " ^ (name2str(sym)) ^ " = ");
	     print_discrete_exp(outstream, discexp,i+1))

	  | equation(Ast.ALGEBRAIC_EQN(sym, e, pos), i) =
	    (indent(i);
	     sayln("ALGEBRAIC_EQN" ^ (name2str(sym)) ^ " = ");
	     exp(e, i+1))

	  | equation(Ast.DIFFERENTIAL_EQN(sym, diff_args, p), i) =
	    (indent(i);
	     sayln("DIFFERENTIAL_EQN " ^ (name2str(sym)) ^ " with ARGS:");
	     print_diff_args(outstream, diff_args, i+1))

    in
	equation(top_equation, 1); 
	sayln("");
	TextIO.flushOut outstream
    end

fun getcond condmap condid =
    (case condid of
	 SOME condid => UniqueId.look(condmap, condid)
       | NONE => NONE)

fun print_cond(outstream, {relop, quantity1, quantity2, parentid}, i) =

    let
	val indent_factor = 2
			    
	val say = say (outstream)
	val sayln = sayln (say)
	val ind = ind (say)
	val indent = indent (ind) (indent_factor)
    in
	(indent(i);
	 sayln("relop=" ^ relop2str(relop) ^ " " ^
	       "quantity1=" ^ (Symbol.name(quantity1)) ^ " " ^
	       "quantity2=" ^ (Symbol.name(quantity2)) ^
		   "parentid=" ^ (case parentid of 
							  SOME parentid => id2str parentid
							| NONE => "NONE")))
    end

fun print_env_equation (outstream, eq, i, condmap): unit =

let
    val indent_factor = 2
    val say = say (outstream)
    val sayln = sayln (say)
    val ind = ind (say)
    val indent = indent (ind) (indent_factor)

    val getcond = fn(condition) => valOf (getcond condmap (SOME condition))
in

    case eq of 
	AlgebraicEquation {line, exp} =>

	(indent (i);
	 sayln ("AlgebraicEquation: ");
	 indent (i+1);
	 sayln ("line=" ^ (Int.toString line));
	 indent (i+1);
	 sayln ("exp="); (print_exp (outstream, exp)))
		
	
      | DifferenceEquation {line, exp} =>
	(indent (i);
	 sayln ("DifferenceEquation: ");
	 indent (i+1);
	 sayln ("line=" ^ (Int.toString line));
	 indent (i+1);
	 sayln ("exp="); (print_exp (outstream, exp)))

      | DifferentialEquation {line, arg} =>
	(indent (i);
	 sayln ("DifferentialEquation: ");
	 indent (i+1);
	 sayln ("line=" ^ (Int.toString line));
	 indent (i+1);
	 sayln ("arg="); (print_diff_args (outstream, arg, i+1)))

					      
      | DiscreteEquation {line, exp} =>
	(indent (i);
	 sayln ("DiscreteEquation: ");
	 indent (i+1);
	 sayln ("line=" ^ (Int.toString line));
	 indent (i+1);
	 sayln ("exp="); (print_discrete_exp (outstream, exp, i+1)))

      | ConditionalBlock {line, condition, eq} =>
	(indent (i);
	 sayln ("ConditionalBlock: ");
	 indent (i+1);
	 sayln ("line=" ^ (Int.toString line));
	 indent (i+1);
	 sayln ("cond="); (app (fn(cond) => print_cond (outstream, cond, i+1)) (getcond condition));
	 indent (i+1);
	 sayln ("eqs="); print_env_equation (outstream, eq, i+1, condmap))

      | EventBlock {line, event, eq} =>
	(indent (i);
	 sayln ("EventBlock: ");
	 indent (i+1);
	 sayln ("line=" ^ (Int.toString line));
	 indent (i+1);
	 say ("event="); sayln (name2str event);
	 indent (i+1);
	 sayln ("eqs="); print_env_equation (outstream, eq, i+1, condmap))

end

fun print_env'_equation (outstream, eq, i): unit =

let
    val indent_factor = 2
    val say = say (outstream)
    val sayln = sayln (say)
    val ind = ind (say)
    val indent = indent (ind) (indent_factor)
in

    case eq of 
	DLEnv'.AlgebraicEquation {line, exp} =>

	(indent (i);
	 sayln ("AlgebraicEquation: ");
	 indent (i+1);
	 sayln ("line=" ^ (Int.toString line));
	 indent (i+1);
	 sayln ("exp="); (print_exp (outstream, exp)))
		
	
      | DLEnv'.DifferenceEquation {line, exp} =>
	(indent (i);
	 sayln ("DifferenceEquation: ");
	 indent (i+1);
	 sayln ("line=" ^ (Int.toString line));
	 indent (i+1);
	 sayln ("exp="); (print_exp (outstream, exp)))

      | DLEnv'.DifferentialEquation {line, arg} =>
	(indent (i);
	 sayln ("DifferentialEquation: ");
	 indent (i+1);
	 sayln ("line=" ^ (Int.toString line));
	 indent (i+1);
	 sayln ("arg="); (print_diff_args (outstream, arg, i+1)))

					      
      | DLEnv'.DiscreteEquation {line, exp} =>
	(indent (i);
	 sayln ("DiscreteEquation: ");
	 indent (i+1);
	 sayln ("line=" ^ (Int.toString line));
	 indent (i+1);
	 sayln ("exp="); (print_discrete_exp (outstream, exp, i+1)))


end

fun print_typeinfo (outstream, typeinfo, i) =
    let
	val indent_factor = 2

	val say = say (outstream)
	val sayln = sayln (say)
	val ind = ind (say)
	val indent = indent (ind) (indent_factor)

	val {args, ret_type, pos} = typeinfo

	val print_arg = 
	    (fn({name, typ}) =>
	       (indent (i+1);
		sayln ("argument    = " ^ (name2str (name)) ^ ": "  ^
		       (DLTypes.typeSetToString (typ)))))
    in
	app print_arg args; 
	indent (i+1);
	sayln ("return type = " ^ (DLTypes.typeSetToString (ret_type)))
    end

fun print_typeinfo' (outstream, typeinfo, i) =
    let
	val indent_factor = 2

	val say = say (outstream)
	val sayln = sayln (say)
	val ind = ind (say)
	val indent = indent (ind) (indent_factor)

	val {args, ret_type} = typeinfo

	val print_arg = 
	    (fn({name, typ}) =>
	       (indent (i+1);
		sayln ("argument    = " ^ (name'2str (name)) ^ ": "  ^
		       (DLTypes.typeSetToString (typ)))))
    in
	app print_arg args; 
	indent (i+1);
	sayln ("return type = " ^ (DLTypes.typeSetToString (ret_type)))
    end
    

fun print_enventry (outstream, entry, i, condmap) = 
    let
	val indent_factor = 2

	val say = say (outstream)
	val sayln = sayln (say)
	val ind = ind (say)
	val indent = indent (ind) (indent_factor)
    in
	case entry of
	    
	    EventEntry {name, arguments} => sayln ("EventEntry " ^ (name2str name))
	  | EventArgument {name, event} => sayln ("EventArgument " ^ (name2str name))

	  | TimeEntry {quantity, initval, eq} => sayln ("TimeEntry " ^ (name2str quantity))

	  | ScalarParEntry {quantity, value, description} =>
	    (sayln ("ScalarParEntry: ");
	     indent(i+1);
	     sayln("quantity    = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln("value       = " ^ (Real.toString(value)));
	     indent(i+1);
	     case description of 
		 SOME description => sayln("description = " ^ description)
	       | NONE => ())

	  | VectorParEntry {quantity, value, description} =>
	    (sayln ("VectorParEntry: ");
	     indent(i+1);
	     sayln("quantity    = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln("value       = [" ^ 
		   (Array.foldl (fn (x,str) => str ^ (Real.toString(x)) ^ " ")
				"" value) ^ "]");
	     indent(i+1);
	     case description of 
		 SOME description => sayln("description = " ^ description)
	       | NONE => ())

	  | ScalarStateEntry {quantity, initval, method, eqs,
			      description} =>
	    (sayln ("ScalarStateEntry: ");
	     indent(i+1);
	     sayln("quantity    = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln("initval     = " ^ (Real.toString(initval)));
	     indent(i+1);
	     sayln("method      = " ^ (case method of
					   NONE           => "NONE"
					 | SOME Ast.MAU   => "MAU"
					 | SOME Ast.EULER => "EULER"));
	     indent(i+1);
	     sayln("equations:     " );
	     app (fn(eq)=>print_env_equation (outstream, eq, i+1, condmap)) eqs;
	     case description of 
		 SOME description => sayln("description = " ^ description)
	       | NONE => ())

	  | VectorStateEntry {quantity, initval, method, eqs,
			      description} =>
	    (sayln ("VectorStateEntry: ");
	     indent(i+1);
	     sayln("quantity    = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln("initval       = [" ^ 
		   (Array.foldl (fn (x,str) => str ^ (Real.toString(x)) ^ " ")
				"" initval) ^ "]");
	     indent(i+1);
	     sayln("method      = " ^ (case method of
					   NONE           => "NONE"
					 | SOME Ast.MAU   => "MAU"
					 | SOME Ast.EULER => "EULER"));
	     indent(i+1);
	     sayln("equations:     " );
	     app (fn(eq)=>print_env_equation (outstream, eq, i+1, condmap)) eqs;
	     case description of 
		 SOME description => sayln("description = " ^ description)
	       | NONE => ())

	  | IntegerStateEntry {quantity, initval, eqs, description} =>
	    (sayln ("IntegerStateEntry: ");
	     indent(i+1);
	     sayln("quantity    = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln("initval     = " ^ (Int.toString(initval)));
	     indent(i+1);
	     sayln("equations:     " );
	     app (fn(eq)=>print_env_equation (outstream, eq, i+1, condmap)) eqs;
	     case description of 
		 SOME description => sayln("description = " ^ description)
	       | NONE => ())

	  | ExternalStateEntry {quantity, direction, channel, eqs, description} =>
	    (sayln ("ExternalStateEntry: ");
	     indent(i+1);
	     sayln("quantity    = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln("direction   = " ^ (case direction of 
					   Ast.INPUT => "in"
					 | Ast.OUTPUT => "out"));
	     sayln("channel   = " ^ (case channel of 
					   NONE => "NONE"
					 | SOME n => (Int.toString n)));
	     indent(i+1);
	     sayln("equations:     " );
	     (case eqs of 
		  SOME eqs => app (fn(eq)=>print_env_equation (outstream, eq, i+1, condmap)) eqs
		| NONE => ());
	     case description of 
		 SOME description => sayln("description = " ^ description)
	       | NONE => ())

	  | DiscreteStateEntry {quantity, set, eqs, description} =>
	    (sayln ("DiscreteStateEntry: ");
	     indent(i+1);
	     sayln("quantity    = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln("set         = ");
	     printset (outstream, set, i+1);
	     indent(i+1);
	     sayln("equations:     " );
	     app (fn(eq)=>print_env_equation (outstream, eq, i+1, condmap)) eqs;
	     case description of 
		 SOME description => sayln("description = " ^ description)
	       | NONE => ())
	    

	  | ScalarStateFunEntry {quantity, eqs, description} =>
	    (sayln ("ScalarStateFunEntry: ");
	     indent(i+1);
	     sayln("quantity    = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln("equations:     " );
	     app (fn(eq)=>print_env_equation (outstream, eq, i+1, condmap)) eqs;
	     case description of 
		 SOME description => sayln("description = " ^ description)
	       | NONE => ())

	  | VectorStateFunEntry {quantity, eqs, size, description} =>
	    (sayln ("VectorStateFunEntry: ");
	     indent(i+1);
	     sayln("quantity    = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln("size        = " ^ (case size of 
					   SOME size => Int.toString(size)
					 | NONE => "NONE"));
	     indent(i+1);
	     sayln("equations:     " );
	     app (fn(eq)=>print_env_equation (outstream, eq, i+1, condmap)) eqs;
	     case description of 
		 SOME description => sayln("description = " ^ description)
	       | NONE => ())

	  | TableFunEntry {quantity, body, low, high, step, argument,
			   description} =>
	    (sayln ("TableFunEntry: ");
	     indent(i+1);
	     sayln("quantity    = " ^ (name2str(quantity)));
	     indent(i+1);
	     sayln ("body       ="); (print_exp (outstream, body));
	     indent(i+1);
	     sayln ("low=" ^ (Real.toString low));
	     indent(i+1);
	     sayln ("high=" ^ (Real.toString high));
	     indent(i+1);
	     sayln ("step=" ^ (Real.toString step));
	     indent(i+1);
	     sayln ("argument=" ^ (name2str argument));
	     
	     case description of 
		 SOME description => (indent(i+1); sayln("description = " ^ description))
	       | NONE => ())

	  | FunAlias {name, entryname, typeinfo} =>
	    (sayln ("FunAlias: ");
	     indent(i+1);
	     sayln("name    = " ^ (name2str(name)));
	     indent(i+1);
	     sayln("entryname = " ^ (name2str(entryname)));
	     indent(i+1);
	     sayln ("typeinfo="); (print_typeinfo (outstream, typeinfo, i)))

	  | FunEntry {name, body, typeinfo, calls, callhistory} =>
	    (sayln ("FunEntry: ");
	     indent(i+1);
	     sayln("name    = " ^ (name2str(name)));
	     indent(i+1);
	     sayln ("body   ="); (print_exp (outstream, body));
	     indent(i+1);
	     sayln ("typeinfo="); (print_typeinfo (outstream, typeinfo, i));
	     sayln ("calls=" ^ (Int.toString (!calls))))

	  | BuiltinFunEntry {name, typeinfo, callhistory} =>
	    (sayln ("BuiltinFunEntry: ");
	     indent(i+1);
	     sayln("name     = " ^ (name2str(name)));
	     indent(i+1);
	     sayln("typeinfo ="); (print_typeinfo (outstream, typeinfo, i+1)))

	  | BuiltinScalar {name} =>
	    (sayln ("BuiltinScalar: ");
	     indent(i+1);
	     sayln("name     = " ^ (name2str(name))))

	  | ForeignScalar {name} =>
	    (sayln ("ForeignScalar: ");
	     indent(i+1);
	     sayln("name     = " ^ (name2str(name))))

	  | Dummy => (sayln ("Dummy"))

	  | SetEntry {name, value} =>
	    (sayln ("Set Entry: ");
	     indent(i + 1);
	     sayln("name = " ^ (name2str name));
	     indent(i + 1);
	     sayln("value = " ^ (Int.toString value)))
    end

fun print_env (outstream, env, condmap) =

    let
	val enventryList = Symbol.listItems(env)
    in
	app (fn(enventry) => print_enventry(outstream, enventry, 0, condmap)) enventryList
    end


fun strof_condition(outstream, condition:condition list option, i) =
    let
	fun cond2str({relop, quantity1, quantity2, parentid}, str) =
	    str ^ 
	    (relop2str(relop)) ^ 
	    " " ^ 
	    (Symbol.name quantity1) ^
	    " " ^
	    (Symbol.name quantity2) ^
	    ", "

    in
	case condition of
	    SOME condition => foldl cond2str "" condition
	  | NONE => "NONE"
    end

fun print_enventry' (outstream, entry:DLEnv'.enventry', i, condmap) = 
    let
	val indent_factor = 2

	val say = say (outstream)
	val sayln = sayln (say)
	val ind = ind (say)
	val indent = indent (ind) (indent_factor)
		     
	val getcond = getcond condmap
    in
	case entry of
	    DLEnv'.EventEntry {name, arguments} => sayln ("EventEntry " ^ (name'2str name))
	  | DLEnv'.EventArgument {name, event} => sayln ("EventArgument " ^ (name'2str name))
	    
	  | DLEnv'.TimeEntry ({quantity, ...}) => sayln ("TimeEntry " ^ (name'2str quantity))

	  | DLEnv'.ScalarParEntry {quantity, value, description} =>
	    (sayln ("ScalarParEntry: ");
	     indent(i+1);
	     sayln("quantity    = " ^ (name'2str(quantity)));
	     indent(i+1);
	     sayln("value       = " ^ (Real.toString(value)));
	     indent(i+1);
	     case description of 
		 SOME description => sayln("description = " ^ description)
	       | NONE => ())

	  | DLEnv'.VectorParEntry {quantity, value, description} =>
	    (sayln ("VectorParEntry: ");
	     indent(i+1);
	     sayln("quantity    = " ^ (name'2str(quantity)));
	     indent(i+1);
	     sayln("value       = [" ^ 
		   (Array.foldl (fn (x,str) => str ^ (Real.toString(x)) ^ " ")
				"" value) ^ "]");
	     indent(i+1);
	     case description of 
		 SOME description => sayln("description = " ^ description)
	       | NONE => ())

	  | DLEnv'.ScalarStateEntry {quantity, initval, method, 
				     condition, event, 
				     eq, description} =>
	    (sayln ("ScalarStateEntry: ");
	     indent(i+1);
	     sayln("quantity    = " ^ (name'2str(quantity)));
	     indent(i+1);
	     sayln("initval     = " ^ (Real.toString(initval)));
	     indent(i+1);
	     sayln("method      = " ^ (case method of
					   NONE           => "NONE"
					 | SOME Ast.MAU   => "MAU"
					 | SOME Ast.EULER => "EULER"));
	     indent(i+1);
	     sayln("condition = " ^ strof_condition(outstream, getcond(condition), i+1));
	     indent(i+1);
	     sayln("equation:     " );
	     print_env'_equation (outstream, eq, i+1);
	     case description of 
		 SOME description => sayln("description = " ^ description)
	       | NONE => ())

	  | DLEnv'.VectorStateEntry {quantity, initval, method, 
				     condition, event, 
				     eq, description} =>
	    (sayln ("VectorStateEntry: ");
	     indent(i+1);
	     sayln("quantity    = " ^ (name'2str(quantity)));
	     indent(i+1);
	     sayln("initval     = [" ^ 
		   (Array.foldl (fn (x,str) => str ^ (Real.toString(x)) ^ " ")
				"" initval) ^ "]");
	     indent(i+1);
	     sayln("method      = " ^ (case method of
					   NONE           => "NONE"
					 | SOME Ast.MAU   => "MAU"
					 | SOME Ast.EULER => "EULER"));
	     indent(i+1);
	     sayln("condition = " ^ strof_condition(outstream, getcond condition, i+1));
	     indent(i+1);
	     sayln("equation:     " );
	     print_env'_equation (outstream, eq, i+1);
	     case description of 
		 SOME description => sayln("description = " ^ description)
	       | NONE => ())

	  | DLEnv'.IntegerStateEntry {quantity, initval, 
				      condition, event, 
				      eq, description} =>
	    (sayln ("IntegerStateEntry: ");
	     indent(i+1);
	     sayln("quantity    = " ^ (name'2str(quantity)));
	     indent(i+1);
	     sayln("initval     = " ^ (Int.toString(initval)));
	     indent(i+1);
	     sayln("condition = " ^ strof_condition(outstream, getcond condition, i+1));
	     indent(i+1);
	     sayln("equation:     " );
	     print_env'_equation (outstream, eq, i+1);
	     case description of 
		 SOME description => sayln("description = " ^ description)
	       | NONE => ())

	  | DLEnv'.ExternalStateEntry {quantity, direction, channel, 
				       condition, event,
				       eq, description} =>
	    (sayln ("ExternalStateEntry: ");
	     indent(i+1);
	     sayln("quantity    = " ^ (name'2str(quantity)));
	     indent(i+1);
	     sayln("direction   = " ^ (case direction of 
					   Ast.INPUT => "in"
					 | Ast.OUTPUT => "out"));
	     sayln("channel   = " ^  (Int.toString channel));
	     indent(i+1);
	     sayln("condition = " ^ strof_condition(outstream, getcond condition, i+1));
	     indent(i+1);
	     sayln("equations:     " );
	     (case eq of 
		  SOME eq => print_env'_equation (outstream, eq, i+1)
		| NONE => ());
	     case description of 
		 SOME description => sayln("description = " ^ description)
	       | NONE => ())

	  | DLEnv'.DiscreteStateEntry {quantity, set, 
				       condition, event, 
				       eq, description} =>
	    (sayln ("DiscreteStateEntry: ");
	     indent(i+1);
	     sayln("quantity    = " ^ (name'2str(quantity)));
	     indent(i+1);
	     sayln("set         = ");
	     printset' (outstream, set, i+1);
	     indent(i+1);
	     sayln("condition = " ^ strof_condition(outstream, getcond condition, i+1));
	     indent(i+1);
	     sayln("equation:     " );
	     print_env'_equation (outstream, eq, i+1);
	     case description of 
		 SOME description => sayln("description = " ^ description)
	       | NONE => ())
	    

	  | DLEnv'.ScalarStateFunEntry {quantity, condition, event, 
					eq, description} =>
	    (sayln ("ScalarStateFunEntry: ");
	     indent(i+1);
	     sayln("quantity    = " ^ (name'2str(quantity)));
	     indent(i+1);
	     sayln("condition   = " ^ strof_condition(outstream, getcond condition, i+1));
	     indent(i+1);
	     sayln("equation:     " );
	     print_env'_equation (outstream, eq, i+1);
	     case description of 
		 SOME description => sayln("description = " ^ description)
	       | NONE => ())

	  | DLEnv'.VectorStateFunEntry {quantity, 
					condition, event, 
					eq, size, description} =>
	    (sayln ("VectorStateFunEntry: ");
	     indent(i+1);
	     sayln("quantity    = " ^ (name'2str(quantity)));
	     indent(i+1);
	     sayln("condition   = " ^ strof_condition(outstream, getcond condition, i+1));
	     indent(i+1);
	     sayln("size        = " ^ (Int.toString(size)));
	     indent(i+1);
	     sayln("equations     " );
	     print_env'_equation (outstream, eq, i+1);
	     case description of 
		 SOME description => sayln("description = " ^ description)
	       | NONE => ())

	  | DLEnv'.TableFunEntry {quantity, body, low, high, step, argument,
				 description} =>
	    (sayln ("TableFunEntry: ");
	     indent(i+1);
	     sayln("quantity    = " ^ (name'2str(quantity)));
	     indent(i+1);
	     sayln ("body       ="); (print_exp (outstream, body));
	     indent(i+1);
	     sayln ("low=" ^ (Real.toString low));
	     indent(i+1);
	     sayln ("high=" ^ (Real.toString high));
	     indent(i+1);
	     sayln ("step=" ^ (Real.toString step));
	     indent(i+1);
	     sayln ("argument=" ^ (name'2str argument));
	     
	     case description of 
		 SOME description => (indent(i+1); sayln("description = " ^ description))
	       | NONE => ())

	  | DLEnv'.FunEntry {name, body, typeinfo} =>
	    (sayln ("FunEntry: ");
	     indent(i+1);
	     sayln("name    = " ^ (name'2str(name)));
	     indent(i+1);
	     sayln ("body   ="); (print_exp (outstream, body));
	     indent(i+1);
	     sayln ("typeinfo="); (print_typeinfo' (outstream, typeinfo, i)))

	  | DLEnv'.FunAlias {name, entryname, typeinfo} =>
	    (sayln ("FunAlias: ");
	     indent(i+1);
	     sayln("name    = " ^ (name'2str(name)));
	     indent(i+1);
	     sayln ("entryname   ="); (name'2str(entryname));
	     indent(i+1);
	     sayln ("typeinfo="); (print_typeinfo' (outstream, typeinfo, i)))

	  | DLEnv'.BuiltinFunEntry {name, typeinfo} =>
	    (sayln ("BuiltinFunEntry: ");
	     indent(i+1);
	     sayln("name     = " ^ (name'2str(name)));
	     indent(i+1);
	     sayln("typeinfo ="); (print_typeinfo' (outstream, typeinfo, i+1)))

	  | DLEnv'.BuiltinScalar {name} =>
	    (sayln ("ForeignScalar: ");
	     indent(i+1);
	     sayln("name     = " ^ (name'2str(name))))

	  | DLEnv'.ForeignScalar {name} =>
	    (sayln ("ForeignScalar: ");
	     indent(i+1);
	     sayln("name     = " ^ (name'2str(name))))

	  | DLEnv'.SetEntry {name, value} =>
	    (sayln ("Set Entry: ");
	     indent(i + 1);
	     sayln("name = " ^ (name'2str name));
	     indent(i+1);
	     sayln("value    = " ^ (Int.toString(value))))

	  | DLEnv'.Dummy => (sayln ("Dummy"))
    end


    fun print_env' (outstream, env':DLEnv'.env', condmap) =
	let
	    val enventryList = Symbol.listItems env'
	in
	    app (fn(enventry) => print_enventry'(outstream, enventry, 0, condmap)) enventryList
	end    

    fun print_depcondition (outstream, depcond, i) =
    let
	val indent_factor = 2

	val say = say (outstream)
	val sayln = sayln (say)
	val ind = ind (say)
	val indent = indent (ind) (indent_factor)
		     
	val {relop, quantity1, quantity2} = depcond
    in
	indent(i+1);
	sayln ((depname2str quantity1) ^ " " ^ 
	       (relop2str relop) ^ " " ^
	       (depname2str quantity2))
    end




    fun print_depexp (outstream, exp: DLDeps.depexp, i) =
	let
	    val indent_factor = 2
	    val say = say (outstream)
	    val sayln = sayln (say)
	    val ind = ind (say)
	    val indent = indent (ind) (indent_factor)
	in
	    indent (i);
	    case exp of 
		DLDeps.CONSTINT n => sayln ("CONSTINT " ^ (Int.toString n))
	      | DLDeps.CONSTREAL n => sayln ("CONSTREAL " ^ (Real.toString n))
	      | DLDeps.ID n  => sayln ("ID " ^ (depname2str n))
	      | DLDeps.BINOP {oper, left, right} => 
		(sayln ("BINOP " ^ (binop2str oper));
		 indent(i+1);
		 sayln ("LEFT "); print_depexp (outstream, left, i+2);
		 indent(i+1);
		 sayln ("RIGHT "); print_depexp (outstream, right, i+2))
	      | DLDeps.RELOP {oper, left, right} =>
		(sayln ("RELOP " ^ (relop2str oper));
		 indent(i+1);
		 sayln ("LEFT "); print_depexp (outstream, left, i+2);
		 indent(i+1);
		 sayln ("RIGHT "); print_depexp (outstream, right, i+2))
	      | DLDeps.FUNCALL {function, arguments} =>
		(sayln("FUNCTION CALL " ^ (depname2str function));
		 indent(i+1);
		 sayln("ARGUMENTS ");
		 app (fn(x) => print_depexp(outstream, x, i+2)) arguments)
	      | DLDeps.UNARYEXPOP {oper, exp} =>
		(sayln ("UNARYEXPOP " ^ (unaryop2str oper));
		 print_depexp(outstream, exp, i+2))
	      | DLDeps.CONDITIONAL {condition, iftrue, iffalse} =>
		(sayln ("CONDITIONAL ");
		 indent(i+1);
		 sayln ("CONDITION "); print_depexp (outstream, condition, i+2);
		 indent(i+1);
		 sayln ("IFTRUE "); print_depexp (outstream, iftrue, i+2);
		 indent(i+1);
		 sayln ("IFFALSE "); print_depexp (outstream, iffalse, i+2))
		
	end

    fun print_indepexp (outstream, exp, i) =
	let
	    val indent_factor = 2
	    val say = say (outstream)
	    val sayln = sayln (say)
	    val ind = ind (say)
	    val indent = indent (ind) (indent_factor)
	in
	    indent (i);
	    case exp of 
		DLInDeps.CONSTINT n => sayln ("CONSTINT " ^ (Int.toString n))
	      | DLInDeps.CONSTREAL n => sayln ("CONSTREAL " ^ (Real.toString n))
	      | DLInDeps.ID n  => sayln ("ID " ^ (indepname2str n))
	      | DLInDeps.BINOP {oper, left, right} => 
		(sayln ("BINOP " ^ (binop2str oper));
		 indent(i+1);
		 say ("LEFT "); print_indepexp (outstream, left, i+2);
		 indent(i+1);
		 say ("RIGHT "); print_indepexp (outstream, right, i+2))
	      | DLInDeps.RELOP {oper, left, right} =>
		(sayln ("RELOP " ^ (relop2str oper));
		 indent(i+1);
		 sayln ("LEFT "); print_indepexp (outstream, left, i+2);
		 indent(i+1);
		 sayln ("RIGHT "); print_indepexp (outstream, right, i+2))
	      | DLInDeps.FUNCALL {function, arguments} =>
		(sayln("FUNCTION CALL " ^ (indepname2str function));
		 indent(i+1);
		 sayln("ARGUMENTS ");
		 app (fn(x) => print_indepexp(outstream, x, i+2)) arguments)
	      | DLInDeps.UNARYEXPOP {oper, exp} =>
		(sayln ("UNARYEXPOP " ^ (unaryop2str oper));
		 indent(i+1);
		 print_indepexp(outstream, exp, i+2))
	      | DLInDeps.CONDITIONAL {condition, iftrue, iffalse} =>
		(sayln ("CONDITIONAL ");
		 indent(i+1);
		 sayln ("CONDITION "); print_indepexp (outstream, condition, i+2);
		 indent(i+1);
		 sayln ("IFTRUE "); print_indepexp (outstream, iftrue, i+2);
		 indent(i+1);
		 sayln ("IFFALSE "); print_indepexp (outstream, iffalse, i+2))
		
	end

    fun print_depdiscrete_exp (outstream, exp, i) =
	let
	    val indent_factor = 2
	    val say = say (outstream)
	    val sayln = sayln (say)
	    val ind = ind (say)
	    val indent = indent (ind) (indent_factor)
	in
	    indent (i);
	    case exp of 
		DLDeps.DISCRETE_ID n => sayln ("DISCRETE_ID " ^ (depname2str n)) 
	      | DLDeps.DISCRETE_CASE clauses =>
		app (fn(clause) => 
		       case clause of 
			   DLDeps.DISCRETE_COND (exp,name) => 
			   (sayln "DISCRETE_COND";
			    print_depexp (outstream, exp, i+1);
			    indent (i+1);
			    sayln ("=> " ^ (depname2str name)))
			 | DLDeps.DEFAULT_COND name =>
			   (sayln ("DEFAULT_COND " ^ (depname2str name))))
		    clauses
	end

    fun print_deparg (outstream, arg, i) =
	let
	    val indent_factor = 2
	    val say = say (outstream)
	    val sayln = sayln (say)
	    val ind = ind (say)
	    val indent = indent (ind) (indent_factor)
	in
	    indent (i);
	    case arg of 
	     DLDeps.EULER_ARGS exp => (sayln ("EULER_ARGS "); 
				       print_depexp (outstream, exp, i+1))
	   | DLDeps.MAU_ARGS (exp1, exp2) => (say ("MAU_ARGS "); 
					      print_depexp (outstream, exp1, i+1);
					      sayln ", ";
					      print_depexp (outstream, exp2, i+1))
	end


    fun print_depenv_equation (outstream, eq, i): unit =
	
	let
	    val indent_factor = 2
	    val say = say (outstream)
	    val sayln = sayln (say)
	    val ind = ind (say)
	    val indent = indent (ind) (indent_factor)
	in

	    case eq of 
		DLDeps.AlgebraicEquation {line, exp, deps} =>
		(indent (i);
		 sayln ("AlgebraicEquation: ");
		 indent (i+1);
		 sayln ("line=" ^ (Int.toString line));
		 indent (i+1);
		 sayln ("exp="); (print_depexp (outstream, exp, i));
		 indent (i+1);
		 sayln ("dependencies=");
		 indent (i+1);
		 sayln (foldl (fn(d,s) => 
				 (if s = "" then "" else s^",") ^
				 (depname2str d)) "" deps))
		
		
	      | DLDeps.DifferenceEquation {line, exp, deps} =>
		(indent (i);
		 sayln ("DifferenceEquation: ");
		 indent (i+1);
		 sayln ("line=" ^ (Int.toString line));
		 indent (i+1);
		 sayln ("exp="); (print_depexp (outstream, exp, i));
		 indent (i+1);
		 sayln ("dependencies=");
		 indent (i+1);
		 sayln (foldl (fn(d,s) => 
				 (if s = "" then "" else s^",") ^
				 (depname2str d)) "" deps))

	      | DLDeps.DifferentialEquation {line, arg, deps} =>
		(indent (i);
		 sayln ("DifferentialEquation: ");
		 indent (i+1);
		 sayln ("line=" ^ (Int.toString line));
		 indent (i+1);
		 sayln ("arg="); (print_deparg (outstream, arg, i+1));
		 indent (i+1);
		 sayln ("dependencies=");
		 indent (i+1);
		 sayln (foldl (fn(d,s) => 
				 (if s = "" then "" else s^",") ^
				 (depname2str d)) "" deps))

		
	      | DLDeps.DiscreteEquation {line, exp, deps} =>
		(indent (i);
		 sayln ("DiscreteEquation: ");
		 indent (i+1);
		 sayln ("line=" ^ (Int.toString line));
		 indent (i+1);
		 sayln ("exp="); (print_depdiscrete_exp (outstream, exp, i+1));
		 indent (i+1);
		 sayln ("dependencies=");
		 indent (i+1);
		 sayln (foldl (fn(d,s) => 
				 (if s = "" then "" else s^",") ^
				 (depname2str d)) "" deps))



	end


exception InvalidEntry

    fun print_depenventry env (outstream, entry, i) = 
	let
	    val indent_factor = 2

	    val say = say (outstream)
	    val sayln = sayln (say)
	    val ind = ind (say)
	    val indent = indent (ind) (indent_factor)

	    fun getcond (condition) = 
		let
		    val condentry = valOf (UniqueId.look (env, condition))
		in
		    case condentry of 
			DLDeps.ConditionEntry {condlist, ...}  =>
			condlist
		      | _ => raise InvalidEntry
		end
			 
	in
	    case entry of

		DLDeps.ConditionEntry {id, condlist, deps} =>
		(indent(i);
		 sayln ("ConditionEntry: ");
		 indent(i+1);
		 sayln("id    = " ^ (id2str id));
		 indent(i+1);
		 sayln ("conditions:");
		 app (fn(cond) => print_depcondition (outstream, cond, i+1)) condlist;
		 indent(i+1);
		 say ("dependencies:");
		 sayln (foldl (fn(d,s) => s^", "^(depname2str d)) "" deps))

	      | DLDeps.EventEntry {name, arguments}  =>
		(indent(i);
		 sayln (String.concat
			    (["EventEntry: ",
			      (depname2str name),
			      "("] @ (foldr (fn(d,s) => " " :: (depname2str d) :: s) [] arguments) @ [")"])))

	      | DLDeps.TimeEntry {quantity, initval, eq}  =>
		(indent(i);
		 sayln ("TimeEntry: " ^ (depname2str quantity)))
		
	      | DLDeps.ScalarParEntry {quantity, value, description} =>
		(indent(i);
		 sayln ("ScalarParEntry: " ^ (depname2str quantity));
		 indent(i+1);
		 sayln("value    = " ^ (Real.toString value));
		 (case description of 
		      SOME description  => (indent (i+1);
					    sayln ("description: " ^ description))
		    | NONE => ()))
		
	      | DLDeps.VectorParEntry {quantity, value, description} =>
		(indent(i);
		 sayln ("VectorParEntry: " ^ (depname2str quantity));
		 indent(i+1);

		 sayln("value = [" ^ (Array.foldl (fn (x,str) => str ^ (Real.toString(x)) ^ " ") "" 
						  value) ^ "]");
		 (case description of 
		      SOME description  => (indent (i+1);  
					    sayln ("description: " ^ description))
		    | NONE => ()))

	      | DLDeps.ScalarStateEntry {quantity, initval, method, 
					 condition, event, 
					 eq, description, ...} =>
		(indent (i);
		 sayln ("ScalarStateEntry: " ^ (depname2str quantity));
		 indent(i+1);
		 sayln("initval     = " ^ (Real.toString(initval)));
		 indent(i+1);
		 sayln("method      = " ^ (case method of
					       NONE           => "NONE"
					     | SOME Ast.MAU   => "MAU"
					     | SOME Ast.EULER => "EULER"));
		 (case condition of 
		      SOME condition =>
		      (indent(i+1);
		       sayln("condition = ");
		       app (fn(cond) => print_depcondition (outstream, cond, i+2)) 
			   (getcond condition))
		    | NONE => ());
		 indent(i+1);
		 sayln("equation:     " );
		 print_depenv_equation (outstream, eq, i+2);
		 case description of 
		     SOME description => (indent(i+1);
					  sayln("description = " ^ description))
		   | NONE => ())

	      | DLDeps.VectorStateEntry {quantity, initval, method, 
					 condition, event, 
					 eq, description, ...} =>
		(indent (i);
		 sayln ("VectorStateEntry: " ^ (depname2str quantity));
		 indent(i+1);
		 sayln("initval = [" ^ (Array.foldl (fn (x,str) => str ^ (Real.toString(x))) ""
						    initval) ^ "]");
		 indent(i+1);
		 sayln("method      = " ^ (case method of
					       NONE           => "NONE"
					     | SOME Ast.MAU   => "MAU"
					     | SOME Ast.EULER => "EULER"));
		 
		 (case condition of 
		      SOME condition =>
		      (indent(i+1);
		       sayln("condition: ");
		       app (fn(cond) => print_depcondition (outstream, cond, i+2)) 
			   (getcond condition))
		    | NONE => ());
		 indent(i+1);
		 sayln("equation:     " );
		 print_depenv_equation (outstream, eq, i+2);
		 case description of 
		     SOME description => (indent (i+1);
					  sayln("description = " ^ description))
		   | NONE => ())

	      | DLDeps.IntegerStateEntry {quantity, initval, 
					  condition, event, 
					  eq, description, ...} =>
		(indent (i);
		 sayln ("IntegerStateEntry: " ^ (depname2str quantity));
		 indent(i+1);
		 sayln("initval     = " ^ (Int.toString(initval)));
		 (case condition of 
		      SOME condition =>
		      (indent(i+1);
		       sayln("condition: ");
		       app (fn(cond) => print_depcondition (outstream, cond, i+2)) 
			   (getcond condition))
		      | NONE => ());
		 indent(i+1);
		 sayln("equation:     " );
		 print_depenv_equation (outstream, eq, i+2);
		 case description of 
		     SOME description => (indent (i+1);
					  sayln("description = " ^ description))
		   | NONE => ())

	      | DLDeps.ExternalStateEntry {quantity, direction, channel, 
					   condition, event, 
					   eq, description} =>
		(indent (i);
		 sayln ("ExternalStateEntry: " ^ (depname2str quantity));
		 indent(i+1);
		 sayln("direction   = " ^ (case direction of 
					       Ast.INPUT => "in"
					     | Ast.OUTPUT => "out"));
		 sayln("channel   = " ^ (Int.toString channel));

		 (case condition of 
		      SOME condition =>
		      (indent(i+1);
		       sayln("condition: ");
		       app (fn(cond) => print_depcondition (outstream, cond, i+2)) 
			   (getcond condition))
		    | NONE => ());
		 (case eq of 
		     SOME eq =>
		     (indent (i+1);
		      sayln("equation:     " );
		      print_depenv_equation (outstream, eq, i+2))
		   | NONE => ());
		 case description of 
		     SOME description => (indent (i+1);
					  sayln("description = " ^ description))
		   | NONE => ())

	      | DLDeps.DiscreteStateEntry {quantity, set, 
					   condition, event,
					   eq, description, ...} =>
		(indent(i);
		 sayln ("DiscreteStateEntry: " ^ (depname2str quantity));
		 indent(i+1);
		 sayln("set         = ");
		 printset' (outstream, set, i+1);
		 (case condition of 
		      SOME condition =>
		      (indent(i+1);
		       sayln("condition: ");
		       app (fn(cond) => print_depcondition (outstream, cond, i+2)) 
			   (getcond condition))
		    | NONE => ());
		 indent(i+1);
		 sayln("equation:     " );
		 print_depenv_equation (outstream, eq, i+2);
		 case description of 
		     SOME description => (indent(i+1);
					  sayln("description: " ^ description))
		   | NONE => ())

	      | DLDeps.VectorStateFunEntry {quantity, condition, event, 
					    eq, size, description, ...} =>
		(indent (i);
		 sayln ("VectorStateFunEntry: " ^ (depname2str quantity));
		 indent(i+1);
		 sayln("size        = " ^ (Int.toString size));
		 (case condition of 
		      SOME condition =>
		      (indent(i+1);
		       sayln("condition: ");
		       app (fn(cond) => print_depcondition (outstream, cond, i+2)) 
			   (getcond condition))
		    | NONE => ());
		 indent(i+1);
		 sayln("equation:     " );
		 print_depenv_equation (outstream,  eq, i+2);
		 case description of 
		     SOME description => (indent (i+1);
					  sayln("description: " ^ description))
		   | NONE => ())

	      | DLDeps.ScalarStateFunEntry {quantity, condition, event, eq, description, ...} =>
		(indent(i);
		 sayln ("ScalarStateFunEntry: "  ^ (depname2str quantity));
		 (case condition of 
		      SOME condition =>
		      (indent(i+1);
		       sayln("condition: ");
		       app (fn(cond) => print_depcondition (outstream, cond, i+2)) 
			   (getcond condition))
		    | NONE => ());
		 indent(i+1);
		 sayln("equation:     " );
		 print_depenv_equation (outstream,  eq, i+2);
		 case description of 
		 SOME description => (indent (i+1);
				      sayln("description: " ^ description))
	       | NONE => ())
		
	      | DLDeps.TableFunEntry       {quantity, body, low, high, step,
				     argument, description} =>
		(indent(i);
		 sayln ("TableFunEntry: " ^ (depname2str quantity));
		 indent(i+1);
		 sayln ("body="); (print_indepexp (outstream, body, i+1));
		 indent(i+1);
		 sayln ("low=" ^ (Real.toString low));
		 indent(i+1);
		 sayln ("high=" ^ (Real.toString high));
		 indent(i+1);
		 sayln ("step=" ^ (Real.toString step));
		 indent(i+1);
		 sayln ("argument=" ^ (depname2str argument));
	     	 case description of 
		     SOME description => (indent(i+1); 
					  sayln("description = " ^ description))
		   | NONE => ())

	      | DLDeps.FunEntry            {name, body, typeinfo} =>
		(indent(i);
		 sayln ("FunEntry: " ^ (depname2str name));
		 indent(i+1);
		 sayln ("body   ="); (print_indepexp (outstream, body,	i+1));
		 indent(i+1);
		 sayln ("typeinfo:"); (print_typeinfo' (outstream, typeinfo, i+1)))
		
		  | DLDeps.FunAlias {name, entryname, typeinfo} =>
			(indent(i);
			 sayln ("FunAlias: " ^ (depname2str name));
			 indent(i+1);
			 sayln ("entryname: "); (depname2str entryname);
			 indent(i+1);
			 sayln ("typeinfo:"); (print_typeinfo' (outstream, typeinfo, i+1)))
			
	      | DLDeps.BuiltinFunEntry     {name, typeinfo} =>
		(indent (i);
		 sayln ("BuiltinFunEntry: " ^ (depname2str name));
		 indent(i+1);
		 sayln("typeinfo ="); (print_typeinfo' (outstream, typeinfo, i+1)))

	      | DLDeps.BuiltinScalar       {name} =>
		(indent (i);
		 sayln ("BuiltinScalar: " ^ (depname2str name)))

	      | DLDeps.ForeignScalar       {name} =>
		(indent (i);
		 sayln ("ForeignScalar: " ^ (depname2str name)))

	      | DLDeps.SetEntry            {name, value} =>
		(indent (i);
		 sayln ("SetEntry: " ^ (depname2str name));
		 indent (i+1);
		 sayln ("value = " ^ (Int.toString value)))
	end


    fun print_depenv (outstream, env:DLDeps.depenv) =
	let
	    val enventryList = UniqueId.listItems env
	    val print_depenventry = print_depenventry env
	in
	    app (fn(enventry) => print_depenventry (outstream, enventry, 0)) enventryList
	end    

(*	    

    fun print_node (outstream, g, node) =
	let
	    val indent_factor = 2
				
	    val say = say (outstream)
	    val sayln = sayln (say)
	    val ind = ind (say)
	    val indent = indent (ind) (indent_factor)
			 
	    val (pred, node, {color, entry}, succ) = Graph.context(node, g)
						     
	    val _ = say ("node: "^((name2str o nameofentry) entry)^"\n")
	    val _ = say ("numb: "^(Int.toString node)^"\n")
		    
	    val _ = say ("pred: ")
	    val _ = app (say o (fn(s) => (s^" ")) o Int.toString o (fn((_,x)) => x)) pred
	    val _ = say ("\n")
		    
	    val _ = say ("succ: ")
	    val _ = app (say o (fn(s) => (s^" ")) o Int.toString o (fn((_,x)) => x)) succ
	    val _ = say ("\n\n")
	in
	    ()
	end

    fun print_context (outstream, g, node) =
	let
	    val indent_factor = 2
				
	    val say = say (outstream)
	    val sayln = sayln (say)
	    val ind = ind (say)
	    val indent = indent (ind) (indent_factor)
			 
	    val (pred, node, {color, entry}, succ) = node
						     
	    val _ = say ("node: "^((name2str o nameofentry) entry)^"\n")
	    val _ = say ("numb: "^(Int.toString node)^"\n")
		    
	    val _ = say ("pred: ")
	    val _ = app (say o (fn(s) => (s^" ")) o Int.toString o (fn((_,x)) => x)) pred
	    val _ = say ("\n")
		    
	    val _ = say ("succ: ")
	    val _ = app (say o (fn(s) => (s^" ")) o Int.toString o (fn(_,x) => x)) succ
	    val _ = say ("\n\n")
	in
	    ()
	end

	    


    fun print_eqngraph (outstream, g) =

	let
	    val indent_factor = 2

	    val say = say (outstream)
	    val sayln = sayln (say)
	    val ind = ind (say)
	    val indent = indent (ind) (indent_factor)

	    fun gapp f = (fn(g) => app f (map (fn(node) => Graph.context(node,g)) (Graph.nodes g)))

    	    fun prnodes (g, i) = 
		(gapp (fn((p,v,{color, dep},s)) => 
			 (
			  (let
			       val id = case dep of
					    DLDeps.NEW {id, ...} => id
					  | DLDeps.OLD {id, ...} => id
					  | DLDeps.STATELESS {id, ...} => id
					  | DLDeps.BUILTIN {id, ...} => id
					  | DLDeps.CONDITION id => id
					  | DLDeps.TABLE {id, ...} => id
					  | DLDeps.PARAM {id, ...} => id
			   in
			       indent (i+1);
			       sayln ((depname2idstr dep) ^ 
				      " [label = \"" ^ (id2str(id)) ^ "\\n" ^ 
				      (depname2idstr dep)
				      ^ "\\ncolor: " ^ (Int.toString color) ^ "\"];")
			   end);
			  
			  ())) g)

	    fun predgeset (label, s, i) =
		(
		(app (fn(({visited}, node)) => 
			let 
			    val (nodelab,out) = Graph.fwd (node, g)
			    val {color, dep} = nodelab
			in
			    indent (i+1);
			    sayln (label ^ " -> " ^ 
				   (depname2idstr dep) ^ 
				   " [label=\"" ^ (if visited then "visited" else "unvisited")  ^ "\"]" ^
				   ";")
			end) s))
		

    	    fun traverse (g, i) = 
		(gapp (fn((p,v,{color, dep},s)) => 
			 ((let 
			       val label = (depname2idstr dep) 
			   in
			       predgeset (label, s, i+1)
			   end);
			  ())) g)
		
	in
	    sayln "digraph eqns {";
(*	    indent(1);
	    sayln "page = \"8.5,11\"";*)
	    prnodes (g, 0);
	    sayln "";
	    traverse (g, 0);
	    sayln "}"
	end

    fun print_expgraph (outstream, g) =

	let
	    val indent_factor = 2

	    val say = say (outstream)
	    val sayln = sayln (say)
	    val ind = ind (say)
	    val indent = indent (ind) (indent_factor)

	    fun gapp f = (fn(g) => app f (map (fn(node)=>Graph.context(node,g)) (Graph.nodes g)))

	    fun node2str (n) = ("n" ^ (Int.toString n))

    	    fun prnodes (g, i) = 
		(gapp (fn((p,v,nodelab,s)) => 
			 (
			  (let
			       val labelstr = ExpGraph.nodelabel2str nodelab
			   in
			       indent (i+1);
			       sayln ((node2str v) ^ 
				      " [label = \"" ^ labelstr ^ "\\n" ^ "\"];")
			   end);
			  ())) g)

	    fun predgeset (label, s, i) =
		(app (fn(edgelab, s) => 
			let 
			    val labelstr = ExpGraph.edgelabel2str edgelab
			in
			    indent (i+1);
			    sayln ((label) ^ " -> " ^ (node2str s) ^
				   " [label=\"" ^ labelstr  ^ "\"]" ^
				   ";")
			end) s)
		

    	    fun traverse (g, i) = 
		(gapp (fn((p,v,nodelab,s)) => 
			 ((let 
			       val label = node2str v
			   in
			       predgeset (label, s, i+1)
			   end);
			  ())) g)
		
	in
	    sayln "digraph eqns {";
(*	    indent(1);
	    sayln "page = \"8.5,11\"";*)
	    prnodes (g, 0);
	    sayln "";
	    traverse (g, 0);
	    sayln "}"
	end
*)

end



