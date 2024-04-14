%option c++ prefix="mml_scanner_" outfile="mml_scanner.cpp"
%option stack noyywrap yylineno 8bit debug
%{ 
// make relevant includes before including the parser's tab file
#include <string>
#include <sstream>
#include <cdk/ast/sequence_node.h>
#include <cdk/ast/expression_node.h>
#include <cdk/ast/lvalue_node.h>
#include "mml_parser.tab.h"

static std::ostringstream strlit;

// don't change this
#define yyerror LexerError
%}

SPACE     ([ \t\r]|\n)  

%x X_COMMENT
%x X_STRING X_BACKSLASH
%x X_OCTAL   X_OCT_INT
%%
  yydebug=1; set_debug(1);

  /* ====================================================================== */
  /* ====[                 Operadores de expressões               ]==== */
  /* ====================================================================== */

[\[\](),?]       return *yytext;
[-+*/%~<>@?=]        return *yytext;
">="                   return tGE;
"<="                   return tLE;
"=="                   return tEQ;
"!="                   return tNE;
"->"									 return tRETURNS;
"&&"                   return tAND;
"||"                   return tOR;
"input"                return tREAD; 

  /* ====================================================================== */
  /* ====[              Delimitadores e separadores             ]==== */
  /* ====[              Corpo (de função)                       ]==== */
  /* ====[                 Instrução de retorno                 ]==== */
  /* ====================================================================== */
"return"               return tRETURN;
"!"                    return tPRINT; 
"!!"									 return tPRINTLN;
[#\[\](){};:,]                return *yytext;

"while"                return tWHILE;
"stop"                 return tSTOP;
"next"                 return tNEXT;

"if"                   return tIF;
"elif"								 return tELIF;
"else"                 return tELSE;

"begin"                return tBEGIN;
"end"                  return tEND;
"public"							 return tPUBLIC;
"foreign"							 return tFOREIGN;
"forward"							 return tFORWARD;
"sizeof"               return tSIZEOF;
"null"								 return tNULL;


  /* ====================================================================== */
  /* ====[                   Tipos de dados                    ]==== */
  /* ====================================================================== */
"int"									 return tINTEGER_TYPE;
"double"							 return tDOUBLE_TYPE;
"string"							 return tSTRING_TYPE;
"void"								 return tVOID_TYPE;
"auto"								 return tAUTO_TYPE;

  /* ====================================================================== */
  /* ====[                Identificadores (nomes)               ]==== */
  /* ====================================================================== */
[A-Za-z_]([A-Za-z_]|[0-9])*  yylval.s = new std::string(yytext); return tIDENTIFIER;

  /* ====================================================================== */
  /* ====[                   inteiros                         ]==== */
  /* ====================================================================== */

0[0-7]+                {try {
                          yylval.i = std::stoi(yytext, nullptr, 8); 
                          return tINTEGER; 
                        } 
                        catch (const std::out_of_range& e){
                          yyerror("Base 8 out of range.");
                          } 
                        }

0[0-9]+                yyerror("Invalid base 8 number.");

0|[1-9][0-9]*          {try {
                          yylval.i = std::stoi(yytext, nullptr, 10); 
                          return tINTEGER;
                        } 
                        catch (const std::out_of_range& e) {
                          yyerror("Integer is out of range");
                          }
                        }


 /* ====================================================================== */
  /* ====[                   reais                            ]==== */
  /* ====================================================================== */

([0-9]*\.[0-9]+|[0-9]+\.[0-9]*)([eE][-+]?[0-9]+)? {try{
                                                    yylval.d = std::stol(yytext, nullptr, 10);
                                                    return tDOUBLE;
                                                  }
                                                  catch (const std::out_of_range& e) {
                                                    yyerror("Double is out of range");
                                                    }
                                                  }
  /* ====================================================================== */
  /* ====[                      COMMENTS                        ]==== */
  /* ====================================================================== */
"//".*$                /* match the whole line (it is ignored) */

"/*"                   yy_push_state(X_COMMENT);
<X_COMMENT>"*/"        yy_pop_state();
<X_COMMENT>.|"\n"      ;  /* ignore inside comments */

  /* ====================================================================== */
  /* ====[              Cadeias de caracteres                 ]==== */
  /* ====================================================================== */

"\""                           yy_push_state(X_STRING);
<X_STRING>\\                   yy_push_state(X_BACKSLASH);
<X_STRING>"\""                 {
                                 yylval.s = new std::string(strlit.str());
                                 strlit.str("");
                                 yy_pop_state();
                                 return tSTRING;
                               }
<X_STRING>.                    strlit << *yytext;

<X_BACKSLASH>n                 strlit << '\n'; yy_pop_state();
<X_BACKSLASH>r                 strlit << '\r'; yy_pop_state();
<X_BACKSLASH>t                 strlit << '\t'; yy_pop_state();
<X_BACKSLASH>\\                strlit << '\\'; yy_pop_state();

<X_BACKSLASH>[0-7]{1,3} {
                                 strlit << (char)(unsigned char)std::stoi(yytext, nullptr, 8);
                                 yy_pop_state();
                               }
<X_BACKSLASH>.                 strlit << *yytext; yy_pop_state();


  /* ====================================================================== */
  /* ====[                    (other characters)                      ]==== */
  /* ====================================================================== */
[[:space:]]

.                      yyerror("Unknown character");

%%
