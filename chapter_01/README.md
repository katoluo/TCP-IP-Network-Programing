# 第1章	理解网络编程和套接字

网络编程就是编写程序使两台连网的计算机相互交换数据。网络编程又称套接字编程。



#### 1.1 理解网络编程和套接字

**网络编程中接受连接请求的套接字(服务器端)创建过程可整理如下：**

- 第一步：调用socket函数创建套接字。

```c
#include <sys/socket.h>
int socket(int domain, int type, int protocol);
// 成功时返回文件描述符，失败时返回-1
```

- 第二步：调用bind函数分配IP地址和端口号

```c
#include <sys/socket.h>
int bind(int sockfd, struct sockaddr *myaddr, socklen_t addrlen); // 书上的
int bind(int socket, const struct sockaddr *address,
           socklen_t address_len); // man手册上的
// 成功时返回0,失败时返回-1
```

- 第三步：调用listen函数转为可接受请求状态

```c
#include <sys/socket.h>
int listen(int sockfd, int backlog); // 书上的
int listen(int socket, int backlog); // man手册
// 成功时返回0,失败时返回-1
```

- 第四步：调用accept函数受理连接请求。

```c
#include <sys/socket.h>
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) // 书上的
int accept(int socket, struct sockaddr *restrict address,
           socklen_t *restrict address_len); // man手册上的
```

记住并掌握这些步骤就相当于为套接字编程勾勒好了轮廓。

服务器端：[hello_server.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_01/hello_server.c)



**客户端套接字用于请求连接的，使用如下函数：**

```c
#include <sys/socket.h>
int connect(int sockfd, struct sockaddr *serv_addr, socklen_t addrlen); //书上的
int connect(int socket, const struct sockaddr *address,
           socklen_t address_len); // man手册上的
```

客户端程序只有“调用socket函数创建套接字”和“调用connect函数向服务器端发送连接请求”这两个步骤，因此比较简单。

客户端：[hello_client.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_01/hello_client.c)



**在Linux平台下运行**

编译：

```
gcc hello_server.c -o hserver   // 编译源文件并生成可执行文件hserver
gcc hello_client.c -o hclient   // 编译源文件并生成可执行文件hclient
```

运行：

```bash
# 终端1
$ ./hserver 8888

# 正常情况下程序将停留在此状态，因为额哦服务器端调用的accept函数还未返回。
```

```bash
# 终端2
$ ./hclient 127.0.0.1 8888
Message from server: Hello World!
$
```



### 1.2 基于Linux的文件操作

在Linux世界里，socket也被认为是文件的一种，因此在网络数据传输过程中自然可以使用文件I/O的相关函数。

**底层文件访问（Low-Level File Access）和文件描述符（File Descriptor）**

“底层”可以理解为“与标准无关的操作系统独立提供的”。稍后讲解的函数是Linux提供的，而非ANSI标准定义的函数。

文件描述符是系统分配给文件或套接字的整数。文件和套接字一般经过创建过程才会被分配文件描述符。

例如：

| 文件描述符 |           对象            |
| :--------: | :-----------------------: |
|     0      | 标准输入：Standard Input  |
|     1      | 标准输出：Standard Output |
|     2      | 标准错误：Standard Error  |



**打开文件**

open函数需传递两个参数：第一个参数是打开的目标文件名及路径信息，第二个参数是文件打开模式。

```c
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
int open(const char *path, int flag);
// 成功是返回文件描述符，失败时返回-1。
```

参数flag可能的常量值及含义，如下表：

| 打开模式 |            含义            |
| :------: | :------------------------: |
| O_CREAT  |       必要时创建文件       |
| O_TRUNC  |      删除全部现有数据      |
| O_APPEND | 维持现有数据，保存到其后面 |
| O_RDONLY |          只读打开          |
| O_WRONLY |          只写打开          |
|  O_RDWR  |          读写打开          |



**关闭文件**

使用文件后必须关闭，关闭文件时调用close函数：

```c
#include <unistd.h>
int close(int fd);
// 成功时返回0,失败时返回-1。
```



**将数据写入文件**

使用write函数向文件输出（传输）数据：

```c
#include <unistd.h>
ssize_t write(int fd, const void *buf, size_t nbytes);
// 成功时返回写入的字节数，失败时返回-1。
```

此函数定义中，size_t是通过typedef声明的unsigned int类型。ssize_t为signed int类型。



例子：[low_open.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_01/low_open.c)

运行结果：

```bash
$ gcc low_open.c -o lopen
$ ./lopen
file descriptor: 3
$ cat data.txt
Let's go!
$
```



**阅读文件中的数据**

read函数用来输入（接受）数据：

```c
#include <unistd.h>
ssize_t read(int fd, void *buf, size_t nbytes);
// 成功时返回接受的字节数(但遇到文件结尾则返回0)，失败时返回-1。
```

例子：[low_read.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_01/low_read.c)

运行结果：

```bash
$ gcc low_read.c -o lread
$ ./lread
file descriptor: 3
file data: Let's go!
$
```



#### 1.3 基于Windows平台的实现

略

#### 1.4 基于Windows的套接字相关函数及示例

略



#### 1.5 习题

（初学，答案不一定正确。）

1.  套接字在网络编程中的作用是什么？为何称它为套接字？

> 套接字是网络数据传输用的软件设备；为了与远程计算机进行数据传输，需要连接到因特网，而编程中的“套接字”就是用来连接该网络的工具。它本身就带有“连接”的含义，如果将其引申，则还可以表示两台计算机之间的网络连接。

2. 在服务器创建套接字后，会依次调用listen函数和accept函数。请比较说明二者作用。

> 调用listen函数将服务器端套接字转为可接收链接状态；调用accept函数受理连接请求，如果在没有连接请求的情况下调用该函数，则不会返回，知道有链接请求为止。

3. Linux中，对套接字数据进行I/O时可以直接使用文件I/O相关函数；而在Windows中则不可以。原因为何？

> 对于Linux而言，socket操作和文件操作没有区别，但是在Windows中socket进行文件操作，则需要调用特殊的数据传输相关函数。

4. 创建套接字后一般会给它分配地址，为什么？为了完成地址分配需要调用哪些函数？

> 区分来自不同机器的套接字，通过bind()分配地址信息(IP地址和端口号)。

5. Linux中的文件描述符与Windows的句柄实际上非常类似。请以套接字为对象说明他们的含义。

> Linux中的文件描述符是系统分配给文件或套接字的整数；Windows中和Linux概念相同，不过Windows要区分文件句柄和套接字句柄。

6. 底层文件I/O函数与ANSI标准定义的文件I/O函数之间有何区别？

> [看下大佬的解释](https://blog.csdn.net/owen7500/article/details/53263981)

7. 参考本书给出的示例low_open.c和low_read.c,分别利用底层文件I/O和ANSI标准I/O编写文件复制程序。可任意指定复制程序的使用方法。

> [low_copy.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_01/low_copy.c)
>
> [ansi_copy.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_01/ansi_copy.c)



