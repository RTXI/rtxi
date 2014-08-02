
signature DL_PARSE = 
sig
  type model
  type filearg

  val parse : string -> filearg * model
end

functor DLParse (structure Ast: DL_AST): DL_PARSE =
struct 

  open DLErrors

  structure DLLrVals = DLLrValsFun(structure Token=LrParser.Token
				   structure Ast=Ast)
  structure Lex = DLLexFun(structure Pos=Pos
			   structure Tokens=DLLrVals.Tokens)

  structure DLP = JoinWithArg(structure ParserData = DLLrVals.ParserData
			      structure Lex=Lex
			      structure LrParser=LrParser)

  type model = Ast.system

  exception ParserError

  fun parse (filepath: string) =
      let 
	  val la    = Lex.UserDeclarations.newLexerState(filepath)
	  val file  = TextIO.openIn filepath
		 
	  val filename = OS.Path.mkCanonical  filepath
	  val filepath = OS.Path.dir filepath

	  fun get _ = TextIO.input file
	  fun parseerror(s, p1, p2) = 
	      let
		  val filearg = {filename=(#file_name la), filepath=(#file_path la), 
				 posmap=(!(#pos_map la)), errors=(#errno la)}
				
	      in
		  printError (filearg, p1, ERR_SYNTAX s)
	      end

	  val lexer = LrParser.Stream.streamify (Lex.makeLexer get la)
	  val (absyn, _) = DLP.parse(30,lexer,parseerror,())
	  val _ = TextIO.closeIn file
      in 
	  ({filename=filename, filepath=filepath, posmap=(!(#pos_map(la))), errors = ref 0}, 
	   absyn)
      end 
	  handle LrParser.ParseError => raise ParserError
	       
end
