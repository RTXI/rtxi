structure ErrorData =
struct
(*--------------------------------------------------------------------*)
(* a position holds the filename, line and column number.             *)
(*--------------------------------------------------------------------*)
type ErrorPosition = string * (int * int)
val nullPosition = ("",0,0)

(*--------------------------------------------------------------------*)
(* information about the file being processed.                        *)
(*--------------------------------------------------------------------*)
type filearg = {filename:string, filepath:string, posmap: Pos.posmap, errors: int ref}

type typeset = DLTypes.TypeSet.set
type dltype = DLTypes.dl_type

datatype Error = 
	 (* syntax errors *)
	 ERR_SYNTAX of string

       (* type errors *)
       | ERR_NEGATIVE_TIMEDIV
       | ERR_TIMEDIV_IN_TIMEDIV
       | ERR_INVALID_REAL_CONST_EXPR
       | ERR_INVALID_INT_CONST_EXPR
       | ERR_DUP_EVENT                  of {name: DLAst.name}
       | ERR_EVENT_IN_COND              of {name: DLAst.name}
       | ERR_EVENT_IN_EVENT             of {name: DLAst.name}
       | ERR_UNKNOWN_ID                 of {name: DLAst.name}
       | ERR_INVALID_FUN_ENTRY          of {name: DLAst.name}
       | ERR_INVALID_NUM_ARGS           of {expected: int, actual: int}
       | ERR_INVALID_TABLE_ARG          of {name: DLAst.name}
       | ERR_INVALID_DT_ARGS            of {quantity: DLAst.name}
       | ERR_INVALID_ARG_TYPE           of {name: DLAst.name, expected: dltype, actual: dltype}
       | ERR_INVALID_EXP_TYPE           of {quantity: DLAst.name, expected: dltype, actual: typeset}
       | ERR_UNRESOLVED_EXP_TYPE        of {exp: DLAst.exp, typ: typeset}
       | ERR_UNRESOLVED_TYPE_ARG        of {function: DLAst.name, arg: DLAst.name}
       | ERR_UNRESOLVED_TYPE_FUN        of {function: DLAst.name}
       | ERR_UNRESOLVED_IF_TYPE         of {falsetype: typeset, truetype: typeset}
       | ERR_EXT_TIMEDIV                of {quantity: DLAst.name}
       | ERR_EXT_INPUT_EQN              of {quantity: DLAst.name}
       | ERR_INVALID_SET_ELM            of {quantity: DLAst.name, element: DLAst.name}
       | ERR_MULTIPLE_DECLS             of {quantity: DLAst.name}
       | ERR_SAME_SETNUM                of {quantity: DLAst.name}
       | ERR_INVALID_DIFF_EQN_QUANTITY  of {quantity: DLAst.name}
       | ERR_INVALID_DT_EQN_QUANTITY    of {quantity: DLAst.name}
       | ERR_INVALID_ALG_EQN_QUANTITY   of {quantity: DLAst.name}
       | ERR_CYCLE                      of {quantities: Symbol.symbol list}
       | ERR_INVALID_VECTOR_SIZE_BINOP  of {oper: DLAst.binop}
       | ERR_INVALID_VECTOR_SIZE_RELOP  of {oper: DLAst.relop}
       | ERR_INVALID_VECTOR_SIZE_COND  
       | ERR_INVALID_VECTOR_ARG_SIZE    of {function: DLAst.name}
       | ERR_INVALID_VECTOR_COMPARISON  of {oper: DLAst.relop}
       | ERR_INVALID_VECTOR_BINOP       of {oper: DLAst.binop}
       | ERR_INVALID_VECTOR_UNARYOP     of {oper: DLAst.unaryexpop}
       | ERR_INVALID_VECTOR_STATE_FUN_SIZE of {quantity: DLAst.name}
       | ERR_NO_DEFAULT                 of {quantity: DLAst.name}
       | ERR_CHANNEL_COLLISION          of {quantity: DLAst.name, channel: int}

       (* Option errors*)
       | ERR_OPT of string

datatype Warning = 

	 WARN_NO_STATE_DECL
	 
       | WARN_UNUSED_STATE of {quantity: DLAst.name}
       | WARN_MULT_DECL    of {quantity: DLAst.name}
end
