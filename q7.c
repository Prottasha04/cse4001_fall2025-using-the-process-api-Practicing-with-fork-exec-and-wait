#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

int main(void) {
    pid_t rc = fork();
    if (rc < 0) { perror("fork"); exit(1); }

    if (rc == 0) {
        close(STDOUT_FILENO);
        int n = printf("child: this should NOT appear\n");
        if (n < 0) perror("printf after closing stdout");
        if (fflush(stdout) == EOF) perror("fflush(stdout)");
        _exit(0);
    } else {
        wait(NULL);
        printf("parent: stdout still works\n");
    }
    return 0;
}

/* explanation: printf() writes into the C library’s stdout buffer, not directly to the
file descriptor. After the child closes STDOUT_FILENO, the buffered printf
doesn’t error until a flush happens; fflush(stdout) then tries to write to
fd 1 and fails with EBADF (Bad file descriptor). The error message appears
because perror prints to stderr (fd 2), which is still open. The parent’s stdout
is unaffected because it has its own file descriptors after fork(). */
