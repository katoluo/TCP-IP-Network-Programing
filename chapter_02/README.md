# 第2章	套接字类型与协议设置



#### 套接字协议及其数据传输特性

**创建套接字**

```c
#include <sys/socket.h>
int socket(int domain, int type, int protocol);
/* 成功时返回文件描述符，失败时返回-1。
 * domain  : 套接字中使用的协议族(Protocol Family)信息
 * type    : 套接字数据传输类型信息
 * protocol: 计算机间通信中使用的协议信息
 */
```

- **协议族（Protocol Family）**

头文件<sys/socket.h>中声明的协议族：

|   名称    |        协议族        |
| :-------: | :------------------: |
|  PF_INET  |   IPv4互联网协议族   |
| PF_INET6  |   IPv6互联网协议族   |
| PF_LOCAL  | 本地通信的UNIX协议族 |
| PF_PACKET |  底层套接字的协议族  |
|  PF_IPX   |   IPX Novell协议族   |

套接字中实际采用的最终协议信息是通过socket函数的第三个参数传递的。在指定的协议族范围内通过第一个参数决定第三个参数。

- **套接字类型（Type）**

套接字类型指的是套接字的数据传输方式，通过socket函数的第二个参数传递，只有这样才能决定创建的套接字的数据传输方式。

​	**套接字类型1：面向连接的套接字（SOCK_STREAM）**

面向连接的套接字具有的特点：

​	:one: 传输过程中数据不会消失

​	:two: 按序传输数据

​	:three: 传输的数据不存在数据边界（Boundary）

收发数据的套接字内部有缓冲（buffer），通过套接字传输的数据将保存到该数组。只要不超过数组容量，则有可能在数据填充缓冲后通过1次read函数调用读取全部，也有可能分成多次read函数调用进行读取。

可靠的、按序传递的、基于字节的面向连接的数据传输方式的套接字。

​	**套接字类型2：面向消息的套接字（SOCK_DGRAM）**

特点：

​	:one: 强调快速传输而非传输顺序

​	:two: 传输的数据可能丢失也可能损坏

​	:three: 传输的数据有数据边界

​	:four: 限制每次传输的数据大小

存在数据边界意味着接收数据的次数应和传输次数相同。

“不可靠的、不按序传递的、以数据的高速传输为目的的套接字”

- **协议的最终选择**

socket函数的第三个参数决定最终采用的协议。

大部分情况下，前面两个参数可以决定创建所需的套接字，所以可以向第三个参数传递0；除非遇到同一协议族中存在多个数据传输方式相同的协议，此时需要通过第三个参数具体指定协议信息。

```c
/* "IPv4协议族中面向连接的套接字"
 * 满足前两个参数的协议只有IPPROTO_TCP，我们可以传递0；
 * 这种套接字称为TCP套接字
 */
int tcp_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

/* "IPv4协议族中面向消息的套接字"
 * 满足前两个参数的协议只有IPPROTO_UDP，我们可以传递0；
 * 这种套接字称为UDP套接字
 */
int udp_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
```



**面向连接的套接字：TCP套接字示例**

:arrow_right: [tcp_server.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_02/tcp_server.c)

:arrow_forward: [tcp_client.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_02/tcp_client.c)

示例的目的是为了验证此前说的TCP套接字传输的数据不存在数据边界。只要证明让write函数的调用次数不同于read函数的调用次数即可。

运行结果：

```bash
$ gcc tcp_client.c -o tcp_client
$ ./tcp_client 127.0.0.1 8888
Message from server: Hello World!
Function read call count: 13
```

从结果可以看出，服务器端发送了13字节的数据，客户端调用13次read函数进行读取。



#### 习题

1. 什么是协议？在收发数据中定义协议有何意义？

> 协议是为了完成数据交换而定好的约定。形成统一的标准，以便于首发双方对消息的格式达成共识，便于数据的解析。

2. 面向连接的TCP套接字传输特性有3点，请分别说明。

> 传输过程中数据不会丢失和损坏；按序传输数据；传输的数据不存在边界。

3. 下列哪些是面向消息的套接字的特性？

> ace

4. 下列数据适合用哪类套接字传输？并给出原因。
   - 演唱会现场直播的多媒体数据（UDP，直播追求实时性，要求传输数据快的套接字，少量的丢失数据不影响观赏性）
   - 某人压缩过的文本文件（TCP，需保证数据的完整性）
   - 网上银行用户与银行之间的数据传递（TCP，可靠）

5. 何种类型的套接字不存在数据边界？这类套接字接收数据时需要注意什么？

> 面向连接的套接字不存在数据边界。注意处理分包、粘包等问题。

6. tcp_client.c和tcp_server.c中需多次调用read函数读取服务器端调用1次write函数传递的字符串。更改程序，使服务器端多次调用（次数自拟）write函数传输数据，客户端调用1次read函数进行读取。为达到这一目的，客户端需延迟调用read函数，因为客户端要等待服务器端传输所有数据。让CPU执行多余任务以延迟代码运行的方式称为“Busy Waiting”。

> [tcp_client_6.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_02/tcp_client_6.c)
>
> [tcp_server_6.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_02/tcp_server_6.c)



