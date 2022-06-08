%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "node.h"
    #include "lex.yy.c"
    extern Node* Root;
    extern int error_count;
    extern int lex_error_line;
    int yyerror(char const *msg);
%}
/* declared types */
%union {
  struct TNode* node;
}

/* declared tokens */
/*
%token <node> SEMI COMMA ASSIGNOP RELOP PLUS MINUS  STAR DIV AND OR NOT TYPE DOT
%token <node> LP RP LB RB LC RC 
%token <node> STRUCT RETURN IF ELSE WHILE 
%token <node> INT FLOAT ID
*/
%token <node> INT FLOAT ID SEMI COMMA ASSIGNOP RELOP PLUS MINUS STAR DIV 
%token <node> AND OR DOT NOT TYPE LP RP LB RB LC RC STRUCT RETURN IF ELSE WHILE

/* declared non-terminals */
/*
%type <node> Program ExtDefList ExtDef ExtDecList Specifier FunDec CompSt VarDec  
%type <node> StructSpecifier OptTag DefList Tag ParamDec
%type <node> VarList StmtList Stmt Def Dec DecList Args Exp
*/

%start Program
%right ASSIGNOP
%left OR
%left AND 
%left  RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT 
%left  DOT 
%left LB RB 
%left LP RP

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE


//non-terminal type
%type <node> Program ExtDefList ExtDef ExtDecList Specifier
%type <node> StructSpecifier OptTag Tag VarDec FunDec VarList
%type <node> ParamDec CompSt StmtList Stmt DefList Def DecList
%type <node> Dec Exp Args


%%
//High-level Definitions
Program     :  ExtDefList { $$=createNode("Program",""); load_child(1,$$,$1);Root=$$;}
            ;

ExtDefList  :  ExtDef ExtDefList { $$=createNode("ExtDefList",""); load_child(2,$$,$1,$2);}
            | /*empty*/ { $$ = NULL;}
            ;

ExtDef      :  Specifier ExtDecList SEMI {$$=createNode("ExtDef",""); load_child(3,$$,$1,$2,$3);}
            //|  Specifier error SEMI {}//{printf("ExtDef :Specifier error SEMI\n");}
            |  Specifier SEMI  {$$=createNode("ExtDef",""); load_child(2,$$,$1,$2);}
            |  Specifier error {}//{printf("ExtDef :Specifier error\n");}
            |  Specifier FunDec CompSt {$$=createNode("ExtDef",""); load_child(3,$$,$1,$2,$3);}
            |  error SEMI {}//{printf("ExtDef :error SEMI\n");}
            ;

ExtDecList  :  VarDec {$$=createNode("ExtDecList",""); load_child(1,$$,$1);}
            |  VarDec COMMA ExtDecList {$$=createNode("ExtDecList",""); load_child(3,$$,$1,$2,$3);}
            |  VarDec COMMA error  {}//{printf("ExtDef :VarDec COMMA error\n");}
            ;

//Specifiers
Specifier   :  TYPE {$$=createNode("Specifier",""); load_child(1,$$,$1);}
            |  StructSpecifier {$$=createNode("Specifier",""); load_child(1,$$,$1);}
            ;

StructSpecifier :  STRUCT OptTag LC DefList RC {$$=createNode("StructSpecifier",""); load_child(5,$$,$1,$2,$3,$4,$5);}
                |  STRUCT Tag {$$=createNode("StructSpecifier",""); load_child(2,$$,$1,$2);}
                |  STRUCT OptTag LC error RC {}//
                ;

OptTag      :  ID {$$=createNode("OptTag",""); load_child(1,$$,$1);}
            | /*empty*/ {$$=NULL;}
            ;

Tag         : ID {$$=createNode("Tag",""); load_child(1,$$,$1);}
            ;

//Declarators
VarDec      : ID                            {$$=createNode("VarDec",""); load_child(1,$$,$1);}
            | VarDec LB INT RB              {$$=createNode("VarDec",""); load_child(4,$$,$1,$2,$3,$4);}
            ;
            
FunDec      : ID LP VarList RP              {$$=createNode("FunDec",""); load_child(4,$$,$1,$2,$3,$4);}
            //| ID LP error RP                {}//{printf("FunDec :ID LP error RP\n");}
            | error RP                      {}//{printf("FunDec :error RP\n");}
            | ID LP RP                      {$$=createNode("FunDec",""); load_child(3,$$,$1,$2,$3);}
            ;
            
VarList     : ParamDec COMMA VarList        {$$=createNode("VarList",""); load_child(3,$$,$1,$2,$3);}
            | ParamDec                      {$$=createNode("VarList",""); load_child(1,$$,$1);}
            ;
            
ParamDec    : Specifier VarDec              {$$=createNode("ParamDec",""); load_child(2,$$,$1,$2);}
            ;

//Statements
CompSt      : LC DefList StmtList RC        {$$=createNode("CompSt",""); load_child(4,$$,$1,$2,$3,$4);}
            //| LC DefList error RC           {}//{printf("CompSt: LC DefList error RC\n");}
            | LC error StmtList RC          {}//{printf("CompSt: LC error StmtList RC\n");}
            | error SEMI                    {}//{printf("CompSt: error SEMI\n");}
            | error RC                      {}//
            ;
StmtList    : Stmt StmtList                 {$$=createNode("StmtList",""); load_child(2,$$,$1,$2);}
            | /*empty*/                     {$$=NULL;}
            ;
            
Stmt    : Exp SEMI                                      {$$=createNode("Stmt",""); load_child(2,$$,$1,$2);}
        | Exp error                                     {}//{printf("Stmt: Exp error\n");}
        //| error SEMI                                    {}//{printf("Stmt: error SEMI\n");}
        | CompSt                                        {$$=createNode("Stmt",""); load_child(1,$$,$1);}
        | RETURN Exp SEMI                               {$$=createNode("Stmt",""); load_child(3,$$,$1,$2,$3);}          
        | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE       {$$=createNode("Stmt",""); load_child(5,$$,$1,$2,$3,$4,$5);}
        | IF LP error RP Stmt %prec LOWER_THAN_ELSE     {}
        | IF LP Exp RP Stmt ELSE Stmt                   {$$=createNode("Stmt",""); load_child(7,$$,$1,$2,$3,$4,$5,$6,$7);}
        | IF LP error RP Stmt ELSE Stmt                 {}//
        | WHILE LP Exp RP Stmt                          {$$=createNode("Stmt",""); load_child(5,$$,$1,$2,$3,$4,$5);}
        | WHILE LP error RP Stmt                        {}//
        ;

//Local Definitions

DefList : Def DefList               {$$=createNode("DefList",""); load_child(2,$$,$1,$2);}
        | /*empty*/                 {$$=NULL;}
        ;
        
Def     : Specifier DecList SEMI    {$$=createNode("Def",""); load_child(3,$$,$1,$2,$3);}
        | Specifier error           {}//{printf("Def: Specifier error \n");}
        | Specifier error SEMI      {}//{printf("Def: Specifier error SEMI \n");}
        ;

DecList : Dec                       {$$=createNode("DecList",""); load_child(1,$$,$1);}
        | Dec COMMA DecList         {$$=createNode("DecList",""); load_child(3,$$,$1,$2,$3);}
        ;
        
Dec     : VarDec                    {$$=createNode("Dec",""); load_child(1,$$,$1);}
        | VarDec ASSIGNOP Exp       {$$=createNode("Dec",""); load_child(3,$$,$1,$2,$3);}
        ;
            
            
//Expressions
Exp     : Exp ASSIGNOP Exp      {$$=createNode("Exp",""); load_child(3,$$,$1,$2,$3);}
        | Exp AND Exp           {$$=createNode("Exp",""); load_child(3,$$,$1,$2,$3);}
        | Exp OR Exp            {$$=createNode("Exp",""); load_child(3,$$,$1,$2,$3);}
        | Exp RELOP Exp         {$$=createNode("Exp",""); load_child(3,$$,$1,$2,$3);}
        | Exp PLUS Exp          {$$=createNode("Exp",""); load_child(3,$$,$1,$2,$3);}
        | Exp MINUS Exp         {$$=createNode("Exp",""); load_child(3,$$,$1,$2,$3);}
        | Exp STAR Exp          {$$=createNode("Exp",""); load_child(3,$$,$1,$2,$3);}
        | Exp DIV Exp           {$$=createNode("Exp",""); load_child(3,$$,$1,$2,$3);}
        | LP Exp RP             {$$=createNode("Exp",""); load_child(3,$$,$1,$2,$3);}
        | MINUS Exp             {$$=createNode("Exp",""); load_child(2,$$,$1,$2);}
        | NOT Exp               {$$=createNode("Exp",""); load_child(2,$$,$1,$2);}
        | ID LP Args RP         {$$=createNode("Exp",""); load_child(4,$$,$1,$2,$3,$4);}
        | ID LP RP              {$$=createNode("Exp",""); load_child(3,$$,$1,$2,$3);}
        | Exp LB Exp RB         {$$=createNode("Exp",""); load_child(4,$$,$1,$2,$3,$4);}
        | Exp DOT ID            {$$=createNode("Exp",""); load_child(3,$$,$1,$2,$3);}
        | ID                    {$$=createNode("Exp",""); load_child(1,$$,$1);}
        | INT                   {$$=createNode("Exp",""); load_child(1,$$,$1);}
        | FLOAT                 {$$=createNode("Exp",""); load_child(1,$$,$1);}
        ;
    
Args        : Exp COMMA Args        {$$=createNode("Args",""); load_child(3,$$,$1,$2,$3);}
            | Exp                   {$$=createNode("Args",""); load_child(1,$$,$1);}
            ;

%%
int yyerror(char const *msg){
        //printf("debug: %d %d \n",lex_error_line,yylineno);
        if(lex_error_line != yylineno){
                error_count++;
                printf("Error type B at line %d: %s\n", yylineno, msg);
        }
}


        