** General info on the swig-generated wrappers for Comedilib **

1) Regenerating the wrappers
	The wrapper were made using swig-1.3.19. Any of the swig-1.3.x seris should work
	N.B. the default swig on redhat systems is version 1.1. Upgrade to v-1.3.19. It's better!
	
	run $> swig -python comedi.i
	
2) Building the module (section obsolete -Frank Hess)
	i) edit the setup.py file so that the include and lib paths are correct for your system
	ii) run $>	python setup.py build
	
3) Installing the module (section obsolete -Frank Hess)
	i) Manual installation I'm afraid. Copy comedi.py and _comedi.pyd files to somewhere in your PYTHONPATH
	
4) Using the module
	All the comedilib functions are translated directly to python function. The various comedi structs
	are now available as python classes (e.g. comedi_cmd_struct). The members of each struct are now
	attributes of the class and can be set and retrieved in the usual way. Comedilib Functions which 
	take a pointer to a comedilib struct as an argument (in C) now, in python, accept the appropriate 
	struct python object.
	
	For a multichannel acquisition, a C-array containing the channel list, gains and referencing is
	required. This can be created using a swig-generated helper class: chanlist(n). This creates a C-array
	of length n and type Unsigned Int. Individual members of the array can be accessed/set using 
	pythons indexing syntax:
		mylist = chanlist(3)   #creates a chanlist array of length 3
		mylist[0] = 100 #set some values
		mylist[1] = 200
		mylist[2] = 300
		
	The chanlist object can then be passed to a comedi_cmd_struct object, for example. N.B. The
	chanlist object contains *no* length-checking or other error protection so use with care! Don't
	try to get/set indexes outside the array bounds.
	
	All the comedilib macros (e.g. CR_PACK) are now available as python functions.
	
	check out the example test_comedi.py to clarify the above.
	
	
