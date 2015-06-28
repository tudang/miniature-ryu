#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

#define SIZE 10

int check_order(FILE *file);

int main(int argc, char *argv[]) {

    FILE *file; 
    int i;
    if (argc > 1) {
        for (i = 1; i < argc; i++) { 
            file = fopen(argv[i], "r");
            if (file == NULL) {
                perror("Opening file");
                exit(1);
            }
            check_order(file);
        }
    }
    else
        check_order(stdin);

    
    return 0;
}

int check_order(FILE *file) {
    int reorder = 0;
    int lost = 0;
    int count = 0;

    char line[SIZE];
    char str_number[6];
    int sequence = 1;
    node_t *head = NULL;
    head = malloc(sizeof(node_t));
    if (head == NULL) {
        return 1;
    }

    while ( fgets(line, SIZE, file) != NULL) /* read a line */
    {
        strncpy(str_number, line+2, 6);
        int value = atoi(str_number);
        count++;
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

    print_list(head);
    lost = count_list(head) - 1; // Do not count head value
    
    printf("Lost:%d\tReodered:%d\tCount:%d\n", lost, reorder, count);

    if (file != stdin) 
        fclose(file);
}
