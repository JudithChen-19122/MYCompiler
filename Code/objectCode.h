#pragma once
#include "intermediate.h"


typedef struct Stack_node* Stnode;
typedef struct VarDesc* Var;

struct Register{
	int isfree; //1:free 0:used 
	int age; //记录年龄，年龄最大的最可能被释放
	char *name; //寄存器名称 便于打印
	struct VarDesc* op;//寄存器中存储的对象
};

struct VarDesc
{
	int offset;//栈中偏移量
	int reg_no;//寄存器编号
	Operand op;//变量对象
	struct VarDesc* next;
};

void init_register();
void object_code_head(FILE *fp);
void handle_function(struct InterCodes* x, FILE* fp);
void handle_assign(struct InterCodes* x, FILE* fp);
void handle_add(struct InterCodes* x, FILE* fp);
void handle_sub(struct InterCodes* x, FILE* fp);
void handle_mul(struct InterCodes* x, FILE* fp);
void handle_div(struct InterCodes* x, FILE* fp);
void handle_get_addr(struct InterCodes* x, FILE* fp);
void handle_ifgoto(struct InterCodes* x, FILE* fp);
void handle_return(struct InterCodes* x, FILE* fp);
void handle_dec(struct InterCodes* x, FILE* fp);
void handle_arg(struct InterCodes* x, FILE* fp);
void handle_call(struct InterCodes* x, FILE* fp);
void handle_param(struct InterCodes* x, FILE* fp);
void handle_read(struct InterCodes* x, FILE* fp);
void handle_write(struct InterCodes* x, FILE* fp);
void handle_array_assign(struct InterCodes* x, FILE* fp);
void transfer_code(FILE* fp);
void get_object_code(char* filename);
void add_var_to_mem(Operand op);
void del_var_of_mem(Var dvar);
void add_var(int reg_no, Var new_var);
void del_var(Var dvar);
int compare_op(Operand op1, Operand op2);
int ensure(FILE* fp, Operand op);
int allocate(Operand op, FILE* fp);
Var check_if_in_mem(Operand op);
void push_reg(FILE* fp);
void pop_reg(FILE* fp);





