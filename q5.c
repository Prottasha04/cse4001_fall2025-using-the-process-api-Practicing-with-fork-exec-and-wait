#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

int main(void) {
    pid_t rc = fork();
    if (rc < 0) { perror("fork"); exit(1); }

    if (rc == 0) { 
        printf("child: running (pid=%d)\n", getpid());
        pid_t w = wait(NULL); 
        if (w == -1) perror("child wait");
        _exit(7); 
    } else {
        int status = 0;
        pid_t w = wait(&status);
        printf("parent: wait() returned pid=%d\n", (int)w);
        if (WIFEXITED(status))
            printf("parent: child exit status=%d\n", WEXITSTATUS(status));
    }
    return 0;
}

/*explanation: After fork(), the parent calls wait(&status), which blocks until the child exits; the kernel
then returns the child’s PID to the parent and fills status with the child’s exit code.
If the child calls wait(), it fails with -1 and errno=ECHILD because it has no children to reap. */
