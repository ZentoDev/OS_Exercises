
#include <stdio.h>
#include <unistd.h>

int main() {
    fork();
    printf("A\n");
    fork();
    printf("1\n");
    return 0;
}