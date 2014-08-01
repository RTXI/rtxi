signature TABLE = 
sig
   type key

   type 'a table
   val empty       : 'a table
   val enter       : 'a table * key * 'a -> 'a table
   val look        : 'a table * key -> 'a option
   val find        : 'a table * key -> 'a option
   val remove      : 'a table * key -> 'a table * 'a
   val numItems    : 'a table -> int
   val listKeys    : 'a table -> int list
   val listItems   : 'a table -> 'a list
   val listItemsi  : 'a table -> (int * 'a) list
   val map         : ('a -> 'b) -> 'a table -> 'b table
end

