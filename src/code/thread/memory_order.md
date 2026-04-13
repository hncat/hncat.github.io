---
title: 同步操作和强制排序
date: 2026-04-03
tags:
    - 并发
    - c/c++
categories:
    - 并发
    - c/c++
isOriginal: true
order: 5
dir:
    order: 5
---
## 同步操作和强制排序
<a id="code-example-1">代码清单1 不同线程对数据的读写</a>

```c++
#include <vector>
#include <atomic>
#include <iostream>

std::vector<int> data;
std::atomic<bool> data_ready{false};

void ready_thread() {
  while (!data_ready.load()) { // 1
    std::this_thread::sleep(std::milliseconds(1));
  }
  std::cout << "The answer=" << data[0] << "\m"; // 2
}

void write_thread() {
  data.push_back(42); // 3
  data_ready = true; // 4
}
```
> 循环1(你需要这个循环，否则无法在线程间进行数据共享：因为每一个数据项必须是原子的)。当非原子读2和写3对同一个数据结构进行无序访问时，将会导致未定义行为所以这个循环就是确保访问是被严格遵守的。
> 必须的强制顺序来自对原子变量data_ready上的操作，它们通过内存模型关系`先发生于(happens-before)`和`同步发生(synchronizes-with)`提供了必要的顺序。对数据的写操作3发生在对data_ready标记的写操作4之前，对标志的读操作1发生在对数据的读操作2之前。当从data_ready中读取的值1为真时，写操作与读操作同步，创建了一个`先发生于(happens-before)`关系。因为`先发生于(happens-before)`是可传递的，写入数据3发生在写入标志4前，写入标志又发生在从标志中读出true值1之前，读出true值又发生在读取数据2之前，然后你有一个强制顺序；数据的写入发生在数据的读取之前，一切正常。

#### 执行顺序图
![执行顺序](/thread/thread_01.svg)

#### 关键结论
通过 **synchronizes-with** 关系的传递性，我们建立了跨线程的 happens-before 链：
> **3 → 4 → 1 → 2**
这保证了 `write_thread` 对 `data` 的写入对 `ready_thread` 可见。

### 1. `同步发生(synchronizes-with)`
`同步发生(synchronizes-with)`只能在原子类型的操作之间获得。如果数据结构包含原子类型，并且数据结构上的操作在内部执行适当的原子操作，那么数据结构上的操作(比如锁住互斥锁)可能提供这种关系。

> [!important]
> `同步发生`的基本思想：原子写操作W对变量x进行标记，同步与对x进行原子读操作，读取的是W操作写入的内容；或 是W之后，同一线程上的原子写操作对x写入的值；亦或是任意线程对x的一系列原子读-改-写操作(例如， fetch_add()或compare_exchange_weak())。这里，第一个线程所读取到的值为W操作写入
> 因为所有对原子类型的操作，默认都是“`适当标记(suitable-tagged)`”的。这实际上就是.：如果线程A存储了一个值，并且线程B读取了这个值，线程A的存储操作与线程B的载入操作就是同步发生的关系。

![同步发生(synchronizes-with)](/thread/thread_02.svg)

| 场景             | 同步关系                                                      |
| -------------- | --------------------------------------------------------- |
| **R 读到 W 的值**  | W ↔ R 直接建立 `synchronizes-with`                            |
| **R 读到 W1 的值** | W1 ↔ R 建立 `synchronizes-with`（W 通过 `sequenced-before` 传递） |
| **RMW 操作**     | 其读部分可同步于 W/W1，写部分成为新的同步点                                  |

### 2. `先行发生(happens-before)`
> [!important]
> “先行发生”关系是一个程序中，基本构建块的操作顺序：它指定了某个操作去影响另一个操作。对于单线程来说：当一个操作排在另一个之后，那么这个操作就是先行执行的。如果源码中操作A发生在操作B之前，那么A就先行于B发生。列如[代码清单1](#code-example-1): 对data的写入③先于对data_ready④的写入。如果操作在同时发生，因为操作间无序执行，通常情况下它们就没有先行关系了。这就是另一种排序未被指定的情况。

<a id="code-example-2">代码清单2 对于参数中的函数调用顺序是未指定顺序的</a>

```c++
#include <iostream>

void foo(int a, int b) {
  std::cout << a << ", " << b << std::endl;
}

int get_num() {
  static int i = 0;
  return ++i;
}

int main() {
  // 输出“1，2”或“2，1”，因为两个get_num()的执行顺序未被指定。
  foo(get_num(), get_num()); // 无序调用get_num()
}
```
> [!tip]
> 在某些情况下，单个语句的操作是有序的，比如使用内置的逗号操作符，或者将一个表达式的结果用作另一个表达式的参数。单通常，单个语句中的操作是无序的，它们之间没有`先序于(sequenced-before)`(指同一个线程内的求值顺序，一个求值先于另一个求值)(因此也没有`先行发生(happens-before)`)关系。一个语句中的所有操作都发生在下一个语句中的所有操作之前

> 如果一个线程上的操作A“线程间先发于”(inter-thread happens before)另一个线程上的操作B，那么A就先行于B。这并没有什么：只是添加了一个新的关系“线程间先发于”(inter-thread happens before)。但当在编写多线程程序时，这就是一个至关重要的关系了。

#### 2.1 线程间 `先行发生`(inter-thread happens before)
> [!important]
> 线程间的先行比较简单，并且依赖与同步关系：如果操作A在一个线程上，与另一个线程上的操作B同步，那么A就线程间先行于B。这同样是一个传递关系：如果A线程间先行于B，并且B线程间先行于C，那么A就线程间先行于C。
> 线程间先行可以与排序先行相结合：如果操作A排序先行于操作B，并且操作B线程间先行于操作C，那么A线程间先行于C。同样的，如果A同步于B，并且B排序先于C，那么A线程间先行于C。两者的结合意味着，当你对数据进行一系列修改(单线程)时，为线程后续执行C，只需要对数据进行一次同步即可。

![线程间先行发生(inter-thread happens before)](/thread/thread_03.svg)

#### 2.2 强先行发生(strong happens-before)
> [!important]
> 强先行发生关系会有一些不同，不过在大多数情况下是一样的。如果操作A与 操作B同步，或操作A的顺序在操作B之前，那么A就是强先行于B。也适用于顺序传递：如果A强先行于B，并且B强先行 于C，那么A就肯定强先行于C。事件在线程间的先行关系与普通事件间的关系有所区别，这里的区别就在于操作被标记 为memory_order_consume，但不是强先行关系。由于大多数代码并不适用 memory_order_consume内存序，因此这种区别在实际中可能不会表现的很明显。

![强先行发生(strong happens-before)](/thread/thread_04.svg)


| 特性      | 强先行发生（Strong HB）                                   | 普通先行发生（Plain HB）       |
| ------- | -------------------------------------------------- | ---------------------- |
| 触发来源    | **synchronizes-with（acquire / release / seq_cst）** | **依赖 + consume（理论存在）** |
| 同步范围    | **写线程中 release 之前的所有操作**                           | **仅数据依赖链上的操作**         |
| 传递性     | ✅ **完全传递（transitive）**                             | ⚠️ **仅依赖链上传递**         |
| 是否形成 HB | ✅ 标准 happens-before                                | ⚠️ 边缘语义（几乎废弃）          |
| 实际使用    | ✅ 广泛使用（推荐）                                         | ❌ 几乎没人用（实现被弱化）         |

> C++ 的 consume 几乎废弃主要原因：编译器很难正确实现“依赖链追踪”
> 所以几乎所有实现（gcc/clang）：
> 👉 直接把 consume 当 acquire 处理
> ✔ 也就是说现实中：
> memory_order_consume ≈ memory_order_acquire

### 3. 原子操作的内存顺序
> [!important]
> 六种内存序:memory_order_relaxed, memory_order_consume,memory_order_acquire, memory_order_release, memory_order_acq_rel, 以及memory_order_seq_cst(默认内存序)。
> 三种内存模型:
> 1. 排序一致序列(默认内存模型sequentially consistent, memory_order_seq_cst)
> 2. 获取-释放序列(memory_order_consume, memory_order_acquire,memory_order_release和memory_order_acq_rel)
> 3. 松散序列(memory_order_relaxed)。

#### 3.1 排序一致序列(memory_order_seq_cst)
> [!tip]
> 默认序列命名为排序一致，因为程序中的行为从任意角度去看，序列顺序都保持一致。如果原子类型实例上的所有操作都是序列一致的，那么一个多线程程序的行为，就会以某种特殊的排序执行，如单线程那样。所有线程都必须了解，不同的操作也要遵守相同的顺序。因为其简单的行为，可以使用原子变量进行编写。通过不同的线程，可以写出所有序列上可能的操作，就可以消除那些不一致，以及验证代码的行为是否与预期相符。也就意味着，所有操作都不能重排；如果代码在一个线程中，将一个操作放在另一个操作前面，那么这个顺序就必须让其他线程有所了解。
> 从同步的角度看，对于同一变量的存储操作同步与载入操作。这就提供了一种对两个(以上)线程操作的排序约束，但排序一致的功能要比排序约束大的多，所以对于使用排序一致的原子操作，都会在对值进行存储后进行加载。这种约束不是线程在松散内存序列中使用原子操作；这些线程依旧可以知道，操作以不同顺序排列，所以必须使用排序一致的操作去保证在多线的情况下，有加速的效果。


<a id="code-example-3">代码清单3 全序-序列一致</a>

```c++
#include <atomic>
#include <thread>
#include <assert.h>

std::atomic<bool> x,y;
std::atomic<int> z;

void write_x() {
  x.store(true, std::memory_order_seq_cst); // 1
}

void write_y() {
  y.store(true, std::memory_order_seq_cst); // 2
}

void read_x_then_y() {
  while (!x.load(std::memory_order_seq_cst));
  if (y.load(std::memory_order_seq_cst)) // 3
    ++z;
}

void read_y_then_x() {
  while (!y.load(std::memory_order_seq_cst));
  if (x.load(std::memory_order_seq_cst)) // 4
    ++z;
}

int main() {
  x = false;
  y = false;
  z = 0;
  std::thread a{write_x};
  std::thread b{write_y};
  std::thread c{read_x_then_y};
  std::thread d{read_y_then_x};
  a.join();
  b.join();
  c.join();
  d.join();
  assert(z.load() != 0); // 5
}
```

> assert⑤语句是永远不会触发的，因为不是存储x的操作①发生，就是存储y的操作②发生。如果在read_x_then_y中加载y③返回false，是因为存储x的操作肯定发生在存储y的操作之前，在这种情况下在read_y_then_x中加载x④必定会返回true，因为while循环能保证在某一时刻y是true。因为memory_order_seq_cst的语义需要一个全序将所有操作都标记为memory_order_seq_cst，这就暗示着“加载y并返回false③”与“存储y①”的操作，需要有一个确定的顺序。只有在全序时，当一个线程看到x==true，随后又看到y==false，这就意味着在总序列中存储x的操作发生在存储y的操作之前。当然，因为事情都是对称的，所以就有可能以其他方式发生，比如：加载x④的操作返回false，或强制加载y③的操作返回true。这两种情况下，z都等于1。当两个加载操作都返回true，z就等于2；所以任何情况下，z都不能是0。当read_x_then_y知道x为true，并且y为false，那么这些操作就有“先发执行”关系了。

![排序一致序列(memory_order_seq_cst)](/thread/thread_05.svg)

#### 3.2 非排序一致内存模型
> [!tip]
> 当踏出序列一致的世界，事情就开始变的复杂。可能最需要处理的问题就是：再也不会有全局的序列了。这就意味着 不同线程看到相同操作，不一定有着相同的顺序，还有对于不同线程的操作，都会一个接着另一个执行的想法不在可行。不仅是有没有考虑事情真的同时发生的问题，还有就是线程没办法保证一致性。为了写出(或仅是了解)任何一段使用非默认内存序列的代码，这不仅仅是编译器可以重新排列指令的问题。即使线程运行相同的代码，它们都能拒绝遵循事件发生的顺序，因为操作在其他线程上没有明确的顺序限制；而不同的CPU缓存和内部缓冲区，在同样的存储空 间中可以存储不同的值。这非常重要，这里我再重申一遍：线程没办法保证一致性。
> 不仅是要摒弃串行执行操作的想法，还要放弃使用编译器或处理器重排指令的想法。在没有明确的顺序限制下，唯一 的要求就是：所有线程都要统一对每一个独立变量的修改顺序。对不同变量的操作可以体现在不同线程的不同序列 上，提供的值要与任意附加顺序限制保持一致。

##### 3.2.1 松散序列(memory_order_relaxed)
> [!tip]
> 原子类型上的操作以松散序列执行，没有任何同步关系。同一线程中对于同一变量的操作还是服从先发执行的关系， 但是不同线程几乎不需要相对的顺序。唯一的要求是在访问同一线程中的单个原子变量不能重排序，当给定线程看到原子变量的特定值时，随后线程的读操作就不会去检索变量较早的那个值。当使用memory_order_relaxed，就不需要任何额外的同步，对于每个变量的修改顺序只是线程间共享的事情。

<a id="code-example-4">代码清单4 宽松操作只有很少的顺序要求</a>

```c++
#include <atomic>
#include <thread>
#include <assert.h>

std::atomic<bool> x, y;
std::atomic<int> z;

void write_x_then_y() {
  x.store(true, std::memory_order_relaxed); // 1
  y.store(true, std::memory_order_relaxed); // 2
}

void read_y_then_x() {
  while (!y.load(std::memory_order_relaxed)); // 3
  if (x.load(std::memory_order_relaxed)) // 4
    ++z;
}

int main() {
  x = false;
  y = false;
  z = 0;
  std::thread a{write_x_then_y};
  std::thread b{read_y_then_x};
  a.join();
  b.join();
  assert(z.load() != 0);
}
```

> 这次assert⑤可能会触发，因为加载x的操作④可能读取到false，即使加载y的操作③读取到true，并且存储x的操 作①先发与存储y的操作②。x和y是两个不同的变量，所以这里没有顺序去保证每个操作产生相关值的可见性。

> [!tip]
> 松散内存对于不同变量可以自由重排序，只要它们服从任意的先发执行关系即可（比如，在同一个线程中），它们不会引入同步相关的顺序。尽管在不同的store/load操作间有着先发执行关系，这里不是在一对存储于载入之间了，所以载入操作可以看到‘违反“顺序的存储操作。

![松散序列与先发执行](/thread/thread_06.svg)

<a id="code-example-5">代码清单5 多线程版上的松散操作</a>

```c++
#include <thread>
#include <atomic>
#include <iostream>

std::atomic<int> x{0}, y{0}, z{0}; // 1
std::atomic<bool> go{false}; // 2

const unsigned int loop_count = 10;

struct ready_values {
  int x, y, z;
};

read_values values1[loop_count];
read_values values2[loop_count];
read_values values3[loop_count];
read_values values4[loop_count];
read_values values5[loop_count];

void increment(std::atomic<int> *var_to_inc, read_values &values) {
  while (!go)
    std::this_thread::yield(); // 3 自旋，等待信号
  for (unsigned int i = 0; i < loop_count; ++i) {
    values[i].x = x.load(std::memory_order_relaxed);
    values[i].y = y.load(std::memory_order_relaxed);
    values[i].z = z.load(std::memory_order_relaxed);
    var_to_inc->store(i + 1, std::memory_order_relaxed); // 4
    std::this_thread::yield();
  }
}

void read_vals(read_values *values) {
  while (!go)
    std::this_thread::yield(); // 5 自旋，等待信号
  for (unsigned int i = 0; i < loop_count; ++i) {
    values[i].x = x.load(std::memory_order_relaxed);
    values[i].y = y.load(std::memory_order_relaxed);
    values[i].z = z.load(std::memory_order_relaxed);
    std::this_thread::yield();
  }
}

void print(read_values *v) {
  for (unsigned int i = 0; i < loop_count; ++i) {
    if (i)
      std::cout << ",";
    std::cout << "(" << v[i].x << "," << v[i].y << "," << v[i].z << ")";
  }
  std::cout << std::endl;
}

int main() {
  std::thread t1{increment, &x, values1};
  std::thread t2{increment, &y, values2};
  std::thread t3{increment, &z, values3};
  std::thread t4{read_vals, values4};
  std::thread t5{read_vals, values5};

  go = true; // 6 开始执行主循环信号

  t5.join();
  t4.join();
  t3.join();
  t2.join();
  t1.join();

  print(values1); // 7 打印最终结果
  print(values2);
  print(values3);
  print(values4);
  print(values5);
  return 0;
}
```

> 代码本质上很简单，有三个全局原子变量①和五个线程。每一个线程循环10次，使用时memory_order_relaxed读取三个原子变量的值，并且将它们存储在一个数组上。其中三个线程每次通过循环④来更新其中一个原子变量，这时剩下的两个线程就负责读取。当线程都“加入”，就能打印出来每个线程存到数组上的值了。原子变量go②用来确保线程在同时退出。启动线程是昂贵的操作，并且没有明确的延迟，第一个线程可能在最后一个线程开始前结束。每个线程都在等待go变为true前都在进行循环③⑤，并且一旦go设置为true所有线程都会开始运行⑥。
> 前三行中线程都做了更新，后两行线程只是做读取。每三个值都是一组x，y和z，并按照这样的顺序依次循环。对于输出，需要注意的一些事是：
> 1. 第一组值中x增1，第二组值中y增1，并且第三组中z增1。
> 2. x元素只在给定集中增加，y和z也一样，但是增加是不均匀的，并且相对顺序在所有线程中都不同。
> 3. 线程3看不到x或y的任何更新；它能看到的只有z的更新。这并不妨碍别的线程观察z的更新，并同时观察x和y的更新。

程序一种可能的输出结果:
```txt
(0,0,0),(1,0,0),(2,0,0),(3,0,0),(4,0,0),(5,7,0),(6,7,8),(7,9,8),(8,9,8),(9,9,10)
(0,0,0),(0,1,0),(0,2,0),(1,3,5),(8,4,5),(8,5,5),(8,6,6),(8,7,9),(10,8,9),(10,9,10)
(0,0,0),(0,0,1),(0,0,2),(0,0,3),(0,0,4),(0,0,5),(0,0,6),(0,0,7),(0,0,8),(0,0,9)
(1,3,0),(2,3,0),(2,4,1),(3,6,4),(3,9,5),(5,10,6),(5,10,8),(5,10,10),(9,10,10),(10,10,10)
(0,0,0),(0,0,0),(0,0,0),(6,3,7),(6,5,7),(7,7,7),(7,8,7),(8,8,7),(8,8,9),(8,8,9)
```

##### 3.2.2 获取-释放序列(memory_order_acquire/memory_order_release)
> [!tip]
> 这个序列是松散序列(relaxed ordering)的加强版；虽然操作依旧没有统一的顺序，但是在这个序列引入了同步。 这种序列模型中，原子加载就是获取(acquire)操作(memory_order_acquire)，原子存储就是释放 (memory_order_release)操作，原子读-改-写操作(例如fetch_add()或exchange())在这里，不是“获取”， 就是“释放”，或者两者兼有的操作(memory_order_acq_rel)。这里，同步在线程释放和获取间是成对的 (pairwise)。释放操作与获取操作同步，这样就能读取已写入的值。这意味着不同线程看到的序列虽不同，但这些 序列都是受限的。

<a id="code-example-6">代码清单6 获取-释放并不意味着全序</a>

```c++
#include <atomic>
#include <thread>
#include <assert.h>

std::atomic<bool> x, y;
std::atomic<int> z;

void write_x() {
  x.store(true, std::memory_order_release);
}

void write_y() {
  y.store(true, std::memory_order_release);
}

void read_x_then_y() {
  while (!x.load(std::memory_order_acquire));
  if (y.load(std::memory_order_acquire))  // 1
    ++z;
}

void read_y_then_x() {
  while (!y.load(std::memory_order_acquire));
  if (x.load(std::memory_order_acquire)) // 2
    ++z;
}

int main() {
  x = false;
  y = false;
  z = 0;
  std::thread a{write_x};
  std::thread b{write_y};
  std::thread c{read_x_then_y};
  std::thread d{read_y_then_x};
  a.join();
  b.join();
  c.join();
  d.join();
  assert(z.load() != 0); // 3
}
```

> 例子中断言③可能会触发(就如同自由排序那样)，因为可能在加载x②和y①的时候，读取到的是false。因为x和y是由不同线程写入，所以序列中的每一次释放到获取都不会影响到其他线程的操作。

![获取-释放，以及先行过程](/thread/thread_07.svg)

核心区别对比：
| 特性                | `acquire/release` | `seq_cst`         |
| ----------------- | ----------------- | ----------------- |
| **同步范围**          | 单变量配对同步           | 全局全序              |
| **跨变量保证**         | ❌ 无               | ✅ 有               |
| **① 和 ② 的关系**     | 独立，无顺序            | 有确定全局顺序           |
| **C 看到 x=true 时** | y 可能为 false       | y 必然为 true（若 ①→②） |
| **assert 结果**     | ⚠️ **可能触发**       | ✅ 永不触发            |
> 关键洞察：acquire/release 只建立单变量的 synchronizes-with（①→C 的 x，②→D 的 y），不建立跨变量的关系。C 通过 x 的同步知道 A 的操作，但不知道 B 对 y 的操作；D 通过 y 的同步知道 B 的操作，但不知道 A 对 x 的操作。没有全局全序，可能出现 C 看到 y=false 同时 D 看到 x=false。

![松散序列对比获取-释放序列](/thread/thread_08.svg)

> 为了了解获取-释放序列的优点，需要考虑将两次存储由一个线程来完成，就像[代码清单4](#code-example-4)那样。当需要使用memory_order_release改变y中的存储，并且使用memory_order_acquire来加载y中的值，就像下面程序清单所做的那样，而后，就会影响到序列中对x的操作。

<a id="code-example-7">代码清单7 获取-释放操作可以在松散操作上强加顺序</a>

```c++
#include <atomic>
#include <thread>
#include <assert.h>

std::atomic<bool> x, y;
std::atomic<int> z;

void write_x_then_y() {
  x.store(true, std::memory_order_relaxed); // 1
  y.store(true, std::memory_order_release); // 2
}

void read_y_then_x() {
  while (!y.load(std::memory_order_acquire)); // 3 自旋，等待y被设置为true
  if (x.load(std::memory_order_relaxed)) // 4
    ++z;
}

int main() {
  x = false;
  y = false;
  z = 0;
  std::thread a{write_x_then_y};
  std::thread b{read_y_then_x};
  a.join();
  b.join();
  assert(z.load() != 0); // 5
}
```
> 读取y③时会得到true，和存储时写入的一样②。因为存储使用的是memory_order_release，读取使用的 是memory_order_acquire，存储就与读取就同步了。因为这两个操作是由同一个线程完成的，所以存储x①先行于 加载y②。对y的存储同步与对y的加载，存储x也就先行于对y的加载，并且扩展先行于x的读取。因此，加载x的值必 为true，并且断言⑤不会触发。如果对于y的加载不是在while循环中，情况可能就会有所不同；加载y的时候可能会读取到false，这种情况下对于读取到的x是什么值，就没有要求了。为了保证同步，加载和释放操作必须成对。所以，无论有何影响，释放操作存储的值必须要让获取操作看到。当存储如②或加载如③，有一个是relaxed时，对x的访问就无序了，也就无法保证④处读到的是true，并且还会触发断言。

![获取-释放操作可以在松散操作上强加顺序](/thread/thread_09.svg)

##### 3.2.3 与同步传递相关的获取-释放序列
<a id="code-example-8">代码清单8 使用获取和释放顺序进行同步传递</a>

```c++
#include <atomic>
#include <assert.h>

std::atomic<int> data[5];
std::atomic<bool> sync1{false}, sync2{false};

void thread_1() {
  data[0].store(42, std::memory_order_relaxed);
  data[1].store(97, std::memory_order_relaxed);
  data[2].store(17, std::memory_order_relaxed);
  data[3].store(-141, std::memory_order_relaxed);
  data[4].store(2003, std::memory_order_relaxed);
  sync1.store(true, std::memory_order_release); // 1 设置sync1
}

void trhead_2() {
  while (!sync1.load(std::memory_order_acquire)); // 2 直到sync1设置后，循环结束
  sync2.store(true, std::memory_order_release); // 3 设置sync2
}

void thread_3() {
  while (!sync2.load(std::memory_order_acquire)); // 4. 直到sync2设置后，循环结束
  assert(data[0].load(std::memory_order_relaxed) == 42);
  assert(data[1].load(std::memory_order_relaxed) == 97);
  assert(data[2].load(std::memory_order_relaxed) == 17);
  assert(data[3].load(std::memory_order_relaxed) == -141);
  assert(data[4].load(std::memory_order_relaxed) == 2003);
}
```

> 尽管thread_2只接触到变量syn1②和sync2③，不过这对于thread_1和thread_3的同步就足够了，这能保证断言不会触发。首先，thread_1将数据存储到data中先行于存储sync1①(它们在同一个线程内)。因为加载sync1①的是一个while循环，它最终会看到thread_1存储的值(是从“释放-获取”对的后半对获取)。因此，对于sync1的存储先行于最终对于sync1的加载(在while循环中)。thread_3的加载操作④，位于存储sync2③操作的前面(也就是先行)。存储sync2③因此先行于thread_3的加载④，加载又先行于存储sync2③，存储sync2又先行于加载sync2④，加载syn2又先行于加载data。因此，thread_1存储数据到data的操作先行于thread_3中对data的加载，并且保证断言都不会触发。

![获取和释放顺序进行同步传递](/thread/thread_10.svg)

| 环节       | 关系 | 说明                               |
| -------- | -- | -------------------------------- |
| data → ① | sb | Thread 1 内，data 存储先于 sync1 存储    |
| ① → ②    | sw | release-acquire 同步（Thread 1 ↔ 2） |
| ② → ③    | sb | Thread 2 内，sync1 加载先于 sync2 存储   |
| ③ → ④    | sw | release-acquire 同步（Thread 2 ↔ 3） |
| ④ → data | sb | Thread 3 内，sync2 加载先于 data 加载    |


或者将sync1和sync2通过thread_2中使用"读-改-写"操作(memory_order_acq_rel)合并成一个独立的变量。
```c++
std::atomic<int> sync{0};

void thread_1() {
  // ...
  sync.store(1, std::memory_order_release);
}

void thread_2() {
  int expected = 1;
  while (!sync.compare_exchange_strong(expected, 2, std::memory_order_acq_rel))
    expected = 1;
}

void thread_3() {
  while (sync.load(std::memory_order_acquire) < 2);
  // ...
}
```


##### 3.2.4 获取-释放序列和memory_order_consume的数据相关性
> 因为memory_order_consume很特别：它完全依赖于数据，并且其展示了与线程间先行关系的不同之处。这个内存序非常特殊，即使在C++17中也不推荐你使用它。
> 数据依赖的概念相对简单：如果第二个操作数对第一个操作的结果进行操作，那么两个操作之间就存在数据依赖关系。有两种新关系用来处理数据依赖：“依赖先序于”(dependency-ordered-before)和“依赖带入”(carries-a-dependency-to)。就像“先序于”，“依赖带入”严格应用于一个线程内并在操作之间塑造数据依赖；如果操作A的结果作为一个操作数应用于操作B，那么A”依赖带入“B。如果A操作的结果是一个标量，比如int，当A的结果存储在一个变量中，并且这个变量作为操作数被操作B使用，那么关系任然存在。这个操作也是可以传递的，所以当A”依赖带入“B，并且B”依赖带入“C，就可以得出A“依赖带入”C的关系。
> 另一方面，“依赖先序于”关系可以应用于线程间，通过std::memory_order_consume的原子加载操作引入。它是std::memory_order_acquire的特殊情况，限制了同步数据为直接依赖；标记为memory_order_release， memory_order_acq_rel， memory_order_seq_cst的存储操作A”依赖先序于“标记为memory_order_consume的加载操作B，如果消费操作读取了存储值。这和你用memory_order_acquire加载获得的”同步于“关系截然相反。如果操作B”依赖带入”某个操作C，那么A也“依赖先序于”C。
> 如果他不影响“线程间先发于”关系时，对于同步来说并未带来任何的好处，但是它确实有影响：当A“依赖先序于”B，那么A“线程间先发于”B。

这种内存序列的一个很重要使用方式，在原子操作载入指向数据的指针时。当使用memory_order_consume作为加载语义，并且memory_order_release作为之前的存储语义，要保证指针指向的值是已同步的，并且不需要对其他任何非独立数据施加任何同步要求。下面的代码就展示了这么一个场景。

<a id="code-example-9">代码清单9 使用 std::memroy_order_consume 同步数据</a>

```c++
#include <atomic>

struct X {
  int i;
  std::string s;
};

std::atomic<X *> p;
std::atomic<int> a;

void create_x() {
  X *x = new X;
  x->i = 42;
  x->s = "hello";
  a.store(99, std::memory_order_relaxed); // 1
  p.store(x, std::memory_order_release);  // 2
}

void use_x() {
  X *x;
  while (!(x = p.load(std::memory_order_consume))) // 3
    std::this_thread::sleep(std::chrono::microseconds(1));
  assert(x->i == 42); // 42
  assert(x->s == "hello"); // 5
  assert(a.load(std::memory_order_relaxed) == 99); // 6
}

int main() {
  std::thread t1{create_x};
  std::thread t2{use_x};
  t1.join();
  t2.join();
  return 0;
}
```

> 尽管，对a的存储①在存储p②之前，并且存储p的操作标记为memory_order_release，加载p③的操作标记为memory_order_consume，这意味着存储p仅先行那些需要加载p的操作。同样，也意味着X结构体中数据成员所在的断言语句④⑤不会被触发，因为对x变量操作的表达式对加载p的操作携带有依赖。另一方面，对于加载变量a⑥的断言就不能确定是否会被触发；这个操作并不依赖于p的加载操作，所以这里没法保证数据已经被读取。当然，这个情况也很明显，因为这个操作被标记为memory_order_relaxed。

![使用 std::memroy_order_consume 同步数据](/thread/memory_order_execution_graph.svg)

> 有时，不想为携带依赖增加其他开销。想让编译器在寄存器中缓存这些值，以及优化重排序操作代码，而不是对这些依赖大惊小怪。可以使用 std::kill_dependecy() 来显式打破依赖链。 std::kill_dependency() 是一个简单的函数模板，会复制提供的参数给返回值。例如，当你拥有一个全局的只读数组，当其他线程对数组索引进行检索时，你使用的是 std::memory_order_consume ，那么你可以使用 std::kill_dependency() 让编译器知道这里不需要重新读取该数组的内容，就像下面的例子一样：

```c++
int global_data[] = {...};
std::atomic<int> index;

void f() {
  int i = index.load(std::memory_order_consume);
  do_something_with(global_data[std::kill_dependency(i)]);
}
```

> [!important]
> 实际操作中，应该持续使用memory_order_acquire，而对于 memory_order_consume和 std::kill_dependency 的使用是没有必要的。

#### 3.3 "释放序列"与"同步"
> [!important]
> 你可以在存储一个原子变量和另一个加载这个原子变量的线程之间获得一个“同步于”关系，即使有一系列的“读-改-写”操作在存储和加载之间，只要所有操作都被适当标记就没问题。当存储操作被标记为memory_order_release，memory_order_acq_rel或memory_order_seq_cst，加载被标记为memory_order_consume，memory_order_acquire或memory_order_ord_seq_cst，并且操作链上的每个操作加载的是由前面的操作写入的，那么操作链就构成了一个释放序列(release sequence)，并且最初的存储“同步于”(对应memory_order_acquire或memory_order_seq_cst)或是“依赖先序于”(对应memory_order_consume)最终的加载。操作链上的任何原子“读-改-写”操作可以拥有任意的内存顺序(甚至时memory_order_relaxed)。

<a id="code-example-10">代码清单10 使用原子操作从队列中读取值</a>

```c++
#include <atomic>
#include <thread>
#include <vector>

std::vector<int> queue_data;
std::atomic<int> count;

void wait_for_more_items() {
  //...
}

void process(int data) {
  //...
}

void populate_queue() {
  unsigned const number_of_items = 20;
  queue_data.clear();
  for (unsigned i = 0; i < number_of_itmes; ++i) {
    queue_data.push_back(i);
  }
  count.store(number_of_items, std::memory_order_release); // 1 初始化存储
}

void consume_queue_items() {
  while (true) {
    int item_index;
    if ((item_index = count.fetch_sub(1, std::memory_order_acquire)) <= 0) { // 2 一个“读-改-写”操作
      wait_for_more_items(); // 3 等待更多元素
      continue;
    }
    process(queue_data[item_index - 1]); // 4 安全读取queue_data
  }
}

int main() {
  std::thread a{populate_queue};
  std::thread b{consume_queue_items};
  std::thread c{consume_queue_items};
  a.join();
  b.join();
  c.join();
  return 0;
}
```

> 一种处理方式是让线程产生数据，并存储到一个共享缓存中，而后调用count.store(number_of_items,memory_order_release)①让其他线程知道数据是可用的。线程群会消耗队列中的元素，之后可能调用count.fetch_sub(1, memory_order_acquire)②向队列索取一个元素。在这之前，需要对共享缓存进行完整的读取④。一旦count归零，那么队列中就没有元素了，当没有元素耗线程必须等待③。
> 当只有一个消费者线程时还好，fetch_sub()是一个带有memory_order_acquire的读取操作，并且存储操作是带有memory_order_release语义，所以存储与加载同步，线程可以从缓存中读取元素。当有两个读取线程时，第二个fetch_sub()操作将看到被第一个线程修改的值，且没有值通过store写入其中。先不管释放序列的规则，第二个线程与第一个线程不存在先行关系，并且对共享缓存中值的读取也不安全，除非第一个fetch_sub()是带有memory_order_release语义的，这个语义为两个消费者线程建立了不必要的同步。要没有释放序列的规则，或者memory_order_release语义的fetch_sub操作，第二个消费者轻易就可以看到queue_data的存储操作，你将面临数据竞争问题。好在第一个fetch_sub()参与到“释放序列”中，所以store()能“同步于”第二个fetch_sub()操作。连个消费者线程间任然不存在“同步于”关系。

![队列操作的释放顺序 图解1](/thread/release_sequence_graph.svg)

> [!important]
> 核心机制：释放序列（Release Sequence）
> 根据C++标准` [Cpp Reference](https://en.cppreference.com/w/cpp/atomic/memory_order.html) ` ，释放序列的定义是：
> 在一个原子对象M上执行release操作A后，由相同线程执行的写入操作，或任意线程执行的原子RMW（读-改-写）操作，组成的最长连续子序列称为"以A为头的释放序列"。

![队列操作的释放顺序 图解2](/thread/release_sequence_dark_cn.svg)