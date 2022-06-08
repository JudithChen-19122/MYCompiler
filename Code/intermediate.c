#include "intermediate.h"
#include "semantics.h"
struct InterCodes* code_head=NULL;
struct InterCodes* code_tail=NULL;

extern HNode* hashTable[HASH_SIZE];

int lable_count=0; //lable 1, lable 2, lable 3 
int temp_var_count=0; //t1, t2, t3
int var_count=0;// v1, v2, v3

//划分基本块
void splitBasicBlocks()
{
    int count = 0;
    struct InterCodes* temp_code = code_head;
    while(temp_code)
    {
        if(temp_code->code.kind == IC_FUNCTION)
            ++count;
        temp_code->code.block_num = count;
        if(temp_code->code.kind == IC_LABEL)
        {
            if(temp_code->prev && temp_code->code.block_num == temp_code->prev->code.block_num)
                temp_code->code.block_num = ++count;
        }
        if(temp_code->code.kind == IC_GOTO || temp_code->code.kind == IC_IFGOTO)
            temp_code->code.block_num = ++count;

        temp_code = temp_code->next;
    }
}


//插入双向链表
void add_intercodes(struct InterCodes* new_intercodes){
    if(code_head==NULL){
        code_head=new_intercodes;
        code_tail=new_intercodes;
    }
    else{
        code_tail->next=new_intercodes;
        new_intercodes->prev = code_tail;
        code_tail = new_intercodes;
    }
}

void create_one_intercode(Operand op, int kind){

    if(op->kind == TEMP_ADDR_OP && op->u.is_addr==0)
	{
		Operand t1 = new_temp();
		create_two_intercode(t1, op, IC_INTO_ADDR_RIGHT);
		op = t1;
	}

	struct InterCodes * new_code = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	new_code->next=NULL;
	new_code->prev=NULL;
	new_code->code.kind=kind;
	new_code->code.u.one.op = op;
	add_intercodes(new_code);
}

void create_two_intercode(Operand left, Operand right, int kind){
    //IC_ASSIGN,IC_CALL,IC_GET_ADDR, IC_ARRAY_ASSIGN,
    if(kind == IC_ASSIGN && (left->kind == TEMP_ADDR_OP || right->kind == TEMP_ADDR_OP))
	{
		if(left->kind == TEMP_ADDR_OP && right->kind != TEMP_ADDR_OP)
			create_two_intercode(left, right, IC_INTO_ADDR_LEFT);
		else if(right->kind == TEMP_ADDR_OP && left->kind != TEMP_ADDR_OP)
			create_two_intercode(left, right, IC_INTO_ADDR_RIGHT);
		else
		{
			Operand t1 = new_temp();
			create_two_intercode(t1, right, IC_INTO_ADDR_RIGHT);
			create_two_intercode(left, t1, IC_INTO_ADDR_LEFT);						
		}
	}
    else if(kind == IC_GET_ADDR && (right->kind == ADDRESS_OP||right->kind == TEMP_ADDR_OP)){
        struct InterCodes * new_code = (struct InterCodes*)malloc(sizeof(struct InterCodes));
        new_code->next=NULL;
		new_code->prev=NULL;
		new_code->code.kind=IC_ASSIGN;
		new_code->code.u.two.left = left;
		new_code->code.u.two.right = right;
		add_intercodes(new_code);
    }
    else{
		struct InterCodes * new_code = (struct InterCodes*)malloc(sizeof(struct InterCodes));
		new_code->next=NULL;
		new_code->prev=NULL;
		new_code->code.kind=kind;
		new_code->code.u.two.left = left;
		new_code->code.u.two.right = right;
		add_intercodes(new_code);
    }
}

void create_three_intercode(Operand result, Operand op1, Operand op2, int kind){
    //3 OP  IC_ADD,IC_SUB,IC_MUL,IC_DIV,
    if(op1->kind == TEMP_ADDR_OP && op1->u.is_addr==0)
	{
		Operand t1 = new_temp();
		create_two_intercode(t1, op1, IC_INTO_ADDR_RIGHT);
		op1 = t1;
	}
	if(op2->kind == TEMP_ADDR_OP && op2->u.is_addr==0)
	{
		Operand t1 = new_temp();
		create_two_intercode(t1, op2, IC_INTO_ADDR_RIGHT);
		op2 = t1;
	}

    struct InterCodes * new_code = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	new_code->next=NULL;
	new_code->prev=NULL;
	new_code->code.kind=kind;
	new_code->code.u.three.result = result;
	new_code->code.u.three.op1 = op1;
	new_code->code.u.three.op2 = op2;
	add_intercodes(new_code);

}

//IF(op1 relop op2)GOTO label
void create_four_intercode(Operand relop, Operand op1, Operand op2,Operand label, int kind){

    if(op1->kind == TEMP_ADDR_OP && op1->u.is_addr==0)
	{
		Operand t1 = new_temp();
		create_two_intercode(t1, op1, IC_INTO_ADDR_RIGHT);
		op1 = t1;
	}
	if(op2->kind == TEMP_ADDR_OP && op2->u.is_addr==0)
	{
		Operand t1 = new_temp();
		create_two_intercode(t1, op2, IC_INTO_ADDR_RIGHT);
		op2 = t1;
    }

    struct InterCodes * new_code = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	new_code->next=NULL;
	new_code->prev=NULL;
	new_code->code.kind=kind;
	new_code->code.u.ifgoto.relop = relop;
	new_code->code.u.ifgoto.op1 = op1;
	new_code->code.u.ifgoto.op2 = op2;
    new_code->code.u.ifgoto.label = label;
	add_intercodes(new_code);
}

void create_dec_intercode(Operand op, int size){
        struct InterCodes * new_code = (struct InterCodes*)malloc(sizeof(struct InterCodes));
		new_code->next=NULL;
		new_code->prev=NULL;
		new_code->code.kind=IC_DEC;
		new_code->code.u.dec.op = op;
		new_code->code.u.dec.size = size;
		add_intercodes(new_code);
}


int get_size(Type* type)
{
	if(type == NULL)
		return 0;
	else if(type->kind == BASIC)
	{
		return 4;
	}
	else if(type->kind == ARRAY)
	{
		return type->u.array.size * get_size(type->u.array.elem);
	}
	else if(type->kind == STRUCTURE)
	{
		int size=0;
		FieldList temp = type->u.structure;
		while(temp != NULL)
		{
			if(temp->sno->type->kind == BASIC)
			{
				size=size+4;
			}
			else if(temp->sno->type->kind == ARRAY)
			{
				size = size+ temp->sno->type->u.array.size*get_size(temp->sno->type->u.array.elem);
			}
			else if(temp->sno->type->kind == STRUCTURE)
			{
				size = size + get_size(temp->sno->type);
			}
			temp = temp->tail;
		}
		return size;
	}
	return 0;
}

//为了便于处理，假设一开始添加的都是TEMP_VAR,用于初始化分配空间
Operand new_temp(){
    temp_var_count++;
    Operand ntemp = (Operand)malloc(sizeof(struct Operand_));
	ntemp->kind = TEMP_VAR_OP;
	ntemp->u.temp_no=temp_var_count;
    ntemp->u.is_addr=0;
    ntemp->u.for_add_addr=1;
	return ntemp;
}

Operand new_label(){
    lable_count++;
    Operand op = (Operand)malloc(sizeof(struct Operand_));
	op->kind = LABEL_OP;
	op->u.lable_no=lable_count;
	return op;
}


/*Exp: Exp ASSIGNOP Exp      OK

        | Exp AND Exp          OK
        | Exp OR Exp           OK  
        | Exp RELOP Exp        OK

        | Exp PLUS Exp         OK 
        | Exp MINUS Exp        OK
        | Exp STAR Exp         OK 
        | Exp DIV Exp          OK


        | LP Exp RP            OK

        | MINUS Exp            OK 
        | NOT Exp              OK

        | ID LP Args RP     
        | ID LP RP                 
        | Exp LB Exp RB       
        | Exp DOT ID      

        | ID          OK    
        | INT         OK          
        | FLOAT       //
*/

void translate_EXP(Node* exp, Operand place){
    if(place==NULL){
        place=new_temp();
    }
    if(exp->child_count==1){
        //exp-> INT|ID     NOTE: FLOAT can be ignore for lab3
        if(strcmp(exp->children[0]->name,"INT")==0){
            //exp->INT
            //temp_var_count--;//不是TEMP_VAR,故减去初始化时增加的TEMP_VAR计数
            place->kind=CONSTANT_OP;
            place->u.value=atoi(exp->children[0]->content);
            return;
        }
        else if(strcmp(exp->children[0]->name, "ID")==0){
            //exp->ID
            //temp_var_count--;//不是TEMP_VAR,故减去初始化时增加的TEMP_VAR计数
            SNode snode=check_symbol_table_by_name(exp->children[0]->content);
            if(snode->var_no==-1){
                var_count++;
                snode->var_no=var_count;
            }
            if((snode->type->kind==STRUCTURE||snode->type->kind==ARRAY)){
                if(snode->is_para){
                    place->kind = ADDRESS_OP; //&vi
                    place->u.is_addr=1;
                    place->u.for_add_addr=1;
                    place->u.var_no=snode->var_no;
                    place->u.name=exp->children[0]->content;
                }
                else{
                    place->u.is_addr=1;
                    place->kind = VARIABLE_OP;//vi
                    place->u.var_no=snode->var_no;
                    place->u.name=exp->children[0]->content;
                }        
            }
            else{
                place->u.is_addr=0;
                place->kind = VARIABLE_OP;//vi
                place->u.var_no=snode->var_no;
                place->u.name=exp->children[0]->content;
            }
            //place->u.name=exp->children[0]->content;
            return;
        }
        else{
            //temp_var_count--;//不是TEMP_VAR,故减去初始化时增加的TEMP_VAR计数
            place->kind=CONSTANT_OP;
            place->u.value=0;
            return;
        }
    }
    else if(exp->child_count==2){
        //exp->  MINUS Exp  | NOT Exp
        if(strcmp(exp->children[0]->name,"MINUS")==0){
            //EXP-> #0-EXP
		    Operand t1 = (Operand)malloc(sizeof(struct Operand_));
		    t1->kind = CONSTANT_OP;
		    t1->u.value = 0;
            Operand t2 = new_temp();
		    translate_EXP(exp->children[1],t2);
		    create_three_intercode(place, t1, t2, IC_SUB);
        }
        else{
            //NOT EXP    //     !EXP
            Operand label1 = new_label();
            Operand label2 = new_label();

            Operand t1 = (Operand)malloc(sizeof(struct Operand_));
		    t1->kind = CONSTANT_OP;
		    t1->u.value = 0; //FALSE
            Operand t2 = (Operand)malloc(sizeof(struct Operand_));
		    t2->kind = CONSTANT_OP;
		    t2->u.value = 1; //TRUE

            create_two_intercode(place, t1, IC_ASSIGN);
		    translate_Cond(exp, label1, label2);
		    create_one_intercode(label1, IC_LABEL);
		    create_two_intercode(place, t2, IC_ASSIGN);
		    create_one_intercode(label2, IC_LABEL);
        }
    }
    else if(exp->child_count==3){
        if(strcmp(exp->children[0]->name,"LP")==0){
            //EXP->(EXP)
            translate_EXP(exp->children[1],place);
            return;
        }
        else if(strcmp(exp->children[1]->name,"ASSIGNOP")==0){
            //exp-> exp = exp

            Type* type1=Exp(exp->children[0]);
            Type* type2=Exp(exp->children[2]);
            if(type1->kind==ARRAY&&type2->kind==ARRAY){
                Operand t1 = new_temp();
                translate_EXP(exp->children[0],t1);

                Operand t2 = new_temp();
                translate_EXP(exp->children[2],t2);
                
                int size1 = get_size(type1);
                int size2 = get_size(type2);
                
                if(size1!=size2){
                    printf("Cannot translate: The size of two array is not equal");
                }
                else{
                    copy_array(t1,t2,size1);
                }
                return;
            }
            else{
                Operand t1 = new_temp();
                translate_EXP(exp->children[0],t1);

                Operand t2 = new_temp();
                translate_EXP(exp->children[2],t2);

                create_two_intercode(t1, t2, IC_ASSIGN);  // 规定不出现a=b=c
                return;
            }
        }
        else if(strcmp(exp->children[1]->name,"PLUS")==0){
            //exp-> exp + exp
            Operand t1 = new_temp();
            translate_EXP(exp->children[0],t1);

            Operand t2 = new_temp();
            translate_EXP(exp->children[2],t2);

            create_three_intercode(place, t1, t2, IC_ADD);
            return;
        }
        else if(strcmp(exp->children[1]->name,"MINUS")==0){
            //exp-> exp - exp
            Operand t1 = new_temp();
            translate_EXP(exp->children[0],t1);

            Operand t2 = new_temp();
            translate_EXP(exp->children[2],t2);

            create_three_intercode(place, t1, t2, IC_SUB);
            return;
        }
        else if(strcmp(exp->children[1]->name,"STAR")==0){
            //exp-> exp * exp
            Operand t1 = new_temp();
            translate_EXP(exp->children[0],t1);

            Operand t2 = new_temp();
            translate_EXP(exp->children[2],t2);

            create_three_intercode(place, t1, t2, IC_MUL);
            return;
        }
         else if(strcmp(exp->children[1]->name,"DIV")==0){
            //exp-> exp / exp
            Operand t1 = new_temp();
            translate_EXP(exp->children[0],t1);

            Operand t2 = new_temp();
            translate_EXP(exp->children[2],t2);

            create_three_intercode(place, t1, t2, IC_DIV);
            return;
        }
        else if(strcmp(exp->children[1]->name,"AND")==0||
        strcmp(exp->children[1]->name,"OR")==0||
        strcmp(exp->children[1]->name,"RELOP")==0||
        strcmp(exp->children[1]->name,"AND")==0){
            Operand label1 = new_label();
            Operand label2 = new_label();

            Operand t1 = (Operand)malloc(sizeof(struct Operand_));
		    t1->kind = CONSTANT_OP;
		    t1->u.value = 0; //FALSE
            Operand t2 = (Operand)malloc(sizeof(struct Operand_));
		    t2->kind = CONSTANT_OP;
		    t2->u.value = 1; //TRUE

            create_two_intercode(place, t1, IC_ASSIGN);
		    translate_Cond(exp, label1, label2);
		    create_one_intercode(label1, IC_LABEL);
		    create_two_intercode(place, t2, IC_ASSIGN);
		    create_one_intercode(label2, IC_LABEL);	
        }

        else if(strcmp(exp->children[1]->name,"DOT")==0){
            //EXP->EXP.ID
            //get exp: t1
            Operand s_id = new_temp();
		    translate_EXP(exp->children[0], s_id);
		    Operand t1;
		    if(s_id->kind != ADDRESS_OP && s_id->kind!=TEMP_ADDR_OP)
		    {
			    t1 = new_temp();
                t1->kind=TEMP_ADDR_OP;
                t1->u.is_addr=1;
                t1->u.for_add_addr=1;
			    create_two_intercode(t1, s_id, IC_GET_ADDR);
		    }
		    else
		    {
			    t1 = s_id;
		    }
           
            Type* type;
            if(s_id->u.name!=NULL){
                SNode snode = check_symbol_table_by_name(s_id->u.name);
                type=snode->type;
            }
            else{
                type=Exp(exp->children[0]);
            }
           
            FieldList fieldlist;
            fieldlist=type->u.structure;
            int offset=0;
            while(fieldlist != NULL)
		    {
			    if(strcmp(fieldlist->sno->name,exp->children[2]->content)==0)
				    break;
			    else
				    offset=offset+get_size(fieldlist->sno->type);
			    fieldlist= fieldlist->tail;
		    }
            Operand t2 = (Operand)malloc(sizeof(struct Operand_));
		    t2->kind = CONSTANT_OP;
		    t2->u.value = offset;
            SNode c_snode=check_symbol_table_by_name(exp->children[2]->content);
            if(c_snode->type->kind==BASIC){
                place->u.is_addr=1;
                place->u.for_add_addr=1;
                place->kind = TEMP_ADDR_OP;
		        place->u.name = exp->children[2]->content;
		        create_three_intercode(place,t1,t2,IC_ADD);
                place->u.is_addr=0;
            }
            else{
                place->u.is_addr=1;
                place->u.for_add_addr=1;
                place->kind = TEMP_ADDR_OP;
		        place->u.name = exp->children[2]->content;
		        create_three_intercode(place,t1,t2,IC_ADD);
                place->u.is_addr=1;
            }

        }
        else if(strcmp(exp->children[1]->name,"LP")==0)
        {   //EXP->ID()
            SNode snode= check_symbol_table_by_name(exp->children[0]->content);
            Operand func = (Operand)malloc(sizeof(struct Operand_));
	        func->kind = FUNCTION_OP;
	        func->u.name = exp->children[0]->content; 
            if(strcmp(exp->children[0]->content,"read")==0){
                create_one_intercode(place, IC_READ);
            }
            else{
                create_two_intercode(place, func, IC_CALL);
            }
        }
    }
    else{
        if(strcmp(exp->children[1]->name,"LP")==0){
            // ID LP Args RP id(args)
            /*
            function = lookup(sym_table, ID)
            arg_list = NULL
            code1 = translate_Args(Args, sym_table, arg_list)
            if (function.name == “write”) return code1 + [WRITE arg_list[1]] + [place := #0]
            for i = 1 to length(arg_list) code2 = code2 + [ARG arg_list[i]]
            return code1 + code2 + [place := CALL function.name]
            */
            SNode snode = check_symbol_table_by_name(exp->children[0]->content);
            Operand func = (Operand)malloc(sizeof(struct Operand_));
            func->kind = FUNCTION_OP;
	        func->u.name = exp->children[0]->content; 
            Arg_List arg_head=NULL;
            get_args(exp->children[2],&arg_head);
            if(strcmp(func->u.name,"write")==0){
                create_one_intercode(arg_head->op, IC_WRITE);
            }
            else{
                Arg_List temp=arg_head;
                while(temp!=NULL){
                    create_one_intercode(temp->op,IC_ARG);
                    temp=temp->next;
                }
                if(place != NULL)
					create_two_intercode(place, func, IC_CALL);
				else
				{
					Operand t1 = new_temp();
					create_two_intercode(t1, func, IC_CALL);
				}
            }
        }
        else{
            //Exp LB Exp RB id[int]
		    Operand id = new_temp();
            translate_EXP(exp->children[0],id);
            Operand t1 = new_temp();
		    translate_EXP(exp->children[2],t1);

            SNode snode=check_symbol_table_by_name(id->u.name);

		    Operand t_size = (Operand)malloc(sizeof(struct Operand_));
		    t_size->kind = CONSTANT_OP;
		    t_size->u.value = get_size(snode->type->u.array.elem);

            Operand t2 = new_temp();
		    create_three_intercode(t2,t1,t_size,IC_MUL);
            Operand t3;
            if(id->kind!=ADDRESS_OP && id->kind!=TEMP_ADDR_OP){
                t3 = new_temp();
                t3->kind = TEMP_ADDR_OP;
                t3->u.is_addr=1;
                t3->u.for_add_addr=1;
		        create_two_intercode(t3, id, IC_GET_ADDR);
            }
            else{
                t3=id;
            }
            place->kind = TEMP_ADDR_OP; 
            Type* type =  Exp(exp->children[0]);
            Type* intype =type->u.array.elem;
            if(intype->kind==BASIC){
                place->u.is_addr = 0;
                place->u.for_add_addr = 1;
            }
            else{
                place->u.is_addr=1;
            }
		    create_three_intercode(place,t3,t2,IC_ADD);   
        }
    }
}

//get arg_list
void get_args(Node* arg, Arg_List* head){
    Arg_List new_arg = (Arg_List)malloc(sizeof(struct Arg_List_));
    if(arg->child_count==3){//Args        : Exp COMMA Args 

        new_arg->op=new_temp();
        translate_EXP(arg->children[0],new_arg->op);
        
        if(head!=NULL){
            new_arg->next=*head;
            *head=new_arg;
        }
        else{
            *head = new_arg;
            (*head)->next=NULL;
        }

        get_args(arg->children[2],head);
    }
    else{//Args        : Exp 
        new_arg->op=new_temp();
        translate_EXP(arg->children[0],new_arg->op);
        if(head!=NULL){
            new_arg->next=*head;
            *head=new_arg;
        }
        else{
            *head = new_arg;
            (*head)->next=NULL;
        }
    }
}


void copy_array(Operand op1, Operand op2, int size){
    int temp=0;
    Operand t1=new_temp();
    Operand t2=new_temp();
    Operand t3=new_temp();

    if(op1->kind==VARIABLE_OP){
        t1->kind=TEMP_ADDR_OP;
        t1->u.is_addr=1;
        t1->u.for_add_addr=1;
		create_two_intercode(t1, op1, IC_GET_ADDR);
    }
    else{
        create_two_intercode(t1, op1, IC_ASSIGN);
    }
    if(op2->kind==VARIABLE_OP){
        t2->kind=TEMP_ADDR_OP;
        t2->u.is_addr=1;
        t2->u.for_add_addr=1;
		create_two_intercode(t2, op2, IC_GET_ADDR);
    }
    else{
        create_two_intercode(t2, op2, IC_ASSIGN);
    }

    t3->kind=CONSTANT_OP;
    t3->u.is_addr=0;
    t3->u.for_add_addr=0;
    t3->u.value=4;

    while(temp!=size){
        create_two_intercode(t1,t2,IC_ARRAY_ASSIGN);
        temp=temp+4;
        if(temp==size) break;
        create_three_intercode(t1,t1,t3,IC_ADD);        
        create_three_intercode(t2,t2,t3,IC_ADD); 
    }
}


//translate_cond
void translate_Cond(Node* exp, Operand label_true, Operand label_false){
    if(exp->child_count==1){
        Operand t1 = new_temp();
        translate_EXP(exp, t1);
        Operand t2 = (Operand)malloc(sizeof(struct Operand_));
		t2->kind = CONSTANT_OP;
		t2->u.value = 0;
        Operand relop = (Operand)malloc(sizeof(struct Operand_));
        relop->kind=RELOP_OP;
        relop->u.name="!=";
        create_four_intercode(relop,t1,t2,label_true,IC_IFGOTO);
        create_one_intercode(label_false, IC_GOTO);

    }
    else if(strcmp(exp->children[1]->name,"RELOP")==0){
        /*
        t1 = new_temp()
        t2 = new_temp()
        code1 = translate_Exp(Exp1, sym_table, t1)
        code2 = translate_Exp(Exp2, sym_table, t2)
        op = get_relop(RELOP);
        code3 = [IF t1 op t2 GOTO label_true]
        return code1 + code2 + code3 + [GOTO label_false]
        */
        Operand t1 = new_temp();
        translate_EXP(exp->children[0], t1);
        Operand t2 = new_temp();
        translate_EXP(exp->children[2], t2);
        Operand relop = (Operand)malloc(sizeof(struct Operand_));
        relop->kind=RELOP_OP;
        relop->u.name=exp->children[1]->content;
        create_four_intercode(relop,t1,t2,label_true,IC_IFGOTO);
        create_one_intercode(label_false, IC_GOTO);
    }
    else if(strcmp(exp->children[1]->name,"AND")==0){
        Operand label1 = new_label();
        translate_Cond(exp->children[0], label1, label_false);
        create_one_intercode(label1, IC_LABEL);		
        translate_Cond(exp->children[2], label_true, label_false);
    }
    else if(strcmp(exp->children[1]->name,"OR")==0){
        Operand label1 = new_label();
        translate_Cond(exp->children[0], label_true, label1);
        create_one_intercode(label1, IC_LABEL);		
        translate_Cond(exp->children[2], label_true, label_false);
    }
    else if(exp->child_count==2){
        //!exp
        if(strcmp(exp->children[0]->name,"NOT")==0)
            translate_Cond(exp->children[1], label_false, label_true);
        else //-exp
        {
            translate_Cond(exp->children[1],label_true,label_false);
        }
    }
    else{
        Operand t1 = new_temp();
        translate_EXP(exp, t1);
        Operand t2 = (Operand)malloc(sizeof(struct Operand_));
		t2->kind = CONSTANT_OP;
		t2->u.value = 0;

        Operand relop = (Operand)malloc(sizeof(struct Operand_));
        relop->kind=RELOP_OP;
        relop->u.name="!=";

        create_four_intercode(relop,t1,t2,label_true,IC_IFGOTO);
        create_one_intercode(label_false, IC_GOTO);
    }

}

void translate_StmtList(Node* stmtlist){
    // StmtList->Stmt StmtList||NULL
    if(stmtlist==NULL||stmtlist->child_count==0)return;
    else{
        translate_Stmt(stmtlist->children[0]);
        translate_StmtList(stmtlist->children[1]);
    }
}

void translate_para_VarDec(Node* vardec, Operand place){
    //ID || VarDec LB INT RB
    if(place==NULL){place=new_temp();}
    if(vardec->child_count==1){//ID
        Node* id = vardec->children[0];
        //temp_var_count--;//不是TEMP_VAR,故减去初始化时增加的TEMP_VAR计数
        SNode snode=check_symbol_table_by_name(id->content);
        if(snode->var_no==-1){
            var_count++;
            snode->var_no=var_count;
        }
        if(snode->type->kind==ARRAY){
			place->kind = VARIABLE_OP;
            place->u.is_addr=0;
			place->u.name = snode->name;
            place->u.var_no=snode->var_no;
			//create_dec_intercode(op,get_size(snode->type));
        }
        else if(snode->type->kind==STRUCTURE){
			place->kind = VARIABLE_OP;
            place->u.is_addr=0;
			place->u.name = snode->name;
            place->u.var_no=snode->var_no;
            //create_dec_intercode(op,get_size(snode->type));
        }                  
        else{    //basic
            place->kind = VARIABLE_OP;//vi
            place->u.is_addr=0;
            place->u.var_no=snode->var_no;
            place->u.name=id->content;
        } 
    }
    else{//VarDec LB INT RB
        translate_para_VarDec(vardec->children[0],place);
    }
}

void translate_VarDec(Node* vardec, Operand place){
    //ID || VarDec LB INT RB
    if(place==NULL){place=new_temp();}
    if(vardec->child_count==1){//ID
        Node* id = vardec->children[0];
        //temp_var_count--;//不是TEMP_VAR,故减去初始化时增加的TEMP_VAR计数
        SNode snode=check_symbol_table_by_name(id->content);
        if(snode->var_no==-1){
            var_count++;
            snode->var_no=var_count;
        }
        if(snode->type->kind==ARRAY){
			place->kind = VARIABLE_OP;
            place->u.is_addr=1;
			place->u.name = snode->name;
            place->u.var_no=snode->var_no;
			create_dec_intercode(place,get_size(snode->type));
        }
        else if(snode->type->kind==STRUCTURE){
			place->kind = VARIABLE_OP;
            place->u.is_addr=1;
			place->u.name = snode->name;
            place->u.var_no=snode->var_no;
            create_dec_intercode(place,get_size(snode->type));
        }                  
        else{    //basic
            place->kind = VARIABLE_OP;//vi
            place->u.is_addr=0;
            place->u.var_no=snode->var_no;
            place->u.name=id->content;
        } 
    }
    else{//VarDec LB INT RB
        translate_VarDec(vardec->children[0],place);
    }
}

void translate_Dec(Node* dec){
    //VarDec ASSIGNOP Exp ||VarDec
	if(dec->child_count==1)
	{
        translate_VarDec(dec->children[0], NULL);
	}
	else{
        Operand t1 = new_temp();		
		translate_VarDec(dec->children[0], t1);
        Operand t2 = new_temp();
		translate_EXP(dec->children[2], t2);
		create_two_intercode(t1,t2,IC_ASSIGN);
    }

}

void translate_DecList(Node* declist){
    //Dec COMMA DecList||Dec
    if(declist->child_count==3){
        translate_Dec(declist->children[0]);
        translate_DecList(declist->children[2]);
    }
    else{
        translate_Dec(declist->children[0]);
    }
}

void translate_Def(Node* def){
    //Specifier DecList SEMI
    translate_DecList(def->children[1]);
}

void translate_DefList(Node* deflist){
    //Def DefList ||null
    if(deflist==NULL||deflist->child_count==0) return;
    else{
        translate_Def(deflist->children[0]);
        translate_DefList(deflist->children[1]);
    }
}


void translate_CompSt(Node* compst){
    //LC DefList StmtList RC
    translate_DefList(compst->children[1]);
    translate_StmtList(compst->children[2]);
}

void translate_Stmt(Node* stmt){
    //Stmt-> Exp SEMI
    if(stmt->child_count==2){
        translate_EXP(stmt->children[0], NULL);
    }
    //CompSt
    else if(stmt->child_count==1){
        translate_CompSt(stmt->children[0]);
    }
    //RETURN Exp SEMI
    else if(strcmp(stmt->children[0]->name,"RETURN")==0){
        Operand t1=new_temp();
        translate_EXP(stmt->children[1], t1);
        create_one_intercode(t1,IC_RETURN);
    }
    //WHILE LP Exp RP Stmt1
    else if(strcmp(stmt->children[0]->name,"WHILE")==0){
        /*
        label1 = new_label()
        label2 = new_label()
        label3 = new_label()
        code1 = translate_Cond(Exp, label2, label3, sym_table)
        code2 = translate_Stmt(Stmt1, sym_table)
        return [LABEL label1] + code1 + [LABEL label2] + code2
        + [GOTO label1] + [LABEL label3]
*/
        Operand label1 = new_label();
        Operand label2 = new_label();
        Operand label3 = new_label();
        create_one_intercode(label1,IC_LABEL);
        translate_Cond(stmt->children[2], label2, label3);
        create_one_intercode(label2,IC_LABEL);
        translate_Stmt(stmt->children[4]);
        create_one_intercode(label1,IC_GOTO);
        create_one_intercode(label3,IC_LABEL);
    }
    //IF LP Exp RP Stmt1
    else if(stmt->child_count==5){
        /*label1 = new_label()
        label2 = new_label()
        code1 = translate_Cond(Exp, label1, label2, sym_table)
        code2 = translate_Stmt(Stmt1, sym_table)
        return code1 + [LABEL label1] + code2 + [LABEL label2]*/    
        Operand label1 = new_label();
        Operand label2 = new_label();
        translate_Cond(stmt->children[2], label1, label2);
        create_one_intercode(label1,IC_LABEL);
        translate_Stmt(stmt->children[4]);
        create_one_intercode(label2,IC_LABEL);
    }
    //IF LP Exp RP Stmt1 ELSE Stmt2
    else{
        Operand label1 = new_label();
        Operand label2 = new_label();
        Operand label3 = new_label();
        translate_Cond(stmt->children[2], label1, label2);
        create_one_intercode(label1,IC_LABEL);
        translate_Stmt(stmt->children[4]);
        create_one_intercode(label3,IC_GOTO);
        create_one_intercode(label2,IC_LABEL);
        translate_Stmt(stmt->children[6]);
        create_one_intercode(label3,IC_LABEL);
    }

}

void translate_ParamDec(Node* paramdec){
    //Specifier VarDec 
    Node* vardec=paramdec->children[1];
    Operand para = (Operand)malloc(sizeof(struct Operand_));
    translate_para_VarDec(vardec,para);
    //printf("para: \n  %s:\n %d\n",para->u.name,para->u.var_no);
	create_one_intercode(para, IC_PARAM);
}

void translate_VarList(Node* varlist){
    //ParamDec COMMA VarList|| ParamDec
    if(varlist->child_count==3){
        translate_ParamDec(varlist->children[0]);
        translate_VarList(varlist->children[2]);
    }
    else{
        translate_ParamDec(varlist->children[0]);
    }

}


void translate_FunDec (Node* fundec){
    if(fundec->child_count==3){//ID LP RP
        Operand func = (Operand)malloc(sizeof(struct Operand_));
	    func->kind = FUNCTION_OP;
	    func->u.name = fundec->children[0]->content;
	    create_one_intercode(func, IC_FUNCTION);
    }
    else{//ID LP VarList RP
        Operand func = (Operand)malloc(sizeof(struct Operand_));
	    func->kind = FUNCTION_OP;
	    func->u.name = fundec->children[0]->content;
	    create_one_intercode(func, IC_FUNCTION);
        translate_VarList(fundec->children[2]);

    }   
}

void translate_ExtDecList(Node* exdl){
    //VarDec||VarDec COMMA ExtDecList
    if(exdl->child_count==1){
        translate_VarDec(exdl->children[0],NULL);
    }
    else{
        translate_VarDec(exdl->children[0],NULL);
        translate_ExtDecList(exdl->children[2]);
    }

}

void translate_ExtDef(Node* extdef){
// Specifier ExtDecList SEMI||Specifier SEMI||Specifier FunDec CompSt
    if(extdef->child_count==2) return;//Specifier SEMI
    else if(strcmp(extdef->children[2]->name,"SEMI")==0){
        // Specifier ExtDecList SEMI
        translate_ExtDecList(extdef->children[1]);
    }
    else{
        //Specifier FunDec CompSt
        translate_FunDec(extdef->children[1]);
        translate_CompSt(extdef->children[2]);
    }

}


void translate_ExtDefList(Node* te){
//ExtDef ExtDefList||null
    if(te==NULL||te->child_count==0)return;
    else{
        translate_ExtDef(te->children[0]);
        translate_ExtDefList(te->children[1]);
    }

}

void translate_Program(Node* program){
//ExtDefList
    translate_ExtDefList(program->children[0]);
}


void print_op_to_file(FILE* fp, Operand op)
{
    switch(op->kind)
	{
		case VARIABLE_OP:
            fprintf(fp,"v%d",op->u.var_no);break;
        case ADDRESS_OP:
			//if(op->u.is_addr)
               fprintf(fp,"v%d",op->u.var_no);
            //else
            //    fprintf(fp,"*v%d",op->u.var_no);
            break;
		case CONSTANT_OP:
			fprintf(fp,"#%d",op->u.value);break;
		case LABEL_OP:
			fprintf(fp,"label%d",op->u.lable_no);break;
        case TEMP_ADDR_OP:
            //if((!op->u.is_addr) && (!op->u.for_add_addr)){
            //    fprintf(fp,"*t%d",op->u.temp_no);
            //}
            //else{
                fprintf(fp,"t%d",op->u.temp_no);
            //    op->u.for_add_addr=0;
            //}
            break;    
        case TEMP_VAR_OP:
            fprintf(fp,"t%d",op->u.temp_no);break;
		case FUNCTION_OP:
		case RELOP_OP:
			fprintf(fp,"%s",op->u.name);break;
	}
}

void print_arg_op_to_file(FILE* fp, Operand op)
{
    switch(op->kind)
	{
        case VARIABLE_OP:
            if(op->u.is_addr==0)
                fprintf(fp,"v%d",op->u.var_no);
            else
                fprintf(fp,"&v%d",op->u.var_no);
            break;
        case ADDRESS_OP:
			if(op->u.is_addr)
               fprintf(fp,"v%d",op->u.var_no);
            else
                fprintf(fp,"*v%d",op->u.var_no);
            break;
		case CONSTANT_OP:
			fprintf(fp,"#%d",op->u.value);break;
		case LABEL_OP:
			fprintf(fp,"label%d",op->u.lable_no);break;
        case TEMP_ADDR_OP:
            if((!op->u.is_addr) && (!op->u.for_add_addr)){
                fprintf(fp,"*t%d",op->u.temp_no);
            }
            else{
                fprintf(fp,"t%d",op->u.temp_no);
                op->u.for_add_addr=0;
            }
            break;    
        case TEMP_VAR_OP:
            fprintf(fp,"t%d",op->u.temp_no);break;
		case FUNCTION_OP:
		case RELOP_OP:
			fprintf(fp,"%s",op->u.name);break;
	}
}


void output_code(FILE* fp){
    struct InterCodes* x = code_head;
    while(x != NULL)
    {
    	switch(x->code.kind)
    	{
    		case IC_LABEL:
    			fprintf(fp, "LABEL ");
    			print_op_to_file(fp, x->code.u.one.op);
    			fprintf(fp, " :");
    			break;
    		case IC_FUNCTION:
    		    fprintf(fp, "FUNCTION ");
    			print_op_to_file(fp, x->code.u.one.op);
    			fprintf(fp, " :");
    			break;
    		case IC_ASSIGN:
    		    print_op_to_file(fp, x->code.u.two.left);
		        fprintf(fp, " := ");
				print_op_to_file(fp, x->code.u.two.right);
    			break;
    		case IC_ADD:
    			print_op_to_file(fp, x->code.u.three.result);
		        fprintf(fp, " := ");
				print_op_to_file(fp, x->code.u.three.op1);
				fprintf(fp, " + ");
				print_op_to_file(fp, x->code.u.three.op2);
				break;
    		case IC_SUB:
    			print_op_to_file(fp, x->code.u.three.result);
		        fprintf(fp, " := ");
				print_op_to_file(fp, x->code.u.three.op1);
				fprintf(fp, " - ");
				print_op_to_file(fp, x->code.u.three.op2);
				break;
    		case IC_MUL:
    			print_op_to_file(fp, x->code.u.three.result);
		        fprintf(fp, " := ");
				print_op_to_file(fp, x->code.u.three.op1);
				fprintf(fp, " * ");
				print_op_to_file(fp, x->code.u.three.op2);
				break;
    		case IC_DIV:
    			print_op_to_file(fp, x->code.u.three.result);
		        fprintf(fp, " := ");
				print_op_to_file(fp, x->code.u.three.op1);
				fprintf(fp, " / ");
				print_op_to_file(fp, x->code.u.three.op2);
				break;
			case IC_GET_ADDR:
    		    print_op_to_file(fp, x->code.u.two.left);
		        fprintf(fp, " := &");
				print_op_to_file(fp, x->code.u.two.right);
    			break;	

			case IC_INTO_ADDR_RIGHT:
			   	print_op_to_file(fp, x->code.u.two.left);
		        fprintf(fp, " := *");
				print_op_to_file(fp, x->code.u.two.right);
    			break;			
			case IC_INTO_ADDR_LEFT:
				fprintf(fp, "*");
    		    print_op_to_file(fp, x->code.u.two.left);
		        fprintf(fp, " := ");
				print_op_to_file(fp, x->code.u.two.right);
    			break;
            /*
            case IC_ADDR_LEFT_RIGHT:
                fprintf(fp, "*");
    		    print_op_to_file(fp, x->code.u.two.left);
		        fprintf(fp, " := *");
				print_op_to_file(fp, x->code.u.two.right);
            */
    		case IC_GOTO:
    			fprintf(fp, "GOTO ");
    			print_op_to_file(fp, x->code.u.one.op);
    			break;
    		case IC_IFGOTO:
    			fprintf(fp, "IF ");
    			print_op_to_file(fp, x->code.u.ifgoto.op1);
    			fprintf(fp, " ");
     			print_op_to_file(fp, x->code.u.ifgoto.relop);
     			fprintf(fp, " ");
     			print_op_to_file(fp, x->code.u.ifgoto.op2);
     			fprintf(fp, " GOTO ");
    			print_op_to_file(fp, x->code.u.ifgoto.label);
    			break;  
    		case IC_RETURN:
    			fprintf(fp, "RETURN ");
    			print_op_to_file(fp, x->code.u.one.op);
    			break;
    		case IC_DEC:
    			fprintf(fp, "DEC ");
    			print_op_to_file(fp, x->code.u.dec.op);
    			fprintf(fp, " ");
    			fprintf(fp, "%d",x->code.u.dec.size);
    			break;
    		case IC_ARG:
    			fprintf(fp, "ARG ");
    			print_op_to_file(fp, x->code.u.one.op);
                //print_arg_op_to_file(fp, x->code.u.one.op);
    			break;
    		case IC_CALL:
    			print_op_to_file(fp, x->code.u.two.left);
    			fprintf(fp, " := CALL ");
    			print_op_to_file(fp, x->code.u.two.right);
    			break;
    		case IC_PARAM:
    			fprintf(fp, "PARAM ");
    			print_op_to_file(fp, x->code.u.one.op);
    			break;
    		case IC_READ:
    			fprintf(fp, "READ ");
    			print_op_to_file(fp, x->code.u.one.op);
    			break;    			
    		case IC_WRITE:
    			fprintf(fp, "WRITE ");
    			print_op_to_file(fp, x->code.u.one.op);
    			break;   

            case IC_ARRAY_ASSIGN:
                fprintf(fp, "*");
                print_op_to_file(fp, x->code.u.two.left);
		        fprintf(fp, " := *");
				print_op_to_file(fp, x->code.u.two.right);
                break;		 			
    	}
    	fprintf(fp, "\n");
        x = x->next;
    }
}


void get_translate_program(Node* root,char* filename){
    lable_count=0; //lable 1, lable 2, lable 3 
    temp_var_count=0; //t1, t2, t3
    var_count=0;// v1, v2, v3
    code_head=NULL;
    code_tail=NULL;
    translate_Program(root);
    splitBasicBlocks();
    /*
    FILE* fp = fopen(filename, "wt+");
	if (!fp)
	{
		perror(filename);
		return ;
	}
    output_code(fp);
    fclose(fp);
    */
    //print_intercode();
}


void print_arg_op(Operand op){
    switch(op->kind)
	{
		case VARIABLE_OP:
            if(op->u.is_addr==0)
                printf("v%d",op->u.var_no);
            else
                printf("&v%d",op->u.var_no);
            break;
        case ADDRESS_OP:
			if(op->u.is_addr)
                printf("v%d",op->u.var_no);
            else
                printf("*v%d",op->u.var_no);
            break;
		case CONSTANT_OP:
			printf("#%d",op->u.value);break;
		case LABEL_OP:
			printf("label%d",op->u.lable_no);break;
        case TEMP_ADDR_OP:
            if((!op->u.is_addr) && (!op->u.for_add_addr)){
                printf("*t%d",op->u.temp_no);
            }
            else{
                printf("t%d",op->u.temp_no);
                op->u.for_add_addr=0;
            }
            break;    
        case TEMP_VAR_OP:
            printf("t%d",op->u.temp_no);break;
		case FUNCTION_OP:
		case RELOP_OP:
			printf("%s",op->u.name);break;
	}
}


void print_op(Operand op)
{
	switch(op->kind)
	{
		case VARIABLE_OP:
            printf("v%d",op->u.var_no);break;
        case ADDRESS_OP:
			//if(op->u.is_addr)
                printf("v%d",op->u.var_no);
            //else
            //    printf("*v%d",op->u.var_no);
            break;
		case CONSTANT_OP:
			printf("#%d",op->u.value);break;
		case LABEL_OP:
			printf("label%d",op->u.lable_no);break;
        case TEMP_ADDR_OP:
            //if((!op->u.is_addr) && (!op->u.for_add_addr)){
            //    printf("*t%d",op->u.temp_no);
            // }
            //else{
                printf("t%d",op->u.temp_no);
            //    op->u.for_add_addr=0;
            //}
            break;    
        case TEMP_VAR_OP:
            printf("t%d",op->u.temp_no);break;
		case FUNCTION_OP:
		case RELOP_OP:
			printf("%s",op->u.name);break;
	}
}


void print_intercode()
{
    struct InterCodes* x = code_head;
    //int num=1;
    while(x != NULL)
    {
    	//printf("%d  ",num);
    	//num++;
    	switch(x->code.kind)
    	{
    		case IC_LABEL:
    			printf("LABEL ");
    			print_op(x->code.u.one.op);
    			printf(" :");
    			break;
    		case IC_FUNCTION:
    		    printf("FUNCTION ");
    			print_op(x->code.u.one.op);
    			printf(" :");
    			break;
    		case IC_ASSIGN:
    		    print_op(x->code.u.two.left);
		        printf(" := ");
				print_op(x->code.u.two.right);
    			break;
    		case IC_ADD:
    			print_op(x->code.u.three.result);
		        printf(" := ");
				print_op(x->code.u.three.op1);
				printf(" + ");
				print_op(x->code.u.three.op2);
				break;
    		case IC_SUB:
    			print_op(x->code.u.three.result);
		        printf(" := ");
				print_op(x->code.u.three.op1);
				printf(" - ");
				print_op(x->code.u.three.op2);
				break;
    		case IC_MUL:
    			print_op(x->code.u.three.result);
		        printf(" := ");
				print_op(x->code.u.three.op1);
				printf(" * ");
				print_op(x->code.u.three.op2);
				break;
    		case IC_DIV:
    			print_op(x->code.u.three.result);
		        printf(" := ");
				print_op(x->code.u.three.op1);
				printf(" / ");
				print_op(x->code.u.three.op2);
				break;
			case IC_GET_ADDR:
    		    print_op(x->code.u.two.left);
		        printf(" := &");
				print_op(x->code.u.two.right);
    			break;		

			case IC_INTO_ADDR_RIGHT:
			   	print_op(x->code.u.two.left);
		        printf(" := *");
				print_op(x->code.u.two.right);
    			break;			
			case IC_INTO_ADDR_LEFT:
				printf("*");
    		    print_op(x->code.u.two.left);
		        printf(" := ");
				print_op(x->code.u.two.right);
    			break;
            /*
            case IC_ADDR_LEFT_RIGHT:
                printf("*");
    		    print_op(x->code.u.two.left);
		        printf(" := *");
				print_op(x->code.u.two.right);
            */
    		case IC_GOTO:
    			printf("GOTO ");
    			print_op(x->code.u.one.op);
    			break;
    		case IC_IFGOTO:
    			printf("IF ");
    			print_op(x->code.u.ifgoto.op1);
    			printf(" ");
     			print_op(x->code.u.ifgoto.relop);
     			printf(" ");
     			print_op(x->code.u.ifgoto.op2);
     			printf(" GOTO ");
    			print_op(x->code.u.ifgoto.label);
    			break;  
    		case IC_RETURN:
    			printf("RETURN ");
    			print_op(x->code.u.one.op);
    			break;
    		case IC_DEC:
    			printf("DEC ");
    			print_op(x->code.u.dec.op);
    			printf(" ");
    			printf("%d",x->code.u.dec.size);
    			break;
    		case IC_ARG:
    			printf("ARG ");
                print_op(x->code.u.one.op);
    			//print_arg_op(x->code.u.one.op);
    			break;
    		case IC_CALL:
    			print_op(x->code.u.two.left);
    			printf(" := CALL ");
    			print_op(x->code.u.two.right);
    			break;
    		case IC_PARAM:
    			printf("PARAM ");
    			print_op(x->code.u.one.op);
    			break;
    		case IC_READ:
    			printf("READ ");
    			print_op(x->code.u.one.op);
    			break;    			
    		case IC_WRITE:
    			printf("WRITE ");
    			print_op(x->code.u.one.op);
    			break;    

            case IC_ARRAY_ASSIGN:
                printf("*");
                print_op(x->code.u.two.left);
		        printf(" := *");
				print_op(x->code.u.two.right);
                break;   			
    			      			 			
    	}
    	printf("\n");
        x = x->next;
    }
    //printf("\n");
}



