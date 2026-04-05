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
> [!note]
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
