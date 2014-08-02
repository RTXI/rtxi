


signature ErrorMessage =
   sig
       exception InternalError of string * string * string
       exception NoSuchFile of string * string
			       
       val errorMessage   : ErrorData.Error -> string list
       val warningMessage : ErrorData.Warning -> string list
						 
       val formatMessage : int * int -> string list -> string 
   end
       
structure ErrorMessage : ErrorMessage =
struct
open 
    ErrorData 
		  
exception InternalError of string * string * string
exception NoSuchFile of string * string

(*--------------------------------------------------------------------*)
(* generate a list/string of n times character c.                     *)
(*--------------------------------------------------------------------*)
fun nCharsC c n = if n>0 then c::nCharsC c (n-1) else nil
fun nChars c n = String.implode (nCharsC c n)
val nBlanks = nChars #" "

fun name2str(name) =
    let
	val {name=n, pos=_} = name
    in
	Symbol.name(n)
    end

fun formatMessage (indentWidth,lineWidth) strs =
    let 
	val indent = nBlanks indentWidth
	val nl = "\n"^indent
	val blank = " "
	val dot = "." 

	fun isSep c = #" "=c orelse #"\n"=c orelse #"\t"=c

	fun go (w,yet) nil = List.rev ("\n"::yet)
	  | go (w,yet) (x::xs) = 
	    let 
		val y = if null xs then x^dot else x 
		val l = String.size y
		val w1 = w+l
		val (w2,yet2) = if w1 <= lineWidth then (w1,y::yet)
				else (indentWidth + l, y::nl::yet)
		val (w3,yet3) = if null xs then (w2, yet2)
				else (if w2 < lineWidth then (w2+1, blank::yet2)
				      else (indentWidth, nl::yet2))
	    in go (w3,yet3) xs
	    end
		
	val tokens = List.concat (map (String.tokens isSep) strs)
	val fragments = go (0,nil) tokens
    in 
	String.concat fragments
    end

fun errorMessage err =
    case err of 
     (* syntax errors *)
	 ERR_SYNTAX str => ["syntax error:", str]
       

       (* type errors *)
       | ERR_NEGATIVE_TIMEDIV        => ["negative time divisor"]
       | ERR_TIMEDIV_IN_TIMEDIV      => ["time divisor block embedded in another time divisor block"]

       | ERR_INVALID_REAL_CONST_EXPR => ["invalid real constant expression"]
       | ERR_INVALID_INT_CONST_EXPR  => ["invalid integer constant expression"]

       | ERR_UNKNOWN_ID        {name} => ["unknown identifier", name2str name]

       | ERR_INVALID_FUN_ENTRY {name} => ["invalid function entry", name2str name]

       | ERR_INVALID_NUM_ARGS  {expected, actual} => 
	 ["invalid number of arguments:", 
	  "expected:", Int.toString expected, 
	  "actual:", Int.toString actual]

       | ERR_INVALID_ARG_TYPE  {name, expected, actual} =>
	 ["invalid type of argument", 
	  name2str name, 
	  "expected:", DLTypes.typeToString expected, 
	  "actual:", DLTypes.typeToString actual]

       | ERR_INVALID_TABLE_ARG  {name} =>
	 ["invalid table argument: ", 
	  name2str name, "is not a state"]

       | ERR_INVALID_EXP_TYPE  {quantity, expected, actual} =>
	 ["invalid type of expression", 
	  name2str quantity, 
	  "expected:", DLTypes.typeToString expected, 
	  "actual:", DLTypes.typeSetToString actual]

       | ERR_UNRESOLVED_EXP_TYPE        {exp, typ} =>
	 ["expression has unresolved type:",
	  DLTypes.typeSetToString typ]

       | ERR_UNRESOLVED_IF_TYPE {falsetype, truetype} =>
	 ["expressions in the two branches of a conditional expression have differing types",
	  "if false: ", DLTypes.typeSetToString falsetype, 
	  "if true: ", DLTypes.typeSetToString truetype ]
	 
       | ERR_UNRESOLVED_TYPE_FUN {function} =>
	 ["unresolved return type of function", 
	  name2str function]

       | ERR_UNRESOLVED_TYPE_ARG {function, arg} =>
	 ["unresolved type of function argument", 
	  name2str function, 
	  name2str arg]

       | ERR_EXT_TIMEDIV     {quantity} =>
	 ["external input equation for quantity ", name2str quantity, 
	  " cannot be within a time divisor block"] 

       | ERR_EXT_INPUT_EQN     {quantity} =>
	 ["equation assigned to external input", name2str quantity] 
	 
       | ERR_INVALID_SET_ELM   {quantity, element} =>
	 ["invalid element", name2str element, 
	  "assigned to set", name2str quantity]

       | ERR_MULTIPLE_DECLS    {quantity} =>
	 ["multiple declarations of quantity", name2str quantity]

       | ERR_INVALID_DT_ARGS            {quantity} => 
	 ["invalid integration arguments in the equation of quantity", 
	  name2str quantity]

       | ERR_INVALID_DIFF_EQN_QUANTITY  {quantity} =>
	 ["quantity", name2str quantity, "cannot have a difference equation"]

       | ERR_INVALID_DT_EQN_QUANTITY    {quantity} =>
	 ["quantity", name2str quantity, "cannot have a differential equation"]

       | ERR_INVALID_ALG_EQN_QUANTITY   {quantity} =>
	 ["quantity", name2str quantity, "cannot have an algebraic equation"]

       (* equation sorting errors *)
       | ERR_CYCLE {quantities} =>
	 ["loop detected: "] @ 
	 (map (fn (q) => (Symbol.name q) ^ " -> ") quantities) @ 
	 [Symbol.name (hd quantities)]

       | ERR_INVALID_VECTOR_SIZE_BINOP  {oper} =>
	 ["the left-hand side and right-hand side of operator "] @
	 [PrettyPrint.binop2str oper] @
	 [" are vectors of different sizes"]
	 
       | ERR_INVALID_VECTOR_SIZE_RELOP  {oper} =>
	 ["the left-hand side and right-hand side of operator "] @
	 [PrettyPrint.relop2str oper] @
	 [" are vectors of different sizes"]

       | ERR_INVALID_VECTOR_SIZE_COND   =>
	 ["the left-hand side and right-hand side of a conditional " ^
	  "are vectors of different sizes"]

       | ERR_INVALID_VECTOR_ARG_SIZE    {function} => 
	 ["the argument to function "] @ [name2str function] @
	 [" is a vector of invalid size"]

       | ERR_INVALID_VECTOR_COMPARISON  {oper} => 
	 ["comparison operator "] @
	 [PrettyPrint.relop2str oper] @
	 [" cannot be used for vectors"]

       | ERR_INVALID_VECTOR_BINOP       {oper} =>
	 ["operator "] @
	 [PrettyPrint.binop2str oper] @
	 [" has an invalid combination of vector and scalar operands"]
	 
       | ERR_INVALID_VECTOR_UNARYOP     {oper} =>
	 ["operator "] @
	 [PrettyPrint.unaryop2str oper] @
	 [" cannot have a vector operand"]

       | ERR_INVALID_VECTOR_STATE_FUN_SIZE {quantity} =>
	 ["call to function "] @
	 [name2str quantity] @
	 [" has vector arguments of different sizes"]

       | ERR_SAME_SETNUM {quantity} =>
	 ["set element "] @ 
	 [name2str quantity] @ 
	 [" has a duplicate integer value"]

       | ERR_CHANNEL_COLLISION {quantity, channel} =>
	 ["external state "] @ 
	 [name2str quantity] @ 
	 [" is using channel "^ (Int.toString channel) ^ 
	  ", which is in use by another external state"]

       | ERR_OPT str =>
	 ["invalid option: ", str]

       | ERR_NO_DEFAULT {quantity} =>
	 ["quantity "] @
	 [name2str quantity] @
	 [" has no default equation"]

       | ERR_DUP_EVENT {name} =>
	 ["event "] @ 
	 [name2str name] @ 
	 [" has a duplicate definition"]
						  
       | ERR_EVENT_IN_COND {name} =>
	 ["event "] @ 
	 [name2str name] @ 
	 [" is defined inside a conditional block"]
						  
       | ERR_EVENT_IN_EVENT {name} =>
	 ["event "] @ 
	 [name2str name] @ 
	 [" is defined inside another event"]
						  

fun warningMessage warn =
    case warn of 

	 WARN_NO_STATE_DECL => ["no state declarations"]
	 
       | WARN_UNUSED_STATE {quantity} => ["unused state", name2str quantity]
       | WARN_MULT_DECL    {quantity} => ["multiple declarations of state", name2str quantity]

end
