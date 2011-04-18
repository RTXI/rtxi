structure DLErrors =
   struct
      open
	 ErrorData ErrorMessage 
      
      type ErrorPosition = ErrorData.ErrorPosition

      fun Position2String (fname,(l,c)) = 
	 if fname="" orelse (l = ~1 andalso c = ~1) then "" 
	 else String.concat ["[",fname,":",Int.toString l,".",Int.toString c,"]"]
	      
      val O_ERROR_LINEWIDTH = 80

      fun printError(filearg:filearg,poskey,err) = 
	  let
	      val pos = case poskey of 
			    ~1 => (~1, ~1)
			  | _ => Pos.pos(#posmap(filearg), poskey)
	  in
	      (#errors filearg) := !(#errors filearg) + 1;
	      TextIO.output
		  (TextIO.stdErr,
		   formatMessage (4,O_ERROR_LINEWIDTH) 
				 ([Position2String (#filename filearg, pos),
				   " Error:"] @ (errorMessage err)))
	  end
	  
      fun printWarning(filearg: filearg, poskey,warn) = 
	  let
	      val pos = case poskey of 
			    ~1 => (~1, ~1)
			  | _ => Pos.pos(#posmap(filearg), poskey)
	  in
	      TextIO.output
		  (TextIO.stdErr,
		   formatMessage (4,O_ERROR_LINEWIDTH) 
				 (Position2String (#filename filearg, pos)^" Warning:"::warningMessage warn))
	  end
   end
