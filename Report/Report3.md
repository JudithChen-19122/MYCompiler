## LAB3-Report

##### 191220013 陈奕诺

### 一、实验要求

在词法分析、语法分析和语义分析程序的基础上，将C−−源代码翻译为中间代码，并将中间代码输出成线性结构。

### 二、实验内容

#### 1.构建中间代码数据结构

考虑到需要生成线性中间代码的需要，为了便于后续对于代码的优化，增加代码的灵活性，我在本次实验中采取了双向链表来表示中间代码，存储链表的头指针和尾指针，便于后续操作和处理。

```c
struct InterCodes
{
    struct InterCode code;
    struct InterCodes *prev;
    struct InterCodes *next;
};
```

注意`InterCode`是单句中间代码的存储结构。对于`InterCode`我们的具体设计如下：

```c
struct InterCode
{
    enum{ 
        IC_FUNCTION,IC_PARAM,IC_RETURN,IC_LABEL,IC_GOTO,IC_ARG,IC_WRITE,IC_READ, //1 OP
        IC_ASSIGN,IC_CALL,IC_GET_ADDR, IC_ARRAY_ASSIGN,IC_DEC,//2 OP
        IC_ADD,IC_SUB,IC_MUL,IC_DIV,//3 OP
        IC_IFGOTO//4 OP
        } kind;
    union {
        struct
        {
            Operand op;
        } one;
        ...
     }u;
}
```

将代码依据实际生成需要，分为上述几类，不同的类型对应不同的存储，且只存储`Operand`和`kind`，后续生成并输出具体的代码时，依据`kind`类型，生成相应的语句，并调用`Operand`的输出函数，结合`Operand`的类型和内容进行输出即可。这里我们简单了解一下`Operand`的结构设计：

```c
typedef struct Operand_ *Operand;
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
    } u;
};
```

如上所示，不同的`Operand`对应于不同的类型，这里进行简单的解释。其中`VARIABLE_OP`对应源程序中的变量，在中间代码中记为`v1,v2...`；`CONSTANT_OP` 对应于源代码中的常量；`ADDRESS_OP`对应于源程序中的指针变量，在本实验中，只作为函数参数（数组、结构体）的形式出现；`FUNCTION_OP`对应中间代码的函数;`LABEL_OP`对应中间代码的标签；`TEMP_VAR_OP`对应中间代码中的临时变量，在中间代码中记为`t1,t2...`；`TEMP_ADDR_OP`对应中间代码存储地址的临时变量，同样记为`t1,t2...`；最后，`RELOP_OP`表示条件比较符。

在实验指导中，推荐的结构是采用联合体结构来进行存储，这里换用了结构体，是考虑到后续对于结构体和数组等的处理中，需要更多的判断信息，所以采取了结构体进行处理，虽然浪费了部分空间，但是方便了后续的判断和处理。

此外，为了方便函数`Args`对应代码的倒序输出，我们利用一个链表结构来对`Args`中的`Operand`进行倒序存储，如下所示：

```c
struct Arg_List_{
    Operand op;
	struct Arg_List_ *next;
};
```

至此，本实验所需的存储结构已经完成了设计。

#### 2.生成代码前的准备工作

考虑到生成代码的需要，在语义分析阶段，我们首先对于无法生成代码的情况做了判断，也就是对于数组做了特别的判断。同时考虑到函数参数的特殊处理，在变量的`snode`中添加了 `int is_para`的信息存储。为了便于生成代码中原变量的查找，变量的`snode`中还添加了 `int var_no`的信息存储，进行变量标号存储，为后续的代码生成做好相应的准备工作。以下为更改后的`Snode`：

```c
//Node of symbol table
typedef struct SNode_{
    char* name;
    Type* type;
    //for lab3
    int var_no;
    int is_para; //0: not para 1:para
}SNode_t;
```

#### 3.生成中间代码

与语义分析相似，生成中间代码的操作也要基于实验一中构建的语法分析树，在进行了词法分析、语法分析和语义分析之后，在没有相关错误的前提下，我们基于语法分析树生成中间代码。注意，为了使得代码的结构更加清晰，我们将语义分析与生成代码的过程分离开来，重新遍历语法树来进行中间代码生成。对于中间代码的生成，与语义分析类似的，我们针对几乎每一个语法单元都构建了对应的函数来进行相应的分析。

在编写代码时，主要的思路是对语法树进行自顶向下的分析，这样更便于对语法树建立整体的认识，也符合生成代码的思维逻辑。考虑上层的节点时，可以确定需要知道的各个子节点不同的信息，也就知道了各个子节点应该确定的`Operand`等信息，从而构造相应的函数。在编写子节点代码时，父节点的代码编写已经基本完成，于是我们可以根据子节点的父节点和兄弟节点信息，来进一步完成子节点的编写。

总体而言，是自顶向下分析和编写代码，但是信息，尤其是`Operand`是自底向上传输的。对于每个节点对应的代码生成函数的编写，我这次没有考虑过多的优化，直接采取了讲义中提供的翻译模式，将设计的数据结构和函数套用进去，进行了一些简单的处理，这里不过多的赘述。值得注意的是，为了将生成的代码以设计的结构体进行存储，我们根据不同的`Intercode`类型，主要依据`operand`的数量，设计了不同的函数，将代码信息存储到结构体中，并插入到双向链表，具体函数如下：

```c
void create_one_intercode(Operand op, int kind);
void create_two_intercode(Operand left, Operand right, int kind);
...
```

而具体的代码输出过程，在之前已经有所提及，不再进行具体的描述。下面我们简单讲讲对于结构体的处理。为了便于地址的计算，我们首先设计了一个`int get_size(Type* type);` 用来计算其相对于数据结构起始地址的偏移量。我们可以参考一部分代码(有省略)：

```c
			...
		    if(s_id->kind != ADDRESS_OP && s_id->kind!=TEMP_ADDR_OP)
		    {
			    t1 = new_temp();
                t1->kind=TEMP_ADDR_OP;
				...
			    create_two_intercode(t1, s_id, IC_GET_ADDR);
		    }
		    else t1 = s_id;
			... 
		    t2->kind = CONSTANT_OP;
		    t2->u.value = offset;
            ...
            if(c_snode->type->kind==BASIC){
                place->u.is_addr=0;
                place->u.for_add_addr=1;
                place->kind = TEMP_ADDR_OP;
		        place->u.name = exp->children[2]->content;
		        create_three_intercode(place,t1,t2,IC_ADD);
            }
            else{
                place->u.is_addr=1;
                place->u.for_add_addr=1;
              	...
            }
```

首先获取上层结构体的信息，如果获取到的`Operand`不是一个地址量，那么我们就通过构造引用语句来获取对应的地址，如果是一个地址量，则直接获取。接下来获取上层结构体的类型信息`Type`，如果保留了名称则直接查表，否则通过语法分析得到。进而得到`FieldList`信息，可以生成相应的地址计算的代码，从而得到相应的`Operand`。值得注意的是，对于结构体，其返回的数据存储的是一个地址，但是这个地址可能对应的是一个Basic类型的量，也可能是一个结构体或者数组的开头地址，针对不同的情况，我们进行不同的设计，对于Basic，我们要将`is_addr`设为0，反之，设为0，这样可以便于上层节点操作的判断和便于后续的`Operand`输出判断。

