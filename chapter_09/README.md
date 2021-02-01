# 第9章	套接字的多种可选项

套机诶子具有多种特性，这些特性可通过可选项更改。本章将介绍更爱套接字可选项的方法，并以此为基础进一步观擦套接字内部。

#### 9.1 套接字可选项和I/O缓冲大小

我们进行套机诶子编程时往往只关注数据通信，而忽略了套接字具有的不同特征。但是，理解这些特征并根据一实际需要进行更改也十分重要。

**套接字多种可选项**

我们之前写的程序都是创建好套接字后（未经特别操作）直接使用，此时通过默认的套接字特征进行数据通信。之前的示例较为简单，无需特别操作套接字特性，但有时的确需要更改。表9-1列出了一部分套接字可选项。

![表9-1](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_09/images/%E8%A1%A89-1.png)

从表中可以看出，套接字可选项是分层的。IPPROTO_IP层可选项是IP协议相关事项，IPPROTO_TCP层可选项是TCP协议相关的事项，SOL_SOCKET层套机诶子相关的通用可选项。

也许友人看到表格会产生畏惧感，但现在无需全部背下来或理解，因此不必有负担。实际能够设置的可选项数量就是表的好几倍，也无需一下子理解所有可选项，实际工作中注意掌握即可。接触的可选项多了，自然会掌握大部分重要的。本书也只介绍其中一部分重要的可选项含义及更改方法。

**getsockopt&setsockopt**

我们几乎可以针对表9-1中所有可选项进行读取（Get）和设置（Set）（当然，有些可选项只能进行一种操作）。可选项的读取和设置通过如下2个函数完成。

```c
#include <sys/socket.h>
int getsockopt(int sock, int level, int optname, void *optval, 
              socklen_t *optlen);
/*
 * 成功时返回0, 失败时返回-1
 * sock 用于查看选项套接字文件描述符
 * level 要查看的可选项的协议层
 * optname 要勘察的可选项名
 * optval 保存查看结果的缓冲地址值
 * optlen 向第四个参数optval传递的缓冲大小。调用函数后，该变量中保存通过第四个参数返回的可选项信息的字节数
 */
```

上述函数用于读取套接字可选项，并不难。接下来介绍更改可选项时调用的函数。

```c
#include <sys/socket.h>
int setsockopt(int sock, int level, int optname, const void *optval,
              socklen_t optlen);
/*
 * 成功时返回0,失败时返回-1
 * sock 用于更改可选项的套接字文件描述符
 * level 要更改的可选项协议层
 * optname 要更改的可选项名
 * optval 保存要更改的选项信息的缓冲地址值
 * optlen 向第四个参数optval传递的可选项信息的字节数
 */
```

接下来介绍这些函数的调用方法。关于setsockopt函数的调用方法在其他示例中给出，先介绍getsockopt函数的调用方法。下面是列用协议层为SOL_SOCKET、名为SO_TYPE的可选项查看套接字类型（TCP或UDP）。

[sock_type.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_09/sock_type.c)

![运行结果](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_09/images/%E8%BF%90%E8%A1%8C%E7%BB%93%E6%9E%9C.png)

上述示例给出了调用getsockopt函数查看套接字信息的方法。另外，用于验证套接字类型的SO_TYPE是典型的只读可选项，这一点可以通过下面这句话解释：

> "套接字类型只能在创建时决定，以后不能再更改。"

**SO_SNDBUF & SO_RCVBUF**

前面介绍过，创建套接字将同时生成I/O缓冲。如果各位忘了这部分内容，可以复习第5章。接下来将介绍I/O缓冲相关可选项。

SO_RCVBUF是输入缓冲大小相关可选项，SO_SNDBUF是输出缓冲大小相关可选项。用这2个可选项既可以读取当前I/O缓冲大小，也可以进行更改。通过下列下列读取创建套接字时默认的I/O缓冲大小。

[get_buf.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_09/get_buf.c)

![运行结果2](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_09/images/%E8%BF%90%E8%A1%8C%E7%BB%93%E6%9E%9C2.png)

不同系统运行的结果有较大差异。接下来的程序中将更改I/O缓冲大小。

[set_buf.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_09/set_buf.c)

![运行结果3](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_09/images/%E8%BF%90%E8%A1%8C%E7%BB%93%E6%9E%9C3.png)

输出结果跟我们预想的完全不同，但也算合理。缓冲大小的设置需要谨慎处理，因此不会完全按照我们的要求进行，知识通过调用setsockopt函数向系统传递我们的要求。如果把输出缓冲设置为0并如实反应这种设置，TCP协议将如何进行？如果要实现流控制和错误发生时的重传机制，至少要有一些缓冲空间吧？上述示例虽没有%100按照我们的请求设置缓冲大小，但也大致反映除了通过setsockopt函数设置的缓冲大小。

#### 9.2 SO_REUSEADDR

本节的可选项SO_REUSEADDR及其相关的Time-wait状态很重要，希望大家务必理解并掌握。

**发生地址分配错误（Binding Error）**

学习SO_REUSEADDR可选项之前，应理解好Time-wait状态。我们读完下列示例后在讨论后序内容。

[reuseaddr_eserver.c]()

此示例是之前已实现过多次的回声服务器端，可以结合第4章介绍过的回声客户端运行。下面运行该示例，第29～32行应保持注释状态。通过如下方式终止程序：

> "在客户端控制台输入Q消息，或通过CTRL+C终止程序。"

也就是说，让客户端通知服务器端终止程序。在客户端控制台输入Q消息时调用close函数，向服务器端发生FIN消息并经过四次握手过程。当然，输入CTRL+C时也会向服务器端传递FIN消息。强制终止程序时，由操作系统关闭文件及套接字，此过程相当于调用close函数，也会向服务器端传递DIN消息。

> "但看不到什么特殊现象啊？"

是的，通常都是客户端向请求断开连接，所以不会发生特别的事情。重新运行服务器端也不成问题，但按照如下方式终止程序时则不同。

> "服务器端和客户端已建立连接的状态下，向服务器端控制台输入CTRL+C，即强制关闭服务器端。"

这主要模拟了服务器端向客户端发生FIN消息的情景。但如果以这种方式终止程序，那服务器端重新运行时将产生问题。如果用同一端口号重新运行服务器端，将输出"bind() error"消息，并且无法再次运行。但在这种情况下，再过大约3分钟即可重新运行服务器端。

上述2中运行方式唯一的区别就是谁先传输FIN消息，但结果却不同，原因何在呢？

**Time-wait状态**

相信各位已对四次握手有了很好的理解，先观擦该过程，如图9-1所示。

![图9-1](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_09/images/%E5%9B%BE9-1.png)

假设图9-1中主机A是服务器端，因为是主机A向B发送FIN小，故可以想象成服务器端在控制台输入CTRL+C。但问题是，套接字经过四次握手过程后并非立即消除，而是要经过一段时间的Time-wait状态。当然，只有先断开连接的（先发送FIN消息的）主机才经过Time-wait状态。因此，如服务器端先断开连接，则无法立即重新运行。套接字处在Time-wait过程时，相应端口是正在使用的状态。因此，就像之前验证过的，bind函数调用过程中当然会发生错误。

> 客户端套接字不会经过Time-wait过程嘛？
>
> 先断开连接的套接字必然会经过。但无需考虑客户端Time-wait状态。因为客户端套接字的端口号是任意指定的。

到底为什么会有Time-wait状态呢？图9-1中假设主机A向主机B传输ACK消息（SEQ 5001、ACK 7502）后立即消除套接字。但最后这条ACK消息在传递途中丢失，未能传给主机B。这时会发生什么？主机B会认为之前自己发送的FIN消息（SEQ 7021、ACK 5001）未能抵达主机A，继续而试图重传。但此时主机A已是完全终止的状态，因此主机B永远无法收到从主机A最后传来的ACK消息。相反，若主机A的套接字处在Time-wait状态，则会向主机B重传最后的ACK消息，主机B也可以正常终止。基于这些考虑，先传输FIN消息的主机应经过Time-wait过程。

**地址再分配**

Time-wait看似重要，但并不一定讨人喜欢。考虑一下系统发生故障从而紧急停止的情况。这时需要尽快重启服务器端以提供服务，但因处于Time-wait状态而必须等待几分钟。因此，Time-wait并非只有优点，而且有些情况下肯呢个引发更大问题。图9-2演示了四次握手时不得不延长Time-wait过程的情况。

![图9-2](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_09/images/%E5%9B%BE9-2.png)

如图所示，在主机A的四次握手过程中，如果最后的数据丢失，则主机B会认为主机A未能收到自己发送的FIN消息，因此重传。这时，收到FIN消息的主机A将重启Time-wait计时器。因此，如果网络状况不理想，Time-wait状态将持续。

解决方案就是在套接字的可选项中更改SO_REUSEADDR的状态。适当掉正该参数，可将Time-wait装下的套接字端口号重新分配给新的套接字。SO_REUSEADDR的默认值为0（假），这就意味着无法分配Time-wait状态下的套接字端口号重新分配给新的套接字。因此需要将这个值改成1（真）。具体做法已在示例reuseaddr_eserver.c中给出，只需去掉注释即可。

#### 9.3 TCP_NODELAY

作者教Java网络编程时，经常被问及如下问题：

> "什么是Nagel算法？使用该算法能够获得哪些数据通信特性？"

作者被问到这个问题时感到特别高兴，因为开发人员容易忽视的一个问题就是Nagle算法，下面进行详细讲解。

**Nagle算法**

为防止因数据包过多而发生网络过载，Nagle算法在1984年诞生了。它应用在TCP层，非常简单。其使用与否会导致如图9-3所示差异。

![图9-3](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_09/images/%E5%9B%BE9-3.png)

上图展示了通过Nagle算法发送字符串"Nagle"和未使用Nagle算法的擦汗别。可以得到如下结论：

> "只有接收到前一数据的ACK消息时，Nagle算法才发送下一数据。"

TCP套接字默认使用Nagle算法交换数据，因此最大限度地进行缓冲，知道收到ACK。上左侧正式这种情况。为了发送字符串"Nagle"，将其传递到输出缓冲。这是头字符"N"之前没有其他数据（没有需接收的ACK），因此立即传输。之后开始等待字符"N"的ACK消息，等待过程中，剩下的"agle"填入输出缓冲。接下来，收到字符"N"的ACK消息后，将输出缓冲的"agle"装入下一个数据包发送。也就是说，共需要传递4个数据包以传输1个字符串。

接下来分析未使用Nagle算法时发送字符串"Nagle"的过程。假设字符"N"到"e"依序传到输出缓冲。此时的发送过程与ACK接收与否无关，因此数据到达输出缓冲后将立即被发送出去。从上图右侧可以看出，发送字符串"Nagle"时共需10个数据包。由此可知，不使用Nagle算法将对流量（Traffic：指网络负载或混杂程度）产生负面影响。即使只传输1个字节的数据，其头信息都有可能是几十个字节。因此，为了提高网络传输效率，必须使用Nagle算法。

> 图9-3是极端情况的演示
>
> 在程序中将字符串传给输出缓冲时并不是逐字传递的，故发送字符串"Nagle"的实际情况并非如图所示。但如果隔一段时间再把构成字符串的字符传到输出缓冲（如果存在此类数据传递）的话，则有可能产生类似图中的情况。图中就是隔一段时间向输出缓冲传递发送数据。

但Nagle算法并不是什么时候都适用。根据传输数据的特性，网络流浪未受太大影响时，不使用Nagle算法要比使用它时传输速度快。最典型的是"传输大文件数据"。将文件数据传入输出缓冲不会花太多时间，因此，即便不使用Nagle算法，也会在装满输出缓冲时传输数据包。这不仅不会增加数据包的数量，反而会在无需等待ACK的前提下连续传输，因此可以大大提高传输速度。

一般情况下，不适用Nagle算法可以提高传输速度。但如果无条件放弃使用Nagle算法，就会增加过多的网络流量，反而会影响传输。因此，未准确判断数据特性时不应禁用Nagle算法。

**禁用Nagle算法**

刚才说过的"大文件数据"应禁用Nagle算法。换言之，如果有必要，就应禁用Nagle算法。

> "Nagle算法使用与否在网络流量上差别不大，使用Nagle算法的传输速度更慢"

禁用方法非常简单。从下列代码也可以看出，也需将套接字可选项TCP_NODELAY改为1（真）即可。

```c
int opt_val = 1;
setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void*)&opt_val,
          sizeof(opt_val));
```

可以通过TCP_NODELAY的值查看Nagle算法的设置状态。

```c
int opt_val;
socklen_t opt_len;
opt_len = sizeof(opt_val);
getsockopt(sock, IPPROTO, TCP_NODELAY, (void*)&opt_val, &opt_len);
```

如果正在使用Nagle算法，opt_val变量中会保存0；如果禁用Nagle算法，则保存1。