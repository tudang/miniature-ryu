#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 10

typedef struct node {
    int val;
    struct node *next;
} node_t;

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

int main(int argc, char *argv[]) {

    FILE *file; 

    if (argc > 1) {
        file = fopen(argv[1], "r");
        if (file == NULL) {
            perror("Opening file");
            exit(1);
        }
    }
    else
        file = stdin;
    

    int reorder = 0;
    int lost = 0;

    char line[SIZE];
    char str_number[6];
    int sequence = 0;
    node_t *head = NULL;
    head = malloc(sizeof(node_t));
    if (head == NULL) {
        return 1;
    }

    while ( fgets(line, SIZE, file) != NULL) /* read a line */
    {
        strncpy(str_number, line+2, 6);
        int value = atoi(str_number);
        //printf("sequence:%d\tvalue:%d\n", sequence, value);
        if (sequence == value) {
            sequence++;
            continue;
        }
        else if(sequence > value) {
            reorder++;
            int index = find_list(head, value); 
            printf("value:%d\tindex:%d\n", value, index);
            
            if (index != -1) 
                remove_by_index(&head, index);
        }
        else {
            while (sequence < value) { /* loop until the current value */
                push(head, sequence);
                sequence++;
            }
            sequence++; /* next value */
            //print_list(head);
            printf("----\n");
        }
    }

    //print_list(head);
    lost = count_list(head) - 1; // Do not count head value
    
    printf("Lost:%d\tReodered:%d\n", lost, reorder);

    //fclose(file);
    return 0;
}
