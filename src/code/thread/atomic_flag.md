---
title: std::atomic_flag
date: 2026-03-30
tags:
    - 计算机基础
    - c/c++
categories: c/c++
isOriginal: true
order: 2
dir:
    order: 2
---
## std::atomic_flag
`std::atomic_flag`是最简单的原子类型，它表示了一个bool标志。这个类型的对象可以在两个状态间切换: 设置和清除。
`std::atomic_flag`类型的对象必须且只能被ATOMIC_FLAG_INIT初始化。初始化状态位是‘清除’状态。
```c++
std::atomic_flag f = ATOMIC_FLAG_INIT;
```
当标志对象已初始化，那么只能做三件事情：销毁，清除或设置(查询之前的值)。对应成员函数: clear()、test_and_set()。
> clear()是一个存储操作,所以不能有memory_order_acquire或memory_order_acq_rel语义
> test_and_set()是一个“读-改-写”操作，可以应用于任何memory order。
> 其默认memory order都是memory_order_seq_cst。

```c++
std::atomic_flag f = ATOMIC_FLAG_INIT;

// 使用release语义
f.clear(std::memory_order_release); // 1

// 使用默认语义(std::memory_order_seq_cst)
// x为f的旧值
bool x = f.test_and_set(); //2
```

> [!important]
> 不能拷贝构造另一个 std::atomic_flag 对象；并且，不能将一个对象赋予另一个 std::atomic_flag 对象。这不是 std::atomic_flag 特有的，而是所有原子类型共有的。一个原子类型的所有操作都是原子的，因赋值和拷贝调用了两个对象，这就就破坏了操作的原子性。这样的话，拷贝构造和拷贝赋值都会将第一个对象的值进行读取，然后再写入另外一个。对于两个独立的对象，这里就有两个独立的操作了，合并这两个操作必定是不原子的。因此，操作就不被允许。

### 基于std::atomic_flag的自旋锁
```c++
class spinlock_mutex {
  std::atomic_flag flag;

 public:
  spinlock_mutex() : flag{ATOMIC_FLAG_INIT} {}

  void lock() {
    while (flag.test_and_set(std::memory_order_acquire));
  }

  void unlock() {
    flag.clear(std::memory_order_release);
  }
};
```