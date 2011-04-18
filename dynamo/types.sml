
structure DLTypes =
struct

type symbol = Symbol.symbol

datatype dl_type = Scalar | Vector of int option | Discrete 

datatype unknown = Argument of symbol
		 | Function of symbol

datatype enventrytype =   TimeEntryType
 			| ScalarParEntryType
			| VectorParEntryType of int
			| ScalarStateEntryType 
			| VectorStateEntryType of int
			| IntegerStateEntryType
			| ExternalStateEntryType
			| DiscreteStateEntryType
			| VectorStateFunEntryType of int
			| ScalarStateFunEntryType
			| TableFunEntryType
			| FunEntryType
			| FunAliasType
			| BuiltinFunEntryType
			| BuiltinScalarType
			| ForeignScalarType
			| SetEntryType

		   
(* a comparison function for the elements of type sets *) 
  fun compare (Scalar, Scalar): order              = EQUAL
    | compare (Scalar, _)                          = LESS
    | compare (Vector NONE, Vector NONE)           =  EQUAL
    | compare (Vector (SOME n1), Vector (SOME n2)) = Int.compare (n1, n2)
    | compare (Vector _, Vector _)                 = EQUAL
    | compare (Vector _, Scalar)                   = GREATER
    | compare (Vector _, _)                        = LESS
    | compare (Discrete, Discrete)                 = EQUAL
    | compare (Discrete, _)                        = GREATER

  (* a comparison function for the elements of typepair sets *)
  fun comparePair((left1, right1), (left2, right2)) =
      case compare(left1, left2) of
	  LESS => LESS
	| GREATER => GREATER
	| EQUAL => compare(right1, right2)

  structure TypeSet = RedBlackSetFn (struct
				     type ord_key = dl_type
                                     val compare = compare
                                     end)

  structure TypePairSet = RedBlackSetFn(struct
					type ord_key = (dl_type * dl_type)
					val compare = comparePair
					end)
type typeset = TypeSet.set
fun isemptyset (s: typeset) =
    TypeSet.equal (s, TypeSet.empty)

fun sizeOfSet(s: typeset) =
    length( TypeSet.listItems (s))

fun contains(s: typeset, elem) =
    isemptyset(TypeSet.intersection(s, TypeSet.singleton(elem)))

fun containsOnly (s: typeset, elem) =
    TypeSet.equal (s, TypeSet.singleton(elem))

type type_info = {args: {name: DLAst.name, typ: typeset} list,
		  ret_type: typeset,
		  pos: DLAst.pos}
		 


(* generate set of all possible combinations of types based upon 2
 * typesets.  Result is stored in a TypePairSet.
 *)

fun genpair(left_ty, right_ty) =
   let
       fun f(typ, typset) =
	   typset @ (map (fn(t) => (t,typ))
			  (TypeSet.listItems(right_ty)))

       val listOfPairs = foldl f 
			       nil
			       (TypeSet.listItems(left_ty))
   in
       TypePairSet.addList(TypePairSet.empty, listOfPairs)
   end  
    

(* given a pair of typesets, and a list of valid type pairs and the
type they result in, returns a set of possible types *)

fun possibleTypes (lTypes, rTypes) validTypes =
    let
	val allpairs = genpair(lTypes, rTypes)

	fun validPair(pair, result) =
	    pair


	(* finds the result type for a given type pair *)

	fun rmNone [] 
	    = []
	  | rmNone (NONE::rest) 
	    = rmNone rest
	  | rmNone (SOME x::rest) 
	    = x::(rmNone rest)
			 
	val possibleTyList = map validTypes
				 (TypePairSet.listItems (allpairs))

	val possibleTypes  = TypeSet.addList(TypeSet.empty, rmNone possibleTyList)
			     
    in
	possibleTypes
    end

fun typeToString (t) =
    (case t of 
	Scalar          => "Scalar"
      | Vector (SOME n) => "Vector " ^ (Int.toString n)
      | Vector (NONE)   => "Vector " 
      | Discrete        => "Discrete")

fun typeSetToString (ts) =
    String.concat 
	(foldl 
	     (fn (t, h::r) => (", " ^ (typeToString t)::h::r)
	       | (t, nil) => ((typeToString t)::nil)) 
	     nil (TypeSet.listItems ts))

(* return the set of all types *)
fun allTypes() = 
    TypeSet.addList (TypeSet.empty, 
		     [Scalar, Vector NONE, Discrete])

exception InvalidSetSize

fun singletonToType (typeset) =
    if sizeOfSet(typeset) <> 1 then
	raise InvalidSetSize
    else
	hd (TypeSet.listItems(typeset))

end
