#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>
#define BUF_SIZE 30

int main(void)
{
  fd_set reads, temps;
  int result, str_len;
  char buf[BUF_SIZE];
  struct timeval timeout;

  FD_ZERO(&reads);
  FD_SET(0, &reads); // 0 is standard input(console)

  while (1)
  {
    temps = reads;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    result = select(1, &temps, NULL, NULL, &timeout);
    if (result == -1)
    {
      perror("select");
      break;
    }
    else if (result == 0)
    {
      puts("Time-out");
    }
    else 
    {
      if (FD_ISSET(0, &temps))
      {
        str_len = read(0, buf, BUF_SIZE);
        buf[str_len] = 0;
        printf("message from comsole: %s", buf);
      }
    }
  }
  return 0;
}
