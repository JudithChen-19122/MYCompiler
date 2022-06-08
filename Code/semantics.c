#include "semantics.h"
extern int semantic_error;
extern int fail_to_translate;
extern HNode* hashTable[HASH_SIZE];//16384

int unsignedinthash_pjw(char *name){
    unsigned int val = 0, i;
    for (; *name; ++name)
    {
        val = (val << 2) + *name;
        if (i = val & ~0x3fff)
            val = (val ^ (i >> 12)) & 0x3fff; // size of hash table: 16384
    }
    return val;
}

void add_func_read(){
    Type* newtype = (Type*)malloc(sizeof(Type));
    newtype->kind = BASIC;
    newtype->u.basic=1;
    Func_store func_s = (Func_store)malloc(sizeof(Func_store_t));
    SNode newsnode = (SNode)malloc(sizeof(SNode_t));
    func_s->para_num=0;
    func_s->paralist=NULL;
    func_s->Ret_type=newtype;
    char* func_name = "read";
    newsnode->name = func_name;
    Type* newfunctype = (Type*)malloc(sizeof(Type));
    newfunctype->kind=FUNCTION;
    newfunctype->u.function_s=func_s;
    newsnode->type = newfunctype;
    newsnode->is_para=0;
    insert_symbol_table(newsnode);
}

void add_func_write(){
    Type* newtype = (Type*)malloc(sizeof(Type));
    newtype->kind = BASIC;
    newtype->u.basic=1;
    Func_store func_s = (Func_store)malloc(sizeof(Func_store_t));
    SNode newsnode = (SNode)malloc(sizeof(SNode_t));
    func_s->para_num=1;

    FieldList newfildlist = (FieldList)malloc(sizeof(FieldList_t));
    SNode newsnode_t = (SNode)malloc(sizeof(SNode_t));
    newsnode_t->name=NULL;
    newsnode_t->type=newtype;//int
    newfildlist->sno=newsnode_t;
    newfildlist->tail=NULL;

    func_s->paralist=newfildlist;

    func_s->Ret_type=newtype;

    char* func_name = "write";
    newsnode->name = func_name;
    Type* newfunctype = (Type*)malloc(sizeof(Type));
    newfunctype->kind=FUNCTION;
    newfunctype->u.function_s=func_s;
    newsnode->type = newfunctype;
    newsnode->is_para=0;
    insert_symbol_table(newsnode);
}

int init_hash_table(){
    for(int i=0;i<HASH_SIZE;i++) {
        hashTable[i] = NULL;
    }
    semantic_error=0;
    //add function: int read();
    add_func_read();
    //add function: int write(int);
    add_func_write();
}

int insert_symbol_table(SNode snode){
    //for lab3
    if(snode->type->kind==ARRAY){
        if(snode->type->u.array.elem->kind==ARRAY)
            fail_to_translate++;
    }
    snode->var_no=-1;//for lab3
    int hash_num = unsignedinthash_pjw(snode->name);
    if(hashTable[hash_num] == NULL){
        HNode* thnode = (HNode*)malloc(sizeof(HNode));
        hashTable[hash_num] = thnode;
        thnode->nextHNode = NULL;
        thnode->sno = snode;
    }
    else{
        HNode* thnode = (HNode*)malloc(sizeof(HNode));
        thnode->sno=snode;
        thnode->nextHNode = hashTable[hash_num];
        hashTable[hash_num] = thnode;
    }
}

SNode check_symbol_table_by_name(char* tname){
    int hash_num = unsignedinthash_pjw(tname);
    HNode* temp_hnode = hashTable[hash_num];
    while(temp_hnode!=NULL){
        if(strcmp(temp_hnode->sno->name,tname)==0)
            return temp_hnode->sno;
        temp_hnode = temp_hnode->nextHNode;
    }
    return NULL;
}

//Vardec
SNode Vardec(Node* root, Type* type){
    if(root->child_count==1){ //VarDec → ID
        SNode newsnode = (SNode)malloc(sizeof(SNode_t));
        newsnode->name = root->children[0]->content;
        newsnode->type = type;
        return newsnode;
    }
    //VarDec → VarDec LB INT RB
    else{
        Node* temp=root;
        SNode newsnode = (SNode)malloc(sizeof(SNode_t));
        Type* oldtype = type;
        Type* newtype =(Type*)malloc(sizeof(Type));
        newtype->kind=ARRAY;
        newtype->u.array.size=atoi(root->children[2]->content);
        newtype->u.array.elem=type;
        //temp=temp->children[1];
        oldtype = type;
        while(temp != NULL && temp->child_count!=1){    
            newtype = (Type*)malloc(sizeof(Type));
            newtype->kind=ARRAY;
            newtype->u.array.size=atoi(temp->children[2]->content);
            newtype->u.array.elem=oldtype;
            oldtype = newtype;
            temp=temp->children[0];
        }
        newsnode->type=oldtype;
        newsnode->name=temp->children[0]->content;
        return newsnode;
    }
}

//Struct_Dec
FieldList Struct_Dec(Node* dec,Type* intype,char*struct_name){
    if(dec->child_count==3){//Dec → VarDec ASSIGNOP Exp
        semantic_error++;
        printf("Error type 15 at Line %d:defining,initialize the domaind When defining.\n",dec->row);
    }
    //Dec → VarDec
    SNode newsnode = Vardec(dec->children[0],intype);
    if(check_symbol_table_by_name(newsnode->name)!=NULL || (struct_name!=NULL && strcmp(newsnode->name,struct_name)==0)){
        semantic_error++;
        printf("Error type 15 at Line %d: Redefined field.\n",dec->row);
    }
    else{
        newsnode->is_para=0;
        insert_symbol_table(newsnode);
        FieldList newfieldlist=(FieldList)malloc(sizeof(FieldList_t));
        newfieldlist->sno=newsnode;
        newfieldlist->tail=NULL;
        return newfieldlist;
    }
    return NULL;
}

//Struct_DecList
FieldList Struct_DecList(Node* declist,Type* intype,char*struct_name){
    if(declist!=NULL&&declist->child_count==3){//DecList → Dec COMMA DecList
        Node* dec =declist->children[0];
        FieldList newfildlist = Struct_Dec(dec,intype,struct_name);
        if(newfildlist!=NULL){
            newfildlist->tail=Struct_DecList(declist->children[2],intype,struct_name);
            return newfildlist;
        }
        else
            return Struct_DecList(declist->children[2],intype,struct_name);
    }
    else{//DecList → Dec
        if(declist!=NULL)
            return Struct_Dec(declist->children[0],intype,struct_name);
    }
}

//Struct_Def
FieldList Struct_Def(Node* def, char* struct_name){
    //Def → Specifier DecList SEMI
    Type* intype = Specifier(def->children[0]);// if NULL????????
    Node* declist = def->children[1];
    return Struct_DecList(declist,intype,struct_name);
}

//Struct_DefList
FieldList Struct_DefList(Node* ddeflist,char* struct_name){
    //DefList : Def DefList | /*empty*/  
    //Def     : Specifier DecList SEMI
    //DecList : Dec | Dec COMMA DecList   
    Node* deflist = ddeflist;
    if(deflist != NULL){ // deflist -> Def DefList| empty  
        Node* def = deflist->children[0];
        FieldList newfieldlist=Struct_Def(def,struct_name);
        FieldList newfieldlist_tail = newfieldlist;
        if(newfieldlist==NULL){
            return Struct_DefList(deflist->children[1],struct_name);
        }
        else{
            while( newfieldlist_tail->tail!=NULL){
                newfieldlist_tail=newfieldlist_tail->tail;
            }
            newfieldlist_tail->tail=Struct_DefList(deflist->children[1],struct_name);
            return newfieldlist;
        }
    }
    else{
        return NULL;
    }
}

//StructSpecifier
Type* StructSpecifier(Node* structspecifier){
    Type* newtype = (Type*)malloc(sizeof(Type));
    newtype->kind = STRUCTURE;
    if(structspecifier->child_count==2){//STRUCT Tag    
        char* sid = structspecifier->children[1]->children[0]->content;//struct id
        SNode tsnode = check_symbol_table_by_name(sid);
        if(tsnode==NULL){
            semantic_error++;
            printf("Error type 17 at Line %d: Undefined structure \"%s\".\n",structspecifier->row,sid);
            newtype->u.structure=NULL;
            return NULL;
        }
        else if(tsnode->type!=NULL)
        {
            return tsnode->type;
        }
        else{
            newtype->u.structure=NULL;
            return newtype;
        }
    }
    else if(structspecifier->child_count==5){//STRUCT OptTag LC DefList RC
        Node* deflist = structspecifier->children[3];
        Node* optTag  = structspecifier->children[1];
        char* struct_name = NULL;
        if(optTag!=NULL &&optTag-> child_count!=0 &&optTag->children[0]!=NULL){
            struct_name = optTag->children[0]->content;
        }
        FieldList fildlist = Struct_DefList(deflist,struct_name);
        newtype->u.structure=fildlist;
        if(optTag!=NULL &&optTag-> child_count!=0 &&optTag->children[0]!=NULL){
            SNode newsnode = (SNode)malloc(sizeof(SNode_t));
            newsnode->name = optTag->children[0]->content;
            newsnode->type = newtype;
            if(check_symbol_table_by_name(newsnode->name)!=NULL){
                semantic_error++;
                printf("Error type 16 at Line %d: Duplicated name \"%s\".\n",optTag->row,newsnode->name);
            }
            else{
                newsnode->is_para=0;
                insert_symbol_table(newsnode);
            }
        }
        return newtype;
    }
    else{//STRUCT LC DefList RC
        Node* deflist = structspecifier->children[2];
        FieldList fildlist = Struct_DefList(deflist,"");
        newtype->u.structure=fildlist; 
        return newtype;
    }
}

//Specifier
Type* Specifier(Node* root){ 
    Type* newtype = (Type*)malloc(sizeof(Type));
    if(strcmp(root->children[0]->name,"TYPE")==0){ //TYPE
        newtype->kind = BASIC;
        if(strcmp(root->children[0]->content,"int")==0){ //int
            newtype->u.basic = 1;
        }
        else{ //float
            newtype->u.basic = 2;
        }
        return newtype;
    }
    else {
        //StructSpecifier
        Node* structspecifier=root->children[0];
        return StructSpecifier(structspecifier);
    }
}

//FunDec
int funDec(Node* fundec,Type* the_type){
    //FunDec->ID LP VarList RP|ID LP RP
    //VarList     : ParamDec COMMA VarList | ParamDec 
    //ParamDec    : Specifier VarDec 
    ///FunDec->ID LP RP
    if(fundec->child_count==3){
        SNode newsnode = (SNode)malloc(sizeof(SNode_t));
        Func_store func_s = (Func_store)malloc(sizeof(Func_store_t));
        Type* new_type = (Type*)malloc(sizeof(Type));
        char* func_name = fundec->children[0]->content; 
        func_s->Ret_type = the_type;
        func_s->para_num=0;
        func_s->paralist=NULL;
        if(check_symbol_table_by_name(func_name)!=NULL){
            //Error type 4: duplicate definition of function
            semantic_error++;
            printf("Error type 4 at Line %d: Redefined function %s.\n",fundec->row,func_name); 
            return 0;
        }
        else{
            new_type->kind=FUNCTION;
            new_type->u.function_s = func_s;
            newsnode->name = func_name;
            newsnode->type = new_type;
            newsnode->is_para=0;
            insert_symbol_table(newsnode);
            return 1;
        }
    }
    else{//FunDec->ID LP VarList RP
        Func_store func_s = (Func_store)malloc(sizeof(Func_store_t));
        SNode newsnode = (SNode)malloc(sizeof(SNode_t));
        Node* varlist =  fundec->children[2];
        int count_para=0;
        func_s-> paralist = Varlist(varlist, &count_para);
        func_s-> para_num = count_para;
        Type* new_type = (Type*)malloc(sizeof(Type));
        char* func_name = fundec->children[0]->content; 
        func_s->Ret_type = the_type;
        if(check_symbol_table_by_name(func_name)!= NULL){
            //Error type 4: duplicate definition of function
            semantic_error++;
            printf("Error type 4 at Line %d:Redefined function function %s.\n",fundec->row,func_name); 
            return 0;
        }
        else{
            new_type->kind=FUNCTION;
            new_type->u.function_s = func_s;
            newsnode->name = func_name;
            newsnode->type = new_type;
            newsnode->is_para=0;
            insert_symbol_table(newsnode);
            return 1;
        }
    }
    return 1;
}

//varlist
FieldList Varlist(Node*varlist, int* count_para){
    Node* paramdec = varlist->children[0];
    if(varlist->child_count==3){//ParamDec COMMA VarList    
        //ParamDec->Specifier VarDec
        FieldList newpara=ParamDec(paramdec);
        if(newpara!=NULL){
            (*count_para)+=1;
            newpara->tail=Varlist(varlist->children[2],count_para);
            return newpara;
        }
        else{
            return Varlist(varlist->children[2],count_para);
        }
    }
    else{
        FieldList newpara=ParamDec(paramdec);
        if(newpara!=NULL){
            (*count_para)+=1;
            return newpara;
        }
        else
        {
            return NULL;
        }
    }
}

//ParamDec
FieldList ParamDec(Node* paramdec){
    Type* type = Specifier(paramdec->children[0]);
    if(type!=NULL){
        SNode newsnode_t = Vardec(paramdec->children[1],type);
        if(check_symbol_table_by_name(newsnode_t->name)!=NULL){
            semantic_error++;
            printf("Error type 3 at Line %d:Redefined variable %s.\n",paramdec->children[1]->row,newsnode_t->name);   
            return NULL;
        }
        else{
            newsnode_t->is_para=1;
            if(newsnode_t->type->kind==ARRAY){//for lab3
                fail_to_translate++;
            }
            insert_symbol_table(newsnode_t);
            FieldList newfildlist = (FieldList)malloc(sizeof(FieldList_t));
            newfildlist->sno=newsnode_t;
            newfildlist->tail=NULL;
            return newfildlist;
        }
    }
    return NULL;
}

//EXP
Type* Exp(Node* eexp){
    Node* exp = eexp;
    if(exp==NULL) return NULL;
    if(exp->child_count==1){
        if(strcmp(exp->children[0]->name,"ID")==0){
            SNode snode = check_symbol_table_by_name(exp->children[0]->content);
            if(snode==NULL){
                semantic_error++;
                printf("Error type 1 at Line %d: Undefined variable %s .\n",exp->children[0]->row,exp->children[0]->content); 
                return NULL;
            }
            else return snode->type;
        }
        else if(strcmp(exp->children[0]->name,"INT")==0){
            Type* newtype = (Type*)malloc(sizeof(Type));
            newtype->kind = BASIC;
            newtype->u.basic=1;
            return newtype;
        }
        else if(strcmp(exp->children[0]->name,"FLOAT")==0){
            Type* newtype = (Type*)malloc(sizeof(Type));
            newtype->kind = BASIC;
            newtype->u.basic=2;
            return newtype;   
        }
    }
    else if(exp->child_count==3){
        // Exp ASSIGNOP Exp 
        if(strcmp(exp->children[1]->name,"ASSIGNOP")==0){ 
            if(exp->children[0]->child_count==4 && strcmp(exp->children[0]->children[1]->name,"LP")==0||
            (exp->children[0]->child_count==3 &&strcmp(exp->children[0]->children[1]->name,"LP")==0)
            )
            {
                semantic_error++;
                printf("Error type 6 at Line %d:The left-hand side of an assignment must be a variable.\n",exp->children[0]->row); 
                return NULL;
            }
            else if(
              exp->children[0]->child_count==4  ||
            strcmp(exp->children[0]->children[0]->name,"ID")==0||(exp->children[0]->child_count>1)&&(
            strcmp(exp->children[0]->children[1]->name,"DOT")==0||
            strcmp(exp->children[0]->children[1]->name,"ASSIGNOP")==0)
            ){
                Type* type1=Exp(exp->children[0]);
                Type* type2=Exp(exp->children[2]);
                if(type1==NULL||type2==NULL){
                    return NULL;
                }
                else if(equal_type(type1,type2)==0){
                    //The expression types on both sides of the assignment number do not match
                    semantic_error++;
                    printf("Error type 5 at Line %d:Type mismatched for assignment.\n",exp->children[1]->row); 
                }
                return type1;
            }
            else{
                semantic_error++;
                printf("Error type 6 at Line %d:The left-hand side of an assignment must be a variable.\n",exp->children[0]->row); 
                return NULL;
            }
        }
        // LP Exp RP  (exp)
        else if(strcmp(exp->children[1]->name,"Exp")==0){
            Type* type1=Exp(exp->children[1]);
            return type1;
        }
        //ID LP RP  func()
        else if(strcmp(exp->children[0]->name,"ID")==0){
            SNode ss=check_symbol_table_by_name(exp->children[0]->content);
            Type* type1=NULL;
            if(ss!=NULL)
                type1=ss->type;
            if(type1==NULL){
                semantic_error++;
                printf("Error type 2 at Line %d:Undefined function %s.\n",exp->children[0]->row,exp->children[0]->content); 
                return NULL;
            }
            else if(type1->kind!=FUNCTION){
                semantic_error++;
                printf("Error type 11 at Line %d: It is not a function.\n",exp->children[0]->row); 
                return type1;
            }
            else{
                if(type1->u.function_s->para_num!=0){
                    semantic_error++;
                    printf("Error type 9 at Line %d:The number or type of real participation parameters do not match.\n",exp->children[0]->row);
                    return type1->u.function_s->Ret_type;
                }
                else{
                    return type1->u.function_s->Ret_type;
                }
            }
            return type1;
        }
        //Exp DOT ID
        else if(strcmp(exp->children[1]->name,"DOT")==0){
            Type* type1=Exp(exp->children[0]);
            if(type1 == NULL||type1->kind!=STRUCTURE){
                //Use "." for unstructured body variables Operator
                semantic_error++;
                printf("Error type 13 at Line %d: Illegal use of '.' . \n",exp->children[0]->row);
                return NULL;
            }
            else{
                int i=0;
                FieldList type2=type1->u.structure;
                while(type2!=NULL){
                    if(strcmp(type2->sno->name,exp->children[2]->content)==0){
                            return type2->sno->type;   
                    }
                    type2=type2->tail;
                }
                semantic_error++;
                printf("Error type 14 at Line %d:  Non-existent field. \n",exp->children[0]->row);
                return NULL;
            }
            return NULL;
        }
        else{
            Type* type1=Exp(exp->children[0]);
            Type* type2=Exp(exp->children[2]);
            if(type1==NULL||type2==NULL) return NULL;
            if(strcmp(exp->children[1]->name,"RELOP")==0){
                if(type1->kind!=BASIC||type2->kind!=BASIC){
                    semantic_error++;
                    printf("Error type 7 at Line %d: Type mismatched for operands.\n",exp->children[1]->row); 
                }
            }
            else if(type1->kind!=BASIC||type2->kind!=BASIC||equal_type(type1,type2)==0){
                //The expression types on both sides of the assignment number do not match
                semantic_error++;
                printf("Error type 7 at Line %d: Type mismatched for operands.\n",exp->children[1]->row); 
            }
            if(strcmp(exp->children[1]->name,"RELOP")==0){ //转换成int类型
                Type* newtype = (Type*)malloc(sizeof(Type));
                newtype->kind = BASIC;
                newtype->u.basic=1;
                return newtype;   
            }
            else{
                return type1;

            }
        }
    }
    else if(exp->child_count==2){
            Type* type1=Exp(exp->children[1]);
            if(type1==NULL) return NULL;
            if(type1->kind!=BASIC){
                //The expression types on both sides of the assignment number do not match
                semantic_error++;
                printf("Error type 5 at Line %d: Type mismatched for assignment.\n",exp->children[1]->row); 
            }
            if(strcmp(exp->children[0]->name,"NOT")==0){
                Type* newtype = (Type*)malloc(sizeof(Type));
                newtype->kind = BASIC;
                newtype->u.basic=1;
                return newtype;
            }
            else{
                return type1;
            }
    }
    else if(exp->child_count==4 && strcmp(exp->children[0]->name,"ID")==0){
        //ID LP Args RP func(args)
        SNode ss=check_symbol_table_by_name(exp->children[0]->content);
        Type* type1=NULL;
        if(ss!=NULL)
            type1=ss->type;
        if(type1==NULL){
            semantic_error++;
            printf("Error type 2 at Line %d:Undefined function %s.\n",exp->children[0]->row,exp->children[0]->content); 
            return NULL;
        }
        else if(type1->kind!=FUNCTION){
            semantic_error++;
            printf("Error type 11 at Line %d:It is not a function.\n",exp->children[0]->row); 
            return NULL;
        }
        else{
            if(type1->u.function_s->para_num==0){
                semantic_error++;
                printf("Error type 9 at Line %d:The number or type of real participation parameters do not match.\n",exp->children[0]->row);
                return NULL;
            }
            else{
                int count=0;
                FieldList paralist = type1->u.function_s->paralist;
                Node* args = exp->children[2];
                while(paralist != NULL && args!=NULL &&args->child_count==3){
                    if(equal_type(paralist->sno->type,Exp(args->children[0]))==0){ 
                        count=-10;
                        break;   
                    }
                    count++;
                    paralist=paralist->tail;
                    args = args->children[2];
                }
                if(paralist != NULL){
                    if(equal_type(paralist->sno->type,Exp(args->children[0]))==1){
                        count++; 
                    }
                    else{
                        count=-10;
                    }
                }
                else{
                    count=-10;
                }
                if(count!=type1->u.function_s->para_num){
                    semantic_error++;
                    printf("Error type 9 at Line %d:The number or type of real participation parameters do not match.\n",exp->children[0]->row);
                }
                return type1->u.function_s->Ret_type;
            }
        }
        return type1;
    }
    else {//Exp LB Exp RB  a[][] 
        Type* type1= Exp(exp->children[0]);
        if(type1==NULL) return NULL;
        if(type1->kind!=ARRAY){
            semantic_error++;
            printf("Error type 10 at Line %d:It is not an array.\n",exp->children[0]->row);
            return NULL;
        }
        Type* type2= Exp(exp->children[2]);
        if(type2!= NULL && (type2->kind!=BASIC||type2->u.basic!=1)){
            semantic_error++;
            printf("Error type 12 at Line %d:Non integer in array access operator [...].\n",exp->children[0]->row);
            return NULL;
        }
        return type1->u.array.elem;
    }
}
/*Exp: Exp ASSIGNOP Exp      

        | Exp AND Exp           
        | Exp OR Exp            
        | Exp RELOP Exp         
        | Exp PLUS Exp          
        | Exp MINUS Exp         
        | Exp STAR Exp          
        | Exp DIV Exp


        | LP Exp RP 

        | MINUS Exp             
        | NOT Exp    

        | ID LP Args RP   //       
        | ID LP RP        //      
        | Exp LB Exp RB   //      
        | Exp DOT ID      // 

        | ID              //  
        | INT                   
        | FLOAT 
*/               

//compare types
int equal_type(Type* type1, Type* type2){
    if(type1==NULL||type2==NULL) return 0;
    if(type1->kind!=type2->kind) return 0;
    if(type1->kind==BASIC){
        if(type1->u.basic!=type2->u.basic) return 0;
        else return 1;
    }
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
    if(type1->kind==ARRAY){
        Type* type11= type1;
        Type* type12= type2;
        while(type11->kind==ARRAY && type12->kind==ARRAY){
            if(type11->u.array.size!=type12->u.array.size){
                return 0;
            }
            type11=type11->u.array.elem;
            type12=type12->u.array.elem;
        }
        if(type11->kind==ARRAY || type12->kind==ARRAY)
            return 0;
        if(equal_type(type11,type12)==0)
            return 0;
        return 1;
    }
    return 0;
}

//defList
void defList(Node*ddeflist){
    //DefList : Def DefList | /*empty*/  
    //Def     : Specifier DecList SEMI
    //DecList : Dec | Dec COMMA DecList   
    Node* deflist = ddeflist;
    if(deflist != NULL){ // deflist -> Def DefList| empty  
        Node* def = deflist->children[0];
        Def(def);
        defList(deflist->children[1]);
    }
}

//Def
void Def(Node* def){
    //Def → Specifier DecList SEMI
    Type* intype = Specifier(def->children[0]);// if NULL????????
    Node* declist = def->children[1];
    DecList(declist,intype);
}

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
//Dec
void Dec(Node*dec,Type*intype){
    if(dec->child_count==3 && equal_type(Exp(dec->children[2]),intype)==0){//Dec → VarDec ASSIGNOP Exp
        semantic_error++;
        printf("Error type 5 at Line %d:Type mismatched for assignment.\n",dec->row); 
    }
    else{ //Dec → VarDec
        SNode newsnode = Vardec(dec->children[0],intype);
        if(check_symbol_table_by_name(newsnode->name)!=NULL){
            semantic_error++;
            if(intype->kind==ARRAY)
                printf("Error type 16 at Line %d: Duplicated name \"%s\".\n",dec->row,newsnode->name);
            else
                printf("Error type 3 at Line %d: Redefined variable %s.\n",dec->row,newsnode->name); 
            }
        else{
                newsnode->is_para=0;
                insert_symbol_table(newsnode);
        }
    }
}

//Stmt
void Stmt(Node* stmtt, Type* intype){
    Node* stmt = stmtt;
    //Stmt-> WHILE LP Exp RP Stmt
    if(strcmp(stmt->children[0]->name,"WHILE")==0){
        Exp(stmt->children[2]);
        Stmt(stmt->children[4],intype);
    }
    //Stmt->IF LP Exp RP Stmt ELSE Stmt
    else if(stmt->child_count==7){
        Exp(stmt->children[2]);
         Stmt(stmt->children[4],intype);
          Stmt(stmt->children[6],intype);
    }
    //Stmt->IF LP Exp RP Stmt %prec LOWER_THAN_ELSE
    else if(stmt->child_count==5){
        Exp(stmt->children[2]);
        Stmt(stmt->children[4],intype);
    }
    //Stmt->RETURN Exp SEMI
    else if(stmt->child_count==3){
        Type* type1=Exp(stmt->children[1]);
        if(type1 == NULL || equal_type(type1,intype)==0){
            semantic_error++;
            printf("Error type 8 at Line %d:Type mismatched for return.\n",stmt->children[1]->row); 
        }
    }
    //Stmt->Exp SEMI 
    else if(stmt->child_count==2){
        Exp(stmt->children[0]);
    }
    //CompSt
    else if(stmt->child_count==1){
        Node* compst = stmt->children[0];
        Node* deflist =  compst->children[1];
        Node* stmtlist =  compst->children[2];
        defList(deflist);
        stmtList(stmtlist,intype);
    }
}
//stmtList
void stmtList(Node* stmtlist, Type* ret_type){
 //StmtList -> Stmt StmtList |/*empty*/ 
 //Stmt-> WHILE LP Exp RP Stmt
 //IF LP Exp RP Stmt ELSE Stmt
 //IF LP Exp RP Stmt %prec LOWER_THAN_ELSE 
 //RETURN Exp SEMI
 //CompSt
 //Exp SEMI 
    Node* stmtlist_t = stmtlist;
    while(stmtlist_t!=NULL){
        //printf("here1\n");
        Node* stmt = stmtlist_t->children[0];
        //printf("here2\n");
        Stmt(stmt,ret_type);
        //printf("here3\n");
        stmtlist_t=stmtlist_t->children[1];
        //printf("here4\n");
    }
}
//ExtDecList
void ExtDecList(Node* extdeclist,Type*the_type){
    //ExtDecList -> VarDec COMMA ExtDecList
    if(extdeclist->child_count==3){
        SNode newsnode = Vardec(extdeclist->children[0],the_type);
        if(check_symbol_table_by_name(newsnode->name)!=NULL){
            semantic_error++;
            printf("Error type 3 at Line %d: Redefined variable %s .\n",extdeclist->children[0]->row, newsnode->name); 
        }
        else
        {
            newsnode->is_para=0;
            insert_symbol_table(newsnode);
        }

        ExtDecList(extdeclist->children[2],the_type);
    }
    else{
    //ExtDecList -> VarDec
        SNode newsnode = Vardec(extdeclist->children[0],the_type);
        if(check_symbol_table_by_name(newsnode->name)!=NULL){
            semantic_error++;
            printf("Error type 3 at Line %d:Redefined variable %s .\n",extdeclist->children[0]->row, newsnode->name); 
        }
        else{
            newsnode->is_para=0;
            insert_symbol_table(newsnode);
        }
    }
}

//ExtDef
void ExtDef(Node* root){
    //Specifier SEMI
    if(root->child_count==2){
        Node* specifier_t=root->children[0];
        Type* the_type = Specifier(specifier_t);
        return;
    } 
    Node* specifier_t=root->children[0];
    Type* the_type = Specifier(specifier_t);
    if(the_type==NULL) return;//在specifier中已经打印了错误17
    if(strcmp(root->children[2]->name,"SEMI")==0){//Specifier ExtDecList SEMI
        Node* extdeclist = root->children[1];
        ExtDecList(extdeclist,the_type);
    }
    else{//Specifier FunDec CompSt
        //FunDec-> ID LP VarList RP|ID LP RP
        Node* fundec = root->children[1];
        int res_fundec = funDec(fundec,the_type);
        //CompSt->LC DefList StmtList RC 
        Node* compst = root->children[2];
        Node* deflist =  compst->children[1];
        Node* stmtlist =  compst->children[2];
        //printf("ExtDef 1\n");
        defList(deflist);
        //printf("ExtDef 2\n");
        stmtList(stmtlist,the_type);
        //printf("ExtDef 3\n");
    }
}

//semantic_dfs
void semantic_dfs(Node*parent){
    if(parent==NULL) return;
    if(strcmp(parent->name,"ExtDef")==0){
       ExtDef(parent);
    }
    else{
        for(int i=0;i<parent->child_count;i++){
            semantic_dfs(parent->children[i]);
        }
    }
}
//sematic check
void semantic_check(Node*root){
    init_hash_table();
    semantic_dfs(root);
}


