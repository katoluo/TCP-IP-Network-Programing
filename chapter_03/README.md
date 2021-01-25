# 第3章	地址族与数据序列



#### 分配给套接字的IP地址与端口号

IP是Internet Protocol（网络协议）的简写，是为收发网络数据而分配给计算机的值。端口号并非赋予计算机的值，而是为区别程序中创建的套接字而分配给套接字的序号。

**网络地址（Internet Adress）**

IP地址分为两类：

- IPv4（Internet Protocol version 4）	  4字节地址族
- IPv6（Internet Protocol version 6）	16字节地址族

IPv4标准的4字节IP地址分为网络地址和主机地址，且分为A、B、C、D、E等类型。

![图3-1.png](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_03/images/%E5%9B%BE3-1.png)

网络地址（网络ID）是为区分网络而设置的一部分IP地址。



**网络地址分类与主机地址边界**

只需通过IP地址的第一个字节即可判断网络地址占用的字节数，如下所示：

- A类地址的首字节范围：0～127
- B类地址的首字节范围：128～191
- C类地址的首字节范围：192～223

还有如下这种表达方式：

- A类地址的首位以0开始
- B类地址的首位以10开始
- C类地址的首位以110开始

正因如此，通过套接字收发数据时，数据传到网络后即可轻松找到正确的主机。



**用于区分套接字的端口号**

IP用于区分计算机，只要有IP地址就能向目标主机传输数据，但仅凭这些无法传输给最终的应用程序。

端口号由16位构成，可分配的端口号范围是0～65535。但0～1023是知名端口（Well-konwn PORT），一般分配给特定应用程序，所以应当分配此范围之外的值。TCP套接字和UDP套接字不会共用端口号，所以允许重复。



#### 地址信息的表示

应用程序中使用的IP地址和端口号以结构体的形式给出了定义。



**表示IPv4地址的结构体**

结构体定义如下：

```c
struct sockaddr_in
{
    sa_family_t      sin_family;         // 地址族（Adress Family）
    unit16_t         sin_port;           // 16位TCP/UDP端口号
    struct in_addr   sin_addr;           // 32位IP地址
    char             sin_zero[8];        // 不使用
};
struct in_addr
{
    in_addr_t        s_addr;             // 32位IPv4地址
};
```

此结构体将作为地址信息传递给bind函数。



#### 结构体sockaddr_in的成员分析

**成员sin_family**

|  地址族  |               含义               |
| :------: | :------------------------------: |
| AF_INET  |     IPv4网络协议中使用地址族     |
| AF_INET6 |    IPv6网络协议中使用的地址族    |
| AF_LOCAL | 本地通信中采用的UNIX协议的地址族 |

**成员sin_port**

该成员保存16位端口号，重点在于，它以网络字节序保存。

**成员sin_addr**

该成员保存32位IP地址信息，且也以网络字节序保存。

**成员sin_zero**

无特殊含义。知识为使结构体sockaddr_in的大小与sockaddr结构体保持一致而插入的成员。必须填充为0,否则无法得到想要的结果。

```c
struct sockaddr_in serv_addr;
...
if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
    ...
```

bind函数的第二个参数期望得到sockaddr结构体变量地址值，包括地址族、端口号、IP地址等。从下面代码可看出，直接向sockaddr结构体填充这些信息会带来麻烦。

```c
struct sockaddr
{
    sa_family_t   sin_family;     // 地址族
    char          sa_data[14];    // 地址信息
};
```

此结构体成员sa_data保存的地址信息中需包含IP地址和端口号，剩余部分应填充0,这也是bind函数要求的。



#### 网络字节序与地址转换

不同CPU中，4字节整数型值1在内存空间的保存方式是不同的。4字节整数型值1可用二进制表示如下：

```
00000000 00000000 00000000 00000001
```

有些CPU以这种顺序保存到内存，另一些CPU则以倒序保存：

```
00000001 00000000 00000000 00000000
```

如果不考虑这些就收发数据则会发生问题，因为保存顺序的不同意味着对接收数据的解析顺序也不同。



**字节序（Order）与网络字节序**

- 大端序（Big Endian）：高位字节存放在低位地址。
- 小端序（Little Endian）：高位字节存放在高位地址。

通过示例进行说明：

![图3-4.png](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_03/images/%E5%9B%BE3-4.png)

<center>图3-4   大端序字节表示</center>

![图3-5.png](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_03/images/%E5%9B%BE3-5.png)

<center>图3-5   小端序字节表示</center>



**字节序转换（Endian Conversions）**

帮助转换字节序的函数：

- unsigned short htons(unsigned short);
- unsigned short ntohs(unsigned short);
- unsigned long htonl(unsigned long);
- unsigned long ntohl(unsigned long);

:smile: h代表主机（host）字节序；n代表网络（network）字节序；s指的是short，l指的是long（Linux中long为4字节）。



#### 网络地址的初始化与分配

**将字符串信息转换为网络字节序的整数型**

对于IP地址的表示，我们熟悉的是点分十进制表示法（Dotted Decimal Notation），而非整数型数据表示法。下面函数帮我们将字符串形式的IP地址转换成32位整数型数据：

```c
#include <arpa/inet.h>
in_addr_t inet_addr(const char *string);
// 成功时返回32位大端序整数型值，失败时返回INADDR_NONE
```

例子：运行 [inet_addr.c]() ，结果：

```bash
$ gcc inet_addr.c
$ ./a.out
Network ordered integer addr: 0x4030201
Error occured
```



inet_aton函数与inet_addr函数在功能上完全相同，也将字符串形式IP地址转换为32位网络字节序整数并返回。不过该函数利用了in_addr结构体，且 其使用频率更高。

```c
#include <arpa/inet.h>
int inet_aton(const char *cp, struct in_addr *inp);
```

例子：[inet_aton.c]() ，输出结果一样。

更多具体详细的函数可通过man手册来进行查看了解。