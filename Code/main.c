#include <stdio.h>
#include "node.h"
#include "semantics.h"
#include "intermediate.h"
#include "objectCode.h"

extern void yyrestart(FILE *);
extern int yyparse();
extern int yylineo;
Node* Root = NULL;
int error_count = 0;
int semantic_error = 0;
int lex_error_line = -1;
int fail_to_translate = 0;
HNode* hashTable[HASH_SIZE];

int main(int argc, char **argv)
{
    if (argc <= 2)
        return 1;
    FILE *f = fopen(argv[1], "r");
    if (!f)
    {
        perror(argv[1]);
        return 1;
    }
    char* code_out_file_name=argv[2];
    
    
    yylineno=1;
    yyrestart(f);
    yyparse();

    if(error_count==0){
        //print_tree(Root);
        //printf("\n semantic_check:\n");
        semantic_check(Root);
        if(semantic_error!=0)
            return 0;
        if(fail_to_translate!=0){
            printf("Cannot translate: Code contains variables of multi-dimensional array type or parameters of array type.\n");
            return 0;
        }
        get_translate_program(Root,code_out_file_name);
        get_object_code(code_out_file_name);
    }
    return 0;
}

