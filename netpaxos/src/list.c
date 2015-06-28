#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

#define SIZE 10

void print_list(node_t * head) {
    node_t * current = head;

    while (current != NULL) {
        printf("%d\n", current->val);
        current = current->next;
    }
}

int count_list(node_t * head) {
    int count = 0;
    node_t * current = head;

    while (current != NULL) {
        current = current->next;
        count++;
    }
    return count;
}

int find_list(node_t * head, int val) {
    int index = 0;
    node_t * current = head;

    while (current != NULL) {
        if (current->val == val) 
            return index;
        else {
            current = current->next;
            index++;
        }
    }
    return -1;
}

void push(node_t * head, int val) {
    node_t * current = head;
    while (current->next != NULL) {
        current = current->next;
    }

    /* now we can add a new variable */
    current->next = malloc(sizeof(node_t));
    current->next->val = val;
    current->next->next = NULL;
}

int remove_by_value(node_t ** head, int val) {
    int i = 0;
    int retval = -1;
    node_t *current = *head;
    node_t *temp_node = NULL;
    
    if ((*head)->next == NULL) {
        if ((*head)->val == val) {
            free(*head);
            retval = 0;
        }
        return retval;
    }
    
    while (current->next != NULL) {
        if (current->val == val) {
            temp_node->next = current->next;
            free(current);
            return 0;
        }
        else {
            temp_node = current;
            current = current->next;
        }
    }
    return retval;
}

int pop(node_t ** head) {
    int retval = -1;
    node_t * next_node = NULL;

    if (*head == NULL) {
        return -1;
    }

    next_node = (*head)->next;
    retval = (*head)->val;
    free(*head);
    *head = next_node;

    return retval;
}


int remove_by_index(node_t ** head, int n) {
    int i = 0;
    int retval = -1;
    node_t * current = *head;
    node_t * temp_node = NULL;

    if (n == 0) {
        return pop(head);
    }

    for (i = 0; i < n-1; i++) {
        if (current->next == NULL) {
            return -1;
        }
        current = current->next;
    }

    temp_node = current->next;
    retval = temp_node->val;
    current->next = temp_node->next;
    free(temp_node);

    return retval;
}
