# 进程间通信

第10章讲解了如何创建进程，本章将讨论创建的2个进程之间交换数据的方法。这与构建服务器端无直接关系，但可能有助于构建多种类型服务器端，以及更好地理解操作系统。

#### 11.1 进程间通信的基本概念

进程间通信（Inter Process Communication）意味着两个 不同进程间可以交换数据，为了完成这一点，操作系统应提供两个进程可以同时访问的内存空间。

**对进程间通信的基本理解**

理解号进程间通信并没有想象中那么难，进程A和B之间的如下谈话内容就是一种进程间通信规则。

> "如果我有1个面包，变量bread的值就变为1.如果吃掉这个面包，bread的值又变回0.因此，你可以通过变量bread值判断我的状态。"

也就是说，进程A通过变量bread将自己的状态通知给了进程B，进程B通过变量bread听到了进程A的话。因此，只要有两个进程可以同时访问的内存空间，就可以通过此空间交换数据。但正如第10章所讲，进程具有完全独立的内存结构。就连通过fork函数创建的子进程也不会与父进程共享内存空间。因此，进程间通信只能通过其他特殊方法完成。

各位应该已经明白进程间通信的含义及其无法简单实现的原因，下面正式介绍进程间通信方法。

**通过管道实现进程间通信**

图11-1表示基于管道（PIPE）的进程间通信结构模型。

![图11-1](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_11/images/%E5%9B%BE11-1.png)

从图中可以看到，为了完成进程间通行，需要创建管道。管道并非属于进程的资源，而是和套接字一样，属于操作系统（也就不是fork函数的复制对象）。所以，两个进程通过操作系统提供的内存空间进行通信。下面介绍创建管道的函数。

```c
#include <unistd.h>
int pipe(int filedes[2]);
/*
 * 成功时返回0,失败时返回-1
 * filedes[0] 通过管道接收数据时使用的文件描述符，即管道出口
 * filedes[1] 通过管道传输数据时使用的文件描述符，即管道入口
 */
```

以长度为2的int数组地址值作为参数调用上述函数时，数组中存有两个文件描述符，它们将被用作管道的出口和入口。父进程调用该函数时将创建管道，同时获取对应出入口的文件描述符，此时父进程可以读写同一管道（相信大家也做过这样的实验）。但父进程的目的是与子进程进行数据交换，因此需要将入口或出口的1个文件描述符传递给子进程。如何完成传递呢？答案就是调用fork函数。通过下列示例进行演示。

[pipe1.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_11/pipe1.c)

![运行结果1](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_11/images/%E8%BF%90%E8%A1%8C%E7%BB%93%E6%9E%9C1.png)

上述示例中的通信方法及路径如图11-2所示。终点在于，父子进程都可以访问管道的I/O路径，但子进程仅用输入路径，父进程仅用输出路径。

![图11-2](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_11/images/%E5%9B%BE11-2.png)

以上就是管道的基本原理及通信方法。应用管道时还有一部分内容需要注意，通过双向通信示例进一步说明。

**通过管道进行进程间双向通信**

下面创建2个进程通过1个管道进行双向数据交换的示例，其通信方式如图11-3所示。

![图11-3](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_11/images/%E5%9B%BE11-3.png)

从图可看出，通过一个管道可以进行双向通信。但是采用这种模型时需格外注意。先给出示例，稍后再讨论。

[pipe2.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_11/pipe2.c)

![运行结果2](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_11/images/%E8%BF%90%E8%A1%8C%E7%BB%93%E6%9E%9C2.png)

运行结果应该和大家的预想一致。这次注释第18行代码后在运行（务必亲自动手操作）。虽然这行代码只将代码运行时间延迟了2秒，但已引发运行错误。产生原因是什么呢？

> "向管道传递数据时，先读的进程会把数据取走。"

简言之，数据进入管道后称为无主数据。也就是通过read函数先读取数据的进程将得到数据，即使进程将数据传到了管道。因此，注释第18行将产生问题。在第19行，子进程将读回自己在第17行向管道发送的数据。结果，父进程调用read函数后将无限期等待数据进入管道。

从上述示例中可以看到，只用1个管道进行双向通信并非易事。为了实现这一点，程序需要预测并控制运行流程，这在每种系统中都不同，可以视为不可能完成的任务。既然如此，该如何进行双向通信呢？

> "创建2个管道。"

非常简单，1个管道无法完成双向通信任务，因此需要创建2个管道，各自负责不同的数据流动即可。其过程如图11-4所示。

![图11-4](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_11/images/%E5%9B%BE11-4.png)

由图可知，使用2个管道可以避免程序流程的预测或控制。下面采用上述模型改进pipe2.c。

[pipe3.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_11/pipe3.c)

![运行结果3](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_11/images/%E8%BF%90%E8%A1%8C%E7%BB%93%E6%9E%9C3.png)

#### 11.2 运用进程间通信

上一节学习了基于管道的进程间通信方法，接下来将其运用到网络代码中。如前所述，进程间通信与创建服务器端并没有直接关联，但其有助于理解操作系统。

**保存消息的回声服务器端**

下面扩展第10章的echo_mpserv.c，添加如下功能：

> "将回声客户端传输的字符串按序保存到文件中。"

我希望将该任务委托给另外的进程。换言之，另行创建进程，从向客户端提供服务的进程读取字符串信息。当然，该过程中，需要创建用于接收数据的管道。

下面给出示例。该示例可以与任何回声客户端配合运行，但我们将用第10章介绍过的echo_mpclient.c。

[echo_mpclient.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_11/echo_storeserv.c)



