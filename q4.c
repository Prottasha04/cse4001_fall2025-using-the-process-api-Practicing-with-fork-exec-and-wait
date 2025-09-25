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
and e forms let you supply a custom environment. You see six directory listings because each child successfully execs ls once. */
