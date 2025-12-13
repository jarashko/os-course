#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
    int pipes[2][2];

    if (pipe(pipes[0]) < 0 || pipe(pipes[1]) < 0) {
        fprintf(2, "pingpong: cannot create pipes\n");
        exit(1);
    }

    int pid = fork();
    if (pid < 0) {
        fprintf(2, "pingpong: fork failed\n");
        exit(1);
    }

    if (pid == 0) {
        close(pipes[0][1]); 
        close(pipes[1][0]); 

        char buf[6];
        int n = read(pipes[0][0], buf, sizeof(buf));
        if (n > 0) {
            if (n >= sizeof(buf)) buf[sizeof(buf)-1] = 0;
            printf("%d: got %s\n", getpid(), buf);
        }
        close(pipes[0][0]);

        static char reply[] = "pong";
        write(pipes[1][1], reply, sizeof(reply));
        close(pipes[1][1]);

        exit(0);
    } else {
        close(pipes[0][0]); 
        close(pipes[1][1]); 

        static char msg[] = "ping";
        write(pipes[0][1], msg, sizeof(msg));
        close(pipes[0][1]);

        char buf[6];
        int n = read(pipes[1][0], buf, sizeof(buf));
        if (n > 0) {
            if (n >= sizeof(buf)) buf[sizeof(buf)-1] = 0;
            printf("%d: got %s\n", getpid(), buf);
        }
        close(pipes[1][0]);

        wait(0);
        exit(0);
    }
}
