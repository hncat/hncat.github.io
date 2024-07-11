---
title: 第五章 动态链接
date: 2024-07-08
tags:
    - 计算机基础
    - c/c++
categories: 链接、装载与库
isOriginal: true
order: 5
dir:
    order: 5
---
## 1. 什么是动态链接
> [!important]
> 把链接的过程推迟到运行时进行，这就是动态链接(Dynamic Linking)的基本思想。
> 需要注意的是，程序与动态库之间真正的链接工作是由动态链接器完成的，而不静态链接器ld完成。

## 2. 简单的动态链接例子
```c
// Program1.c
#include "Lib.h"

int main() {
  foobar(1);
  return 0;
}

// Program2.c
#include "Lib.h"

int main() {
  foobar(2);
  return 0;
}

// Lib.c
#include <stdio.h>

void foobar(int i) {
  printf("Printing from Lib.so %d\n", i);
}

// Lib.h
#ifndef __LIB_H__
#define __LIB_H__

void foobar(int i);

#endif
```
使用GCC将Lib.c编译成一个共享对象文件:
```bash
# shared 表示产生共享对象
# -fPIC 表示生成位置无关代码
$> gcc -fPIC -shared -o Lib.so Lib.c
# 分别编译链接Program1和Program2
$> gcc -o Program1 Program1.c ./Lib.so
$> gcc -o Program2 Program2.c ./Lib.so
```

![动态链接过程](/image/linker/chapter05/动态链接过程.png)

> [!important]
> Lib.c被链接成Lib.so共享对象文件，Program1.c被编译成Program1.o之后，链接成为可指向文件Program1。但是和静态链接不同的是，Program1.o被链接成可执行文件这一步。在静态链接中，链接过程会把Program1.o和Lib.o链接到一起，并且输出可执行文件Program1。但是对于动态链接，Lib.o没有被链接起来，链接的输入目标只有Program1.o。但是命令行中又发现Lib.so也参与了链接过程？
> 当程序模块Program1.c被编译成Program1.o时，编译器还不知道foobar()函数的地址。当链接器将Program1.o链接成可执行文件时，这时候链接器必须确定foobar()函数的性质。如果foobar()函数是一个定义于==静态目标模块==中的函数，那么链接器将会按照静态链接的规则，==将Program1.o中的foobar()地址引用重定位==。如果foobar()是一个定义在==动态共享对象==中的函数，那么链接器就会将这个符号的引用标记为一个动态链接的符号，==不对其进行地址重定位，把这个过程保留到装载时进行==。
> 对于链接器是如何知道foobar()函数的引用是一个静态符号还是动态符号？这其实就是我们要用到Lib.so的原因。==Lib.so保留了完整的符号信息（因为运行时进行动态链接还需要使用符号信息）==，把Lib.so也作为链接的输入文件之一，==链接器在解析符号时就可以知道：foobar()函数是一个定义在Lib.so的动态符号==。这样链接器就可以堆foobar()函数的引用进行特殊处理，使它成为一个对动态符号的引用。

**动态链接程序运行时地址空间分布**
> [!note]
> 对于静态链接的可执行文件，整个进程只有可执行文件需要被映射。但是对于动态链接来说，除了可执行文件之外，还有它所依赖的共享目标文件。

对Lib.c的foobar()函数进行修改
```c
// Lib.c
#include <stdio.h>
#include <unistd.h>

void foobar(int i) {
  printf("Printing from Lib.so %d\n", i);
  sleep(-1);
}
```
```bash
$> ./Program1 &
[1] 10370
Printing from Lib.so 1
$> cat /proc/10370/maps
5611c3c79000-5611c3c7a000 r--p 00000000 08:10 117001                     /home/far/worker/linker/chapter04/SimpleDynamicalLinking/Program1
5611c3c7a000-5611c3c7b000 r-xp 00001000 08:10 117001                     /home/far/worker/linker/chapter04/SimpleDynamicalLinking/Program1
5611c3c7b000-5611c3c7c000 r--p 00002000 08:10 117001                     /home/far/worker/linker/chapter04/SimpleDynamicalLinking/Program1
5611c3c7c000-5611c3c7d000 r--p 00002000 08:10 117001                     /home/far/worker/linker/chapter04/SimpleDynamicalLinking/Program1
5611c3c7d000-5611c3c7e000 rw-p 00003000 08:10 117001                     /home/far/worker/linker/chapter04/SimpleDynamicalLinking/Program1
5611c5450000-5611c5471000 rw-p 00000000 00:00 0                          [heap]
7f066eb09000-7f066eb0c000 rw-p 00000000 00:00 0
7f066eb0c000-7f066eb34000 r--p 00000000 08:10 49449                      /usr/lib/x86_64-linux-gnu/libc.so.6
7f066eb34000-7f066ecbc000 r-xp 00028000 08:10 49449                      /usr/lib/x86_64-linux-gnu/libc.so.6
7f066ecbc000-7f066ed0b000 r--p 001b0000 08:10 49449                      /usr/lib/x86_64-linux-gnu/libc.so.6
7f066ed0b000-7f066ed0f000 r--p 001fe000 08:10 49449                      /usr/lib/x86_64-linux-gnu/libc.so.6
7f066ed0f000-7f066ed11000 rw-p 00202000 08:10 49449                      /usr/lib/x86_64-linux-gnu/libc.so.6
7f066ed11000-7f066ed1e000 rw-p 00000000 00:00 0
7f066ed25000-7f066ed26000 r--p 00000000 08:10 116994                     /home/far/worker/linker/chapter04/SimpleDynamicalLinking/Lib.so
7f066ed26000-7f066ed27000 r-xp 00001000 08:10 116994                     /home/far/worker/linker/chapter04/SimpleDynamicalLinking/Lib.so
7f066ed27000-7f066ed28000 r--p 00002000 08:10 116994                     /home/far/worker/linker/chapter04/SimpleDynamicalLinking/Lib.so
7f066ed28000-7f066ed29000 r--p 00002000 08:10 116994                     /home/far/worker/linker/chapter04/SimpleDynamicalLinking/Lib.so
7f066ed29000-7f066ed2a000 rw-p 00003000 08:10 116994                     /home/far/worker/linker/chapter04/SimpleDynamicalLinking/Lib.so
7f066ed2a000-7f066ed2c000 rw-p 00000000 00:00 0
7f066ed2c000-7f066ed2d000 r--p 00000000 08:10 49446                      /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
7f066ed2d000-7f066ed58000 r-xp 00001000 08:10 49446                      /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
7f066ed58000-7f066ed62000 r--p 0002c000 08:10 49446                      /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
7f066ed62000-7f066ed64000 r--p 00036000 08:10 49446                      /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
7f066ed64000-7f066ed66000 rw-p 00038000 08:10 49446                      /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
7ffe6cfa3000-7ffe6cfc4000 rw-p 00000000 00:00 0                          [stack]
7ffe6cfe6000-7ffe6cfea000 r--p 00000000 00:00 0                          [vvar]
7ffe6cfea000-7ffe6cfeb000 r-xp 00000000 00:00 0                          [vdso]
$> kill 10370
[1]  + 10370 terminated  ./Program1
```
观察到整个虚拟地址空间中，多出了几个文件的映射。Lib.so与Program1一样，它们都是被操作系统用同样的方法映射至进程的虚拟地址空间。Program1除了使用Lib.so外，还是用到了动态链接形式的==C语言运行时库(/usr/lib/x86_64-linux-gnu/libc.so.6)==。还有Linux下的==动态链接器(/usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2)==。动态链接器和普通共享对象一样被映射到进程的地址空间，在系统开始运行Program1之前，首先将控制权交给动态链接器，由它完成所有的动态链接工作后再把控制权交给Program1，最后开始执行。

通过readelf查看Lib.so的装载属性:
```bash
$> readelf -l Lib.so

Elf file type is DYN (Shared object file)
Entry point 0x0
There are 11 program headers, starting at offset 64

Program Headers:
  Type           Offset             VirtAddr           PhysAddr
                 FileSiz            MemSiz              Flags  Align
  LOAD           0x0000000000000000 0x0000000000000000 0x0000000000000000
                 0x0000000000000560 0x0000000000000560  R      0x1000
  LOAD           0x0000000000001000 0x0000000000001000 0x0000000000001000
                 0x000000000000017d 0x000000000000017d  R E    0x1000
  LOAD           0x0000000000002000 0x0000000000002000 0x0000000000002000
                 0x00000000000000dc 0x00000000000000dc  R      0x1000
  LOAD           0x0000000000002df8 0x0000000000003df8 0x0000000000003df8
                 0x0000000000000220 0x0000000000000228  RW     0x1000
  DYNAMIC        0x0000000000002e08 0x0000000000003e08 0x0000000000003e08
                 0x00000000000001c0 0x00000000000001c0  RW     0x8
  NOTE           0x00000000000002a8 0x00000000000002a8 0x00000000000002a8
                 0x0000000000000020 0x0000000000000020  R      0x8
  NOTE           0x00000000000002c8 0x00000000000002c8 0x00000000000002c8
                 0x0000000000000024 0x0000000000000024  R      0x4
  GNU_PROPERTY   0x00000000000002a8 0x00000000000002a8 0x00000000000002a8
                 0x0000000000000020 0x0000000000000020  R      0x8
  GNU_EH_FRAME   0x000000000000201c 0x000000000000201c 0x000000000000201c
                 0x000000000000002c 0x000000000000002c  R      0x4
  GNU_STACK      0x0000000000000000 0x0000000000000000 0x0000000000000000
                 0x0000000000000000 0x0000000000000000  RW     0x10
  GNU_RELRO      0x0000000000002df8 0x0000000000003df8 0x0000000000003df8
                 0x0000000000000208 0x0000000000000208  R      0x1

 Section to Segment mapping:
  Segment Sections...
   00     .note.gnu.property .note.gnu.build-id .gnu.hash .dynsym .dynstr .gnu.version .gnu.version_r .rela.dyn .rela.plt
   01     .init .plt .plt.got .plt.sec .text .fini
   02     .rodata .eh_frame_hdr .eh_frame
   03     .init_array .fini_array .dynamic .got .got.plt .data .bss
   04     .dynamic
   05     .note.gnu.property
   06     .note.gnu.build-id
   07     .note.gnu.property
   08     .eh_frame_hdr
   09
   10     .init_array .fini_array .dynamic .got
```

> [!note]
> 除了文件类型与普通程序不同之外，其它几乎与普通程序一样。还有一点不同的是，动态链接模块的装载地址是从地址0x0000000000000000开始的。我们知道这个地址是无效地址，并且从上面的进程虚拟空间分布看到，Lib.so的最终装载地址并不是0x0000000000000000，而是0x7f066ed25000。从这一点可以推断，==共享对象的最终装载地址在编译时是不确定的==，而是在装载时，装载器根据当前地址空间的空闲情况，动态的分配一块足够大小的地址空间给相应的共享对象。

## 3. 地址无关代码
### 3.1 固定装载地址的困扰
> [!note]
> 为了实现动态链接，我们首先会遇到的问题是共享对象地址冲突问题。在动态链接的情况下，如果不同的模块目标装载地址都一样是不行的。而对于单个程序来说，我们可以手工指定各个模块的地址，比如0x1000到0x2000分配给模块A，把地址0x2000到0x3000分配给模块B。但是，如果某个模块被多个程序使用，或者多个模块被多个程序使用呢？这将会使得管理这些模块的地址将是一件无比繁琐的事情。
> 例如程序program1使用了模块B，但是没有使用模块A，所以他认为地址0x1000到0x2000的地址是空闲的，余数分配给了另一个模块C。这样C和原先的模块A的目标地址就冲突了，任何人以后将不能在同一个程序里面使用模块A和C。这种做法就叫做==静态共享库(Static Shared Library)==。相比于静态链接，静态共享库的做法就是将程序的各个模块统一交给操作系统管理，操作系统在某个特定的地址划分出一些地址块，为那些已知的模块预留足够的空间。
> 静态共享库的目标地址导致了很多问题，除了地址冲突外，静态共享库的升级也是问题，因为升级后的共享库必须保持共享库中全局函数和变量地址的不变，如果应用程序在链接时已经绑定了这些地址，一旦变更，就必须重新链接应用程序，否则就会引起应用程序崩溃。即使升级静态共享库后保持原有的函数和变量地址不变，只是增加了一些全局函数或变量，也会受到限制，因为静态共享库被分配到的虚拟地址空间是有限的，不能增长太多，否则就会超出被分配的空间。
> 为了解决这个模块装载地址固定的问题，我们设想是否可以让共享对象在==任意地址==加载？既==共享对象在编译时不能假设自己在进程虚拟地址空间中的位置==。对于可执行文件基本可以确定自己在进程虚拟空间中的起始位置，因为可执行文件往往是第一个被加载的文件。

### 3.2 装载时重定位
> [!important]
> 在链接时，对所有绝对地址的引用不做重定位，而把这一步推迟到装载时完成。一旦模块装载地址确定，既目标地址确定，那么系统就对程序中所有的绝对地址引用进行重定位。
> 静态链接时的重定位被称为==链接时重定位(Link Time Relocation)==，而现在这种情况通常被称为==装载时重定位(Load Time Relocation)==。
> 装载时重定位的问题：动态链接模块被装载至虚拟空间后，指令部分是在多个进程之间共享的，由于装载时重定位的方法需要修改指令，所以没有办法做到同一份指令被多个进程共享，==因为指令部分对于每个进程来说可能会被映射到每个进程的不同虚拟空间，从而导致这些地址对每个进程来说是不同的==。而对于可修改数据部分来说每个进程都有不同的副本，所以可以使用装载时重定位的方法来处理。
> Linux的GCC编译器支持这种装载时重定位的方法，在编译时可以只使用-shared选项而不使用-fPIC选项。

### 3.3 地址无关代码
> [!note]
> 装载时重定位是解决动态模块中没有绝对地址应用的办法之一，但是有一个很大的缺点是指令部分无法在多个进程之间共享，这样就失去了动态链接节省内存的一大优势。
> 现在我们希望程序模块中共享的指令部分在装载时不需要因为装载地址的改变而改变，所以我们只需要将指令中那些需要被修改的部分进行剥离，和数据部分放在一起，这样指令部分就可以保持不变，而数据部分可以在每个进程中拥有一个副本。这种方案就是==地址无关代码（PIC, Position-independent Code）==技术。

将共享对象中的地址引用按照是否跨模块分为: ==模块内部引用和模块外部引用==。

按照不同的引用方式又分为：==指令引用和数据访问==。

因此有如下方式:
- 模块内部的函数调用、跳转等。
- 模块内部的数据访问，比如模块中定义的全局变量、静态变量。
- 模块外部的函数调用、跳转等。
- 模块外部的数据访问，比如其它模块中定义的全局变量。
```c
static int a;
extern int b;
extern void ext();

void bar() {
  a = 1; // 模块内部数据访问
  b= 2;  // 模块外部数据访问
}

void foo() {
  bar(); // 模块内部函数调用
  ext(); // 模块外部函数调用
}
```

```asm
0000000000001139 <bar>:
    1139:       f3 0f 1e fa             endbr64
    113d:       c7 05 ed 2e 00 00 01    movl   $0x1,0x2eed(%rip)        # 4034 <a>
    1144:       00 00 00
    1147:       48 8b 05 8a 2e 00 00    mov    0x2e8a(%rip),%rax        # 3fd8 <b>
    114e:       c7 00 02 00 00 00       movl   $0x2,(%rax)
    1154:       c3                      ret

0000000000001155 <foo>:
    1155:       f3 0f 1e fa             endbr64
    1159:       48 83 ec 08             sub    $0x8,%rsp
    115d:       b8 00 00 00 00          mov    $0x0,%eax
    1162:       e8 09 ff ff ff          call   1070 <bar@plt>
    1167:       b8 00 00 00 00          mov    $0x0,%eax
    116c:       e8 ef fe ff ff          call   1060 <ext@plt>
    1171:       48 83 c4 08             add    $0x8,%rsp
    1175:       c3                      ret
```

**类型一 模块内部调用或跳转**
> [!note]
> 根据objdump反汇编后得到的结果和书中列出的不太一样。实际汇编得到的结果是使用了一种叫==共享对象全局符号介入(Global Symbol Interposition)==的东西。在动态链接的实现中会提到。

这种情况是最简单的，因为被调用的函数与调用者都在同一个模块，所以它们之间的相对位置是固定的，所以这种情况会比较简单。对于现代系统来说，模块内部的跳转、函数调用都可以是相对地址调用，或者是基于寄存器的相对调用，所以对于这种指令是不需要重定位的。

**类型二 模块内部数据访问**

## 4. 延迟般的(PLT)
## 5. 动态链接相关结构
## 6. 动态链接的步骤和实现
## 7. 显示运行时链接
