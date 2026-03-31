---
title: std::atomic<bool>
date: 2026-03-30
tags:
    - 并发
    - c/c++
categories:
    - 并发
    - c/c++
isOriginal: true
order: 3
dir:
    order: 3
---
## std::atomic\<bool\>
### 初始化和赋值

```c++
std::atomic<bool> b{true};
b = false;
```
### 成员函数
> [!important]
> exchange()成员函数允许使用新选的值替换已存储的值，并且会自动检索原始值(即返回旧值)。std::atomic\<bool\>支持通过隐式转换为简单的bool或通过显示调用load()对值进行非修改查询。store()是一个存储操作，而load()是一个加载操作。exchange()是一个“读-改-写”操作

```c++
std::atomic<bool> b;
bool x = b.load(std::memory_order_acquire);
b.store(true);
x = e.exchange(false, std::memory_order_acq_rel);
```
### CAS(compare and swap)
成员函数: compare_exchange_weak()和compare_exchage_strong()

> [!important]
> “比较/交换”操作是原子类型编程的基石，它比较原子变量的当前值和一个期望值，当两值相等时，存储所提供的值；当两值不等，期望值就会被更新为原子变量中的值。“比较/交换”函数的返回值是一个bool变量，当返回true时执行存储操作，false则更新期望值。当存储完成(因为只相等)，则操作时成功的，否则即为失败；操作成功是返回true，失败时返回false。

> [!important]
> 对于compare_exchange_weak()函数，当原始值与预期值一致时，存储也可能会不成功；在这个例子中变量的值不会发生改变，并且compare_exchange_weak()的返回是false。这可能发生在缺少单条CAS操作(“比较-交换”指令)的机器上，当处理器不能保证这个操作能够自动的完成——可能因为线程的操作将指令队列从中间关闭(即处理器不能保证操作原子的完成)，并且另一个线程安排的指令将会被操作系统所替换(这里线程数多于处理器数量)，被称为“伪失败”(spurious failure)，因为造成这种情况的原因是时间(失败的原因是时序的作用)，而不是变量值。

```c++
// 因为compare_exchange_weak()可以伪失败，所以通常需要配合循环使用
bool expected = false;
extern std::atomic<bool> b;
while (!b.compare_exchage_weak(expected, true) && !expected);
```

> [!important]
> 如果值很容易存储，那么使用compare_exchange_weak()能更好的避免一个双重循环的执行，即使compare_exchange_weak()可能会“伪失败”(因此compare_exchange_strong()包含一个循环)。另一方面，如果值的存储本身是耗时的，那么当期望值不变时，使用compare_exchange_strong()可以避免对值的重复计算。
> 比较函数的一个不同之处在于，它们可以使用两个内存顺序参数。这就允许内存序语义在成功和失败的例子中有所不同；可能是对memory_order_acq_rel成功调用，而对memory_order_relaxed语义的失败调用。失败的“比较/交换”将不会进行存储，所以“比较/交换”操作不能拥有memeory_order_release或memory_order_acq_rel语义。因此，不允许为失败提供这些值作为内存序，也不能提供比成功顺序更加严格的失败内存序，当memory_order_acquire或memory_order_seq_cst作为失败序时，必须要如同“指定成功语序”那样去做。
> 如果没有指定失败语序，那就假设和成功的顺序一样，除了release部分的顺序：memory_order_release变成memory_order_relaxed，并且memoyr_order_acq_rel变成memory_order_acquire。如果都不指定，默认顺序将为memory_order_seq_cst，这个顺序提供了对成功和失败的全排序。下面对compare_exchange_weak()的两次调用是等价的：

```c++
std::atomic<bool> b;
bool expected;
b.compare_exchange_weak(expected, true, std::memory_oredr_acq_rel, std::memory_oreder_acquire);
b.compare_exchange_weak(expected, true, std::memory_order_acq_rel);
```

> std::atomic\<bool\>和std::atomic_flag的进一步区别在于，std::atomic\<bool\>可能并不是无锁的。