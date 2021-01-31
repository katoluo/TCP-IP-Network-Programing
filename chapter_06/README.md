# 第6章	基于UDP的服务器端/客户端



#### 理解UDP

**UDP套接字的特点**

下面通过信件说明UDP的工作原理，这是讲解UDP时使用的传统示例，它与UDP特性完全相符。寄信前应先在信封上填好寄信人和收信人的地址，之后贴上邮票放进邮筒即可。当然，信件的特点使我们无法确认对方是否收到。另外，邮寄过程中也可能发生信件丢失的情况。也就是说，信件是一种不可靠的传输方式。与之类似，UDP提供的同样是不可靠的数据传输服务。

> “既然如此，TCP应该是更优质的协议吧？”

如果只考虑可靠性，TCP的确比UDP好。但UDP在结构上比TCP更简洁。UDP不会发送类似ACK的应答消息，也不会像SEQ那样给数据包分配序号。因此，UDP的性能有时比TCP高出很多。编程中实现UDP也比TCP简单。另外，UDP的可靠性虽比不上TCP，当也不会像想像中那么频繁地发生数据损毁。因此，在更重视性而非可靠性的情况下，UDP是一种很好的选择。

既然如此，UDP的作用到底是什么呢？为了提供可靠的数据传输服务，TCP在不可靠的IP层进行流控制，而UDP就缺少这种流控制机制。

> “UDP和TCP的差异只在于流控制机制嘛？”

是的，流控制是区分YDP和TCP的最重要标志。但若从TCP中除去流控制，所剩内容也屈指可数。也就是说，TCP的生命在于流控制。在第5章讲过的“与对方套接字连接及断开连接过程”也属于流控制的一部分。



**UDP内部工作原理**

与TCP不同，UDP不会进行流控制。接下来具体讨论UDP的作用，如图6-1所示。

![图6-1](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_06/images/%E5%9B%BE6-1.png)

从图可以看出，IP的作用就是让离开主机B的UDP数据包准确传递到主机A。但把UDP包最终交给主机A的某一UDP套接字的过程则是由UDP完成的。UDP最重要的作用就是根据端口号将传到主机的数据包交付给最终的UDP套接字。



**UDP的高校使用**

虽然貌似大部分网络编程都基于TCP实现的，但也有一些是基于UDP实现的。接下来考虑何时使用UDP更有效。讲解前希望各位明白，UDP也具有一定的可靠性。网络传输特性导致信息丢失频发，可若要传递压缩文件，则必须使用TCP，因为压缩文件只要丢失一部分就很难解压。但通过网络实时传输视频或音频时的情况有所不同。对于多媒体数据而言，丢失一部分也没有太大问题，这只会引起暂时的画面抖动，或出现细微的杂音。但因为需要提供实时服务，速度就成为非常重要的因素。因此，第5章的流控制就显得有些多余，此时需要考虑使用UDP。但UDP并非每次都快于TCP，TCP比UDP慢的原因通常有一下两点。

:one: 收发数据前后进行的连接设置及清除过程。

:two: 收发数据过程中为保证可靠性而添加的流控制。

如果收发的数据量小但需要频繁连接时，UDP比TCP高效。有机会的话，希望各位深入学习TCP/IP协议的内部构造。C语言程序言懂得计算机结构和操作系统知识就能写出更好的程序，同样，网络程序员若能深入理解TCP/IP协议则可大幅度提高自身实力。



#### 实现基于UDP的服务器端/客户端

接下来通过之前介绍的UDP理论实现真正的程序。



**UDP中服务器端和客户端没有连接**

UDP服务器端/客户端不像TCP那样在连接状态下交换数据，因此与TCP不同，无需经过连接过程。也就是说，不必调用TCP连接过程中调用的listen函数和accept函数。UDP中只有创建套接字的过程和数据交换过程。



**UDP服务器端和客户端均只需1个套接字**

TCP中，套接字之间应该是一对一的关系。若要向10个客户端提供服务，则除了守门的服务器套接字外，还需要10个服务器端套接字。但在UDP中，不管服务器端还是客户端都只需要1个套接字。之前解释UDP原理时举了信件的例子，收发信件时使用的邮筒可以比喻为UDP套接字。只要附近有1个邮筒，就可以通过它向任意地址寄出信件。同样，只需1个UDP套接字就可以向任意主机传输数据，如图6-2所示。

![图6-2](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_06/images/%E5%9B%BE6-2.png)

展示了1个UDP套接字与2个不同主机交换数据的过程。也就是说，只需1个UDP套接字就能和多台主机通信。



**基于UDP的数据I/O函数**

创建好TCP套接字后，传输数据时无需再添加地址信息。因为TCP套接字将保持与对方套接字的连接。换言之，TCP套接字知道目标地址信息。但UDP套接字不会保持连接状态（UDP套接字只有简单的邮筒功能），因此每次传输数据都要添加目标地址信息。这相当于寄信前在信件中填写地址。接下来节扫填写地址并传输数据时调用的UDP相关函数。

```c
#include <sys/socket.h>
ssize_t sendto(int sock, void *buf, size_t nbytes, int flags,
              strutc sockaddr *to, socklen_t addrlen);
/*
 * 成功时返回传输的字节数，失败时返回-1
 * sock 用于传输数据的UDP套接字文件描述符
 * buff 保存待传输数据的缓冲地址值
 * nbytes 待传输的数据长度，以字节为单位
 * flags 可选项参数，若没有则传递0
 * to 存有目标地址信息的sockaddr结构体变量的地址值
 * addrlen 传递给参数to的地址值结构体变量长度
 */
```

上述函数与之前的TCP输出函数最大的区别在于，此函数需要向它传递目标地址信息。接下来介绍接收UDP数据的函数。UDP数据的发送端并不固定，因此该函数定义为可接收发送端信息的形式，也就是将同时返回UDP数据包中的发送端信息。

```c
#include <sys/socket.h>
ssize_t recvfrom(int sock, void *buff, size_t nbytes, int flags,
                struct sockaddr *from, socklen_t *addrlen);
/*
 * 成功时返回接收的字节数，失败时返回-1
 * sock 用于接收数据的UDP套接字文件描述符
 * buff 保存接收数据的缓冲地址值
 * nbytes 可接收的最大字节数，故无法超过参数buff所指的缓冲大小
 * flags 可选项参数，若没有则传入0
 * from 存有发送端地址信息的sockaddr结构体变量的地址值
 * addrlen 保存参数from的结构体变量长度的变量地址值
 */
```

编写UDP程序是最核心的部分就在于上述两个函数，这也说明二者在UDP数据传输中的地位。



**基于UDP的回声服务器端/客户端**

下面结合之前的内容实现回声服务器。需要注意的是，UDP不同于TCP，不存在请求连接和受理过程，因此在某种意义上无法明确区别服务器端和客户端。知识因其提供服务而称为服务器端，希望各位不要误解。

[uecho_server.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_06/uecho_server.c)

> 第24行：为了创建UDP套接字，向socket函数第二个参数传递SOCK_DGRAM。
>
> 第39行：利用第33行分配的地址接收数据。不限制数据传输对象。
>
> 第41行：通过第39行的函数调用同时获取数据传输端的地址。正式利用该地址将接收的数据逆向重传。
>
> 第44行：第37行的while内部从未加入break语句，因此是无限循环。也就是说，close函数不会执行，没有太大意义。

接下来将诶少与上述服务器端协同工作的客户端。这部分代码与TCP客户端不同，不存在connect函数调用。

[uecho_client.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_06/uecho_client.c)

若各位很好地理解了第4章的connect函数，那么读上述代码时应有如下疑问：

> “TCP客户端套接字在调用connect函数时自动分配IP地址和端口号，既然如此，UDP客户端何时分配IP地址和端口号？”

所有套接字都应分配IP地址和端口，问题是直接分配还是自动分配。希望大家独立思考并进行推断，稍后再讨论。



**UDP客户端套接字的地址 分配**

前面讲解了UDP服务器端/客户端的实现方法。但如果仔细观擦UDP客户端会发现，它缺少把IP和端口分配给套接字的过程。YCP客户端调用connect函数自动完成此过程，而UDP中连能承担相同功能的函数调用语句都没有。究竟在何时分配IP和端口号呢？

UDP程序中，调用sendto函数传输数据前应完成对套接字的地址分配工作，因此调用bind函数。当让，bind函数在TCP程序中出现过，但bind函数不区分TCP和UDP，也就是说，在UDP程序中同样可以调用。另外，如果调用sendto函数时发现尚未分配地址信息，则在首次调用sendto函数时给相应套接字自动分配IP和端口。而且此时分配的地址一直保留到程序结束为止，因此也可用来与其他UDP套接字进行数据交换。当然，IP用主机IP，端口号选尚未使用的任意端口号。

综上所述，调用sendto函数时自动分配IP和端口号，因此，UDP客户端中通常无需额外的地址分配过程。所以之前示例中胜略了该过程，这也是普遍的实现方式。



#### UDP的数据传输特性和调用connect函数

我们之前通过示例验证了TCP传输的数据不存在数据边界，本节将验证UDP数据传输中存在数据边界。最后讨论UDP中connect函数的调用，以此结束UDP相关讨论。

**存在数据边界的UDP套接字**

前面说过TCP数据传输中不存在边界，这表示“数据传输过程中调用I/O函数的次数不具有任何意义。”

相反，UDP是具有数据边界的协议，传输中调用I/O函数的次数非常重要。因此，输入函数的调用次数应和输出函数的调用次数完全一致，这样才能保证接收全部已发送叔叔。例如，调用3此输出函数发送的数据必须通过调用3此输入函数才能接收完。下面通过简单示例进行验证。

[bound_host1.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_06/bound_host1.c)

上述示例中需要各位特别留意的是for语句。首先调用了sleep函数，使程序停顿时间等于传递来的时间参数。也就是说，for循环中每隔5秒调用一次recvfrom函数。另外还添加了验证函数调用次数的语句。

接下来的示例向之前的bound_host1.c传输数据，该示例共调用sendto函数3此以传输字符串数据。

[bound_host2.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_06/bound_host2.c)

程序3次调用sendto函数以传输数据，bound_host1.c则调用3此recvfrom函数以接收数据。recvfrom函数调用间隔5秒，因此，调用recvfrom函数前已调用了3次sendto函数。也就是说，此时数据已经传输到bound_host1.c。如果是TCP程序，这时只需要调用1次输入函数即可读入数据。UDP则不同，在这种情况下也需要调用3次recvfrom函数。可通过运行结果进行验证。

![运行结果](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_06/images/%E8%BF%90%E8%A1%8C%E7%BB%93%E6%9E%9C.png)

从运行结果，特别是bound_host1.c的运行结果中可以看出，共调用了3此recvfrom函数。这就验证必须在UDP通信过程中使用I/O函数调用次数保持一致。

> UDP套接字传输的数据包又称数据报，实际上数据报也属于数据包的一种。只是与TCP包不同，其本身可以称为1个完成数据。这与UDP的数据传输特性有关，UDP中存在数据边界，1个数据包即可称为1个完整数据，因此称为数据报。



**已连接（connected）UDP套接字与未连接（unconnected）UDP套接字**

TCP套接字中需注册待传输数据的目标IP和端口号，而UDP中则无序注册。因此，通过sendto函数传输数据的过程大致可分为一下3个阶段。

:one: 向UDP套接字注册目标IP和端口号。

:two: 传输数据。

:three: 删除UDP套接字中注册的目标地址信息。

每次调用sendto函数时重复上述过程。每次都变更目标地址，因此可以重复利用同一UDP套接字向不同目标传输数据。这种未注册目标地址信息的套接字称为未连接套接字，反之，注册了目标地址的套接字称为connected套接字。显然UDP套接字默认属于未连接套接字。但UDP套接字在下述情况下显得不太合理：

> “IP为211.210.147.82的主机82号端口共准备了3个数据，调用3次sendto函数进行传输。”

此时需要重复3此上述三阶段。因此，要与同一主机进行长时间按通信时，将UDP套接字编程已连接套接字会提高效率。上述三个阶段中，第一个和第三个阶段占整个通信过程近1/3的时间，缩短这部分时间将大大提高整体性能。



**创建已连接UDP套接字**

创建已连接UDP套接字的过程格外简单，只需针对UDP套接字调用connect函数。

```c
sock = socket(PF_INET, SOCK_DGRAM, 0);
memset(&adr, 0, sizeof(adr));
adr.sin_family = AF_INET;
adr.sin_addr.s_addr = ...;
adr.sin_port = ...;
connect(sock, (struct sockaddr*)&adr, sizeof(adr));
```

上述代码看似与TCP套接字创建过程一致，但socket函数的第二个参数分明是SOCKDGRAM。也就是说，创建的的确是UDP套接字。当然，针对UDP套接字调用connect函数并不意味着要与对方UDP套接字进行连接，这只是向UDP套接字注册目标IP和端口信息。

之后就与TCP套接字一样，每次调用sendto函数时只需要阐述数据。因为已经指定了收发对象，所以不仅可以使用sendto、recvfrom函数，还可以使用write、read函数进行通信。

下列示例将之前的uecho_client.c程序改成基于已连接UDP套接字的程序，因此可以结合uecho_server.c程序运行。另外，为方便说明，未直接删除uecho_client.c的I/O函数，而是添加了注释。

[uecho_con_client.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_06/uecho_con_client.c)

我认为没有必要给出运行结果和代码说明，故省略。另外需要注意，代码中用write、read函数代替了sendto、recvfrom函数。

