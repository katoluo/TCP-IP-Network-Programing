#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#define BUF_SIZE 100
void error_handing(char *message);

int main(void)
{
  int fd1, fd2;
  int n;
  char buf[BUF_SIZE];

  fd1 = open("data.txt", O_RDONLY);
  if (fd1 == -1)
    error_handing("fd1 open() error");

  n = read(fd1, buf, sizeof(buf));
  if (n == -1)
    error_handing("fd1 read() error");
  
  fd2 = open("data_copy.txt", O_CREAT|O_WRONLY|O_TRUNC);
  if (fd2 == -1)
    error_handing("fd2 open() error");

  if (write(fd2, buf, n) == -1)
    error_handing("fd2 write() error");

  close(fd1);
  close(fd2);
  return EXIT_SUCCESS;
}

void error_handing(char *message)
{
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}
