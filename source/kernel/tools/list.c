#include "tools/list.h"

void List_insert_head(List* list, ListNode* node) {
    if (List_isEmpty(list)) {
        list->head = node;
        list->tail = node;
    } else {
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }
    list->size++;
}

void List_insert_tail(List* list, ListNode* node) {
    if (List_isEmpty(list)) {
        list->head = node;
        list->tail = node;
    } else {
        node->prev = list->tail;
        list->tail->next = node;
        list->tail = node;
    }
    list->size++;
}

ListNode* List_remove_head(List* list){
    if(list == NULL || List_isEmpty(list)){
        return NULL;
    }
    ListNode* node = list->head;
    if(list->size == 1){
        list->head = NULL;
        list->tail = NULL;
    } else {
        list->head = node->next;
        node->next = NULL; // 断开与链表的连接
        node->prev = NULL;
        list->head->prev = NULL;
    }
    list->size--;
    return node;
}

ListNode* List_remove_tail(List* list){
    if(list == NULL || List_isEmpty(list)){
        return NULL;
    }
    ListNode* node = list->tail;
    if(list->size == 1){
        list->head = NULL;
        list->tail = NULL;
    } else {
        list->tail = node->prev;
        node->prev = NULL; // 断开与链表的连接
        node->next = NULL;
        list->tail->next = NULL;
    }
    list->size--;
    return node;
}

ListNode* List_remove_node(List* list, ListNode* node){
    if(list == NULL || List_isEmpty(list) || node == NULL){
        return NULL;
    }
    if(node == list->head){
        return List_remove_head(list);
    } else if(node == list->tail){
        return List_remove_tail(list);
    } else {
        node->prev->next = node->next;
        node->next->prev = node->prev;
        node->next = NULL; // 断开与链表的连接
        node->prev = NULL;
        list->size--;
        return node;
    }
}

