---
title: 内存栅栏(memory barriers)
date: 2026-04-13
tags:
    - 并发
    - c/c++
categories:
    - 并发
    - c/c++
isOriginal: true
order: 6
dir:
    order: 6
---
## 1. 内存栅栏(memory barriers)
> [!important]
> 内存栅栏(memory barriers)在不修改任何数据的情况下对内存序进行约束，典型的做法是与使用memory_order_relaxed约束序的原子操作一起使用。栅栏属于全局操作，执行栅栏操作可以影响到在线程中的其他原子操作。因为这类操作就像画了一条任何代码都无法跨越的线一样，所以栅栏操作通常也被称为内存栅栏(memory barriers或者内存屏障)。通常不同变量的松散操作(std::memory_order_relaxed)可以被编译器或者硬件自由的重排。不过，栅栏操作就会限制这种自由并且引入了之前不存在的“先行发生(happens-before)”和“同步发生(asynchronizes-with)”关系

```c++
#include <atomic>
#include <thread>
#include <assert.h>

std::atomic<bool> x, y;
std::atomic<int> z;

void write_x_then_y() {
  x.store(true, std::memory_order_relaxed); // 1
  std::atomic_thread_fence(std::memory_order_release); // 2
  y.store(true, std::memory_order_relaxed); // 3
}

void read_y_then_x() {
  while (!y.load(std::memory_order_relaxed)); // 4
  std::atomic_thread_fence(std::memory_order_acquire); // 5
  if (x.load(std::memory_order_relaxed)) // 6
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
  assert(z.load() != 0); // 7
}
```

> 释放栅栏②与获取栅栏⑤同步，这是因为加载y的操作④读取③处存储的值。所以，①处存储x先行于⑥处加载x，最后x读取出来必为true，并且断言不会被触发⑦。原先不带栅栏的存储和加载x是无序的，并且断言是可能会触发。这两个栅栏都是必要的：需要在一个线程中进行释放，然后在另一个线程中进行获取，这样才能构建出同步关系。> 这个例子中，如果存储y的操作③标记为memory_order_release，而非memory_order_relaxed，释放栅栏②也同样，当加载y的操作④标记为memory_order_acquire时，获取栅栏⑤也会对之产生影响。使用栅栏的想法是：当获取操作能看到释放栅栏操作后的存储结果，那么这个栅栏就与获取操作同步；并且，当加载操作在获取栅栏操作前，看到一个释放操作的结果，那么这个释放操作同步于获取栅栏。当然，也可以使用双边栅栏操作，举一个简单的例子：当一个加载操作在获取栅栏前，看到一个值有存储操作写入，且这个存储操作发生在释放栅栏后，那么释放栅栏与获取栅栏同步。

![栅栏可以让松散操作(std::memory_order_relaxed)变的有序](/thread/memory_fence_graph.svg)

> 虽然，栅栏同步依赖于读取/写入的操作发生于栅栏之前/后，但是这里有一点很重要：同步点，就是栅栏本身。当执行write_x_then_y，并且在栅栏操作之后对x进行写入，就像下面的代码一样。触发断言的条件就不保证一定为true了，尽管写入x的操作在写入y的操作之前发生。

```c++
void write_x_then_y() {
  std::atomic_thread_fence(std::memory_order_release);
  x.store(true, std::memory_order_relaxed);
  y.store(true, std::memory_order_relaxed);
}
```
> 这里的两个操作就不会被栅栏分开，并且也不再有序。只有当栅栏出现在存储x和存储y操作之间时，这个顺序才是硬性的。当然，栅栏是否存在不会影响任何拥有先行关系的执行序列，这种情况是因为一些其他原子操作。

## 2. 使用内存栅栏(memory barriers)对非原子的操作排序
```c++
#include <atomic>
#include <thread>
#include <assert.h>

bool x = false;
std::atomic<bool> y;
std::atomic<int> z;

void write_x_then_y() {
  x = true; // 1 在栅栏前存储x
  std::atomic_thread_fence(std::memory_order_release);
  y.store(true, std::memory_order_relaxed); // 2 在栅栏后存储y
}

void read_y_then_x() {
  while (!y.load(std::memory_order_relaxed)); // 3 在#2写入前，持续等待
  std::atomic_thread_fence(std::memory_order_acquire);
  if (x) // 4 这里读取到的值，是#1中写入
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
  assert(z.load() != 0); // 5 断言不会触发
}
```

> 栅栏仍然为存储x①和存储y②，还有加载y③和加载x④提供一个执行序列，并且这里仍然有一个先行关系，在存储x和加载x之间，所以断言⑤不会被触发。②中的存储和③中对y的加载，都必须是原子操作；否则，将会在y上产生条件竞争，不过一旦读取线程看到存储到y的操作，栅栏将会对x执行有序的操作。这个执行顺序意味着，即使它被另外的线程修改或被其他线程读取，x上也不存在条件竞争不仅是栅栏可对非原子操作排序。memory_order_release/memory_order_consume也可以，它们会对动态分配对象的非原子访问进行排序。