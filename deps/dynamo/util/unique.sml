signature UNIQUE_ID =
sig
  eqtype uniq_id

  val id2int: uniq_id -> int
  val int2id: int -> uniq_id 
  val id2str: uniq_id -> string

  val genid: unit -> uniq_id
  type 'a table

  val empty : 'a table
  val enter : 'a table * uniq_id * 'a -> 'a table
  val look  : 'a table * uniq_id -> 'a option
  val listItems : 'a table -> 'a list
  val listKeys  : 'a table -> uniq_id list
  val map : ('a -> 'b) -> 'a table -> 'b table
end

structure UniqueId :> UNIQUE_ID =
struct

  type uniq_id = int

  exception UniqueId

  val uniq_id = ref ~1

  fun genid() = (uniq_id := !uniq_id + 1; !uniq_id)

  structure Table = IntMapTable(type key = uniq_id
				fun getInt(k) = k)

  type 'a table= 'a Table.table
  val empty = Table.empty
  val enter = Table.enter
  val look = Table.look
  val listItems = Table.listItems
  val listKeys = Table.listKeys
  val map = Table.map

  fun id2int (id) = id
  fun int2id (i) = i
  fun id2str (id) = Int.toString id

end
