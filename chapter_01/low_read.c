#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#define BUF_SIZE 100
void error_handing(char *message);

int main(void)
{
  int fd;
  char buf[BUF_SIZE];

  fd = open("data.txt", O_RDONLY);
  if (fd == -1)
    error_handing("open() error");
  printf("file descriptor: %d \n", fd);

  if (read(fd, buf, sizeof(buf)) == -1)
    error_handing("read() error");
  printf("file data: %s", buf);
  close(fd);
  return 0;
}


void error_handing(char *message)
{
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}
