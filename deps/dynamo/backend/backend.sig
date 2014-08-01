signature DL_BACKEND = 
sig

    type depenv      = DLDeps.depenv
    type depexp      = DLDeps.depexp
    type indepexp    = DLInDeps.indepexp
    type depname     = DLDeps.depname
    type indepname   = DLInDeps.indepname
    type type_info'  = DLDeps.type_info'
    type depenventry = DLDeps.depenventry

    type eqngraph 

    type depstorage = depname * string option -> string
    type indepstorage = indepname * string option -> string

    datatype progline = SUB of progline list | $ of string
	 
    type integration_method = {name: string,
			       documentation: string,
			       inputs: int,
			       invocation: string * string * string * string list -> progline,
			       body: string * string * string * string list -> progline list,
			       global_setup_code: progline list}

    type exp_info = {body: progline list,
		     vector_vars: string list,
		     scalar_vars: string list,
		     return_vars: string list,
		     return_type: DLTypes.dl_type}

    type external_state = {assignments: depname list ref, 
			   func: (depname list ref * depname * int -> progline list)}

    val integration_methods: integration_method list

    val main: string * string * string * string list * depenv 
              * depenv option * eqngraph
              * eqngraph option -> unit

    val printprog: TextIO.outstream * progline list * int -> unit

    val output_indepexp: indepstorage -> indepexp * depenv * type_info' option -> exp_info
    val output_depexp: depstorage -> depexp * depenv * type_info' option -> exp_info
    val output_eq: (depstorage * string * (depenv * depenventry -> progline list) * 
		    external_state * external_state) -> depenv * depenventry -> progline list
end
