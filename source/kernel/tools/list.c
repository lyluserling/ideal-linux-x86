#include "tools/list.h"
#include "comm/cpu_instr.h"
void list_init(list_t *list)
{
    list->first = list->last = (list_node_t *)0;
    list->count = 0;
}

void list_insert_first(list_t * list, list_node_t * node){
    node->next = list->first;
    node->pre = (list_node_t *)0;

    if(list_is_empty(list)){
        list->last = node;
        list->first = node;
    }
    else{
        list->first->pre = node;//先将原first节点的pre指针指向新插入的节点
        list->first = node;
    }
    list->count++;//节点计数加1
}

void list_insert_last(list_t * list, list_node_t * node){//将节点插入到链表尾部
    node->pre = list->last;//先将新节点的pre指针指向原last节点
    node->next = (list_node_t *)0;
        if(list_is_empty(list)){
            list->first = node;
        }
        else{
            list->last->next = node;//先将原last节点的next指针指向新插入的节点
            
        }    
        list->last = node;        
        list->count++;    
}

list_node_t * list_remove_first(list_t * list){
    if(list_is_empty(list)){
        return (list_node_t *)0;
    }
    
    list_node_t * remove_node = list->first;
        list->first = remove_node->next;
        if(list->first==(list_node_t *)0){//如果原first节点已经是最后一个节点，则last节点也要指向(list_node_t *)0
            list->last = (list_node_t *)0;
        }
        else{
            list->first->pre = (list_node_t *)0;
        }
        remove_node->pre= remove_node->next = (list_node_t *)0;
        list->count--;
        return remove_node;//返回被删除的节点
}

list_node_t * list_remove(list_t * list, list_node_t * node){
    if(node == list->first){
        list->first = node->next;
    }
    if(node == list->last){
        list->last = node->pre;
    }
    if(node->pre){
        node->pre->next= node->next;
    }
    if(node->next){
        node->next->pre = node->pre;
    }

    node->pre = node->next = (list_node_t *)0;
    list->count--;
    return node;
}