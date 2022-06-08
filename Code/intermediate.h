#pragma once
#include "semantics.h"
#include "node.h"

extern HNode *hashTable[HASH_SIZE];
typedef struct Operand_ *Operand;
typedef struct Arg_List_ * Arg_List;
struct Operand_
{
    enum{ VARIABLE_OP, CONSTANT_OP, ADDRESS_OP,
     FUNCTION_OP, LABEL_OP, TEMP_VAR_OP, TEMP_ADDR_OP , RELOP_OP} kind;
    struct{
        int var_no; //VARIBLE  ADDRESS
        int temp_no; //TEMP_VAR TEMP_ADDR
        int lable_no; //LABEL
        int value;  // CONSTANT(int)
        int is_addr;//0:not addr 1:is addr   To judge array and struct
        int for_add_addr; //1: add 0: not add addr
        char* name; // FUNCTION ADDRESS RELOP TEMP_ADDR
        //int is_addr; //0:not addr 1:is addr   To judge array and struct
    } u;
};


//VARIBLE V1 V2
//ADDRESS &V1 &V2
//TEMP_VAR T1 T2
//TEMP_ADDR &T1 &T2


struct InterCode
{
    int block_num;
    enum{ 
        //1 OP
        IC_FUNCTION,IC_PARAM,IC_RETURN,IC_LABEL,IC_GOTO,IC_ARG,
        IC_WRITE,IC_READ,
        //2 OP
        IC_ASSIGN,IC_CALL,
        IC_GET_ADDR, IC_ARRAY_ASSIGN,
        //FOR ARRAY(LAB4)
        IC_INTO_ADDR_RIGHT,IC_INTO_ADDR_LEFT,IC_ADDR_LEFT_RIGHT,
        //3 OP
        IC_ADD,IC_SUB,IC_MUL,IC_DIV,
        //4 OP
        IC_IFGOTO,
        //2 OP
        IC_DEC

        // IC_GET_ADDR,IC_INTO_ADDR_RIGHT,IC_INTO_ADDR_LEFT,

        } kind;

    union {
        struct
        {
            Operand op;
        } one;

        struct
        {
            Operand left,right;
        } two;

        struct
        {
             Operand result,op1,op2;
        }three;

        struct
        {
            Operand relop,op1,op2,label;
        }ifgoto;

        struct 
        {
            Operand op;
            int size;
        }dec;
        
    } u;
};


struct InterCodes
{
    struct InterCode code;
    struct InterCodes *prev;
    struct InterCodes *next;
};

struct Arg_List_{
    Operand op;
	struct Arg_List_ *next;
};



void add_intercodes(struct InterCodes* new_intercodes);
void create_one_intercode(Operand op, int kind);
void create_two_intercode(Operand left, Operand right, int kind);
void create_three_intercode(Operand result, Operand op1, Operand op2, int kind);
void create_four_intercode(Operand result, Operand op1, Operand op2,Operand op3, int kind);
void create_dec_intercode(Operand op, int size);
int get_size(Type* type);
Operand new_temp();
Operand new_label();
void translate_EXP(Node* exp, Operand place);
void get_args(Node* arg, Arg_List *head);
void copy_array(Operand op1,Operand op2, int size);
void translate_Cond(Node* exp, Operand label_true, Operand label_false);
void translate_StmtList(Node* stmtlist);
void translate_VarDec(Node* vardec, Operand place);
void translate_Dec(Node* dec);
void translate_DecList(Node* declist);
void translate_Def(Node* def);
void translate_DefList(Node* deflist);
void translate_CompSt(Node* compst);
void translate_Stmt(Node* stmt);
void translate_ParamDec(Node* paramdec);
void translate_VarList(Node* varlist);
void translate_FunDec (Node* fundec);
void translate_ExtDef(Node* extdef);
void translate_ExtDefList(Node* te);
void translate_Program(Node* program);
void print_op_to_file(FILE* fp, Operand op);
void print_arg_op_to_file(FILE* fp, Operand op);
void output_code(FILE* fp);
void get_translate_program(Node* root,char*filename);
void print_op(Operand op);
void print_arg_op(Operand op);
void print_intercode();
void translate_para_VarDec(Node* vardec, Operand place);