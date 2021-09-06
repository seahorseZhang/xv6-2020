#include "kernel/param.h"
#include "kernel/types.h"
#include "user/user.h"

#define MAX_LINE 256

int readline(char *line) {
  char in;
  int len = 0;
  while (read(0, &in, sizeof(char)) == sizeof(char)) {
    if (in == '\n') {
      break;
    }
    line[len++] = in;
  }
  line[len] = '\0';
  return len;
}

int main(int argc, char *argv[]) {
  char *para[MAXARG];
  int i = 1;
  int nflag = 1;
  if (argc < 2) {
    fprintf(2, "xargs format should [xargs xxx]");
    exit(1);
  }

  // parse xargs para
  int offset = 1;
  if (argc >= 3 && strcmp("-n", argv[1]) == 0 && strcmp("1", argv[2]) == 0) {
    nflag = 1;
    offset = 3;
  }
  i = offset;
  for (; i < argc; ++i) {
    para[i - offset] = argv[i];
  }

  char line[MAX_LINE];
  while (readline(line) > 0) {
    // printf("readline: %s\n", line);
    para[i - offset] = line;
    para[i - offset + 1] = 0;

    int pid = fork();
    if (pid == 0) {
      exec(para[0], para);
    } else if (pid > 0) {
      wait(0);
    } else {
      fprintf(2, "fork fail");
      exit(1);
    }
    if (!nflag) {
      break;
    }
  }
  exit(0);
}