#include "node.h"


Node* createNode(char*name, char*content){
    Node* tnode = (Node*)malloc(sizeof(Node));
    strcpy(tnode->name, name);
    strcpy(tnode->content, content);
    tnode-> row = yylineno;
    tnode->child_count = 0;
    tnode->children = NULL;
    return tnode;
}

void load_child(int child_count, Node* parent, ...){
        va_list temp; 
        va_start(temp, parent);
        parent-> child_count = child_count;
        parent-> children = (Node**)malloc(sizeof(Node*)*child_count);
        Node* tchild = va_arg(temp, Node*);
        parent->children[0] = tchild;
        int row = parent->row;
        if(row > tchild->row) row=tchild->row;
        for(int i=1; i<child_count; i++){
                tchild = va_arg(temp, Node*);
                if(tchild != NULL && row > tchild->row) row=tchild->row;
                parent->children[i]=tchild;
        }
        parent->row = row;
        va_end(temp);
}

int my_atoi(char s[32]){
        if(s[0] != '0') return atoi(s);
        else if(s[1] == 'x'|| s[2]=='X') return strtol(s,NULL,16);
        else return strtol(s,NULL,8);
}



void print_the_tree(Node* parent, int indentation){
        if(parent==NULL) return;
        for(int i=0;i<indentation;i++) printf(" ");
        if(parent->child_count !=0){
                printf("%s (%d)\n", parent->name, parent->row);
                for(int i=0;i<parent->child_count;i++)
                        print_the_tree(parent->children[i],indentation+2);
        }
        else{
                if(strcmp(parent->name, "ID")==0)
                        printf("%s: %s\n", parent->name, parent->content);
                else if(strcmp(parent->name, "TYPE")==0)
                        printf("%s: %s\n", parent->name, parent->content);
                else if(strcmp(parent->name, "INT")==0)
                        printf("%s: %d\n", parent->name, my_atoi(parent->content));
                else if(strcmp(parent->name, "FLOAT")==0)
                        printf("%s: %f\n", parent->name, atof(parent->content));
                else 
                        printf("%s\n", parent->name);
        }

}


void print_tree(Node* root){
        print_the_tree(root,0);
}
   
