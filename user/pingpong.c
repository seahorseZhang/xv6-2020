#include "kernel/types.h"
#include "user/user.h"

int main() {
    int fd[2];
    char buf[20];
    int pipefd = pipe(fd);
    (void)pipefd;
    int pid = fork();
    if (pid == 0) {
        close(0);
        dup(fd[0]);
        close(fd[0]);
        read(0, buf, 19);
        fprintf(1, "%d: received ping\n", getpid());
        close(0);
        close(1);
        write(fd[1], "pong", strlen("pong"));
        close(fd[1]);
    } else if(pid > 0) {
        write(fd[1], "ping", strlen("ping"));
        close(fd[1]);
        close(0);
        dup(fd[0]);
        read(0, buf, 19);
        fprintf(1, "%d: received pong\n", getpid());
        close(1);
    } else {
        fprintf(1, "fork unsucc");
    }
    exit(0);
}