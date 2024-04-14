%{
//-- don't change *any* of these: if you do, you'll break the compiler.
#include <algorithm>
#include <memory>
#include <cstring>
#include <cdk/compiler.h>
#include <cdk/types/types.h>
#include ".auto/all_nodes.h"
#define LINE                         compiler->scanner()->lineno()
#define yylex()                      compiler->scanner()->scan()
#define yyerror(compiler, s)         compiler->scanner()->error(s)
//-- don't change *any* of these --- END!
%}

%parse-param {std::shared_ptr<cdk::compiler> compiler}

%union {
  //--- don't change *any* of these: if you do, you'll break the compiler.
  YYSTYPE() : type(cdk::primitive_type::create(0, cdk::TYPE_VOID)) {}
  ~YYSTYPE() {}
  YYSTYPE(const YYSTYPE &other) { *this = other; }
  YYSTYPE& operator=(const YYSTYPE &other) { type = other.type; return *this; }

  std::shared_ptr<cdk::basic_type> type;        /* expression type */
  //-- don't change *any* of these --- END!
	std::shared_ptr<cdk::functional_type> functionalType;
	std::vector<std::shared_ptr<cdk::basic_type>> *typeVec;
  int                   i;	/* integer value */
  double								d;
	std::string          *s;	/* symbol name or string literal */
	
  cdk::basic_node      *node;	/* node pointer */
  cdk::sequence_node   *sequence;
  cdk::expression_node *expression; /* expression nodes */
  cdk::lvalue_node     *lvalue;
	
	mml::program_node *program;
	mml::variable_declaration_node *variable;
	mml::block_node				*block;
};

%token <i> tINTEGER
%token <d> tDOUBLE
%token <s> tIDENTIFIER tSTRING
%token tPRIVATE tPUBLIC tFOREIGN tFORWARD tSIZEOF tSTOP tNEXT tRETURN tWHILE tPRINT tPRINTLN tREAD tBEGIN tEND tNULL tRETURNS tAND tOR // O tRETURNS é o '->'
%token tINTEGER_TYPE tDOUBLE_TYPE tSTRING_TYPE tVOID_TYPE tAUTO_TYPE


%type <node> stmt onfalse file
%type <sequence> declarations instructions argdecs arg_expressions blockDeclarations expressions
%type <expression> expr funcdef integer double
%type <lvalue> lval
%type <block> block
%type <type> type functionalType
//%type <functionalType> functionalType
%type <typeVec> types
%type <variable> decl argdec bDecl
%type <program> program

%type <s> string


%nonassoc tIF
%nonassoc tTHEN
%nonassoc tELIF tELSE

%right '='
%left tOR
%left tAND
%right '~'
%left tEQ tNE
%left tGE tLE '>' '<' 
%left '+' '-'
%left '*' '/' '%'

%nonassoc tUNARY

%nonassoc '('

%nonassoc tFUNC


%{
//-- The rules below will be included in yyparse, the main parsing function.
%}
%%

file 	 :  /* EMPTY */         { compiler->ast($$ = new cdk::sequence_node(LINE)); }
		 |  program { compiler->ast($$ = new cdk::sequence_node(LINE,$1)); }
		 | declarations { compiler->ast($$ = $1); }
		 | declarations program { compiler->ast($$ = new cdk::sequence_node(LINE, $2, $1)); }
		 ;

program	 :tBEGIN tEND { $$ = new mml::program_node(LINE, new mml::block_node(LINE,nullptr,nullptr)); }
		 |tBEGIN instructions tEND { $$ = new mml::program_node(LINE, new mml::block_node(LINE,nullptr,$2)); }
		 |tBEGIN blockDeclarations  tEND { $$ = new mml::program_node(LINE, new mml::block_node(LINE,$2,nullptr)); }
		 |tBEGIN blockDeclarations instructions tEND { $$ = new mml::program_node(LINE, new mml::block_node(LINE,$2,$3)); }
	     ;

stmt : expr ';'                         { $$ = new mml::evaluation_node(LINE, $1); }
	 | expressions tPRINT					{ $$ = new mml::print_node(LINE, $1, false); }
	 | expressions tPRINTLN					{ $$ = new mml::print_node(LINE, $1, true); }
   	 | tWHILE '(' expr ')' stmt         { $$ = new mml::while_node(LINE, $3, $5); }
	 | tSTOP ';'                   { $$ = new mml::stop_node(LINE); } 
	 | tNEXT  ';'                   { $$ = new mml::next_node(LINE); } 
     | tRETURN ';'                 { $$ = new mml::return_node(LINE); } 
	 | tSTOP tINTEGER  ';'                   { $$ = new mml::stop_node(LINE, $2); }
     | tNEXT  tINTEGER ';'                   { $$ = new mml::next_node(LINE, $2); }
     | tRETURN expr ';'                 { $$ = new mml::return_node(LINE, $2); }
     | tIF '(' expr ')' stmt  	%prec tTHEN	{ $$ = new mml::if_node(LINE, $3, $5); }
	 | tIF '(' expr ')' stmt onfalse	{ $$ = new mml::if_else_node(LINE, $3, $5, $6); }
     | block		                    { $$ = $1; }
     ;
 

onfalse				: tELSE stmt						{ $$ = $2; }
				 	| tELIF '(' expr ')' stmt %prec tTHEN          { $$ = new mml::if_node(LINE, $3, $5); }
					| tELIF '(' expr ')' stmt onfalse   { $$ = new mml::if_else_node(LINE, $3, $5, $6); }
					;


expr 		: integer                   { $$ = $1; }
		 	| string                     { $$ = new cdk::string_node(LINE, $1); }
			| tREAD                   		{ $$ = new mml::read_node(LINE); }
			| double						{ $$ = $1; }
			| tNULL							{ $$ = new mml::nullptr_node(LINE); }
    		| '-' expr %prec tUNARY       { $$ = new cdk::neg_node(LINE, $2); }
			|'+' expr %prec tUNARY  { $$ = $2; }
			| '~' expr                { $$ = new cdk::not_node(LINE, $2); }
   			| expr '+' expr	          { $$ = new cdk::add_node(LINE, $1, $3); }
     		| expr '-' expr	          { $$ = new cdk::sub_node(LINE, $1, $3); }
     		| expr '*' expr	          { $$ = new cdk::mul_node(LINE, $1, $3); }
     		| expr '/' expr	          { $$ = new cdk::div_node(LINE, $1, $3); }
     		| expr '%' expr	          { $$ = new cdk::mod_node(LINE, $1, $3); }
     		| expr '<' expr	          { $$ = new cdk::lt_node(LINE, $1, $3); }
     		| expr '>' expr	          { $$ = new cdk::gt_node(LINE, $1, $3); }
     		| expr tGE expr	          { $$ = new cdk::ge_node(LINE, $1, $3); }
     		| expr tLE expr               { $$ = new cdk::le_node(LINE, $1, $3); }
     		| expr tNE expr	          { $$ = new cdk::ne_node(LINE, $1, $3); }
     		| expr tEQ expr	          { $$ = new cdk::eq_node(LINE, $1, $3); }
				|	expr tAND expr					{ $$ = new cdk::and_node(LINE, $1, $3); }
				| expr tOR expr					{ $$ = new cdk::or_node(LINE, $1, $3); }
     		| '(' expr ')'                { $$ = $2; }
			| '[' expr ']'                { $$ = new mml::stack_alloc_node(LINE,$2); }
			| expr '('')'           { $$ = new mml::function_call_node(LINE, $1, new cdk::sequence_node(LINE), 0); }
			| '@' '('')'  		{ $$ = new mml::function_call_node(LINE, nullptr, new cdk::sequence_node(LINE), 1); }
     		| expr '(' arg_expressions ')' { $$ = new mml::function_call_node(LINE, $1, $3, 0); }
			| '@' '(' arg_expressions ')'   		{ $$ = new mml::function_call_node(LINE, nullptr, $3, 1); }
			| funcdef											{ $$ = $1; }
			| lval                        { $$ = new cdk::rvalue_node(LINE, $1); } 
     		| lval '=' expr               { $$ = new cdk::assignment_node(LINE, $1, $3); }
		 	| tSIZEOF '(' expr ')'         { $$ = new mml::sizeof_node(LINE, $3); }
     		| lval '?' 												{ $$ = new mml::address_node(LINE, $1); }
			;

funcdef							:'('')' tRETURNS type block			{ $$ = new mml::function_def_node(LINE, $4, new cdk::sequence_node(LINE), $5); }
								| '(' argdecs ')' tRETURNS type block			{ $$ = new mml::function_def_node(LINE, $5, $2, $6); }  
								;

argdecs							: argdec											{ $$ = new cdk::sequence_node(LINE, $1); }
								| argdecs ',' argdec					{ $$ = new cdk::sequence_node(LINE, $3, $1); }
								;

argdec							: type tIDENTIFIER						{ $$ = new mml::variable_declaration_node(LINE, tPRIVATE, $1, *$2,nullptr); }
								;// function calls don't work like this, because the function name already returns an expression through lval.



arg_expressions 				: expr												{ $$ = new cdk::sequence_node(LINE, $1); }
								| arg_expressions ',' expr 		{ $$ = new cdk::sequence_node(LINE, $3, $1); }
								;

block							: '{' blockDeclarations instructions '}'						{ $$ = new mml::block_node(LINE, $2, $3); }
								|	'{' blockDeclarations '}'														{ $$ = new mml::block_node(LINE, $2, nullptr); }
								|	'{' instructions '}' 																{ $$ = new mml::block_node(LINE, nullptr, $2); }
								|	'{' '}'																							{ $$ = new mml::block_node(LINE, nullptr, nullptr);}
								;

declarations					: decl										{ $$ = new cdk::sequence_node(LINE, $1); }
								| declarations decl				{ $$ = new cdk::sequence_node(LINE, $2, $1); }
								;

blockDeclarations				: bDecl										{ $$ = new cdk::sequence_node(LINE, $1); }
								| blockDeclarations bDecl				{ $$ = new cdk::sequence_node(LINE, $2, $1); }
								;

bDecl							: type tIDENTIFIER ';'					{ $$ = new mml::variable_declaration_node(LINE, tPRIVATE, $1, *$2, nullptr); }
								| type tIDENTIFIER '=' expr ';'			{ $$ = new mml::variable_declaration_node(LINE, tPRIVATE, $1, *$2, $4); }
								| tAUTO_TYPE tIDENTIFIER '=' expr ';' { $$ = new mml::variable_declaration_node(LINE, tPRIVATE, nullptr, *$2, $4); }
								;

decl							: type tIDENTIFIER ';'					{ $$ = new mml::variable_declaration_node(LINE, tPRIVATE, $1, *$2, nullptr); }
								| type tIDENTIFIER '=' expr ';'			{ $$ = new mml::variable_declaration_node(LINE, tPRIVATE, $1, *$2, $4); }
								| tAUTO_TYPE tIDENTIFIER '=' expr ';' { $$ = new mml::variable_declaration_node(LINE, tPRIVATE, nullptr, *$2, $4); }
								| tPUBLIC type tIDENTIFIER ';' 		{ $$ = new mml::variable_declaration_node(LINE, tPUBLIC, $2, *$3, nullptr); }
								| tFOREIGN type tIDENTIFIER ';' 		{ $$ = new mml::variable_declaration_node(LINE, tFOREIGN, $2, *$3, nullptr); }
								| tFORWARD type tIDENTIFIER ';' 		{ $$ = new mml::variable_declaration_node(LINE, tFORWARD, $2, *$3, nullptr); }
								| tPUBLIC type tIDENTIFIER '=' expr ';' { $$ = new mml::variable_declaration_node(LINE, tPUBLIC, $2, *$3, $5); }
								| tPUBLIC tIDENTIFIER '=' expr ';' { $$ = new mml::variable_declaration_node(LINE, tPUBLIC, nullptr, *$2, $4); }
								| tPUBLIC tAUTO_TYPE tIDENTIFIER '=' expr ';' { $$ = new mml::variable_declaration_node(LINE, tPUBLIC, nullptr, *$3, $5); }
								;

expressions						: expr										{ $$ = new cdk::sequence_node(LINE, $1); } //NOT SURE WHAT NODE THIS SHOULD BE???
								| expressions ',' expr				{ $$ = new cdk:: sequence_node(LINE, $3, $1); }  // EXPRESSIONS HAVE ATLEAST ONE EXPR SO SHOULD NOT HAVE EMPTY CASE RIGHT???
								;

instructions					: stmt										{ $$ = new cdk::sequence_node(LINE, $1); }
								| instructions stmt				{ $$ = new cdk:: sequence_node(LINE, $2, $1); }
								;



types							: type										{ std::vector<std::shared_ptr<cdk::basic_type>> *input = new std::vector<std::shared_ptr<cdk::basic_type>>();
              																input->push_back($1); 
            																$$ = input; }
					 			|	types ',' type					{ $1->push_back($3); $$ = $1; }
								;

type							: tINTEGER_TYPE							{ $$ = cdk::primitive_type::create(4, cdk::TYPE_INT); }
								| tDOUBLE_TYPE							{ $$ = cdk::primitive_type::create(8, cdk::TYPE_DOUBLE); }
								| tSTRING_TYPE							{ $$ = cdk::primitive_type::create(4, cdk::TYPE_STRING); }
								| tVOID_TYPE								{ $$ = cdk::primitive_type::create(0, cdk::TYPE_VOID); }
								| '[' type ']'							{ $$ = cdk::reference_type::create(4, $2); }
								| functionalType						{ $$ = $1; }
					 			;

functionalType 					: type '<' types'>'								{ $$ = cdk::functional_type::create(*$3, $1); }
										| type '<' '>'											  { std::vector<std::shared_ptr<cdk::basic_type>> *input = new std::vector<std::shared_ptr<cdk::basic_type>>();
            																$$ = cdk::functional_type::create(*input , $1); }
								;

integer							: tINTEGER									{ $$ = new cdk::integer_node(LINE, $1); }
								;

double							: tDOUBLE										{ $$ = new cdk::double_node(LINE, $1); }
								;

string							: tSTRING										{ $$ = $1; }
								| string tSTRING						{ $$ = $1; $$->append(*$2); delete $2;}
								;

lval 							: tIDENTIFIER             			{ $$ = new cdk::variable_node(LINE, $1); }
     							| lval '[' expr ']'					{ $$ = new mml::index_node(LINE, new cdk::rvalue_node(LINE, $1), $3); }
		 						| '(' expr ')' '[' expr ']'          { $$ = new mml::index_node(LINE, $2, $5); }
								;

%%
