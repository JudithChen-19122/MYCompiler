#pragma once
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern int yylineno;

typedef struct TNode{
        char name[32];
        char content[32];
        int row;
        int child_count;
        struct TNode** children;
}Node;

void load_child(int child_count, Node* parent, ...);
Node* createNode(char*name, char*content);
void print_tree(Node* root);