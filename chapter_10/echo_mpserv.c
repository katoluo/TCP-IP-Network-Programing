#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUF_SIZE 30
void error_handing(char *message);
void read_childproc(int sig);

int main(int argc, char *argv[])
{
  int serv_sock, clnt_sock;
  struct sockaddr_in serv_addr, clnt_addr;

  pid_t pid;
  struct sigaction act;
  socklen_t clnt_addr_sz;
  int str_len, state;
  char buf[BUF_SIZE];
  if (argc != 2) {
    printf("Usage: %s <port> \n", argv[0]);
    exit(1);
  }

  act.sa_handler = read_childproc;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  state = sigaction(SIGCHLD, &act, 0);
  serv_sock = socket(PF_INET, SOCK_STREAM, 0);
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(atoi(argv[1]));

  if (bind(serv_sock, (struct sockaddr*)&serv_addr, 
        sizeof(serv_addr)) == -1)
    error_handing("bind() error");
  if (listen(serv_sock, 5) == -1) error_handing("listen() error");

  while (1)
  {
    clnt_addr_sz = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, 
        &clnt_addr_sz);
    if (clnt_sock == -1) continue;  // 连接失败，继续连接
    else 
      puts("new client connected...");
    pid = fork(); // 创建子进程
    if (pid == -1) // 创建子进程失败
    {
      close(clnt_sock); // 关闭成功连接的套接字
      continue; // 重新进行连接客户端
    }
    if (pid == 0) // 创建子进程成功，并且识别子进程
    {
      close(serv_sock); // 子进程程序关闭本程序中的服务器监听套接字
      while ((str_len = read(clnt_sock, buf, BUF_SIZE)) != 0)
        write(clnt_sock, buf, str_len); // 回声

      close(clnt_sock); // 完成任务，关闭成功连接的套接字
      puts("client disconnected...");
      return 0;
    }
    else 
      close(clnt_sock); // 父进程不需要进程回声操作，关闭套接字
  }
  close(serv_sock); // 程序结束前关闭服务器端套接字
  return 0;
}

void read_childproc(int sig)
{
  pid_t pid;
  int status;
  pid = waitpid(-1, &status, WNOHANG);
  printf("remove proc id: %d\n", pid);
}
void error_handing(char *message)
{
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}
