(* The Equation Sorting Phase of the DL Translator
 *
 * Steps involved:			 
 *    1) insert nodes into the graph
 *    2) create dependency edges in the graph
 *    3) color the trees and create a graph list where each
 *       graph contains a tree
 *    4) create an equation list for each tree
 *
 * Nodes Contain an equation structure
 * Equation Structures are defined to contain:
 *     1) the quantity the equation acts upon
 *     2) the right hand side of the equation (Env.equation)
 *  
 * Dependency edges point from a node to a node it is dependent upon
 *
 * To color the trees:
 *   add all nodes that can be reached from some node into a node set.
 *   create 2 graphs: 
 *       1) one that contains the nodes and edges for the nodes
 *          in the nodeset
 *       2) one that contains the remainder of the graph
 *   repeat until the remainder of the graph is empty
 *
 *   if while encountering nodes a cycle is detected
 *       (a cycle is defined as crossing an edge not in an edgeset of
 *        traversed edges and reaching a node in the nodeset)
 *   report the error
 *  
 * To form an equation list for a tree:
 *    (provably complete since no cycle exists within a tree)
 *    while the tree is non empty:
 *        1) remove a node with no dependency edges.
 *        2) add the node's equation to the end of the equation list
 *           for that tree
 * 
 * Each equation list is guaranteed to be non-dependent upon all the others
 * so they could be executed in any order or in parallel.
*)


signature DL_EQNGRAPH =
sig
    type filearg

    type symbol      = Symbol.symbol
		 
    type depenventry = DLDeps.depenventry
    type depenv
    type depname     = DLDeps.depname

    type uniq_id     = UniqueId.uniq_id

    val eqngraph : (symbol * depenv) -> 
		   (depenventry, unit, unit) Graph.graph
		   
    val color : ((depenventry, unit, unit) Graph.graph) -> (uniq_id list) list


end

structure EqnGraph: DL_EQNGRAPH =
struct

open DLErrors

type symbol      = Symbol.symbol

type depenventry = DLDeps.depenventry
type depenv      = DLDeps.depenv
type depname     = DLDeps.depname

type uniq_id      = UniqueId.uniq_id

exception InvalidIdentifier
exception CycleFound

val idof = DLDeps.idofdepname

val symof = DLDeps.symofdepname

val id2int = UniqueId.id2int

val posof = DLAst.posofname

structure ColorMap = IntMapTable (type key = int
                                  val getInt = fn (x) => x)


fun eqngraph (modelname, env) =
    let
(*
 val _ = print "DEPENV: \n"
 val _ = PrettyPrint.print_depenv(TextIO.stdOut, env)
*)
	val envlist = List.filter 
		      (fn (entry) => 
			  case entry of 
			      DLDeps.FunEntry       _   => false
			    | DLDeps.BuiltinFunEntry _  => false
			    | DLDeps.FunAlias       _   => false
			    | _ => true)
		      (UniqueId.listItems env)


	val G as Graph.GRAPH g = DirectedGraph.graph(Symbol.name modelname,(),length envlist) :
				 (depenventry,unit,unit) Graph.graph 

	val _ = app (#add_node g) (ListPair.zip (map (id2int o DLDeps.idofentry) envlist, 
						 envlist))

	val nodes   =  (#nodes g) ()

	val _ = app (fn (entry) => 
			let val t     = DLDeps.idofentry entry
			    val deps  = DLDeps.depsofentry entry
			in 
			    app (#add_edge g) 
				(List.mapPartial 
				     (fn(d) => let val s = idof d 
					       in 
						   if s=t then NONE 
						   else (case d of 
							     DLDeps.OLD _  => NONE
							   | _             => SOME (id2int s,id2int t,()))
					       end) deps)
			end) envlist

    in
	G
    end



fun color (G as Graph.GRAPH g) = 
    let val cycles  =  GraphCycles.cycles G (fn(c,cs) => c :: cs)  []
	val _       =  if cycles = [] then () else 
		       raise CycleFound 

	val nodes   =  (#nodes g) ()
	val roots   =  List.mapPartial (fn ((n,_)) => if List.null ((#in_edges g) n) 
				       then SOME n else NONE) nodes
	val (dist, max)  =  GraphBFS.bfsdist G roots
	val colormap     =  DynArray.foldli
				(fn (n,c,colormap: (int list) ColorMap.table) => 
				    let val ns = 
					    (case ColorMap.find (colormap,c) of 
						 SOME ns    => ns
					       | NONE       => [])
				    in if c > ~1 
				       then ColorMap.enter (colormap,c,n :: ns) 
				       else colormap end)
				ColorMap.empty dist
    in 
	foldl (fn (c,ax) => let val ns  = valOf (ColorMap.find (colormap,c))
				val ids = map UniqueId.int2id ns
			    in  
				ids :: ax
			    end)
	     [] (ColorMap.listKeys colormap)
    end

end
