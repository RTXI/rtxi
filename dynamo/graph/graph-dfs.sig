(*
 * Some simple routines for performing depth first search.
 * 
 * -- Allen
 *)

signature GRAPH_DEPTH_FIRST_SEARCH = 
sig

   (* depth first search *)

   val dfs : ('n,'e,'g) Graph.graph  -> 
             (Graph.node_id -> unit) ->
             ('e Graph.edge -> unit) -> 
             Graph.node_id list -> unit

   val dfsfold : ('n,'e,'g) Graph.graph  -> 
                 (Graph.node_id * 'a -> 'a) ->
                 ('e Graph.edge * 'b -> 'b) -> 
                 Graph.node_id list -> 'a * 'b -> 'a * 'b
   val dfsnum  : ('n,'e,'g) Graph.graph ->
                 Graph.node_id list ->
                 { dfsnum  : int DynArray.array,  (* dfs numbering *)
                   compnum : int DynArray.array   (* completion time *)
                 }

      (* preorder/postorder numbering *)
   val preorder_numbering  : ('n,'e,'g) Graph.graph -> int -> int DynArray.array
   val postorder_numbering : ('n,'e,'g) Graph.graph -> int -> int DynArray.array

end

