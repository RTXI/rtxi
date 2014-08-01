(* 
 * ast.sml
 *
 * Abstract syntax tree definition for surface-level syntax.
 *
 * Version $Revision: 1.5 $
 *
 *
 * Copyright 2005 Ivan Raikov and the Georgia Institute of Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * A full copy of the GPL license can be found on Debian systems in 
 * /usr/share/common-licenses/GPL-2
 *
 *
 *)

signature DL_AST =
sig

type symbol = Symbol.symbol
type pos = int
type name = {name: symbol, pos: pos}


val symofname : name -> symbol
val posofname : name -> pos
val createname: symbol * pos -> name

(* A system consists of three components:
        - name

	- a sequence of declarations for the various quantities in the
          system

	- a sequence of time blocks, which contain the equations that
          govern the behavior of the system

 *)


(* A set element in a discrete set;
 * similar to the enum type in C
 *)
type set_element = name * int option



datatype system = SYSTEM of {name: name,
			     decl: declaration list,
			     time: system_time list}
	     
(* Each declaration can be one of:
	- a component (states, parameters, state functions, etc.)
	- a time declaration (time variable)
	- a function definition 
          (a named function, with arguments, that evaluates an
           expression that only involves the function arguments)
 *)
and declaration = 
    COMPONENT_DECL of component_decl
  | TIME_DECL      of name
  | EVENT_DECL     of name
  | FUNCTION_DEF   of {name: name,
		       args: name list,
		       body: exp,
		       pos: pos}
  | C_DECL of string

(* Component declarations: *)
and component_decl = 
    SCALAR_STATE_DECL of {quantity: name,
			  init: exp,
			  method: method option,
			  description: string option,
			  pos: pos}
  | VECTOR_STATE_DECL of {quantity: name,
			  initval: real Array.array,
			  method: method option,
			  description: string option,
			  pos: pos}
  | INT_STATE_DECL of {quantity: name,
		       init: exp,
		       description: string option,
		       pos: pos}
  | SCALAR_PAR_DECL of {quantity: name,
			init: exp,
			description: string option,
			pos: pos}
  | VECTOR_PAR_DECL of {quantity: name,
			initval: real Array.array,
			description: string option,
			pos: pos}
  | SCALAR_STATE_FUNC_DECL of {quantity: name,
			       description: string option,
			       pos: pos}
  | VECTOR_STATE_FUNC_DECL of {quantity: name,
			       description: string option,
			       pos: pos}
  | TABLE_DECL of {quantity: name,
		   parameters: name list,
		   body: exp,
		   low: real,
		   high: real,
		   step: real,
		   argument: name,
		   description: string option,
		   pos: pos}
  | C_ID_DECL of {quantity: name,
		  pos: pos}
  | EXTERNAL_DECL of {quantity: name,
		      direction: external_dir,
		      channel: int option,
		      description: string option,
		      pos: pos}
  | DISCRETE_DECL of {quantity: name,
		      set: set_element list,
		      description: string option,
		      pos: pos}
		     
		     
(* Integration methods for ODEs *)
			 
and method = 
    MAU
  | EULER


(* external states are an abstract representation of external systems,
   such as data acquisition boards *)
and external_dir = 
    INPUT 
  | OUTPUT

(* a time block in the system can be either a start block, which is
   executed only once (e.g. for initialization purposes), or a
   run-time block, which is evaluated once per iteration of the
   system. *)
and system_time = 
    STARTTIME of time_entry list
  | RUNTIME of name * (time_entry list)

and time_entry = 
   COND  of {relop: relop, 
	      quantity1: name,   
	      quantity2: name,   
	      body: time_entry list,
	      pos: pos}
  | EVENT of {name: name, 
	      arguments: name list,   
	      equations: time_entry list,
	      pos: pos}
  | EQN   of equation




and equation = 
    DIFFERENCE_EQN of name * exp * pos
  | DISCRETE_EQN of name * discrete_exp * pos 
  | DIFFERENTIAL_EQN of name * differential_arguments * pos
  | ALGEBRAIC_EQN of name * exp * pos
				    

and differential_arguments = 
    EULER_ARGS of exp
  | MAU_ARGS of exp * exp
   
(* Discrete expressions consist of either:
 *   - another discrete quantity to be used for assignment
 *   - a series of conditonal clauses
 *		   the first matching clause quantity is
 *		   the result of the series
 *     note: each series of conditional clauses MUST have
 *           a default clause
 *)
and discrete_exp = 
    DISCRETE_ID of name
  | DISCRETE_CASE of discrete_exp_clause list

(* Discrete expression clauses:
 *    either are the default case and resolve to the
 *     default quantity, or
 *    have a condition stored as a general expression
 *     to be evaluated, and a quantity in case of a
 *     match.
 *)
and discrete_exp_clause = 
    DISCRETE_COND of exp * name
  | DEFAULT_COND of name
		
and exp = 
    CONSTINT of int
  | CONSTREAL of real
  | ID of name
  | BINOP of {oper: binop, left: exp, right: exp, pos: pos}
  | RELOP of {oper: relop, left: exp, right: exp, pos: pos}
  | FUNCALL of {function: name, arguments: exp list, pos: pos}
  | UNARYEXPOP of {oper: unaryexpop, exp: exp, pos: pos}
  | CONDITIONAL of {condition: exp, iftrue: exp, iffalse: exp, pos: pos}
		   
and typ = 
    INT
  | DOUBLE

and binop =
    UNKNOWN       of poly_binop
  | SCALAR        of scalar_binop
  | SCALAR_VECTOR of scalar_vector_binop
  | VECTOR        of vector_binop

and poly_binop = 
    PLUS
  | MINUS
  | TIMES
  | DIVIDE
  | MODULUS
  | POWER
  | CROSSP

and scalar_binop =
    S_PLUS
  | S_MINUS
  | S_TIMES
  | S_DIVIDE
  | S_MODULUS
  | S_POWER

and vector_binop = 
    V_PLUS  of int
  | V_MINUS of int
  | V_TIMES of int
  | V_CROSSP of int
    
and scalar_vector_binop = 
    SV_TIMES of int


and unaryexpop = 
    UMINUS
  | UPLUS
  | NOT
    
and relop = 
    EQ 
  | GT
  | LT
  | GE
  | LE
  | NE
  | AND
  | OR

end


structure DLAst: DL_AST = 
struct

type symbol = Symbol.symbol
type pos = int
type name = {name: symbol, pos: pos}

(* take the position of a name *)
fun symofname (n) =
    let 
	val {pos=_, name=sym} = n
    in
	sym
    end

	
(* take the position of a name *)
fun posofname (n) =
    let 
	val {pos=pos, name=_} = n
    in
	pos
    end

	
(* create a name *)
fun createname (n, p) = {name=n, pos=p}


type set_element = name * int option



datatype system = SYSTEM of {name: name,
			     decl: declaration list,
			     time: system_time list}
and declaration = 
    COMPONENT_DECL of component_decl
  | TIME_DECL      of name
  | EVENT_DECL     of name
  | FUNCTION_DEF   of {name: name,
		       args: name list,
		       body: exp,
		       pos: pos}
  | C_DECL of string

and component_decl = 
    SCALAR_STATE_DECL of {quantity: name,
			  init: exp,
			  method: method option,
			  description: string option,
			  pos: pos}
  | VECTOR_STATE_DECL of {quantity: name,
			  initval: real Array.array,
			  method: method option,
			  description: string option,
			  pos: pos}
  | INT_STATE_DECL of {quantity: name,
		       init: exp,
		       description: string option,
		       pos: pos}
  | SCALAR_PAR_DECL of {quantity: name,
			init: exp,
			description: string option,
			pos: pos}
  | VECTOR_PAR_DECL of {quantity: name,
			initval: real Array.array,
			description: string option,
			pos: pos}
  | SCALAR_STATE_FUNC_DECL of {quantity: name,
			       description: string option,
			       pos: pos}
  | VECTOR_STATE_FUNC_DECL of {quantity: name,
			       description: string option,
			       pos: pos}
  | TABLE_DECL of {quantity: name,
		   parameters: name list,
		   body: exp,
		   low: real,
		   high: real,
		   step: real,
		   argument: name,
		   description: string option,
		   pos: pos}
  | C_ID_DECL of {quantity: name,
		  pos: pos}
  | EXTERNAL_DECL of {quantity: name,
		      direction: external_dir,
		      channel: int option,
		      description: string option,
		      pos: pos}
  | DISCRETE_DECL of {quantity: name,
		      set: set_element list,
		      description: string option,
		      pos: pos}
		     
		     
and method = 
    MAU
  | EULER

and external_dir = 
    INPUT 
  | OUTPUT

and system_time = 
    STARTTIME of time_entry list
  | RUNTIME of name * (time_entry list)

and time_entry = 
   COND    of conditional_block
  | EVENT   of event_block
  | EQN     of equation

and equation = 
    DIFFERENCE_EQN of name * exp * pos
  | DISCRETE_EQN of name * discrete_exp * pos 
  | DIFFERENTIAL_EQN of name * differential_arguments * pos
  | ALGEBRAIC_EQN of name * exp * pos
				    

and differential_arguments = 
    EULER_ARGS of exp
  | MAU_ARGS of exp * exp
   
and discrete_exp = 
    DISCRETE_ID of name
  | DISCRETE_CASE of discrete_exp_clause list

and discrete_exp_clause = 
    DISCRETE_COND of exp * name
  | DEFAULT_COND of name
		
and exp = 
    CONSTINT of int
  | CONSTREAL of real
  | ID of name
  | BINOP of {oper: binop, left: exp, right: exp, pos: pos}
  | RELOP of {oper: relop, left: exp, right: exp, pos: pos}
  | FUNCALL of {function: name, arguments: exp list, pos: pos}
  | UNARYEXPOP of {oper: unaryexpop, exp: exp, pos: pos}
  | CONDITIONAL of {condition: exp, iftrue: exp, iffalse: exp, pos: pos}
		   
and typ = 
    INT
  | DOUBLE

and binop =
    UNKNOWN       of poly_binop
  | SCALAR        of scalar_binop
  | SCALAR_VECTOR of scalar_vector_binop
  | VECTOR        of vector_binop

and poly_binop = 
    PLUS
  | MINUS
  | TIMES
  | DIVIDE
  | MODULUS
  | POWER
  | CROSSP

and scalar_binop =
    S_PLUS
  | S_MINUS
  | S_TIMES
  | S_DIVIDE
  | S_MODULUS
  | S_POWER

and vector_binop = 
    V_PLUS  of int
  | V_MINUS of int
  | V_TIMES of int
  | V_CROSSP of int
    
and scalar_vector_binop = 
    SV_TIMES of int


and unaryexpop = 
    UMINUS
  | UPLUS
  | NOT
    
and relop = 
    EQ 
  | GT
  | LT
  | GE
  | LE
  | NE
  | AND
  | OR


withtype conditional_block = {relop: relop, 
			 quantity1: name,   
			 quantity2: name,   
			 body: time_entry list,
			 pos: pos}

and event_block = {name: name, 
		   arguments: name list,   
		   equations: time_entry list,
		   pos: pos}


end

(*
 * $Id: ast.sml,v 1.5 2007/04/02 12:40:18 ivan_raikov Exp $
 *
 * 
 * $Log: ast.sml,v $
 * Revision 1.5  2007/04/02 12:40:18  ivan_raikov
 * Some restructuring of the Dynamo code.
 *
 * Revision 1.4  2007/02/07 20:38:36  ivan_raikov
 * Introduced a fix to support expression arguments in the MAU
 * method. Previously, the implementation only supported single
 * identifiers as arguments.
 *
 * Revision 1.3  2007/02/05 01:41:46  ivan_raikov
 * - Added support for constant expressions in initial values.
 * - Added hybrid_inhib_chem example from Marder lab.
 *
 * Revision 1.2  2006/04/27 20:19:03  ivan_raikov
 * Added Dynamo event interface.
 *
 * Revision 1.1  2006/02/23 01:35:34  ivan_raikov
 * Source tree merged with RTXI.
 *
 *)

