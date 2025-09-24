TEST
# Assignment: Practicing the Process API
Practicing with fork, exec, wait. 

### Overview

In this assignment, you will practice using the Process API to create processes and run programs under Linux. The goal is to gain hands-on experience with system calls related to process management. Specifically, you will practice using the unix process API functions 'fork()', 'exec()', 'wait()', and 'exit()'. 

⚠️ Note: This is not an OS/161 assignment. You will complete it directly on Linux. 

Use the Linux in your CSE4001 container. If you are using macOS, you may use the Terminal (you may need to install development tools with C/C++ compilers). 

**Reference Reading**: Arpaci-Dusseau, *Operating Systems: Three Easy Pieces*, Chapter 5 (Process API Basics)
 👉 [Chapter 5 PDF](http://pages.cs.wisc.edu/~remzi/OSTEP/cpu-api.pdf)

---

### **Steps to Complete the Assignment**

1. **Accept the GitHub Classroom Invitation**
    [GitHub Link](https://classroom.github.com/a/FZh4BrQG)
2. **Set up your Repository**
   - Clone the assignment repository.
3. **Study the Reference Materials**
   - Read **Chapter 5**.
   - Download and explore the sample programs from the textbook repository:
      [OSTEP CPU API Code](https://github.com/remzi-arpacidusseau/ostep-code/tree/master/cpu-api).
4. **Write Your Programs**
   - Adapt the provided example code to answer the assignment questions.
   - Each program should be clear, well-commented, and compile/run correctly.
   - Add your solution source code to the repository.

5. **Prepare Your Report**
   - Answer the questions in the README.md file. You must edit the README.md file and not create another file with the answers. 
   - For each question:
     - Include your **code**.
     - Provide your **answer/explanation**.
6. **Submit Your Work via GitHub**
   - Push both your **program code** to your assignment repository.
   - This push will serve as your submission.
   - Make sure all files, answers, and screenshots are uploaded and rendered properly.








---
### Questions
1. Write a program that calls `fork()`. Before calling `fork()`, have the main process access a variable (e.g., x) and set its value to something (e.g., 100). What value is the variable in the child process? What happens to the variable when both the child and parent change the value of x?


```cpp
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

//explanation: the prog starts with x=100. fork() creates a child process that is a exact copy
//of parent, which is 100. then child runs its branch, changes x to 200 and prints it. Then parent
//branch runs, changes x to 300. After fork(), the parent and child have separate copies of memory.
//so, changing x in one process does not affect x in the other.   
```


2. Write a program that opens a file (with the `open()` system call) and then calls `fork()` to create a new process. Can both the child and parent access the file descriptor returned by `open()`? What happens when they are writing to the file concurrently, i.e., at the same time?

```cpp
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
That’s why the file ends up with both parent and child messages, often interleaved in unpredictable order.  
```

3. Write another program using `fork()`.The child process should print “hello”; the parent process should print “goodbye”. You should try to ensure that the child process always prints first; can you do this without calling `wait()` in the parent?

```cpp
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
unused pipe ends prevents accidental blocking and keeps things tidy.
  
```


4. Write a program that calls `fork()` and then calls some form of `exec()` to run the program `/bin/ls`. See if you can try all of the variants of `exec()`, including (on Linux) `execl()`, `execle()`, `execlp()`, `execv()`, `execvp()`, and `execvpe()`. Why do you think there are so many variants of the same basic call?

```cpp
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

static void must(int ok, const char *what){ if (!ok){ perror(what); exit(1); } }

int main(void){
    char *vec_ls[]    = { "ls", "-1", NULL };
    char *env_basic[] = { "PATH=/bin:/usr/bin", "DEMO=1", NULL };
    const char *names[] = { "execl", "execle", "execlp", "execv", "execvp", "execvpe" };

    for (int i = 0; i < 6; i++){
        pid_t rc = fork();
        must(rc >= 0, "fork");
        if (rc == 0){
            fprintf(stderr, "\n--- %s ---\n", names[i]);
            switch (i){
                case 0: execl("/bin/ls", "ls", "-1", (char*)NULL); break;
                case 1: execle("/bin/ls", "ls", "-1", (char*)NULL, env_basic); break;
                case 2: execlp("ls", "ls", "-1", (char*)NULL); break;
                case 3: execv("/bin/ls", vec_ls); break;
                case 4: execvp("ls", vec_ls); break;
                case 5:
                #ifdef __linux__
                    execvpe("ls", vec_ls, env_basic); break;
                #else
                    fprintf(stderr, "(execvpe not available on this platform)\n");
                    _exit(0);
                #endif
            }
            perror("exec"); _exit(1);
        } else {
            must(waitpid(rc, NULL, 0) >= 0, "waitpid");
        }
    }
    return 0;
}


/*explanation: The program forks a new child for each exec*() variant; the child prints a header,
then calls exec to replace itself with /bin/ls, so control doesn’t return unless exec fails.
The parent waits for that child and then moves on to the next variant. The l vs v forms differ
in how arguments are passed (list vs vector), p forms search PATH for the program name,
and e forms let you supply a custom environment. You see six directory listings because each child successfully execs ls once. 
```

5. Now write a program that uses `wait()` to wait for the child process to finish in the parent. What does `wait()` return? What happens if you use `wait()` in the child?

```cpp
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
If the child calls wait(), it fails with -1 and errno=ECHILD because it has no children to reap.  
```

6. Write a slight modification of the previous program, this time using `waitpid()` instead of `wait()`. When would `waitpid()` be useful?

```cpp
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
```

7. Write a program that creates a child process, and then in the child closes standard output (`STDOUT FILENO`). What happens if the child calls `printf()` to print some output after closing the descriptor?

```cpp
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
```

