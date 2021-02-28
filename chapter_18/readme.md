# 第18章	多线程服务器端的实现

#### 理解线程的概念

本章给出的是关于线程的通用说明

**引入线程的背景**

第10章介绍了多进程服务器端的实现方法。多进程模型与select或epoll相比的确有自身的有点，但同时也有问题。如前所述，创建进程（复制）的工作本身会给操作系统带来相当沉重的负担。而且，每个进程具有独立的内存空间，所以进程间通信的实现难度也会随之提高（参考第11章）。换言之，多进程模型的缺点可概括如下。

:one: 创建进程的过程会带来一定的开销。

:two: 为了完成进程间数据交换，需要特殊的IPC技术。

但相比于下面的缺点，上述2个缺点不算什么。

> "美妙少则数十次、多则数千次的'上下文切换'（Context Switching）是创建进程时最大的开销。"

只有1个CPU（准确地说是CPU的运算设备CORE）的系统中不是也可以同时运行多个进程嘛？这是因为系统将CPU时间分成多个微小的块后分配给了多个进程。为了分时使用CPU，需要"上下文切换"过程。下面了解一下"上下文切换"的概念。运行程序前需要将相应进程信息读入内存，如果运行进程A后需要接着运行进程B，就应该将进程A相关信息移出内存，并读入进程嗯B相关信息。这就是上下文切换。但此时进程A的数据将被移动到硬盘，所以上下文切换需要很长时间。即使通过优化加快速度，也会存在一定的局限。

> **上下文切换**
>
> 通过学习计算机结构和操作系统相关知识，可以了解到上下文切换中具体的工作过程。但我为了讲述网络编程，值介绍了基础概念。实际上该过程应该通过CPU内部的寄存器来解释。



为了保持多进程的优点，同时在一定程度上克服其缺点，人们引入了线程（Thread）。这是为了将进程的各种劣势降至最低限度（不是直接消除）而设计的一种"轻量级进程"。线程相比于进程具有如下有点。

:one: 线程的创建和上下文切换比进程的创建和上下文切换更快。

:two: 线程间交换数据时无需特殊技术。

各位很快会逐渐体会到这些优点，可以通过接下来的说明和线程相关代码进行准确理解。

**线程和进程的差异**

线程是为了解决如下困惑登场的：

> "嘿！为了得到多条代码执行流而复制整个内存区域的负担太重了！"

每个进程的内存空间都由保存全局变量的"数据区"、向malloc等函数的动态分配提供空间的堆（Heap）、函数运行时使用的栈（Stack）构成。每个进程都拥有这种独立空间，多个进程的内存结构如图18-1所示。

![图18-1](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_18/images/%E5%9B%BE18-1.png)

但如果以获得多个代码执行流为主要目的，则不应该像图18-1那样完全分离内存结构，而只需分离栈区域。通过这种方式可以获得如下优势。

:one: 上下文切换时不需要切换数据区和堆。

:two: 可以利用数据区和堆进行交换数据。

实际上这就是线程。线程为了保持多条代码执行流而隔开了栈区域，因此具有如图18-2所示的内存结构。

![图18-2](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_18/images/%E5%9B%BE18-2.png)

如图18-2所示，多个线程将共享数据区和堆。为了保持这种结构，线程将在进程内创建并运行。也就是说，进程和线程可以定义为如下形式。

:one: 在操作系统构成单独执行流的单位。

:two: 在进程构成单独执行流的单位。

如果说进程在操作系统内部生成多个执行流，那么线程就在同一进程内部创建多条执行流。因此，操作系统、进程、线程之间的关系可以通过图18-3表示。

![图18-3](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_18/images/%E5%9B%BE18-3.png)

以上就是线程的理论说明。没有实际编程就很难理解好线程，希望各位通过学习线程相关代码理解全部内容。

#### 线程创建及运行

POSIX是Portable Operating System Interface for Computer Environment（使用于计算机环境的可移植操作系统接口）的简写，是为了提高UNIX系列操作系统间的一致性而制定的API规范。下面要介绍的线程创建方法也是以POSIX标准为依据的。因此，它不仅适用于Linux，也适用于大部分UNIX系列的操作系统。

**线程的创建和执行流程**

线程具有单独的执行流，因此需要单独定义线程的main函数，还需要请求操作系统在单独的执行流中执行该函数，完成该功能的函数如下。

```c
#include <pthread.h>
int pthread_create(
    pthread_t * restrict thread, const pthread_attr_t * restrict attr,
    void *(*start_routine) (void *), void *arg
);
/*
 * 成功时返回0,失败时返回其他值
 * thread 保存新创建线程ID的变量地址值。线程与进程相同，也需要用于区分不同线程的ID。
 * attr 用于传递线程属性的参数，传递NULL时，创建默认属性的线程。
 * start_routine 相当于线程main函数的、在单独执行流中执行的函数地址值（函数指针）。
 * arg 通过第三个参数传递调用函数时包含传递参数信息的变量地址值。
 */
```

要想理解好上述函数的参数，需要熟练掌握restrict关键字和函数指针相关语法。但如果只关注使用方法（当然以后要掌握restrict和函数指针），那么该函数的使用比想象中要简单。下面通过简单示例了解该函数的功能。

[thread1.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_18/thread1.c)

![运行结果1](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_18/images/%E8%BF%90%E8%A1%8C%E7%BB%93%E6%9E%9C1.png)

从上述运行结果中可以看到，线程相关代码在编译时需要添加-pthread选项声明需要连接线程库，只有这样才能调用头文件pthread.h中声明的函数。上述程序的执行流程如图18-4所示。

![图18-4](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_18/images/%E5%9B%BE18-4.png)

图18-4中虚线代表执行流称，向下的箭头指的是执行流，横向箭头是函数调用。这些都是简单的符号，可以结合示例理解。接下来将上述示例的第15行sleep函数的调用语句改成如下形式：

> sleep(2);

各位运行后可以看到，此时不会像代码中写的那样输出5次"running thread"字符串。因为main函数返回后整个进程被销毁，如图18-5所示。

![图18-5](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_18/images/%E5%9B%BE18-5.png)

正因如此，我们在之前的示例中通过调用sleep函数向线程提供了充足的执行时间。

> "那线程相关程序中必须适当调用sleep函数！"

并非如此！通过调用sleep函数控制线程的执行相当于预测程序的执行流程，但实际上这是不可能完成的事情。而且稍有不慎，很可能干扰程序的正常执行流。例如，怎么可能在上述示例中准确预测thread_create函数的运行时间，并让main()函数恰好等待这么长时间呢？因此，我们不用sleep函数，而是通常利用下面的函数控制线程的执行流。通过下列函数可以更有效地解决现讨论的问题，还可同时了解线程ID的用法。

```c
#include <pthread.h>
int pthread_join(pthread_t thread, void **status);
/*
 * 成功时返回0,失败时返回其他值
 * thread 该参数值ID的线程终止后才会从该函数返回
 * status 保存线程的main函数返回值指针变量地址值
 */
```

简言之，调用该函数的进程（或线程）将进入等待状态，直到第一个参数为ID的线程终止位置。而且可以得到线程的main函数返回值，所以该函数比较有用。下面通过示例了解该函数的功能。

[thread2.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_18/thread2.c)

![运行结果2](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_18/images/%E8%BF%90%E8%A1%8C%E7%BB%93%E6%9E%9C2.png)

最后，为了让大家更好的理解该示例，给出其执行流程图，如图18-6所示。请注意观擦程序暂停后从线程终止时（线程main函数返回时）重新执行的部分。

![图18-6](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_18/images/%E5%9B%BE18-6.png)

**可在临界区内调用的函数**

之前的示例中只创建了1个线程，接下来的示例将开始创建多个线程。当然，无论创建多少线程，其创建方法没有区别。但关于线程的运行需要考虑"多个线程同时调用函数时（执行时）可能产生问题"。这类函数内部存在临界区（Critical Section），也就是说，多个线程同时执行这部分代码时，可能引起问题。临界区中至少存在1条这类代码。

稍后将讨论哪些代码可能 称为临界区，多个线程同时执行临界区代码时会产生哪些问题等内容。现阶段只需理解临界区的概念即可。根据临界区是否引起问题，函数可分为一下2类。

:one: 线程安全函数（Thread-safe function）

:two: 非线程安全函数（Thread-unsafe function）

线程安全函数被多个线程同时调用时也不会引发问题。反之，非线程安全函数被同时调用时会引发问题。但这并非关于有无临界区的讨论，线程安全的函数中同样可能存在临界区。知识在线程安全函数中，同时被多个线程调用时可通过一些措施避免问题。

幸运的是，大多数标准函数都是线程安全的函数。更幸运的是，我们不用自己区分线程安全的函数和非线程安全的函数（在Windows程序中同样如此）。因为这些平台在定义非线程安全函数的同时，提供了具有相同功能的线程安全的函数。比如，第8章介绍过的如下函数就不是线程安全的函数：

```c
struct hostent * gethostbyname(const char * hostname)
```

同时提供线程安全的同一功能的函数。

```c
// 书上这么写的
struct hostent * gethostbyname_r(
	const char * name, struct hostent * result, char * buffer, int buflen,
	int * h_errnop);
// man手册这样写的
int gethostbyname_r(const char *name,
               struct hostent *ret, char *buf, size_t buflen,
               struct hostent **result, int *h_errnop);
```

线程安全函数的名称后缀通常为_r（这与Windows平台不同）。既然如此，多个线程同时访问的代码块中应该调用gethostbyname_r，而不是gethostbyname？当然！但这种方法会给程序员带来沉重的负担。幸好可以通过如下方法自动将gethostbyname函数调用改为gethostbyname_r函数调用！

> "声明头文件前定义_REENTRANT宏。"

gethostbyname函数和gethostbyname_r函数的函数名和参数声明都不同，因此，这种宏声明方式拥有巨大的吸引力。另外，无需为了上述宏定义特意添加#define语句，可以在编译时通过添加-D_REENTRANT选项定义宏。

```bash
$ gcc -D_TEENTRANT mythread.c -o mthread -lpthread
```

下面编译线程相关代码时均默认添加-D_REENTRANT选项。

**工作（Worker）线程模型**

之前示例的目的主要是介绍线程概念和创建线程的方法，因此从未涉及1个示例中创建多个线程的情况。下面给出此类示例。

将要介绍的示例将计算1到10的和，但并不是在main函数中进行累加运算，而是创建2个线程，其中一个线程计算1到5的和，另一个线程计算6到10的和，main函数只负责输出运算结果。这种方式的编程模型称为"工作线程（Worker thread）模型"。计算1到5之和的线程与计算6到10之和的线程将称为main线程管理的工作（Worker）。最后，给出示例代码前先给出程序执行流程图，如图18-7所示。

![图18-7](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_18/images/%E5%9B%BE18-7.png)

之前也介绍过类似的图，相信各位很容易看懂图18-7描述的内容（知识单纯说明图，并未使用特殊的表示方法）。另外，线程相关代码的执行流程理解起来相对复杂一些，有必要习惯于这类流程图。

[thread3.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_18/thread3.c)

之前讲过线程调用函数的参数和返回值类型，因此不难理解上述示例中创建线程并执行的部分。但需要注意：

> "2个线程直接访问全局变量sum！"

通过上述示例的第28行可以得出这种结论。从代码的角度看似乎理所应当，但之所以可行完全因为2个线程共享保存全局变量的数据区。

![运行结果3](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_18/images/%E8%BF%90%E8%A1%8C%E7%BB%93%E6%9E%9C3.png)

运行结果是55,虽然正确，但示例本省存在问题。此处存在临界区相关问题，因此再介绍另一示例。该示例与上述示例相似，只是增加了发生临界区相关错误的可能性，即使在高配置系统环境下也容易验证产生的错误。

[thread4.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_18/thread4.c)

上述示例中共创建了100个线程，其中一半执行thread_inc函数中的代码，另一半则执行thread_des函数中的代码。全局变量num经过增减过程后应存有0,通过运行结果观擦是否真能得到。

![运行结果4](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_18/images/%E8%BF%90%E8%A1%8C%E7%BB%93%E6%9E%9C4.png)

运行结果并不是0！而且每次运行的记过均不同。虽然其原因尚不得而知，但可以肯定的是，这对于线程的应用是个大问题。

#### 线程存在的问题和临界区

我们还不知道示例thread4.c中产生问题的原因，下面分析该问题并给出解决方案。

**多个线程访问同一变量是问题**

示例thread4.c的问题如下：

> "2个线程正在同时访问全局变量num。"

此处的"访问"是指值的更改。产生问题的原因可能还有很多，因此需要准确理解。虽然示例中的访问的对象是全局变量，但这并非全局变量引发的问题。任何内存空间——只要被同时访问——都可能发生问题。

> "不是说线程会分时使用CPU嘛？那应该不会出现同时访问变量的情况啊。"

当然，此处的"同时访问"与各位所想的有一定区别。下面通过示例解释"同时访问"的含义，并说明为何会引起问题。假设2个线程要执行将变量值逐次加1的工作，如图18-8所示。

![图18-8](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_18/images/%E5%9B%BE18-8.png)

上图中描述的是2个线程准备将变量num的值加1的情况。在此状态下，线程1将num的值增加到100后，线程2在访问num时，变量num中将按照我们的预想保存101.图18-9是线程1将变量num完全增加后的情形。

![图18-9](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_18/images/%E5%9B%BE18-9.png)

上图中需要注意值的增加方式，值的增加需要CPU运算完成，变量num中的值不会自动增加。线程1首先读该变量的值并将其传递到CPU，获取加1之后的结果，最后再把结构写回变量num，这样num中就保存100.接下来给出线程2的执行过程，如图18-10所示。

![图18-10](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_18/images/%E5%9B%BE18-10.png)

变量num中将保存101,但这是最理想的情况。线程1完全增加num值之前，线程2完全有可能通过切换得到CPU资源。下面从头再来。图18-11描绘的是线程1读取变量num的值并完成加1运算时的情况，知识加1后的结果尚未写入变量num。

![图18-11](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_18/images/%E5%9B%BE18-11.png)

接下来就要将100保存到变量num中，但执行该操作前，执行流程跳转到了线程2.幸运的是（是否真正幸运稍后再论），线程2完成了加1运算，并将加1之后的记过写入变量num，如图18-12所示。

![图18-12](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_18/images/%E5%9B%BE18-12.png)

从上图中可以看到，变量num的值尚未被线程1加到100,因此线程2读到的变量num的值为99,结果是线程2将num值改成100.还剩下线程1将运算后的值写入变量num的操作。接下来给出该过程，如图18-13所示。

![图18-13](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_18/images/%E5%9B%BE18-13.png)

很可惜，此时线程1将自己的运算结果100在此写入变量num，结果变量num编程100.虽然线程1和线程2各做了1次加1运算，却得到了意想不到的结果。因此，线程访问变量num时应该阻止其他线程更访问，知道线程1完成运算。这就是同步（Synchronization）。相信各位也意识到了多线程编程中"同步" 的必要性，且能够理解thread4.c的运行结果。

**临界区位置**

划分临界区并不难。既然临界区定义为如下这种形式，那就在示例thread4.c中寻找。

> "函数内同时运行多个线程时引起问题的多条语句构成的代码块。"

全局变量num是否应该视为临界区？不是1因为它不是引起问题的语句。该变量并非同时运行的语句，知识代表内存区域的声明而已。临界区通常位于由线程运行的函数内部。下面观擦示例thread4.c中的2个main函数。

```c
void *thread_inc(void *arg)
{
    int i;
    for (i = 0; i != 50000000; ++i)
        num += 1;	// 临界区
    return NULL;
}

void *thread_des(void *arg)
{
    int i;
    for (i = 0; i != 50000000; ++i)
        num -= 1;	// 临界区
    return NULL;
}
```

由代码注释可知，临界区并非num本身，而是访问num的2条语句。这2条语句可能由多个线程同时运行，也是引起问题的直接原因。产生的问题可以整理为如下3中情况。

:one: 2个线程同时执行thread_inc函数。

:two: 2个线程同时执行thread_des函数。

:three: 2个线程分别执行thread_inc函数和thread_des函数。

需要关注最后一点，它意味这如下情况也会引发问题：

> "线程1执行thread_inc函数的num+=1语句的同时，线程2执行thread_des函数的num-=1语句。"

也就是说，2条不同语句由不同线程同时执行时，也有可能构成临界区。前提是这2条语句访问同一内存空间。

#### 线程同步

前面探讨了线程中存在的问题，接下来就要讨论解决访问——线程同步。

**同步的两面性**

线程同步用于解决线程访问顺序引发的问题。需要同步的情况可以从如下两个方面考虑。

:one: 同时访问同一内存空间时发生的情况。

:two: 需要指定访问同一内存的线程执行顺序的情况。

之前已解释过前一中情况，因此终点讨论第二中情况。这是"控制（Control）线程执行顺序"的相关内容。假设有A、B两个线程，线程A负责指定内存空间写入（保存）数据，线程B负责取走该数据。这种情况下，线程A首先应该访问约定的内存空间并保存数据。万一线程B先访问并取走数据，将导致错误结果。像这种需要控制执行顺序的情况也需要使用同步技术。

稍后将介绍"互斥量"（Mutex）和"信号量"（Semaphore）这2种同步技术。二者概念上十分接近，只是理解了互斥量就很容易掌握信号量。而且大部分同步技术的原理都大同小异，因此，只要掌握了本章介绍的同步技术，就很容易掌握并运用Windows平台下的同步技术。

**互斥量**

互斥量是"Mutual Exclusion"的简写，表示不允许多个线程同时访问。互斥量主要用于解决线程同步访问的问题。为了理解好互斥量，请观擦如下对话过程。

:a: 东秀："请问里面有人吗？"

:b: 英秀："是的，有人。"

:a: 东秀："您好！"

:b: 英秀："请稍等！"

相信各位也猜到了上述对话发生的场景。显示世界中的临界区就是洗手间。洗手间无法同时容纳多人（比作线程），因此可以将临界区比喻为洗手间。而且这里发生的所有事情几乎可以全部套用到临界区同步过程。洗手间使用规则如下。

:one: 为了保护个人隐私，进洗手间时锁上门，出来时再打开。

:two: 如果有人使用洗手间，其他人需要在外面等待。

:three: 等待的人数可能很多，这些人需排队进入洗手间。

这就是洗手间的使用规则。同样，线程中为了保护临界区也需要套用上述谷子娥。洗手间中存在，但之前的线程示例中缺少的是什么呢？就是锁机制。线程同步中同样向uyao锁，就像洗手间示例中使用的那样。互斥量就是一把优秀的锁，接下来介绍互斥量的创建即销毁函数。

```c
#include <pthread.h>
int pthread_mutex_init(pthread_mutex_t *restrict mutex,
           const pthread_mutexattr_t *restrict attr);
/*
 * 成功时返回0,失败时返回其他值
 * mutex 创建互斥量时传递保存互斥量的变量地址值，销毁时传递需要销毁的互斥量地址值。
 * attr 传递即将创建的互斥量属性，没有特别需要指定的属性时传递NULL。
 */
```

从上述函数声明中也可以看出，为了创建相当于锁系统的互斥量，需要声明如下pthread_mutex_t型变量：

```c
pthread_mutex_t mutex;
```

该变量的地址将传递给pthread_mutex_init函数，用来保存操作系统创建的互斥量（锁系统）。调用pthread_mutex_destroy函数时同样需要该信息。如果不需要配置特殊的互斥量属性，则向第二个参数传递NULL时，可以利用PTHREAD_MUTEX_INITIALIZER宏进行如下声明：

```c
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
```

但推荐各位尽可能使用pthread_mutex_init函数进行初始化，因为通过宏进行初始化时很难发现发生的错误。接下来介绍利用互斥量锁住或释放临界区时使用的函数。

```c
#include <pthread.h>
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
// 成功时返回0,失败时返回其他值
```

函数名本身含有lock、unlock等词汇，很容易理解其含义。进入临界区前调用的函数就是pthread_mutex_lock。调用该函数时，发现有其他线程已进入临界区，则pthread_mutex_lock函数不会返回，知道里面的线程调用pthread_mutex_unlock函数退出临界区为止。也就是说，其他线程让出临界区之前，当前线程将一直处于阻塞状态。接下来整理一下保护临界区的代码块编写方法。创建号互斥量的前提下，可以通过如下结构保护临界区。

```
pthread_mutex_lock(&mutex);
// 临界区的开始
// .....
// 临界区的结束
pthread_mutex_unlock(&mutex);
```

简言之，就是利用lock和unlock函数围住临界区的两端。此时互斥量相当于一把锁，阻止多个线程同时访问。还有一点需要注意，线程退出临界区时，如果忘了调用pthread_mutex_unlock函数，那么其他为了进入临界区而调用pthread_mutex_lock函数的线程就无法摆脱阻塞状态。这种情况称为"死锁"（Dead-lock），需要格外注意。接下来利用互斥量解决示例thread4.c中遇到的问题。

[mutex.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_18/mutex.c)

![运行结果5](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_18/images/%E8%BF%90%E8%A1%8C%E7%BB%93%E6%9E%9C5.png)

从运行结果可以看出，以解决了示例thread4.c中的问题。但确认运行结果需要等待较长时间。因为互斥量lock、unlock函数的调用过程要比想象中话费更长时间。首先分析一下thread_inc函数的同步过程。

```c
void *thread_inc(void *arg)
{
  int i;
  pthread_mutex_lock(&mutex);
  for (i = 0; i != 5000000; ++i)
    num += 1;
  pthread_mutex_unlock(&mutex);
  return NULL;
}
```

以上临界区划分范围较大，但这是考虑到如下优点所做的决定：

> "最大限度减少互斥量lock、unlock函数的调用次数。"

上述示例中，thread_des函数比thread_inc函数多调用很多次互斥量lock、unlock函数，表现出人可以感知的速度差异。如果不太关注线程的等待时间，可以适当扩展临界区。但变量num的值增加到50000000前不允许其他线程访问，这反而成了缺点。起始这里没有正确答案，需要根据不同程序酌情考虑究竟扩大还是缩小临界区。此处没有公式可言，各位需要培养自己的判断能力。

**信号量**

下面介绍信号量。信号量与互斥量极为相似，在互斥量的基础上很容易理解信号量。此处只涉及利用"二进制信号量"（只用0和1）完成"控制线程顺序"为核心的同步方法。下面信号量创建及销毁方法。

```c
#include <semaphore.h>
int sem_init(sem_t *sem, int pshared, unsigned int value);
int sem_destroy(sem_t *sem);
/*
 * 成功时返回0,失败时返回其他值
 * sem 创建信号量时传递保存信号的变量地址值，销毁时传递需要销毁的信号量变量地址值
 * pshared 传递其他值时，创建可由多个进程共享的信号量；传递0时，创建只允许1个进程内部使用的信号量。我们需要完成同一进程内的线程同步，故传递0
 * value 指定新创建的信号量初始值。
 */
```

上述函数的pshared参数超出了我们关注的范围，故默认向其传递0.稍后讲解通过value参数初始化的信号值究竟是多少。接下来介绍信号量中相当于互斥量lock、unlock的函数。

```c
#include <semaphore.h>
int sem_post(sem_t *sem);
int sem_wait(sem_t *sem);
/*
 * 成功时返回0,失败时返回其他值
 * sem 传递保存信号量读取值的变量地址值，传递给sem_post时信号量增1,传递给sem_wait时，信号量减1
 */
```

调用sem_init函数时，操作系统将创建信号量，此对象中记录这"信号量值"（Semaphore Value）整数。该值在调用sem_post函数时增1,调用sem_wait函数时减1.但信号量的值不能小于0,因此，在信号量为0的情况下调用sem_wait函数时，调用函数的线程将进入阻塞状态（因为函数未返回）。当然，此时如果有其他线程调用sem_post函数，信号量的值将变为1,而原本阻塞的线程可以将该信号量重新减为0并挑出阻塞状态。实际上就是通过这种特性完成临界区的同步操作，可以通过如下形式同步临界区（假设信号量的初始值为1）。

```c
sem_wait(&sem); // 信号量变为0
// 临界区的开始
// .....
// 临界区的结束
sem_post(&sem); // 信号值变为1
```

上述代码结构中，调用sem_wait函数进入临界区的线程在调用sem_post函数前不允许其他线程进入临界区。信号量的值在0和1之间跳转，因此，具有这种特性的机制称为"二进制信号量"。接下来给出信号量相关示例。即将介绍的示例并非关于同时访问的同步，而是关于控制访问顺序的同步。该示例的场景如下：

> "线程A从用户输入得到值后存入全局变量num，此时线程B将取走该值并累加。该过程共进行5此，完成后输出总和并退出程序。"

为了按照上述要求构建程序，应按照线程A、线程B的顺序访问变量num，且需要线程同步。接下来给出示例，分析该示例可能需要花费一定时间。

[semaphore.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_18/semaphore.c)

![运行结果6](https://github.com/katoluo/TCP-IP-Network-Programing/raw/master/chapter_18/images/%E8%BF%90%E8%A1%8C%E7%BB%93%E6%9E%9C6.png)

#### 线程的销毁和多线程并发服务器端的实现

我们之前只讨论了线程的创建和控制，而线程的销毁同样重要。下面先介绍线程的销毁，再实现多线程服务器端。

**销毁线程的3中方法**

Linux线程并不是在首次调用的线程main函数返回时自动销毁，所以用如下2中方法之一加以明确。否则由线程创建的内存空间将一致存在。

:one: 调用pthread_join函数。

:two: 调用pthread_detach函数。

之前调用过pthread_join函数。调用该函数时，不仅会等待线程终止，还会引导线程销毁。但该函数的问题是，线程终止前，调用该函数的线程将进入阻塞状态。因此，通常通过如下函数调用引导线程销毁。

```c
#include <pthread.h>
int pthread_detach(pthread_t thread);
/*
 * 成功时返回0,失败时返回其他值
 * thread 终止的同时需要销毁的线程ID
 */
```

调用上述函数不会引起线程终止或进入阻塞状态，可以通过该函数引导销毁线程创建的内存空间。调用该函数后不能再针对相应线程调用pthread_join函数，这需要格外注意。虽然还有方法在创建线程是可以指定销毁时机，但与pthread_detach方式相比，结果上没有太大差异，故省略其说明。在下面的多线程并发服务器端的实现过程中，希望各位同样关注线程销毁的部分。

**多线程并发服务器端的实现**

本节并不打算介绍回声服务器端，而是介绍多个客户端之间可以交换信息的简单的聊天程序。希望各位通过本示例复习线程的使用方法及同步的处理方法，还可以再次思考临界区的处理方式。

无论服务器端还是客户端，代码量都不少，故省略可以从其他示例中得到或从源代码中下载头文件声明。同时最大程度地减少异常处理的代码。

[chat_server.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_18/chat_server.c)

上述示例中，各位必须掌握的并不是聊天服务器端的实现方式，而是临界区的构成形式。上述示例中的临界区具有如下特点：

> "访问全局变量clnt_cnt和数组clnt_socks的代码将构成临界区！"

添加爱或删除客户端时，变量clnt_cnt和数组clnt_socks同时发生变化。因此，在如下情形中均会导致数据不一致，从而引发严重错误。

:one: 线程A从数组clnt_socks中删除套接字信息，同时线程B读取clnt_cnt变量。

:two: 线程A读取变量clnt_cnt，同时线程B将套接字信息添加到clnt_socks数组。

因此，如上述示例所示，访问变量clnt_cnt和数组clnt_socks的代码应组织在一起并构成临界区。大家现在应该对我之前说过的这句话有同感了把：

> "此处的'访问'是指值的更改。产生问题的原因可能还有很多，因此需要准确理解。"

接下来介绍聊天客户端，客户端示例为了分离输入和输出过程而创建了线程。代码分析并不难，故省略源代码相关说明。

[chat_client.c](https://github.com/katoluo/TCP-IP-Network-Programing/blob/master/chapter_18/chat_client.c)



