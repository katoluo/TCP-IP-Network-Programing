#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

void error_handing(char *message);

int main(int argc, char *argv[])
{
  int serv_sock, clnt_sock;
  struct sockaddr_in serv_addr;
  struct sockaddr_in clnt_addr;
  socklen_t clnt_addr_size;

  if (argc != 2) {
    printf("Usage: %s <port> \n", argv[0]);
    exit(1);
  }

  // 创建服务器端套接字（监听套接字）
  serv_sock = socket(PF_INET, SOCK_STREAM, 0);
  if (serv_sock == -1)
    error_handing("socket() error");

  // 声明并初始化地址信息结构体变量
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(atoi(argv[1]));

  // 使用bind函数向套接字分配地址
  if ( bind(serv_sock, (struct sockaddr*)&serv_addr,
        sizeof(serv_addr)) == -1 )
    error_handing("bind() error");

  // 调用listen函数，让服务器端套接字进入等待连接请求状态
  if ( listen(serv_sock, 5) == -1 )
    error_handing("listen() error");

  // 受理客户端的连接请求，并保存成功连接的套接字
  clnt_addr_size = sizeof(clnt_addr);
  clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, 
      &clnt_addr_size);
  if (clnt_addr_size == -1)
    error_handing("accept() error");

  // 向成功连接的套接字写数据
  char message[] = "Hello world";
  write(clnt_sock, message, sizeof(message));
  
  // 关闭套接字
  close(clnt_sock);
  close(serv_sock);

  return 0;
}


void error_handing(char *message)
{
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}
