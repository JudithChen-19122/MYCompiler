#pragma once
#include"node.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define HASH_SIZE 16384 

typedef struct Type_ Type;
typedef struct FieldList_* FieldList;
typedef struct SNode_* SNode;
typedef struct Func_store_* Func_store;
typedef struct Func_store_ Func_store_t;  

struct Type_
{ 
    enum { BASIC, ARRAY, STRUCTURE, FUNCTION} kind;
    union
    {   
        // 基本类型
        int basic; //int:1 float:2
        // 数组类型信息包括元素类型与数组大小构成
        struct { Type* elem; int size; } array;
        //结构体类型信息是一个链表(STRUCTURE_KIND)
        FieldList structure;
        //char* structer_name;
        // 函数定义
        Func_store function_s;

    } u;
};
 
//结构体类型信息
typedef struct FieldList_
{
    SNode sno;
    FieldList tail; // 下一个域
}FieldList_t;

// 函数定义
struct Func_store_{
    int para_num; //参数个数
    Type* Ret_type; //返回值类型
    FieldList paralist; //参数列表
};

//Node of symbol table
typedef struct SNode_{
    char* name;
    Type* type;
    //for lab3
    int var_no;
    int is_para; //0: not para 1:para
    
}SNode_t;

//Node of hash table
typedef struct HNode_{
    SNode sno;
    struct HNode_* nextHNode;
}HNode;


//hash函数
int unsignedinthash_pjw(char* name);
int init_hash_table();
void add_func_read();
void add_func_write();

//symbol_table
SNode check_symbol_table_by_name(char* tname);
int insert_symbol_table(SNode snode);


//compare type
int equal_type(Type* type1, Type* type2);

//checking tool
void ExtDef(Node* root);
void ExtDecList(Node* extdeclist,Type*the_type);

Type* Specifier(Node* root);
Type* StructSpecifier(Node* structspecifier);
FieldList Struct_DefList(Node* ddeflist,char* struct_name);
FieldList Struct_Def(Node* def, char* struct_name);
FieldList Struct_DecList(Node* declist,Type* intype,char*struct_name);
FieldList Struct_Dec(Node* dec,Type* intype,char*struct_name);

int funDec(Node* fundec,Type* the_type);
FieldList Varlist(Node*varlist, int* count_para);
FieldList ParamDec(Node* paramdec);


SNode Vardec(Node* root, Type* type);

void defList(Node*ddeflist);
void Def(Node* def);
void DecList(Node* declist,Type*intype);
void Dec(Node*dec,Type*intype);


void stmtList(Node* stmtlist, Type* ret_type);
void Stmt(Node* stmtt, Type* intype);

Type* Exp(Node* eexp);

//sematic check
void semantic_check();
void semantic_dfs(Node*root);


