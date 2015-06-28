typedef struct node {
    int val;
    struct node *next;
} node_t;

void print_list(node_t * head);
int find_list(node_t * head, int val);
void push(node_t * head, int val);
int remove_by_value(node_t ** head, int val);
int pop(node_t ** head);
int remove_by_index(node_t ** head, int n);
