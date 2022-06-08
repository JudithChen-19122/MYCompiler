#include "objectCode.h"
#include "intermediate.h"
#include "semantics.h"

//中间代码双向链表
extern struct InterCodes* code_tail;
extern struct InterCodes* code_head;

struct VarDesc* var_head = NULL;	//	存在寄存器中的变量链表
struct VarDesc* var_tail = NULL;
struct VarDesc* var_head_mem = NULL;	//存在内存中的变量列表
struct VarDesc* var_tail_mem = NULL;

int cur_block=0;


int is_main = 1; //main 函数特别处理
char* func_name = NULL; //当前处理的函数
int cur_offset=0;
int param_reg_num=4;
int offset=0;//当前sp偏移量


//32个寄存器
struct Register reg[32];

void set_all_reg_free(FILE* fp){
	struct VarDesc* temp=var_head;
	while(temp!=NULL){
		if(temp->op->kind!=CONSTANT_OP){
			//寄存器内容写栈中
				int no = temp->reg_no;
				fprintf(fp, "  sw %s, -%d($fp)\n", reg[no].name,temp->offset);
				//printf("  sw %s, -%d($fp)\n", reg[no].name,temp->offset);
				//del_var(temp);//释放寄存器
		}
		else{
			//del_var(temp);
			;
		}
		temp = temp->next;
	}
	
	var_head=NULL;
	var_tail=NULL;

	for(int i=8;i<26;i++){
		reg[i].age = 0 ;
		reg[i].isfree = 1;
	}
}

void print_stack(){
	Var temp = var_head_mem;
	while(temp!=NULL){
	if(temp->op->kind==VARIABLE_OP)
		printf("v%d\n",temp->op->u.var_no);
	if(temp->op->kind==TEMP_VAR_OP)
		printf("t%d\n",temp->op->u.temp_no);
	temp=temp->next;
	}
}

void print_var_reg(){
	Var temp = var_head;
	while(temp!=NULL){
	if(temp->op->kind==VARIABLE_OP)
		printf("v%d\n",temp->op->u.var_no);
		printf("%d\n",temp->reg_no);
	if(temp->op->kind==TEMP_VAR_OP)
		printf("t%d\n",temp->op->u.temp_no);
	temp=temp->next;
	}
}


Var copy_var(Var old){
	Var new_var = (Var)malloc(sizeof(struct VarDesc));
	new_var->next=NULL;
	new_var->op=old->op;
	new_var->reg_no = old->reg_no;
	new_var->offset=old->offset;
	return new_var;
}


int init_stack(struct InterCodes* the_code){
	struct InterCodes* x=the_code->next;
	int next_func=0;
	while(x != NULL)
    {
    	switch(x->code.kind)
    	{
    		case IC_FUNCTION:
				next_func=1;
				break;
    		case IC_PARAM:
			case IC_RETURN:
			case IC_ARG:
			case IC_WRITE:
			case IC_READ:
				add_var_to_mem(x->code.u.one.op);
				break;
			case IC_ASSIGN:
			case IC_GET_ADDR:
			case IC_ARRAY_ASSIGN:
				add_var_to_mem(x->code.u.two.left);
				add_var_to_mem(x->code.u.two.right);
				break;
			case IC_ADD:
			case IC_SUB:
			case IC_MUL:
			case IC_DIV:
				add_var_to_mem(x->code.u.three.op1);
				add_var_to_mem(x->code.u.three.op2);
				add_var_to_mem(x->code.u.three.result);
				break;
			case IC_IFGOTO:
				add_var_to_mem(x->code.u.ifgoto.op1);
				add_var_to_mem(x->code.u.ifgoto.op2);
				break;
			case IC_INTO_ADDR_RIGHT:
			case IC_INTO_ADDR_LEFT:
				add_var_to_mem(x->code.u.two.left);
				add_var_to_mem(x->code.u.two.right);
			//case IC_DEC: // dec array_name size 
				//add_var_to_mem(x->code.u.dec.op);
				//offset -= 4;
				//offset += x->code.u.dec.size;
				break;
    	}
		if(next_func){
			return offset;
		}
        x = x->next;
    }


}

void init_register(){
	//set free
	for(int i=0;i<32;i++){
		reg[i].isfree=1;
		reg[i].age=0;
	}
	//set name for output
	reg[0].name="$zero";
	reg[1].name="$at";

	reg[2].name="$v0";
	reg[3].name="$v1";

	reg[4].name="$a0";
	reg[5].name="$a1";
	reg[6].name="$a2";
	reg[7].name="$a3";

	reg[8].name="$t0";
	reg[9].name="$t1";
	reg[10].name="$t2";
	reg[11].name="$t3";
	reg[12].name="$t4";
	reg[13].name="$t5";
	reg[14].name="$t6";
	reg[15].name="$t7";

	reg[16].name="$s0";
	reg[17].name="$s1";
	reg[18].name="$s2";
	reg[19].name="$s3";
	reg[20].name="$s4";
	reg[21].name="$s5";
	reg[22].name="$s6";
	reg[23].name="$s7";

	reg[24].name="$t8";
	reg[25].name="$t9";

	reg[26].name="$k0";
	reg[27].name="$k1";
	reg[28].name="$gp";
	reg[29].name="$sp";

	reg[30].name="$fp";
	reg[31].name="$ra";
}

void object_code_head(FILE *fp){
    //data
    fputs(".data\n", fp);
	fputs("_prompt: .asciiz \"Enter an integer:\"\n", fp);
	fputs("_ret: .asciiz \"\\n\"\n", fp);
	fputs(".globl main\n", fp);

	//将数组存在.data段方便访问
	struct InterCodes* x = code_head;
    while(x != NULL)
    {
    	if(x->code.kind==IC_DEC)
    	{
    		fprintf(fp, "v%d: .space %d\n",x->code.u.dec.op->u.var_no, x->code.u.dec.size);
    	} 			
        x = x->next;
    }

	//代码段开始
	fputs(".text\n\n", fp);

    //read
	fputs("read:\n", fp);
	fputs("  li $v0, 4\n", fp);
	fputs("  la $a0, _prompt\n", fp);
	fputs("  syscall\n", fp);
	fputs("  li $v0, 5\n", fp);
	fputs("  syscall\n", fp);
	fputs("  jr $ra\n\n", fp);
    //write
	fputs("write:\n", fp);
	fputs("  li $v0, 1\n", fp);
	fputs("  syscall\n", fp);
	fputs("  li $v0, 4\n", fp);
	fputs("  la $a0, _ret\n", fp);
	fputs("  syscall\n", fp);
	fputs("  move $v0, $0\n", fp);
	fputs("  jr $ra\n\n", fp);
}

void handle_function(struct InterCodes* x, FILE* fp){
	fprintf(fp,"%s",x->code.u.one.op->u.name);
	fprintf(fp, ":\n");

	var_head=NULL;
	var_tail=NULL;
	var_head_mem=NULL;
	var_tail_mem=NULL;
	offset = 0;
	
	for(int i=8;i<26;i++)
	{
		reg[i].isfree=1;
		reg[i].age=0;
	}

	struct InterCodes* temp_code=x;
	init_stack(temp_code);

	if(strcmp(x->code.u.one.op->u.name,"main")==0)
	{
		is_main = 1;
		func_name = NULL;
		fprintf(fp,"  move $fp, $sp\n");
		fprintf(fp, "  addi $sp, $sp, -%d\n", offset);
	}
	else{
		is_main = 0;
		func_name = x->code.u.one.op->u.name;
		fprintf(fp, "  addi $sp, $sp, -%d\n", offset);
	}
	if(!is_main)
	{
		//添加函数参数
		SNode func_temp = check_symbol_table_by_name(func_name);
		int paranum = func_temp->type->u.function_s->para_num;
		int t=0;
		struct InterCodes* temp = x->next;
		while(temp != NULL && temp->code.kind==IC_PARAM)
		{ 			
			//部分在寄存器中
			if(t<4){
				Var temp_var = check_if_in_mem(temp->code.u.one.op);
				add_var(4 + t, temp_var);
			}
			else //其余在栈中
			{
				//取出，存入寄存器
				int reg_no = ensure(fp, temp->code.u.one.op); 
				fprintf(fp, "  lw %s, %d($fp)\n", reg[reg_no].name, (paranum-1-t)*4 + 72);
			}
			t++;
			temp = temp->next;
		}
	}
}

void handle_assign(struct InterCodes* x, FILE* fp){ //left=right
	Operand left = x->code.u.two.left;
    Operand right = x->code.u.two.right;  
    if(right->kind == CONSTANT_OP)
    {
    	int left_no = ensure(fp, left);
    	fprintf(fp, "  li %s, %d\n", reg[left_no].name, right->u.value);
    } 	
    else
    {
    	int left_no = ensure(fp, left);
    	int right_no = ensure(fp, right);
    	fprintf(fp, "  move %s, %s\n", reg[left_no].name, reg[right_no].name);   		
    }
}
void handle_add(struct InterCodes* x, FILE* fp){
		Operand result = x->code.u.three.result;
    	Operand op1 = x->code.u.three.op1;
    	Operand op2 = x->code.u.three.op2;
		if(op1->kind == CONSTANT_OP && op2->kind == CONSTANT_OP)
		{
			int result_no = ensure(fp, result);
			fprintf(fp, "  li %s, %d\n", reg[result_no].name, op1->u.value+op2->u.value);
		}   
    	else if(op1->kind != CONSTANT_OP && op2->kind == CONSTANT_OP)
    	{
 			int op1_no = ensure(fp, op1);
			int result_no = ensure(fp, result);
			fprintf(fp, "  addi %s, %s, %d\n",reg[result_no].name,reg[op1_no].name,op2->u.value);   		
    	}
    	else
    	{
			int op1_no = ensure(fp, op1);
			int op2_no = ensure(fp, op2);
			int result_no = ensure(fp, result);
			fprintf(fp, "  add %s, %s, %s\n",reg[result_no].name,reg[op1_no].name,reg[op2_no].name);
    	}
}

void handle_sub(struct InterCodes* x, FILE* fp){
		Operand result = x->code.u.three.result;
    	Operand op1 = x->code.u.three.op1;
    	Operand op2 = x->code.u.three.op2;
		if(op1->kind == CONSTANT_OP && op2->kind == CONSTANT_OP)
		{
			int result_no = ensure(fp, result);
			fprintf(fp, "  li %s, %d\n", reg[result_no].name, op1->u.value - op2->u.value);
		}   
    	else if(op1->kind != CONSTANT_OP && op2->kind == CONSTANT_OP)
    	{
 			int op1_no = ensure(fp, op1);
			int result_no = ensure(fp, result);
			fprintf(fp, "  addi %s, %s, %d\n",reg[result_no].name,reg[op1_no].name,-op2->u.value);   		
    	}
    	else
    	{
			int op1_no = ensure(fp, op1);
			int op2_no = ensure(fp, op2);
			int result_no = ensure(fp, result);
			fprintf(fp, "  sub %s, %s, %s\n",reg[result_no].name,reg[op1_no].name,reg[op2_no].name);
    	}
}
void handle_mul(struct InterCodes* x, FILE* fp){
		Operand result = x->code.u.three.result;
    	Operand op1 = x->code.u.three.op1;
    	Operand op2 = x->code.u.three.op2;
		if(op1->kind == CONSTANT_OP && op2->kind == CONSTANT_OP)
		{
			int result_no = ensure(fp, result);
			fprintf(fp, "  li %s, %d\n", reg[result_no].name, op1->u.value * op2->u.value);
		}   
    	else
    	{
			int op1_no = ensure(fp, op1);
			int op2_no = ensure(fp, op2);
			int result_no = ensure(fp, result);
			fprintf(fp, "  mul %s, %s, %s\n",reg[result_no].name,reg[op1_no].name,reg[op2_no].name);
    	}
}
void handle_div(struct InterCodes* x, FILE* fp){
		Operand result = x->code.u.three.result;
    	Operand op1 = x->code.u.three.op1;
    	Operand op2 = x->code.u.three.op2;
		if(op1->kind == CONSTANT_OP && op2->kind == CONSTANT_OP)
		{
			int result_no = ensure(fp, result);
			fprintf(fp, "  li %s, %d\n", reg[result_no].name, op1->u.value / op2->u.value);
		}   
    	else
    	{
			int op1_no = ensure(fp, op1);
			int op2_no = ensure(fp, op2);
			int result_no = ensure(fp, result);
			fprintf(fp, "  div  %s, %s\n",reg[op1_no].name,reg[op2_no].name);
			fprintf(fp, "  mflo %s\n", reg[result_no].name);
		}
}

void handle_get_addr(struct InterCodes* x, FILE* fp){
	int left_no = ensure(fp, x->code.u.two.left);
    fprintf(fp, "  la %s, v%d\n", reg[left_no].name, x->code.u.two.right->u.var_no);
}

void handle_into_addr_right(struct InterCodes* x, FILE* fp)
{
	Operand left = x->code.u.two.left;
    Operand right = x->code.u.two.right;
	int left_num = ensure(fp, left);
    int right_num = ensure(fp, right);
    fprintf(fp, "  lw %s, 0(%s)\n", reg[left_num].name, reg[right_num].name);  
}

void handle_into_addr_left(struct InterCodes* x, FILE* fp)
{
	Operand left = x->code.u.two.left;
    Operand right = x->code.u.two.right;
	int left_num = ensure(fp, left);
    int right_num = ensure(fp, right);
    fprintf(fp, "  sw %s, 0(%s)\n", reg[right_num].name, reg[left_num].name); 
}


void handle_ifgoto(struct InterCodes* x, FILE* fp){
		Operand op1 = x->code.u.ifgoto.op1;
		Operand op_relop = x->code.u.ifgoto.relop;
		Operand op2 = x->code.u.ifgoto.op2;
		Operand op_label = x->code.u.ifgoto.label;
		
		int op1_no = ensure(fp, op1);
		int op2_no = ensure(fp, op2);

		if(strcmp(op_relop->u.name,"==")==0)
		{
			fprintf(fp, "  beq %s, %s, label%d\n",reg[op1_no].name, reg[op2_no].name, op_label->u.lable_no);
		}
		else if(strcmp(op_relop->u.name,"!=")==0)
		{
			fprintf(fp, "  bne %s, %s, label%d\n",reg[op1_no].name, reg[op2_no].name, op_label->u.lable_no);			
		}
		else if(strcmp(op_relop->u.name,">")==0)
		{
			fprintf(fp, "  bgt %s, %s, label%d\n",reg[op1_no].name, reg[op2_no].name, op_label->u.lable_no);			
		}
		else if(strcmp(op_relop->u.name,"<")==0)
		{
			fprintf(fp, "  blt %s, %s, label%d\n",reg[op1_no].name, reg[op2_no].name, op_label->u.lable_no);			
		}
		else if(strcmp(op_relop->u.name,">=")==0)
		{
			fprintf(fp, "  bge %s, %s, label%d\n",reg[op1_no].name, reg[op2_no].name, op_label->u.lable_no);			
		}
		else if(strcmp(op_relop->u.name,"<=")==0)
		{
			fprintf(fp, "  ble %s, %s, label%d\n",reg[op1_no].name, reg[op2_no].name, op_label->u.lable_no);			
		}				
}

void handle_return(struct InterCodes* x, FILE* fp){
	int reg_no= ensure(fp, x->code.u.one.op);
	fprintf(fp, "  move $v0, %s\n", reg[reg_no].name);
	fprintf(fp, "  jr $ra\n");
}

void handle_dec(struct InterCodes* x, FILE* fp){
	;
}

void handle_arg(struct InterCodes* x, FILE* fp){
	;
}

void handle_call(struct InterCodes* x, FILE* fp){
	param_reg_num=4;
	//被调用的函数
	SNode func_temp = check_symbol_table_by_name(x->code.u.two.right->u.name);
	int paranum = func_temp->type->u.function_s->para_num;
	int left_no = ensure(fp, x->code.u.two.left);
	//保存$ra
	fprintf(fp, "  addi $sp, $sp, -4\n");
	fprintf(fp, "  sw $ra, 0($sp)\n");	
	//保存通用寄存器
	//push_reg(fp);

	//保存调用者函数形参至栈
	if(!is_main){
		//调用者函数
		SNode out_func = check_symbol_table_by_name(func_name);
		int out_para_num = out_func->type->u.function_s->para_num;
		for(int i=0;i<out_para_num;i++){
			if(i<4){
				//将调用者使用的参数寄存器压入栈
				fprintf(fp, "  addi $sp, $sp, -4\n");
				fprintf(fp, "  sw %s, 0($sp)\n", reg[4+i].name);
				//寻找寄存器对应的参数
				struct VarDesc* p = var_head;
				while(p!=NULL)
				{
					if(p->reg_no==4+i)
						break;
					p=p->next;
				}
				if(p==NULL){
					//printf("error in handle call.\n");
				}
				else{
					;
					//del_var(p);
					//add_var_to_mem(p->op);
				}
			}
		}
	}
	

	//push $fp
	fprintf(fp, "  addi $sp, $sp, -4\n");
	fprintf(fp, "  sw $fp, 0($sp)\n");

	//传入参数
	struct InterCodes* tt = x->prev;
	while(tt!= NULL && tt->code.kind==IC_ARG)
	{
		int reg_num = ensure(fp, tt->code.u.one.op);
		if(param_reg_num<8)
		{
				fprintf(fp, "  move %s, %s\n", reg[param_reg_num].name, reg[reg_num].name);
				param_reg_num++;
		}
		else
		{
			//参数大于4个压栈
			int reg_num = ensure(fp, tt->code.u.one.op);
			fprintf(fp, "  addi $sp, $sp, -4\n");
			fprintf(fp, "  sw %s, 0($sp)\n", reg[reg_num].name);
			param_reg_num++;
		}	
		tt=tt->prev;	
	}


	//保存通用寄存器
	push_reg(fp);


	// $fp=$sp
	fprintf(fp, "  move $fp, $sp\n");


	fprintf(fp, "  jal %s\n",x->code.u.two.right->u.name);	

	//$sp=$fp
	fprintf(fp, "  move $sp, $fp\n");

	//保存通用寄存器
	pop_reg(fp);


	//恢复栈指针(传入参数)
	if(param_reg_num>8){
		fprintf(fp, "  addi $sp, $sp, %d\n", 4*(param_reg_num-8));
	}

	//恢复$fp
	fprintf(fp, "  lw $fp, 0($sp)\n");
	fprintf(fp, "  addi $sp, $sp, 4\n");

	//恢复形参寄存器
	if(!is_main){
		//调用者函数
		SNode out_func = check_symbol_table_by_name(func_name);
		int out_para_num = out_func->type->u.function_s->para_num;
		int temp_para;
		if(out_para_num>=4) temp_para=4;
		else temp_para = out_para_num;
		for(int i=0;i<out_para_num;i++){
			if(i<4){
				//将调用者使用的参数寄存器取出栈(注意倒序问题还未解决)
				fprintf(fp, "  lw %s, 0($sp)\n", reg[4 + temp_para - 1 - i].name);
				fprintf(fp, "   addi $sp, $sp, 4\n");
			}
		}
	}

	//恢复通用寄存器
	//pop_reg(fp);
	//恢复$ra
	fprintf(fp, "  lw $ra, 0($sp)\n");
	fprintf(fp, "  addi $sp, $sp, 4\n");
	//返回值赋值
	fprintf(fp, "  move %s, $v0\n", reg[left_no].name);
}

void handle_param(struct InterCodes* x, FILE* fp){
	;
}

void handle_read(struct InterCodes* x, FILE* fp){
	fprintf(fp, "  addi $sp, $sp, -4\n");
	fprintf(fp, "  sw $ra, 0($sp)\n");
	fprintf(fp, "  jal read\n");
	fprintf(fp, "  lw $ra, 0($sp)\n");
	fprintf(fp, "  addi $sp, $sp, 4\n");
	int no = ensure(fp, x->code.u.one.op);
	fprintf(fp, "  move %s, $v0\n", reg[no].name);
}

void handle_write(struct InterCodes* x, FILE* fp){
	if(is_main)
	{
		int reg_num = ensure(fp, x->code.u.one.op);
		fprintf(fp, "  move $a0, %s\n", reg[reg_num].name);
		fprintf(fp, "  addi $sp, $sp, -4\n");
		fprintf(fp, "  sw $ra, 0($sp)\n");
		fprintf(fp, "  jal write\n");
		fprintf(fp, "  lw $ra, 0($sp)\n");
		fprintf(fp, "  addi $sp, $sp, 4\n");
	}
	else
	{
		//函数内调用，先将a0压栈
		int reg_num = ensure(fp, x->code.u.one.op);
		fprintf(fp, "  addi $sp, $sp, -8\n");
		fprintf(fp, "  sw $a0, 0($sp)\n");
		fprintf(fp, "  sw $ra, 4($sp)\n");
		fprintf(fp, "  move $a0, %s\n", reg[reg_num].name);			
		fprintf(fp, "  jal write\n");
		fprintf(fp, "  lw $a0, 0($sp)\n");
		fprintf(fp, "  lw $ra, 4($sp)\n");			
		fprintf(fp, "  addi $sp, $sp, 8\n");		
	}
}

void handle_array_assign(struct InterCodes* x, FILE* fp){
	Operand left = x->code.u.two.left;
    Operand right = x->code.u.two.right;
	int left_num = ensure(fp, left);
    int right_num = ensure(fp, right);
	fprintf(fp, "  lw $v1, 0(%s)\n", reg[right_num].name);
	fprintf(fp, "  sw $v1, 0(%s)\n", reg[left_num].name);
}

void transfer_code(FILE* fp){
    struct InterCodes* x = code_head;
    while(x != NULL)
    {
		if(x->code.block_num!=cur_block){
			set_all_reg_free(fp);
			cur_block = x->code.block_num;
		}
    	switch(x->code.kind)
    	{
    		case IC_LABEL:
				fprintf(fp,"label%d",x->code.u.one.op->u.lable_no);
    			fprintf(fp,":\n");
				break;
    		case IC_FUNCTION:
				handle_function(x,fp);
				break;
    		case IC_ASSIGN:
				handle_assign(x,fp);
				break;
    		case IC_ADD:
				handle_add(x,fp);
				break;
    		case IC_SUB:
				handle_sub(x,fp);
				break;
    		case IC_MUL:
				handle_mul(x,fp);
				break;
    		case IC_DIV:
				handle_div(x,fp);
				break;
			case IC_GET_ADDR:
				handle_get_addr(x,fp);
				break;

			case IC_INTO_ADDR_RIGHT:
			   	handle_into_addr_right(x,fp);
    			break;			
			case IC_INTO_ADDR_LEFT:
				handle_into_addr_left(x,fp);
    			break;
            

    		case IC_GOTO:
				fprintf(fp, "  j label%d\n",x->code.u.one.op->u.lable_no);
				break;
    		case IC_IFGOTO:
				handle_ifgoto(x,fp);
				break;
    		case IC_RETURN:
				handle_return(x,fp);
				break;
    		case IC_DEC:
				handle_dec(x,fp);
				break;
    		case IC_ARG:
				handle_arg(x,fp);
				break;
    		case IC_CALL:
				handle_call(x,fp);
				break;
    		case IC_PARAM:
				handle_param(x,fp);
				break;
    		case IC_READ:
				handle_read(x,fp);
				break;
    		case IC_WRITE:
				handle_write(x,fp);
				break;
            case IC_ARRAY_ASSIGN:
				handle_array_assign(x,fp);
				break;		 			
    	}
        x = x->next;
    }

}


void get_object_code(char* filename){
	FILE* fp = fopen(filename, "wt+");
	if (!fp)
	{
		perror(filename);
		return ;
	}
    object_code_head(fp);
	init_register();
	transfer_code(fp);
	fclose(fp);
}

void add_var_to_mem(Operand op){
	if(check_if_in_mem(op)!=NULL) return; 
	offset += 4;
	Var new_var = (Var)malloc(sizeof(struct VarDesc));
	new_var->op=op;
	new_var->next=NULL;
	new_var->offset=offset;
	if(var_head_mem == NULL)
	{
		var_head_mem = new_var;
		var_tail_mem = new_var;
	}
	else
	{
		var_tail_mem->next = new_var;
		var_tail_mem = new_var;
	}
}

void del_var_of_mem(Var dvar){
	if(dvar == var_head_mem)
	{
		var_head_mem=var_head_mem->next;
	}
	else
	{
		Var temp = var_head_mem;
		while(temp!=NULL)
		{
			if(temp->next==dvar)
				break;
			temp=temp->next;
		}
		if(var_tail_mem == dvar)
			var_tail_mem = temp;
		temp->next = dvar->next;
		dvar->next = NULL;
	}

}

void add_var(int no, Var the_new_var){
	
	the_new_var->reg_no=no;
	Var new_var = copy_var(the_new_var); 

	if(var_head==NULL)
	{
		var_head = new_var;
		var_tail = new_var;
	}
	else
	{
		var_tail->next=new_var;
		var_tail=new_var;
	}
}

void del_var(Var dvar){

	if(dvar == var_head)
	{
		var_head=var_head->next;
	}
	else
	{
		Var temp = var_head;
		while(temp!=NULL)
		{
			if(temp->next==dvar)
				break;
			temp=temp->next;
		}
		if(var_tail == dvar)
			var_tail = temp;
		temp->next = dvar->next;
		dvar->next=NULL;
	}
}


//比较变量是否是同一变量
int compare_op(Operand op1, Operand op2){
	if(op1->kind!=op2->kind) return 0;
	if(op1->kind == VARIABLE_OP || op1->kind == ADDRESS_OP){
		if(op1->u.var_no==op2->u.var_no) return 1;
		else return 0;
	}
	if(op1->kind == TEMP_VAR_OP || op1->kind == TEMP_ADDR_OP){
		if(op1->u.temp_no==op2->u.temp_no) return 1;
		else return 0;
	}
	if(op1->kind == CONSTANT_OP){
		if(op1->u.value==op2->u.value) return 1;
		return 0;
	}
	return 0;
}

/*
1 Ensure(x):
2 if (x is already in register r)
3 result = r
4 else
5 result = Allocate(x)
6 emit MIPS32 code [lw result, x]
7 return result
*/

int ensure(FILE* fp, Operand op)
{
	for(int i=8;i<26;i++){
		reg[i].age++;
	}

	if(op->kind != CONSTANT_OP)
	{
		//若为变量，则查看变量描述符中是否已经存在
		struct VarDesc* p = var_head;
		while(p!=NULL)
		{
			//存在,直接返回寄存器
			if(p->op->kind != CONSTANT_OP && compare_op(p->op,op)==1){
				reg[p->reg_no].age=0;
				return p->reg_no;
			}
			p=p->next;
		}
		//不存在,新分配一个寄存器
		int result = allocate(op,fp);
		return result;
	}
	else 
	{
		//若为常量，直接分配一个新的寄存器
		int result = allocate(op,fp);
		fprintf(fp, "  li %s, %d\n", reg[result].name,op->u.value);
		return result;
	}
}

/*
Allocate(x):
10 if (there exists a register r that currently has not been assigned to
11 any variable)
12 result = r
13 else
14 result = the register that contains a value whose next use is farthest
15 in the future
16 spill result
17 return result
*/


Var check_if_in_mem(Operand op){
	struct VarDesc* temp = var_head_mem;
	while(temp!=NULL){
		if(compare_op(temp->op,op)){
			return temp;
		}
		temp=temp->next;
	}
	return NULL;
}

int allocate(Operand op, FILE* fp)
{
	//寻找空闲寄存器
	for(int i=8;i<26;i++)
	{
		if(reg[i].isfree)//找到空闲寄存器，存入
		{
			reg[i].isfree=0;
			reg[i].age=0;
			if(op->kind!=CONSTANT_OP){
				Var temp_var = check_if_in_mem(op);//检查是否存在栈中
				//在栈中,从栈中取出，放入寄存器
				if(temp_var==NULL){
					printf("error in allocate1\n");
					printf("op: v%d\n", op->u.var_no);
				}
				fprintf(fp, "  lw %s, -%d($fp)\n", reg[i].name, temp_var->offset);
				add_var(i,temp_var);
				return i;	
			}
			else{
				Var new_var = (Var)malloc(sizeof(struct VarDesc));
				new_var->op=op;
				new_var->next=NULL;
				new_var->offset=0;
				new_var->reg_no=i;
				add_var(i,new_var);
				return i;
			}
		}
	}

	//没有空闲寄存器，释放年龄最大的寄存器
	int max_age = 0 ;
	int max_age_index = 8;
	for(int i=8;i<26;i++){
		if(reg[i].age>max_age){
			max_age=reg[i].age;
			max_age_index = i;
		}
	}

	Var temp = var_head;

	while(temp!=NULL)
	{
		if(temp->reg_no==max_age_index)
		{
			int no = max_age_index;
			reg[no].age=0;
			//寄存器内容写栈中
			if(temp->op->kind!=CONSTANT_OP){
				fprintf(fp, "  sw %s, -%d($fp)\n", reg[no].name,temp->offset);
				del_var(temp);//释放寄存器
			}
			else{
				del_var(temp);//释放寄存器
			}
			if(op->kind!=CONSTANT_OP){
				Var temp_var = check_if_in_mem(op);//检查是否存在栈中
				//在栈中,从栈中取出，放入寄存器
				if(temp_var!=NULL){
					fprintf(fp, "  lw %s, -%d($fp)\n", reg[no].name, temp_var->offset);
					add_var(no,temp_var);
					return no;	
				}
				else{
					printf("error in allocate2: %d\n", op->kind);
					printf("t%d\n",op->u.temp_no);
					return no;
				}
			}
			else{
				Var new_var = (Var)malloc(sizeof(struct VarDesc));
				new_var->op=op;
				new_var->next=NULL;
				new_var->offset=0;
				new_var->reg_no=no;
				add_var(no,new_var); 
				return no;
			}
		}
		temp=temp->next;
	}
	printf("fail to find: %s\n", func_name);
}

void push_reg(FILE* fp)
{
	fprintf(fp, "  addi $sp, $sp, -72\n");
	for(int i=8;i<26;i++)
	{
		fprintf(fp, "  sw %s, %d($sp)\n", reg[i].name,(i-8)*4);
	}
}

void pop_reg(FILE* fp)
{
	for(int i=8;i<26;i++)
	{
		fprintf(fp, "  lw %s, %d($sp)\n", reg[i].name,(i-8)*4);
	}
	fprintf(fp, "  addi $sp, $sp, 72\n");
}

