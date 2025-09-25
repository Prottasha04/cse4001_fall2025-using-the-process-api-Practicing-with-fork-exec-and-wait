#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    pid_t rc = fork();
    if (rc < 0) { perror("fork"); exit(1); }

    if (rc == 0) { 
        printf("child: running (pid=%d)\n", getpid());
        _exit(42); 
    } else {
        int status = 0;
        pid_t w = waitpid(rc, &status, 0);
        printf("parent: waitpid() returned pid=%d\n", (int)w);
        if (WIFEXITED(status))
            printf("parent: child exit status=%d\n", WEXITSTATUS(status));
    }
    return 0;
}

/*explanation: waitpid() lets you wait for a specific child (by PID) instead of “any child,”
which matters when you have multiple children and don’t want to reap the wrong one.
It also supports flags like WNOHANG (don’t block) and WUNTRACED/WCONTINUED
(react to stop/continue), giving finer control. This avoids race conditions
(another child exiting first) and is essential for shells, job control,
and servers that fork many workers. */
