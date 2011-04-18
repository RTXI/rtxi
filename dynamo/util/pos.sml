
(*
 Facilities to map an absolute position within a file (as recorded by
 the lexer in yypos) to a  (row,column) pair. 
*)

signature POS =
sig
  
    structure H: MONO_HASH_TABLE

    eqtype pos
    type posmap = pos H.hash_table

    val newposmap : unit -> posmap
    val pos : posmap * int -> (int * int)
    val inspos : posmap * int * (int * int) -> unit
    val nopos : int * int
end

structure Pos :> POS =
struct

  exception Pos

  (* This is our row, column position type *)
  type pos = int * int

  (* This is the key type used for the monomorphic hashtable below. *)  
  structure POS_key =
  struct
      type hash_key = int
      val hashVal = (fn (k) => Word.fromInt(k))
      fun sameKey (s1, s2) = (s1 = s2)
  end

  structure H = HashTableFn(POS_key)

  type posmap = pos H.hash_table

  val sizeHint = 128
		 
  fun newposmap () = H.mkTable(sizeHint,Pos)
  
  (* Given an integer position, this function returns the corresponding
     (row,column) pair, or (-1,-1) if the position is not found in the
     table. *)
  fun pos (posmap, n) =
      case H.find posmap n of 
	  SOME (x, y) => (x, y)
	| NONE => (~1, ~1)
		  
  (* Inserts a (row,column) pair in the table, with a key the given integer 
     position.  *)
  fun inspos (posmap, n, (x:int, y:int)) =  
      case H.find posmap n of 
	  SOME (x, y) => ()
	| NONE => (H.insert posmap (n,(x,y)))
		  
  (* A dummy position for things that are not located in the file being processed *)
  val nopos = (~1, ~1)

end
