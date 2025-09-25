#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    int p[2];
    if (pipe(p) == -1) { perror("pipe"); exit(1); }

    int rc = fork();
    if (rc < 0) {
        perror("fork");
        exit(1);
    } else if (rc == 0) { 
        close(p[0]); 
        printf("hello\n");
        fflush(stdout); 
        write(p[1], "x", 1); 
        close(p[1]);
        _exit(0);
    } else {   
        close(p[1]);
        char buf;
        read(p[0], &buf, 1); 
        close(p[0]);
        printf("goodbye\n");
    }
    return 0;
}

/*explanation: It uses a 'pipe' for synchronization. The parent and child inherit the pipe
after `fork()`: the parent closes the write end and blocks on `read()`,
while the child closes the read end, prints “hello”, then writes a single byte to the pipe.
That write wakes the parent’s `read()`, so the parent then prints “goodbye”. Closing the
unused pipe ends prevents accidental blocking and keeps things tidy. */
