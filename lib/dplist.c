/**
 * \author Abubakr Ehab Samir Nada
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "dplist.h"

/*
 * definition of error codes
 * */
#define DPLIST_NO_ERROR 0
#define DPLIST_MEMORY_ERROR 1 // error due to mem alloc failure
#define DPLIST_INVALID_ERROR 2 //error due to a list operation applied on a NULL list 

#ifdef DEBUG
#define DEBUG_PRINTF(...) 									                                        \
	do {											                                            \
		fprintf(stderr,"\nIn %s - function %s at line %d: ", __FILE__, __func__, __LINE__);	    \
		fprintf(stderr,__VA_ARGS__);								                            \
		fflush(stderr);                                                                         \
	} while(0)
#else
#define DEBUG_PRINTF(...) (void)0
#endif




#define DPLIST_ERR_HANDLER(condition, err_code)                         \
	do {                                                                \
		if ((condition)) DEBUG_PRINTF(#condition " failed\n");      \
		assert(!(condition));                                       \
	} while(0)


/*
 * The real definition of struct list / struct node
 */




	dplist_t *dpl_create(
			void* (*element_copy)(void *element),
			void (*element_free)(void **element),
			int(*element_compare)(void *x, void *y)

			) {
		dplist_t *list;
		list = malloc(sizeof(struct dplist));
		DPLIST_ERR_HANDLER(list == NULL, DPLIST_MEMORY_ERROR);
		list->head = NULL;
		list->element_copy = element_copy;
		list->element_free = element_free;
		list->element_compare = element_compare;
		
		return list;
	}

void dpl_free(dplist_t **list, bool free_element) {
	if(*list != NULL){
		dplist_node_t* current = (*list)->head; 
		if(current != NULL){
			while(current != NULL){
				dplist_node_t* tmp = current;  
				current = current->next ; 

				if(free_element && (*list)->element_free != NULL){
					(*list)->element_free(&tmp->element); 
					
				}	
				free(tmp); 
			}

		}
		free(*list);
	}


	*list = NULL; 

}

dplist_t *dpl_insert_at_index(dplist_t *list, void *element, int index, bool insert_copy) {
	if(list == NULL){
		return NULL; 
	}
	

	dplist_node_t * new_node = malloc(sizeof(dplist_node_t));  
	if(list->head ==NULL){
		
		new_node->next = NULL; 
		new_node->prev = NULL; 
		if(insert_copy){
            new_node->element = list->element_copy(element); 
			}
        else{
            new_node->element = element; 
        }
		list->head = new_node; 
		return list;  
	}
	if(index <= 0){
 
		new_node->next = list->head; 
		new_node->prev = NULL; 
        if(insert_copy){
            new_node->element = list->element_copy(element); }
        else{
            new_node->element = element; 
        } 
		list->head->prev = new_node; 	
		list->head = new_node; 
		return list; 
	}
	int size = dpl_size(list); 
	if(index > size-1){
		//insert at end; 
		dplist_node_t* current = list->head; 
		while(current->next != NULL){		
			current = current->next; 
		}

		new_node->next = NULL; 
		new_node->prev = current; 
        if(insert_copy){
            new_node->element = list->element_copy(element); }
        else{
            new_node->element = element; 
        } 
       
		current->next = new_node; 	
		
		return list; 
	} 
	dplist_node_t * current = list->head; 
	int count = 0; 
	while(count != index){
		current = current->next; 		
		count ++; 	
	}
	//copy

	new_node->next = current; 
	new_node->prev = current->prev; 
	current->prev->next = new_node; 
	current->prev = new_node; 
	if(insert_copy){
		new_node->element = list->element_copy(element); 
	}
	else{
		new_node->element = element; 
	}

	 
	return list; 	
}

dplist_t *dpl_remove_at_index(dplist_t *list, int index, bool free_element) {
	 
	if(list == NULL){return NULL;}
	if(list->head == NULL){return list; }
	
	if(list->head->next == NULL && list->head->prev == NULL){
		// 1 element list; 
		if(free_element){
		list->element_free(&list->head->element); 
	}
	
		free(list->head); 
		list->head = NULL;
		 
		return list; 
	}
	int size = dpl_size(list); 
	if(index <= 0){ // Redundant could just change index to 0 
		index = 0; 
		dplist_node_t* current = list->head; 

		if(current->next != NULL){current->next->prev = NULL;}
		list->head = current->next; 
		if(free_element){
		list->element_free(&current->element); 
	}
	
		free(current);
		return list; 
	}
	if(index > size-1){index = size-1; } 
	
	
	dplist_node_t* current = list->head; 
	while (index != 0){
		current = current->next; 
		index --; 
	}
	
	
	if(current->next != NULL){
		current->next->prev = current->prev;		
	} 
	if(current->prev!=NULL){
			current->prev->next = current->next;
	}
	if(free_element){
		list->element_free(&current->element); 
	}
	
	free(current); 
	current = NULL; 
	return list; 

}

int dpl_size(dplist_t *list) {
	int size = 0; 
	if (list == NULL){
		return -1; 
	} 
	if(list->head == NULL){return 0; }
	dplist_node_t* current = list->head; 
	while(current != NULL){
		
		current = current->next; 
		 size++;			
	}

	return size; 

}

void *dpl_get_element_at_index(dplist_t *list, int index) {

	if(list == NULL || list->head == NULL){return NULL; }
	if(index <= 0 ) {return list->head->element; } 
	int size = dpl_size(list); 
	if(index > size-1){
		index = size-1;
	} 
	dplist_node_t* current = list->head;
	int count = 0; 
	while (current != NULL){
		if(count == index){break; }
		current = current->next;  
		count++; 
	}


	return current->element;
}

int dpl_get_index_of_element(dplist_t *list, void *element) {
	int index = 0; 
	// traverse list 
	if(list == NULL){
		return -1; 
	}
	dplist_node_t* current = list->head; 

	while (current != NULL) {
		if(list->element_compare(current->element,element)==0){
			
			return index; 
		} 
		current = current->next; 
		index++;
	}
	
	return -1; 
}

dplist_node_t *dpl_get_reference_at_index(dplist_t *list, int index) {
	if(list == NULL || list->head == NULL){return NULL; }
	dplist_node_t* current = list->head; 
	
	int size = dpl_size(list); 
	if(index > size-1){index = size-1;}
	while(index != 0){
		current = current->next; 	
		--index; 
	}
	return current; 
}

void *dpl_get_element_at_reference(dplist_t *list, dplist_node_t *reference) {
	if(list == NULL || list->head == NULL || reference == NULL){
		return NULL; }
	dplist_node_t * current = list->head; 
	while(current != NULL){
		
		if(current == reference){
			return current->element; 
		}
		current = current->next; 
		}


	return NULL; 
}


dplist_node_t *dpl_get_first_reference(dplist_t *list){
	if(list==NULL || list->head == NULL){
		return NULL; 
	}
	return list->head; 
}


dplist_node_t *dpl_get_last_reference(dplist_t *list){
	if(list == NULL || list->head == NULL){return NULL;} 
	dplist_node_t* current = list->head; 
	while(current->next != NULL){
		current = current->next; 
	}
	return current; 
}


dplist_node_t *dpl_get_next_reference(dplist_t *list, dplist_node_t *reference){
	if(list== NULL ||list->head == NULL || reference == NULL){return NULL; } 
	dplist_node_t* current = list->head; 
	while(current != reference && current->next != NULL){
		current = current->next ; 
	}
	return current->next; 
}


dplist_node_t *dpl_get_previous_reference(dplist_t *list, dplist_node_t *reference){

	if(list==NULL || list->head == NULL || reference == NULL){return NULL; } 
	dplist_node_t* current = list->head; 
	while(current != reference && current->next != NULL){
		current = current->next ; 
	}
	return current->prev; 
}


dplist_node_t *dpl_get_reference_of_element(dplist_t *list, void *element){
	if(list == NULL || list->head == NULL ){return NULL;} 
	dplist_node_t* current = list->head; 	
	while(current != NULL){
	
		if(list->element_compare(element, current->element) == 0){
			return current; 
		}
		current = current->next ;  
	}
	return NULL; 
	}


int dpl_get_index_of_reference(dplist_t *list, dplist_node_t *reference){
	int count = 0; 
	if(list == NULL || list->head == NULL || reference == NULL) {return -1; }
	dplist_node_t* current = list->head; 
	while(current != NULL){
		if(current == reference){
			return count; 
		}
		current = current->next; 
		count++; 
	}
	return -1; 
}


dplist_t *dpl_insert_at_reference(dplist_t *list, void *element, dplist_node_t *reference, bool insert_copy){
	// 1 way to do it get index of reference + insert index but it's 2x slower; 
	if(list == NULL || list->head == NULL || reference == NULL){return NULL; } 
	dplist_node_t* current = list->head; 
	while (current != reference && current != NULL){
		if(current == reference){
			dplist_node_t* new_node = malloc(sizeof(dplist_node_t)); 
			if(insert_copy){
				new_node->element = list->element_copy(element); 
			}
			
			else{
				new_node->element = element; }
			new_node->prev = current->prev; 
			new_node->next = current; 
            current->prev->next = new_node; 
			current->prev = new_node; 	
            
		}		
	}
	return list; 
}

	dplist_t *dpl_insert_sorted(dplist_t *list, void *element, bool insert_copy){
		if(list == NULL) {return NULL;}
		// ADD ELEMENT ANYWHERE
		list = dpl_insert_at_index(list, element, 0, insert_copy); 
		// MERGE SORT
       		// merge_sort(list); 			
		return list; 

	}

	
	dplist_t *dpl_remove_at_reference(dplist_t *list, dplist_node_t *reference, bool free_element){
        if(list == NULL || list->head == NULL || reference == NULL){
            return NULL; }
        dplist_node_t* current = list->head;
		
		int index = 0; 
        while(current != NULL){
            if(current == reference) {
				return dpl_remove_at_index(list, index, free_element); 
				/*if(free_element){
					list->element_free(&current->element); 
				}
				
				if(current->prev){
					current->prev->next = current->next;
				}
				if(current->next){
					current->next->prev = current->prev; 
				}
				
				free(current); 
				current = NULL; 
				
				return list ;*/ 
            }
			current = current->next; 
			index ++; 
        }
		
        return list;
    }


	dplist_t *dpl_remove_element(dplist_t *list, void *element, bool free_element){
         
        if(list == NULL || list->head == NULL  ){
            return NULL; }
        dplist_node_t* current = list->head;
        while(current != NULL){
            if(list->element_compare(current->element , element) == 0) {
					if(current->prev == NULL){
					return dpl_remove_at_index(list,0,free_element); 
				}
				 else if(current->prev!=NULL){current->prev->next = current->next;
					
				 }
				if(current->next == NULL){
					return dpl_remove_at_index(list,dpl_size(list)+1,free_element); 
				}
               
                else if(current->next!=NULL){current->next->prev = current->prev;} 
                if(free_element){
                    list->element_free(&current->element); }
                free(current); 
                break; 
            }
        }
        return list; 
	}

void  merge_sort(dplist_t* s_list, dplist_node_t* list) {
	if(list == NULL ){return; }
    dplist_node_t* head; 
    head = list;  
    dplist_node_t* a; 
    dplist_node_t* b; 
    divide_linked_list(head, &a, &b); 

    merge_sort(s_list, a); 
    merge_sort(s_list, b); 

    merge(s_list,a,b);
} 
dplist_node_t* merge(dplist_t* list, dplist_node_t*l1, dplist_node_t*l2){
  dplist_node_t* result = NULL; 
    if(l1 == NULL){return l2; } 
    else if (l2 == NULL){return l1; } 
    
    if(list->element_compare(l1->element, l2->element) >= 0){
        result = l2; 
        result->next = merge(list, l2->next, l1); 
    }
    else{
        result = l1; 
        result->next = merge(list, l1->next, l2); 
    }
    return result; 
}
void divide_linked_list(dplist_node_t* head, dplist_node_t** sub_list1, dplist_node_t**sub_list2){
    dplist_node_t* fast; 
    dplist_node_t* slow;
    slow = head;   
    fast = head; 
    while(1){
        fast = fast->next; 
        if (fast->next != NULL){
            fast = fast->next; 
            slow = slow->next; 
        }
        else{
        break;
        }
    }
    *sub_list1 = head;
    *sub_list2= slow->next;  
    slow->next->prev = NULL; 
    slow->next = NULL; 
}




    
