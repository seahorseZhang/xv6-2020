#include "kernel/types.h"
#include "user/user.h"

__attribute__((noreturn)) void sieve(int leftfd) {
  int prime;
  int size = read(leftfd, &prime, sizeof(int));
  if (size == 0) {
    close(leftfd);
    exit(0);
  }
  fprintf(1, "prime %d \n", prime);
  int midPip[2];
  int ret = pipe(midPip);
  if (ret < 0) {
    fprintf(2, "primes pipe fail \n");
    exit(1);
  }

  int pid = fork();
  if (pid == 0) {
    close(0);
    dup(midPip[0]);
    close(midPip[1]);
    sieve(0);
  } else if (pid > 0) {
    int input;
    while (read(leftfd, &input, sizeof(int)) != 0) {
      if (input % prime != 0) {
        (void)write(midPip[1], &input, sizeof(int));
      }
    }
    close(midPip[1]);
    wait(0);
  } else {
    fprintf(1, "fork fail \n");
    exit(1);
  }
  exit(0);
}

int main() {
  int initialPipe[2];

  int pipefd = pipe(initialPipe);
  if (pipefd < 0) {
    fprintf(2, "create pipe fail \n");
    exit(1);
  }

  int pid = fork();
  if (pid == 0) { // child
    close(0);
    dup(initialPipe[0]);
    close(initialPipe[0]);
    close(initialPipe[1]);
    sieve(0);
  } else if (pid > 0) {
    close(0);
    close(initialPipe[0]);
    fprintf(1, "primes 2 \n");
    for (int i = 2; i < 35; ++i) {
      if (i % 2 != 0) {
        (void)write(initialPipe[1], &i, sizeof(i));
      }
    }
    close(initialPipe[1]);
    wait(0);
  } else {
    fprintf(2, "create child unsucc, pid: %d \n", getpid());
    exit(1);
  }
  exit(0);
}