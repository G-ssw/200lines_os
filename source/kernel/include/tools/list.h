#ifndef LIST_H
#define LIST_H

#include "comm/types.h"

#define NULL ((void*)0)

typedef struct ListNode {
    void* data;
    struct ListNode* next;
    struct ListNode* prev;
} ListNode;

typedef struct List {
    ListNode* head;
    ListNode* tail;
    int size;
} List;

#define relative_offset(type, member) ((uint32_t) &((type *)0)->member)
#define absolute_offset(node, type, member) ((type *)(node ? ((uint32_t)(node) - relative_offset(type, member)) : 0))

static inline void ListNode_init(ListNode* node, void* data) {
    node->data = data;
    node->next = NULL;
    node->prev = NULL;
}

static inline void List_init(List* list) {
    list->head = (ListNode*)0;
    list->tail = (ListNode*)0;
    list->size = 0;
}

static inline ListNode* ListNode_Prev(ListNode* node){
    return node->prev;
}

static inline ListNode* ListNode_Next(ListNode* node){
    return node->next;
}

static inline int List_isEmpty(List* list) {
    return list->size == 0;
}

static inline int List_size(List* list) {
    return list->size;
} 

static inline ListNode* List_Front(List* list) {
    return list->head;
}

static inline ListNode* List_Tail(List* list) {
    return list->tail;
}

void List_insert_head(List* list, ListNode* node);
void List_insert_tail(List* list, ListNode* node);
ListNode* List_remove_head(List* list);
ListNode* List_remove_tail(List* list);
ListNode* List_remove_node(List* list, ListNode* node);

#endif