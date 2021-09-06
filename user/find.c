#include "kernel/fcntl.h"
#include "kernel/fs.h"
#include "kernel/stat.h"
#include "kernel/types.h"
#include "user/user.h"

char *my_strrchr(char *str, char ch) {
  char *last = 0;
  char *pos = str;
  while (*pos) {
    if (*pos == ch) {
      last = pos;
    }
    ++pos;
  }
  return last;
}

void find(char *path, char *object) {
  char buf[64];
  int fd;

  if ((fd = open(path, O_RDONLY)) < 0) {
    return;
  }

  struct stat st;
  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch (st.type) {
  case T_FILE:
    char *rp = my_strrchr(path, '/');
    rp = (rp == 0) ? path : rp;
    if (strcmp(rp + 1, object) == 0) {
      printf("%s\n", path);
    }
    break;

  case T_DIR:
    strcpy(buf, path);
    char *p = buf + strlen(buf);
    *p++ = '/';
    struct dirent de;
    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
      if (de.inum == 0) {
        continue;
      }
      if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) {
        continue;
      }

      int len = strlen(de.name);
      memmove(p, de.name, strlen(de.name));
      *(p + len) = '\0';
      find(buf, object);
    }
    break;
  default:
    break;
  }
  close(fd);
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(2, "find format [find path xxx]");
    exit(1);
  }
  find(argv[1], argv[2]);
  exit(0);
}