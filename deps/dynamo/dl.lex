(*    -*- mode: sml-lex -*- *) 

(* $Id: dl.lex,v 1.4 2007/04/02 12:40:18 ivan_raikov Exp $ *)



open TextIO


structure Tokens = Tokens

(* This is the type passed to the lexer. *)
type lexarg = { file_name:     string, 
	        file_path:     string, 
		file_pos:      int ref,
		pos_map:       Pos.posmap ref,
 		column_number: int ref, 
		line_number:   int ref, 
		in_string:     bool ref, 
		this_string:   string ref,
		nesting_depth: int ref,
		this_block:    string ref,
		errno :        int ref}

type arg = lexarg

type ('a, 'b) token = ('a,'b) Tokens.token

(* Stuff needed by ML-Yacc *)
type pos = int
type svalue = Tokens.svalue
type lexresult  = (svalue,pos) Tokens.token

val maxIdLen = 32

(* List of keywords, followed by a list of the corresponding token type generators.  *) 
val keyword_table = [
		     ("AT",               fn(x) => Tokens.AT(x, x+2)),
		     ("C-DECLARATION",    fn(x) => Tokens.CDECLARATION(x, x+12)),
		     ("C-IDENTIFIER",     fn(x) => Tokens.CIDENTIFIER(x, x+12)),
		     ("CONDITION",        fn(x) => Tokens.CONDITION(x, x+9)),
		     ("DEFAULT",          fn(x) => Tokens.DEFAULT(x, x+7)),
		     ("ARGUMENT",         fn(x) => Tokens.ARGUMENT(x, x+11)),
		     ("DISCRETE",         fn(x) => Tokens.DISCRETE(x, x+14)),
		     ("EVENT",            fn(x) => Tokens.EVENT(x, x+5)),
		     ("EXTERNAL",         fn(x) => Tokens.EXTERNAL(x, x+8)),
		     ("FUNCTION",         fn(x) => Tokens.FUNCTION(x, x+8)),
		     ("HIGH",             fn(x) => Tokens.HIGH(x, x+4)),
		     ("INPUT",            fn(x) => Tokens.INPUT(x, x+5)),
		     ("INTEGER",          fn(x) => Tokens.INTEGER(x, x+13)),
		     ("LOW",              fn(x) => Tokens.LOW(x, x+3)),
		     ("METHOD",           fn(x) => Tokens.METHOD(x, x+6)),
		     ("OUTPUT",           fn(x) => Tokens.OUTPUT(x, x+6)),
		     ("PARAMETER",        fn(x) => Tokens.PARAMETER(x, x+9)),
		     ("STATE",            fn(x) => Tokens.STATE(x, x+5)),
		     ("STEP",             fn(x) => Tokens.STEP(x, x+4)),
		     ("SYSTEM",           fn(x) => Tokens.SYSTEM(x, x+6)),
		     ("TABLE",            fn(x) => Tokens.TABLE(x, x+5)),
		     ("TIME",             fn(x) => Tokens.TIME(x, x+4)),
		     ("VECTOR",           fn(x) => Tokens.VECTOR(x, x+6)),
		     ("CASE",             fn(x) => Tokens.CASE(x, x+4)),
		     ("d(",               fn(x) => Tokens.D(x, x+1)),
		     ("q(",               fn(x) => Tokens.Q(x, x+1)),
		     ("s(",               fn(x) => Tokens.S(x, x+1)),
		     ("X",                fn(x) => Tokens.X(x, x+1))
		     
		     ]


(* finds a keyword in the table, and returns a generating function to make a Token for it *)
fun lookup ([], keyword) = NONE
  | lookup ((key,value)::t, keyword) = 
    (case (String.compare(keyword, key)) of
	 EQUAL => 
	 SOME value
       | _     => 
	 lookup (t, keyword))
    
    
    
(* Create a shiny new argument to be passed to a lexer instance.  *)
fun newLexerState (input_file_path) = 
	({ file_name      = OS.Path.mkCanonical  input_file_path,
	   file_path      = input_file_path, 
	   file_pos      = ref 0,
	   pos_map       = ref (Pos.newposmap()),
	   column_number = ref 0,
	   line_number   = ref 1,
	   in_string     = ref false,
	   this_string   = ref "",
	   nesting_depth = ref 0,
	   this_block    = ref "",
	   errno         = ref 0 })
    
    
(* A function that inserts the current position in the row/column table. *) 
fun storepos (posmap, yypos, line, column) =  
    (Pos.inspos (!posmap, yypos, (line, (column - 1))); ())



(* Mandatory error and end-of-file handlers. *)

open DLErrors

fun error (err, {file_name, file_path, file_pos, pos_map, column_number, 
		 line_number, in_string, this_string, nesting_depth, this_block, 
		 errno}: lexarg) =
    let
	val filearg = {filename=file_name, filepath=file_path, posmap=(!pos_map), errors=errno}
    in
	printError(filearg, !file_pos, err) 
    end

fun eof ({file_pos, ...}: lexarg) =
    Tokens.EOF(!file_pos, 0)

fun ++ (a: int ref) = (a := !a + 1)
fun -- (a: int ref) = (a := !a - 1)
fun ^= (a: string ref, b: string) = (a := !a ^ b)

val debug = false
fun debugprint (s) = if debug then print (s ^ "\n") else ()

%%

%s COMMENT;
%s CBLOCK;
%s STRING;

%arg (la as {file_name, file_path, file_pos, pos_map, column_number, line_number,
	     in_string, this_string, nesting_depth, this_block, errno});

%header (functor DLLexFun(structure Tokens: DL_TOKENS));

KEYWORD = (AT | C-IDENTIFIER | C-DECLARATION | CONDITION | DEFAULT |
	   ARGUMENT |  DISCRETE | EVENT | EXTERNAL | FUNCTION | HIGH | 
	   INPUT | INTEGER | STATE | LOW | METHOD | OUTPUT | 
	   PARAMETER | STATE | STEP | SYSTEM | TABLE | TIME | VECTOR |
	   CASE | d\( | q\( | s\( | X); 


DIGITS	= [0-9]+;
INT	= {DIGITS};
REAL 	= ({DIGITS}?("."{DIGITS})?)|({DIGITS}("."{DIGITS})?[eE][+-]?{DIGITS})|(Inf(inity)?);
ID	= [a-zA-Z][a-zA-Z0-9_]*;
STRING 	= \"[^\"]*\";
WS      = [\012\ \t];

%%
					  

<INITIAL>{KEYWORD} => ((* looks up a keyword in the keyword table and
			  returns it *)
		       let 
			 val tok = (case lookup(keyword_table, yytext) of
				      NONE   => Tokens.ID(yytext, yypos, yypos + (size yytext))
				    | SOME t => t yypos)
		       in
			 debugprint ("DLLex: KEYWORD " ^ yytext);
			 storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
			 tok
		       end);


<INITIAL>"."   => (storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
		   Tokens.PERIOD(yypos, yypos + 1));

<INITIAL>","   => (storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
		   Tokens.COMMA(yypos, yypos + 1));

<INITIAL>"?"   => (storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
		   Tokens.QUESTION(yypos, yypos + 1));
<INITIAL>":"   => (storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
		   Tokens.COLON(yypos, yypos + 1));
<INITIAL>";"   => (storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
		   Tokens.SEMICOLON(yypos, yypos + 1));

<INITIAL>"+"   => (storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
		   Tokens.PLUS(yypos, yypos + 1));
<INITIAL>"-"   => (storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
		   Tokens.MINUS(yypos, yypos + 1));
<INITIAL>"*"   => (storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
		   Tokens.TIMES(yypos, yypos + 1));
<INITIAL>"/"   => (storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
		   Tokens.DIVIDE(yypos, yypos + 1));
<INITIAL>"%"   => (storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
		   Tokens.PERCENT(yypos, yypos + 1));
<INITIAL>"#"   => (storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
		   Tokens.POUND(yypos, yypos + 1));
<INITIAL>"^"  => (storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
		   Tokens.POW (yypos, yypos+2));

<INITIAL>"!"   => (storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
		   Tokens.NEG(yypos, yypos + 1));

<INITIAL>"||"    => (storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
		    Tokens.OR(yypos, yypos + 2));
<INITIAL>"&&"    => (storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
		    Tokens.AND(yypos, yypos+ 2));
<INITIAL>">"    => (storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
		    Tokens.GT(yypos, yypos + 1));
<INITIAL>"<"    => (storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
		    Tokens.LT(yypos, yypos+ 1));
<INITIAL>">="   => (storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
		    Tokens.GE(yypos, yypos + 2));
<INITIAL>"<="   => (storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
		    Tokens.LE(yypos, yypos + 2));
<INITIAL>"="    => (storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
		    Tokens.ASGN(yypos, yypos + 1));

<INITIAL>"("   => (storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
		   Tokens.LPAREN(yypos, yypos + 2));

<INITIAL>")"   => (storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
		   Tokens.RPAREN(yypos, yypos + 2));


<INITIAL>"["   => (storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
		   Tokens.LBRACKET(yypos, yypos + 2));

<INITIAL>"]"   => (storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
		   Tokens.RBRACKET(yypos, yypos + 2));



<INITIAL>"=="   => (storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
		     Tokens.EQ (yypos, yypos+2));
<INITIAL>"!="   => (storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
		    Tokens.NE (yypos, yypos+2));

<INITIAL>"=>"   => (storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
		    Tokens.FOLLOWS (yypos, yypos+2));


<INITIAL>"{"	=> ((* Rules to match blocks of C code. *)
		    nesting_depth := 1;
		    YYBEGIN(CBLOCK);
		    this_block := yytext;
		    continue());
<CBLOCK>"{"	=> (++nesting_depth;
		    ^= (this_block, yytext);
		    continue());
<CBLOCK>"}"	=> (--nesting_depth;
		    ^=(this_block, yytext);
		    if (!nesting_depth) = 0 
		    then
		      (YYBEGIN(INITIAL);
		       storepos(pos_map, yypos, !line_number, (yypos - !file_pos));
		       Tokens.CBLOCK(!this_block, yypos, yypos))
		    else continue ());
<CBLOCK>(\n|\r\n) => (++line_number;
		      ^=(this_block, yytext); 
		      continue ());
<CBLOCK>.	  => (^= (this_block, yytext);
		      continue());



<INITIAL>{INT} => ((* Rule to match integers. *)
		   storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
		   let
                       val intval = (Int.fromString(yytext))
		   in
		       case intval of
			   SOME(a:int) => Tokens.INT(a, yypos, yypos + (size yytext))
			 | NONE => (column_number := (yypos - !column_number);
				    error (DLErrors.ERR_SYNTAX "integer conversion error", la);
				    continue())
		   end);

<INITIAL>{REAL} => ((* Rule to match real numbers. *)
		   storepos(pos_map, yypos, !line_number, (yypos - !file_pos)); 
		   let
                       val realval = (Real.fromString(yytext))
		   in
		       case realval of
			   SOME(a:real) => Tokens.REAL(a, yypos, yypos + (size yytext))
			 | NONE => (column_number := (yypos - !column_number);
				    error (DLErrors.ERR_SYNTAX "real conversion error", la);
				    continue())
		   end);

<INITIAL>{STRING} => ((* Rule to match strings *)
		      let
			val s = substring (yytext, 1, (size yytext) - 2)
		      in
			storepos(pos_map, yypos, !line_number, (yypos - !file_pos));
			Tokens.STRING (s, yypos, yypos + (size s))
		      end);


<INITIAL>Pi	 => (storepos(pos_map, yypos, !line_number, (yypos - !file_pos));
		     Tokens.REAL(Math.pi, yypos, yypos + (size yytext)));

<INITIAL>{ID}    => (storepos(pos_map, yypos, !line_number, (yypos - !file_pos));
		     if size yytext > maxIdLen
		     then
			 (error (DLErrors.ERR_SYNTAX 
				    ("identifier name " ^
				     yytext ^ " " ^
				     "is longer than " ^
				     (Int.toString maxIdLen) ^
				     " characters"), la);
			  continue ())
		     else
			 Tokens.ID(yytext, yypos, yypos + (size yytext)));


<INITIAL>(\n|\r\n) => ((* Rule to match new line. *)
		       ++(line_number);
                       file_pos := yypos;
                       column_number := 0;
		       continue());

<INITIAL>{WS}+ => ((* Rule to match whitespace. *)
		   continue());

<INITIAL>"//".*(\n|\r\n)  =>  (++line_number; continue());
<INITIAL>"/*"             =>  (YYBEGIN(COMMENT); continue());
<COMMENT>[^*\r\n]*               =>  (continue ());
<COMMENT>"*"+[^*/\r\n]*          =>  (continue ());
<COMMENT>(\n|\r\n)               =>  (++line_number; continue());
<COMMENT>"*"+"/"                 =>  (YYBEGIN(INITIAL); continue ());

<COMMENT>.        => ((* Any unmatched characters in comments are ignored *)
		      continue ());

<INITIAL>.        => ((* Any unmatched characters produce an error. *)
		      column_number := (yypos - !file_pos); 
		      error (DLErrors.ERR_SYNTAX ("unrecognized character `" ^ yytext ^ "'"), la);
		      continue());
