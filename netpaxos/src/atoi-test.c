#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    char line[10];
    fgets(line, 9, stdin);
    int value = atoi(line);
    printf("%d\n", value);
    return 0;
}
