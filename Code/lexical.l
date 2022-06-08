%{
    #include<stdio.h>
    #include "syntax.tab.h" 
    #include "node.h"
    extern int error_count;
    extern int lex_error_line;
%}

%option yylineno

SEMI ;
COMMA ,
ASSIGNOP =
RELOP >|<|>=|<=|==|!=
PLUS "+"
MINUS "-"
STAR "*"
DIV "/"
AND "&&"
OR "||"
DOT "."
NOT "!"
TYPE int|float
LP "("
RP ")"
LB "["
RB "]"
LC "{"
RC "}"
STRUCT struct
RETURN return
IF if
ELSE else
WHILE while

SPACE [ \r\n\t]+

DEC 0|([1-9][0-9]*)
OCT 0[0-7]+
ERROR_OCT 0[0-7]*[8-9]+[0-7]*
HEX 0[xX][0-9a-fA-F]+
ERROR_HEX 0[xX][0-9a-fA-F]*[g-zG-Z]+[0-9a-fA-F]*

COMMON_FLOAT [0-9]+\.[0-9]+
ERROR_FLOAT (\.[0-9]+|[0-9]\.|[0-9]+\.[0-9]+)[eE][+-]?
SCIENTIFIC_FLOAT (\.[0-9]+|[0-9]+\.|[0-9]+\.[0-9]+)[eE][+-]?[0-9]+

INT {DEC}|{OCT}|{HEX}  
ERROR_INT {ERROR_OCT}|{ERROR_HEX}
FLOAT {COMMON_FLOAT}|{SCIENTIFIC_FLOAT}
ID [A-Za-z_][A-Za-z0-9_]*
ERROR_ID [0-9]+[A-Za-z_][A-Za-z0-9_]*

%%

{SEMI} { yylval.node = createNode("SEMI",yytext); return SEMI;}
{COMMA} { yylval.node = createNode("COMMA",yytext); return COMMA;}
{ASSIGNOP} { yylval.node = createNode("ASSIGNOP",yytext); return ASSIGNOP;}
{RELOP} { yylval.node = createNode("RELOP",yytext); return RELOP;}
{PLUS} { yylval.node = createNode("PLUS",yytext); return PLUS;}
{MINUS} { yylval.node = createNode("MINUS",yytext); return MINUS;}
{STAR} { yylval.node = createNode("STAR",yytext); return STAR;}
{DIV} { yylval.node = createNode("DIV",yytext); return DIV;}
{AND} { yylval.node = createNode("AND",yytext); return AND;}
{OR} { yylval.node = createNode("OR",yytext); return OR;}
{DOT} { yylval.node = createNode("DOT",yytext); return DOT;}
{NOT} { yylval.node = createNode("NOT",yytext); return NOT;}
{TYPE} { yylval.node = createNode("TYPE",yytext); return TYPE;}
{LP} { yylval.node = createNode("LP",yytext); return LP;}
{RP} { yylval.node = createNode("RP",yytext); return RP;}
{LB} { yylval.node = createNode("LB",yytext); return LB;}
{RB} { yylval.node = createNode("RB",yytext); return RB;}
{LC} { yylval.node = createNode("LC",yytext); return LC;}
{RC} { yylval.node = createNode("RC",yytext); return RC;}
{STRUCT} { yylval.node = createNode("STRUCT",yytext); return STRUCT;}
{RETURN} { yylval.node = createNode("RETURN",yytext); return RETURN;}
{IF} { yylval.node = createNode("IF",yytext); return IF;}
{ELSE} { yylval.node = createNode("ELSE",yytext); return ELSE;} 
{WHILE} { yylval.node = createNode("WHILE",yytext); return WHILE;}


{SPACE} {}

{INT} { yylval.node = createNode("INT",yytext); return INT;}
{ERROR_INT} {lex_error_line=yylineno;error_count++;printf("Error type A at Line %d: Illegal integer number \'%s\'\n",yylineno, yytext); return INT;}
{FLOAT} { yylval.node = createNode("FLOAT",yytext); return FLOAT;}
{ERROR_FLOAT} {lex_error_line=yylineno;error_count++;printf("Error type A at Line %d: Illegal floating point number \'%s\'\n",yylineno, yytext); return FLOAT;}
{ID}  { yylval.node = createNode("ID",yytext); return ID;}
{ERROR_ID} {lex_error_line=yylineno;error_count++;printf("Error type A at Line %d: Illegal ID \'%s\'\n",yylineno, yytext); return ID;}

. {lex_error_line=yylineno;error_count++;printf("Error type A at Line %d: Mysterious characters \'%s\'\n",yylineno, yytext); }

%%

int yywrap() {
    return 1;
}