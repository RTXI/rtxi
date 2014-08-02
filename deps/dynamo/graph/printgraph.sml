(*
 *  Print a graph 
 *
 *  -- Allen
 *)

signature PRINT_GRAPH =
sig

   val toString : ('n -> string) -> ('n,'e,'g) Graph.graph -> string

end

structure PrintGraph : PRINT_GRAPH =
struct

   structure G = Graph

   fun toString node2str (G.GRAPH G) =
   let fun showEdges es = 
          String.concat(
             map (fn (i,j,_) => Int.toString i^" -> "^Int.toString j^"\n") es)
       fun showNodesLabels ns = 
          (String.concatWith "\n" 
			     (map (fn (n,l) => (node2str l) ^ " : " ^ (Int.toString n)) ns))^"\n"
       fun showNodes ns = 
          (String.concatWith ", " 
			     (map (fn (n) => (Int.toString n)) ns))^"\n"
   in     
       #name G ^ "\n" ^
       "nodes: "^showNodesLabels (#nodes G ())^
       "edges:\n"^showEdges(#edges G ())^
       "entry edges:\n"^
           showEdges(List.concat(map (#entry_edges G o #1) (#nodes G ())))^ 
       "exit edges:\n"^
           showEdges(List.concat(map (#exit_edges G o #1) (#nodes G ())))^ 
       "entries: "^showNodes(#entries G ())^
       "exits: "^showNodes(#exits G ())
   end
end

