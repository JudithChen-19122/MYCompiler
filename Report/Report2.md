## LAB2-Report

##### 191220013 陈奕诺

### 一、实验要求

在词法分析和语法分析程序的基础上编写一个程序，对C−−源代码进行语义分析和类型检查，并打印分析结果。

### 二、实验内容

#### 1. 构造符号表

符号表是进行语义分析和类型检查的基础，绝大多数的语义分析和类型检查都是通过填表和查表来完成的。考虑到分析和检查所需的信息，我们将符号的表的每一个表项设定如下：

```c
typedef struct SNode_{
    char* name;
    Type* type;
}SNode_t;
```

也就是将每个符号的名称和类型存储下来，供后续检查所需。而考虑到填表和查表的操作较为频繁，而这些操作在本次实验不考虑作用域的情况下，不需要对符号表的遍历，所以我们采取了HASH表的结构来对每一个表项进行存取，以便于后续的操作。为了将符号表的构造和查找，或者说是对HASH表的操作与后续的分析和检查分离开，这里编写了`SNode check_symbol_table_by_name(char* tname);`用于通过符号名来查找表项，返回相应的表项和`int insert_symbol_table(SNode snode);`用于向符号表中插入对应的表项。

此外，我们还应该建立一个合理的数据结构来存储类型信息，也就是`SNode`中的`Type`，依据现有的信息建立结构体如下，用`FieldList`和`Func_store`构造的链表分别存储结构体结构和函数信息：

```c
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
```

到目前为止，我们已经完成了对于符号表的基本操作，接下来就要进行具体的语义分析和类型检查。

#### 2.语义分析和类型检查

对于代码的语义分析和类型检查是建立在实验一的词法分析和语法分析程序的基础上完成的。在实验一种，我们已经完成了对于语法分析树的构建，这里我们只要在没有词法和语法错误的前提下，借助生成的语法分析树来进行分析和检查即可。

考虑到语义分析和类型检查的需要，我们通过一个DFS的前序遍历来完成分析。考虑到C--的语言文法中有大量的递归文法，我一开始的想法是通过WHILE循环来进行解决，这导致我的第一版代码中有大量的重复代码，且函数的复用性不强，同时由于WHILE循环的边界条件等分析比较困难，加上大量的链表操作，使得程序在测试阶段出现了大量的`segmentation fault`，虽然前期已经实现了语义分析和类型检查的功能要求，但是在后续，我对代码进行了重构，通过函数递归来实现while循环的功能。

为了实现语义分析和类型检查的要求，我们针对几乎每一个语法单元都构建了对应的函数来进行相应语法层的分析和检查。在编写代码时，主要的思路是对语法树进行自顶向下的分析，这样更便于对语法树建立整体的认识，考虑上层的节点时，可以知道要检查各子节点的关系是否合理，要知道各个子节点不同的信息，也就知道了各个子节点应该返回的值，从而构造相应的函数。对于子节点而言，依据L文法的特点，我们知道对于子节点，其左边的兄弟节点信息有时也是判断的重要前提，从而在前序遍历是，父节点已经有了左边的兄弟节点对应函数的返回值从而传入右侧，于是我们可以知道子节点的函数参数可以有哪些，并通过对子节点的进一步分析，自底向上的确定参数。

总而言之，对于语法树节点对应的函数的构造，主要是自顶向下确定返回值类型，自底向上确定函数参数。通过在每个节点的函数进行相应的语义分析和类型检查，来最终实现整体的语义分析和类型检查。主要的函数如下：

```c
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
```

对于变量、参数、函数、结构体、数组等的定义，我们进行查表操作和一系列判断后，若有误，则输出问题，若确认没有错误，就进行填表操作。对于`Stmt`更多的是通过查表操作来判断对应类型的变量，其进行的操作是否符合要求，否则输出问题所在。在确定节点的语法产生式时，主要是通过对子节点的数量和名称的判断来实现，从而对于不同的语法产生式，我们要有不同的操作和判断。如下`DecList`为例：

```c
//DecList
void DecList(Node* declist,Type*intype){
    if(declist!=NULL&&declist->child_count==3){//DecList → Dec COMMA DecList
        Node* dec =declist->children[0];
        Dec(dec,intype);
        DecList(declist->children[2],intype);
    }
    else{//DecList → Dec
        if(declist!=NULL)
            Dec(declist->children[0],intype);
    }
}
```

此外还需要特别关注的是对类型等价的判断，我们在这里设计了`int equal_type(Type* type1, Type* type2);` 函数，若返回0，则类型不等价；若返回1，则等价。本次实验要求完成选做要求2.3，也就是结构体间的类型等价机制由名等价改为结构等价，其处理难度主要集中在循环和递归上,具体操作如下，检查每一个`fieldlist`的类型：

```c
//compare types
int equal_type(Type* type1, Type* type2){
    ...
    if(type1->kind==STRUCTURE)
    {
        if(type1!=NULL && type2!=NULL){
            FieldList fieldlist1= type1->u.structure;
            FieldList fieldlist2= type2->u.structure;
            while(fieldlist1!=NULL&&fieldlist2!=NULL){
                if(equal_type(fieldlist1->sno->type,fieldlist2->sno->type)==0){
                    return 0;
                }
                fieldlist1 = fieldlist1->tail;
                fieldlist2 = fieldlist2->tail;
            }
            if(fieldlist1!=NULL||fieldlist2!=NULL){
                return 0;
            }
            else 
                return 1;
        }
        else
        {
            return 0;
        }
    }
    ...
}
```
