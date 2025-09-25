#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    int x = 100;  
    printf("Before fork: x = %d (pid: %d)\n", x, (int) getpid());

    int rc = fork();
    if (rc < 0) {
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0) {
        
        printf("Child before change: x = %d (pid: %d)\n", x, (int) getpid());
        x = 200; 
        printf("Child after change: x = %d (pid: %d)\n", x, (int) getpid());
    } else {
        
        printf("Parent before change: x = %d (pid: %d)\n", x, (int) getpid());
        x = 300;
        printf("Parent after change: x = %d (pid: %d)\n", x, (int) getpid());
    }
    return 0;
}

/*explanation: the prog starts with x=100. fork() creates a child process that is a exact copy
of parent, which is 100. then child runs its branch, changes x to 200 and prints it. Then parent
branch runs, changes x to 300. After fork(), the parent and child have separate copies of memory.
so, changing x in one process does not affect x in the other. */
