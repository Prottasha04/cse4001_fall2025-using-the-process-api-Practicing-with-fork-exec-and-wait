#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

int main(void) {

    int fd = open("fork_output.txt", O_CREAT | O_TRUNC | O_WRONLY /* | O_APPEND */, 0644);
    if (fd < 0) { perror("open"); exit(1); }

    int rc = fork();
    if (rc < 0) {
        perror("fork");
        exit(1);
    } else if (rc == 0) { 
        const char *msg = "child: hello\n";
        for (int i = 0; i < 5; i++) {
            if (write(fd, msg, strlen(msg)) < 0) { perror("write(child)"); exit(1); }
        }
        _exit(0);
    } else { 
        const char *msg = "parent: hello\n";
        for (int i = 0; i < 5; i++) {
            if (write(fd, msg, strlen(msg)) < 0) { perror("write(parent)"); exit(1); }
        }
        wait(NULL);
    }

    close(fd);
    return 0;
}
/*explanation: When open() is called before fork(), the child inherits the same open file description as the parent, 
so both share the same file descriptor and file offset. This means both processes can write to the same file. 
Each individual write() call is atomic, so lines won’t get cut in half.
but the order of writes depends on the scheduler. whichever process runs first moves the shared offset forward.
That’s why the file ends up with both parent and child messages, often interleaved in unpredictable order. */
