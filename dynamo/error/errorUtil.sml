

signature ErrorUtil =
   sig
      val isSyntaxError     : ErrorData.Error -> bool
      val isTypeError       : ErrorData.Error -> bool
   end

structure ErrorUtil : ErrorUtil = 
   struct
      open ErrorData

      fun isSyntaxError err = 
	 case err
	   of ERR_EMPTY _           => true
	    | ERR_ENDED_BY_EE _     => true
	    | ERR_EXPECTED _        => true
	    | ERR_MISSING_WHITE     => true
	    | ERR_NON_XML_CHAR _    => true
	    | ERR_NON_XML_CHARREF _ => true
	    | _                     => false

      fun isTypeError err = 
	  case err of 
	      
	      _ => false

   end
