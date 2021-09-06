#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

int main() {
  // 创建文件a
  int fd_a = open("a", O_CREAT | O_WRONLY | O_TRUNC, 0644);
  if (fd_a == -1) {
    perror("无法创建文件a");
    return EXIT_FAILURE;
  }

  // 向文件a写入一些内容
  const char *content = "Hello, this is file a.\n";
  if (write(fd_a, content, strlen(content)) == -1) {
    perror("写入文件a失败");
    close(fd_a);
    return EXIT_FAILURE;
  }

  // 关闭文件a
  if (close(fd_a) == -1) {
    perror("关闭文件a失败");
    return EXIT_FAILURE;
  }

  // 创建文件b硬链接到a
  if (link("a", "b") == -1) {
    perror("创建硬链接b失败");
    return EXIT_FAILURE;
  }

  // 使用fstat获取文件信息
  struct stat stat_a, stat_b;

  // 打开文件a用于fstat
  fd_a = open("a", O_RDONLY);
  if (fd_a == -1) {
    perror("无法打开文件a");
    return EXIT_FAILURE;
  }

  // 获取文件a的信息
  if (fstat(fd_a, &stat_a) == -1) {
    perror("获取文件a信息失败");
    close(fd_a);
    return EXIT_FAILURE;
  }

  // 关闭文件a
  close(fd_a);

  // 打开文件b用于fstat
  int fd_b = open("b", O_RDONLY);
  if (fd_b == -1) {
    perror("无法打开文件b");
    return EXIT_FAILURE;
  }

  // 获取文件b的信息
  if (fstat(fd_b, &stat_b) == -1) {
    perror("获取文件b信息失败");
    close(fd_b);
    return EXIT_FAILURE;
  }

  // 关闭文件b
  close(fd_b);

  // 输出文件信息
  printf("文件a的信息:\n");
  printf("  inode号: %lu\n", (unsigned long)stat_a.st_ino);
  printf("  硬链接数: %lu\n", (unsigned long)stat_a.st_nlink);
  printf("  文件大小: %lld 字节\n", (long long)stat_a.st_size);
  printf("  修改时间: %s", ctime(&stat_a.st_mtime));

  printf("\n文件b的信息:\n");
  printf("  inode号: %lu\n", (unsigned long)stat_b.st_ino);
  printf("  硬链接数: %lu\n", (unsigned long)stat_b.st_nlink);
  printf("  文件大小: %lld 字节\n", (long long)stat_b.st_size);
  printf("  修改时间: %s", ctime(&stat_b.st_mtime));

  // 验证inode号是否相同
  if (stat_a.st_ino == stat_b.st_ino) {
    printf("\n验证: 文件a和文件b指向相同的inode\n");
  } else {
    printf("\n错误: 文件a和文件b指向不同的inode\n");
  }

  // 验证硬链接数
  if (stat_a.st_nlink >= 2 && stat_a.st_nlink == stat_b.st_nlink) {
    printf("验证: 文件a和文件b的硬链接数正确（至少为2）\n");
  } else {
    printf("错误: 文件a和文件b的硬链接数异常\n");
  }

  return EXIT_SUCCESS;
}