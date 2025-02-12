

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "dplist.h"




/*
 * The real definition of struct list / struct node
 */

struct dplist_node {
    dplist_node_t *prev, *next;
    void *element;
};

struct dplist {
    dplist_node_t *head;

    void *(*element_copy)(void *src_element);

    void (*element_free)(void **element);

    int (*element_compare)(void *x, void *y);
};


dplist_t *dpl_create(// callback functions
        void *(*element_copy)(void *src_element),
        void (*element_free)(void **element),
        int (*element_compare)(void *x, void *y)
) {
    dplist_t *list;
    list = malloc(sizeof(struct dplist));
    list->head = NULL;
    list->element_copy = element_copy;
    list->element_free = element_free;
    list->element_compare = element_compare;
    return list;
}

//in free, we check if first the list is null, we return nothing because there is nothing
//to process, then if list is not  null, we process each node by freeing its data/element
// if boolean free_element is true and also freeing the node. then after
// freeing each node, we free the head of list and then point it to NULL. We use valgrind
//to check for memory leaks. O memory leaks found
void dpl_free(dplist_t **list, bool free_element) {

    //TODO: add your code here

    if (*list == NULL )
        return ;
    dplist_node_t *currentNode = (*list)->head;
    while (currentNode != NULL) {
        if (free_element){
            (*list)->element_free(&(currentNode->element));
        }
        dplist_node_t *next = currentNode->next;
        free(currentNode);
        currentNode = next;
    }

    free(*list);
    *list=NULL;
}

//same code from exercise 1, we just use the call back function if insert_copy is true
dplist_t *dpl_insert_at_index(dplist_t *list, void *element, int index, bool insert_copy) {

    //TODO: add your code here
    dplist_node_t *ref_at_index, *list_node;
    if (list == NULL) return NULL;

    list_node = malloc(sizeof(dplist_node_t));
    //callback function usage here.
    if (insert_copy) {
        list_node->element = list->element_copy(element);
    } else {
        list_node->element = element;
    }

    // pointer drawing breakpoint
    if (list->head == NULL) { // covers case 1
        list_node->prev = NULL;
        list_node->next = NULL;
        list->head = list_node;
        // pointer drawing breakpoint
    } else if (index <= 0) { // covers case 2
        list_node->prev = NULL;
        list_node->next = list->head;
        list->head->prev = list_node;
        list->head = list_node;
        // pointer drawing breakpoint
    } else {
        ref_at_index = dpl_get_reference_at_index(list, index);
        assert(ref_at_index != NULL);
        // pointer drawing breakpoint
        if (index < dpl_size(list)) { // covers case 4
            list_node->prev = ref_at_index->prev;
            list_node->next = ref_at_index;
            ref_at_index->prev->next = list_node;
            ref_at_index->prev = list_node;
            // pointer drawing breakpoint
        } else { // covers case 3
            assert(ref_at_index->next == NULL);
            list_node->next = NULL;
            list_node->prev = ref_at_index;
            ref_at_index->next = list_node;
            // pointer drawing breakpoint
        }
    }
    return list;

}

//removing at index make sure to take into consideration all full cases
// case 1: list is null or empty
//case 2: remove node at the beginning of list
//case 3: remove nodes somewhere in the list using the dpl_get_reference_at_index
//case 4: removing at end of the list
dplist_t *dpl_remove_at_index(dplist_t *list, int index, bool free_element) {

    //TODO: add your code here
    if (list == NULL)
        return NULL;
    if(list->head==NULL){
        return list;
    }

    if (index <= 0) {
        dplist_node_t *node_to_remove = list->head;
        if (node_to_remove != NULL) {
            if (free_element ) {
                list->element_free(&(node_to_remove->element));
            }
            list->head = node_to_remove->next;
            free(node_to_remove);
        }
    } else {
        dplist_node_t *ref_at_index = dpl_get_reference_at_index(list, index);
        if (ref_at_index != NULL) {
            dplist_node_t *node_to_remove = ref_at_index;
            ref_at_index->prev->next = ref_at_index->next;
            if (ref_at_index->next != NULL) {
                if (free_element ) {
                    list->element_free(&(node_to_remove->element));
                }
                ref_at_index->next->prev = ref_at_index->prev;
            }
            free(node_to_remove);
        }
    }

    return list;

}

//size for all cases, we start from count=1 because we want the size of the list, not index based.
int dpl_size(dplist_t *list) {

    dplist_node_t *dummy;
    //TODO: add your code here
    if (list == NULL) {
        return -1;
    }
    int counter=1;
    dummy = list->head;
    if (list->head==NULL){
        return 0;
    }
    while (dummy->next != NULL) {
        counter++;
        dummy = dummy->next;
    }

    return counter;

}

//same code as get_reference_at_index but here we get the element instead of the pointer to node.
void *dpl_get_element_at_index(dplist_t *list, int index) {
    dplist_node_t *dummy = NULL;
    //TODO: add your code here
    if(list==NULL || list->head==NULL){
        return 0;
    }
    if (index <= 0) {
        dummy = list->head;
    }
    int count=0;
    dummy= list->head;
    while(dummy->next !=NULL && count<index){
        count++;
        dummy=dummy->next;
    }
    return dummy->element;

}

//here null is required to be returned by the problem conditions on the .h file but
//how can NULL be returned where you have to return int type???
int dpl_get_index_of_element(dplist_t *list, void *element) {

    //dplist_node_t *dummy = NULL;
    //TODO: add your code here
    //What do you mean by return NULL??
    if (list==NULL || list->head==NULL){
        return -1;
    }
    int count=0;
    dplist_node_t *dummy=list->head;
    while (dummy != NULL) {
        if (list->element_compare(dummy->element,element)==0 ) {
            return count;
        }
        count++;
        dummy = dummy->next;
    }
    return -1;

}

//here the same code as get element at index
dplist_node_t *dpl_get_reference_at_index(dplist_t *list, int index) {

    //TODO: add your code here
    //dplist_node_t *dummy=NULL;
    if (list == NULL || list->head == NULL) {
        return NULL;
    }
    dplist_node_t *dummy=list->head;

    if (index <= 0) {
        return dummy;
    }

    int count = 0;
    while (dummy-> next != NULL && count < index) {
        count++;
        dummy = dummy->next;

    }

    return dummy;


}

//self-explanatory code
void *dpl_get_element_at_reference(dplist_t *list, dplist_node_t *reference) {

    //TODO: add your code here
    //dplist_node_t *dummy=NULL;
    if (list == NULL || list->head == NULL) {
        return NULL;
    }
    if(reference==NULL){
        return NULL;
    }

    dplist_node_t *dummy=list->head;
    while (dummy != NULL) {
        if (dummy==reference) {
            return dummy->element;
        }
        dummy = dummy->next;
    }
    return NULL;

}


