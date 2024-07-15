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
> 现在我们希望程序模块中共享的指令部分在装载时不需要因为装载地址的改变而改变，所以我们只需要将指令中那些需要被修改的部分进行剥离，和数据部分放在一起，这样指令部分就可以保持不变，而数据部分可以在每个进程中拥有一个副本。这种方案就是==地址无关代码(PIC, Position-Independent Code)==。

将共享对象中的地址引用按照是否跨模块分为: ==模块内部引用和模块外部引用==。

按照不同的引用方式又分为：==指令引用和数据访问==。

因此有如下方式:
- 模块内部的函数调用、跳转等。
- 模块内部的数据访问，比如模块中定义的全局变量、静态变量。
- 模块外部的函数调用、跳转等。
- 模块外部的数据访问，比如其它模块中定义的全局变量。

1. bar为非静态函数时
```c
// pic.c
static int a;
extern int b;
extern void ext();

void bar() {
  a = 1; // 模块内部数据访问
  b = 2;  // 模块外部数据访问
}

void foo() {
  bar(); // 模块内部函数调用
  ext(); // 模块外部函数调用
}
```
```bash
$> gcc -shared -fPIC pic.c -o libpic.so
```
```asm
Disassembly of section .plt.sec:

0000000000001060 <ext@plt>:
    1060:       f3 0f 1e fa             endbr64
    1064:       ff 25 96 2f 00 00       jmp    *0x2f96(%rip)        # 4000 <ext>
    106a:       66 0f 1f 44 00 00       nopw   0x0(%rax,%rax,1)

0000000000001070 <bar@plt>:
    1070:       f3 0f 1e fa             endbr64
    1074:       ff 25 8e 2f 00 00       jmp    *0x2f8e(%rip)        # 4008 <bar+0x2ecf>
    107a:       66 0f 1f 44 00 00       nopw   0x0(%rax,%rax,1)

0000000000001139 <bar>:
    1139:       f3 0f 1e fa             endbr64
    113d:       c7 05 d5 2e 00 00 01    movl   $0x1,0x2ed5(%rip)        # 401c <a>
    1144:       00 00 00
    1147:       48 8b 05 72 2e 00 00    mov    0x2e72(%rip),%rax        # 3fc0 <b>
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
2. bar静态函数时
```c
static int a;
extern int b;
extern void ext();

static void bar() {
  a = 1; // 模块内部数据访问
  b = 2;  // 模块外部数据访问
}

void foo() {
  bar(); // 模块内部函数调用
  ext(); // 模块外部函数调用
}
```
```bash
$> gcc -shared -fPIC pic.c -o libpic.so
```
```asm
0000000000001119 <bar>:
    1119:       c7 05 f1 2e 00 00 01    movl   $0x1,0x2ef1(%rip)        # 4014 <a>
    1120:       00 00 00
    1123:       48 8b 05 96 2e 00 00    mov    0x2e96(%rip),%rax        # 3fc0 <b>
    112a:       c7 00 02 00 00 00       movl   $0x2,(%rax)
    1130:       c3                      ret

0000000000001131 <foo>:
    1131:       f3 0f 1e fa             endbr64
    1135:       48 83 ec 08             sub    $0x8,%rsp
    1139:       b8 00 00 00 00          mov    $0x0,%eax
    113e:       e8 d6 ff ff ff          call   1119 <bar>
    1143:       b8 00 00 00 00          mov    $0x0,%eax
    1148:       e8 03 ff ff ff          call   1050 <ext@plt>
    114d:       48 83 c4 08             add    $0x8,%rsp
    1151:       c3                      ret
```

**类型一 模块内部调用或跳转**
> [!note]
> 根据objdump反汇编后得到的结果和书中列出的不太一样。实际汇编得到的结果是使用了==共享对象全局符号介入(Global Symbol Interposition)==。在动态链接的实现中会提到。
> 但是当你讲bar函数修改为静态成员函数时就是书中想要的结果(或许是书中的纰漏吧，作为补充我在这里给出了书中想要的效果)。

这种情况是最简单的，因为被调用的函数与调用者都在同一个模块，所以它们之间的相对位置是固定的，所以这种情况会比较简单。对于现代系统来说，模块内部的跳转、函数调用都可以是相对地址调用，或者是基于寄存器的相对调用，所以对于这种指令是不需要重定位的。

**类型二 模块内部数据访问**
任何一条指令与它需要访问的模块内部数据之间的相对位置是固定的，那么只需要相对于当前指令(PC值)加上固定的偏移量就可以访问模块内部数据了。例如函数bar中的一段反汇编代码:
```asm
# %rip既是对应的PC值(当前执行的下一条指令)
113d:       c7 05 d5 2e 00 00 01    movl   $0x1,0x2ed5(%rip)        # 401c <a>
1144:       00 00 00 # a内存地址进行了一次内存对齐，所以需要补0所以%rip实际值为0x1147
1147:       48 8b 05 72 2e 00 00    mov    0x2e72(%rip),%rax        # 3fc0 <b>
```
假设该模块经过加载后位于虚拟内存的0x10000000处，那变量a所在的虚拟内存地址为: 0x2ed5 + 0x1147 + 0x10000000 = 0x1000401c。
而通过查看符号表信息可以得到变量a所在elf文件中的位置:
```bash
$> readelf -s libpic.so
# 只显示了部分数据
...
ymbol table '.symtab' contains 29 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
    ...
    10: 000000000000401c     4 OBJECT  LOCAL  DEFAULT   22 a
    ...
...
```

**类型三 模块间数据访问**
要使得代码地址无关，基本的思想是把和地址相关的部分放到数据段里面，很明显，这些其它模块的全局变量的地址是和模块装载地址相关的。elf的做法是在数据段里面创建一个==指向这些变量的指针数组==，也被称为==全局偏移表(Global Offset Table, GOT)==，当代码需要引用该变量时，可以通过GOT中相对应的项间接引用。如图:

![模块间数据访问](/image/linker/chapter05/模块间数据访问.png)

当指令需要访问变量b时，程序会先找到GOT，然后根据GOT中变量所对应的项找到变量的目标地址。链接器在装载模块的时候会查找每个变量所在的地址，然后填充GOT中的各个项，以确保每个指针所指向的地址正确。==由于GOT本身是放在数据段的，所以它可以在模块装载时被修改，并且每个进程都可以有独立的副本，相互不受影响==。

> [!important]
> GOT是如何实现指令的地址无关性的。模块在编译时可以确定模块内部变量相对于当前指令的偏移，那么我们也可以在编译时确定GOT相对于当前指令的偏移。确定GOT的位置和上面的访问变量a的方法基本一样，通过得到PC值然后加上偏移量，就可以得到GOT的位置。然后我们根据变量地址在GOT中的偏移量就可以得到变量的地址，当然GOT中每个地址对应于哪个变量是由编译器决定的，比如第一个地址对应变量b，第二个变量对应变量c等。

假设共享库被加载到0x100000000的位置，观察函数bar()的反汇编代码。为访问变量b，程序首先计算出变量b的地址在GOT中的位置，即0x10000000 + 0x114e + 0x2e72 = 0x10003fc0

查看GOT位置
```bash
$> readelf -S libpic.so
There are 27 section headers, starting at offset 0x3548:

Section Headers:
  [Nr] Name              Type             Address           Offset
       Size              EntSize          Flags  Link  Info  Align
  ...
  [19] .got              PROGBITS         0000000000003fc0  00002fc0
       0000000000000028  0000000000000008  WA       0     0     8
  ...
```
GOT在文件中的偏移为0x3fc0。查看动态链接时重定位项。
```bash
$> readelf -r libpic.so

Relocation section '.rela.dyn' at offset 0x458 contains 8 entries:
  Offset          Info           Type           Sym. Value    Sym. Name + Addend
  ...
  000000003fc0  000200000006 R_X86_64_GLOB_DAT 0000000000000000 b + 0
  ...
```
观察到变量b的地址需要重定位，它位于0x3fc0，也就是GOT中偏移0，相当于是GOT中的第一项（每8字节一项）正好对应通过指令计算出的偏移量0x114e + 0x2e72 = 0x3fc0。

**类型四 模块间调用、跳转**
对于模块间调用和跳转，同样可以采用类型三的方法来解决。但是不同的是，GOT中相应的项保存的是目标函数的地址，当模块需要调用目标函数时，可以通过GOT中的项进行间接跳转基本原理如下图(这是原书可出的可能会和实际不同，但是不影响我们进行分析):

![模块间调用、跳转](/image/linker/chapter05/模块间调用、跳转.png)

```asm
0000000000001060 <ext@plt>:
    1060:       f3 0f 1e fa             endbr64
    1064:       ff 25 96 2f 00 00       jmp    *0x2f96(%rip)        # 4000 <ext>
    106a:       66 0f 1f 44 00 00       nopw   0x0(%rax,%rax,1)

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

观察到调用ext()函数时跳转到了"ext@plt"调用，而该函数内部实现依旧是跳转到GOT表中通过间接跳转实现对函数的调用(0x1064代码处调用)。
> [!note]
> 通过实际的分析发现函数的GOT表并不在.got段中而是在一个名为.got.plt段中(延迟绑定)这是对模块间调用、跳转的一种优化。

**-fpic和-fPIC**
从功能上来说完全一样，都是生成地址无关代码，“-fPIC”生成的代码要大，而“f-pic”生成的代码要小，而且较快。但是由于地址无关代码都是和硬件平台相关的，不同的平台有着不同的实现，“-fpic”在某些平台上会有一些限制，比如全局符号的数量或者代码的长度等，而“-fPIC”则没有这样的限制。所以为了方便，绝大多数情况下都使用”-fPIC“。

**PIC和PIE**
一个以地址无关方式编译的可执行文件被称为==地址无关可执行文件(PIE, Position-Independent Executable)==。对应GCC选项"-fpie"或"-fPIE"。

### 3.4 共享模块的全局变量问题
当一个模块引用了一个定义在共享对象的全局变量的时比如一个共享对象定义了一个全局变量global，而模块module.c是这样引用的：
```c
extern int global;
int foo() {
  global = 1;
}
```
当编译器编译moudule.c时，无法根据这个上下文判断global是定义在同一个模块的其它目标文件还是定义在另一个模块当中，既无法判断是否为跨模块间的调用。

1. 假设module.c是程序可执行文件的一部分，那么这种情况下，由于程序主模块的代码并不是地址无关代码，也就是说代码不会使用这种PIC机制，它引用这个全局变量的方式和普通数据访问方式一样，编译器会产生这样的代码：
```asm
movq $0x1, XXXXXXXXXXXXXXXX # XXXXXXXXXXXXXXXX为global的地址。
```
由于可执行文件在运行时并不进行代码重定位，所以变量的地址必须在链接过程中确定下来。为了能够使得链接过程正常进行，链接器会在创建可执行文件时，==在它的".bss"段创建一个global变量的副本==。那么现在global变量定义在原先的共享对象中，而在可执行文件的".bss"段还有一个副本。如果同一个变量同时存在于多个位置中，这在程序实际运行过程中肯定是不可行的。

如何解决？==那就是所有的使用这个变量的指令都指向位于可执行文件中的那个副本==。ELF共享库在编译时，==默认都把定义在模块内部的全局变量当作定义在其它模块的全局变量==，也就是==类型四 模块间变量访问，通过GOT来实现变量访问==。当共享模块被装载时，==如果某个全局变量在可执行文件中拥有副本，那么动态链接器就会把GOT中的相应地址指向该副本，这样该变量在运行时实际上最终只有一个实例==。如果变量在共享模块中被初始化，那么==动态链接器还需要将该初始化值复制到程序主模块中的变量副本==；如果该全局变量在程序主模块中没有副本，那么==GOT中相应地址就指向模块内部的该变量副本==。

2. 如果module.c是共享对象的一部分，那么GCC编译器在-fPIC的情况下，就会把对global的调用按照跨模块模式产生代码。因为编译器无法确定对global的引用是跨模块的还是模块内部的。即使是模块内部的，还是会产生跨模块代码，因为global可能被可执行文件引用，从而使得共享模块中对global的引用要执行可执行文件中的globa副本。

### 3.5 数据段地址无关性
```c
static int a;
static int *p = &a;
```
如果某个共享对象里有这样一段代码，那么p的地址就是一个绝对地址，它指向变量a，而变量a的地址会随着共享对象的装载地址改变而改变。

对于数据段来说，它在每个进程都有一份独立的副本，所以并不担心被进程改变。从这点来看，我们可以选择装载时重定位的方法来解决数据段中绝对地址引用问题。对于共享对象来说，如果数据段中有绝对地址引用，那么编译器和链接器就会产生一个重定位表，这个重定位表里面里面包含了“R_X86_64_GLOB_DAT"类型的重定位入口，用于解决上述问题。当动态链接器装载共享对象时，如果发现该共享对象有这样的重定位入口，那么动态链接器就会对该共享对象进行重定位。

## 4. 延迟绑定(PLT)
动态链接库比静态链接慢的主要原因：
- 动态链接下对于全局和静态的数据访问都要进行复杂的GOT定位，然后间接寻址，对于模块间的调用也要先定位GOT，然后再进行间接跳转。
- 动态链接的链接工作在运行时完成，即程序开始执行时，动态链接器都要进行一次链接工作，动态链接器会寻找斌装载所需要的共享对象，然后进行符号查找地址重定位等工作。

**延迟绑定实现**
在动态链接下，程序模块之间包含了大量的函数引用(全局变量往往比较少，因为大量的全局变量会导致模块间耦合度变大)，所以在程序开始执行前，动态链接会耗费不少时间用于解决模块之间的函数引用的符号查找和重定位。不过可以想象，在一个程序运行过程中，可能很多函数在程序执行完时都不会被用到，比如一些错误处理函数或者是一些用户很少用到的功能模块等，如果一开始就把所有函数都链接好实际上是一种浪费。所以ELF采用了一种叫 ==延迟绑定(Lazy Binding)== 的做法,其基本思想是当==函数第一次被用到时才进行绑定(符号查找、重定位等)==，如果没有用到则不进行绑定。所以程序开始执行时，模块间的函数调用都没有进行绑定，而是需要用到时才由动态链接器来负责绑定。

ELF使用 ==PLT(Procedure Linkage Table)== 的方法来实现，这种方法使用了一些精巧的指令序列来完成。
> [!note]
> 假设liba.so需要调用libc.so中的bar()函数，那么当liba.so中第一次调用bar()时，这时候就需要调用动态链接器中的某个函数来完成地址绑定工作，我们假设这个函数叫做lookup()，那么lookup()需要知道一些必要信息才能完成这个函数地址绑定的工作:
> - 地址绑定发生在哪个模块，哪个函数。
> 假设lookup的原型为lookup(module, function)，这两个参数的值在我们的列子中分别为liba.so和bar()。在Glibc中，我们这里的lookup()函数正在的名字叫做_dl_runtime_resolve()。

当我们调用某个外部模块的函数时，如果按照通常的做法应该时通过GOT中相应的项进行间接跳转。PLT为了实现延迟绑定，在这个过程中间又增加了一层间接跳转。调用函数并不直接通过GOT跳转，而是通过一个叫做PLT项的结构来进行跳转。每个外部函数在PLT中都有一个相应的项，比如bar()函数在PLT中的项我们称之为bar@plt。

```asm
0000000000001070 <bar@plt>:
    1070:       f3 0f 1e fa             endbr64
    1074:       ff 25 8e 2f 00 00       jmp    *0x2f8e(%rip)        # 4008 <bar+0x2ecf>
    107a:       66 0f 1f 44 00 00       nopw   0x0(%rax,%rax,1)
```

> [!note]
> - 过程链接表(PLT)是一个数组，其中每个条目时16字节代码。PLT[0]时一个特殊条目，它跳转到动态链接库中。每个被可执行程序调用的库函数都有它自己的PLT条目。每个条目都负责调用一个具体的函数。PLT[1]调用系统启动函数(__libc_start_main)，它初始化执行环境。从PLT[2]开始的条目调用用户代码调用的函数。
> - 全局偏移表(GOT)是一个数组，其中每个条目时8字节地址。和PLT联合使用时，GOT[0]和GOT[1]包含动态链接器在解析函数地址时会使用的信息。GOT[2]是动态链接器在ld-linux.so模块的入口点。其余的每个条目对应于一个被调用的函数，其地址需要在运行是被解析。每个条目都有一个相匹配的PLT条目。初始时每个GOT条目都指向对应PLT条目的第二条指令。
> (参考csapp 7.12小节)

```c
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

int x[2] = {1, 2};
int y[2] = {3, 4};
int z[2];

int main() {
  void *handle;
  void (*addvec) (int *, int *, int *, int);
  char *error;

  handle = dlopen("./libvector.so", RTLD_LAZY);
  if (!handle) {
    fprintf(stderr, "%s \n", dlerror());
    exit(1);
  }

  addvec = dlsym(handle, "addvec");
  if ((error = dlerror()) != NULL) {
    fprintf(stderr, "%s\n", error);
    exit(1);
  }

  addvec(x, y, z, 2);
  printf("z = [%d %d]\n", z[0], z[1]);

  if (dlclose(handle) < 0) {
    fprintf(stderr, "%s\n", dlerror());
    exit(1);
  }

  return 0;
}
```

![用PLT和GOT调用外部函数。在第一次调用addvec时，动态链接器解析它的地址](/image/linker/chapter05/got&plt.png)

如何使用GOT和PLT协同工作:
- addvec第一次被调用时，延迟解析它的运行时地址：
  1. 不直接调用addvec，程序调用进入PLT[2]，这是addvec的PLT条目
  2. 第一条PLT指令通过GOT[4]进行间接跳转。因为每个GOT条目初始化时都指向它对应的PLT条目的第二条指令，这个间接跳转只是简单的把控制传送回PLT[2]中的下一条指令。
  3. 在把addvec的ID（0x1）压入栈中之后，PLT[2]跳转到PLT[0]。
  4. PLT[0]通过GOT[1]间接的把动态链接器的一个参数压入栈中，然后通过GOT[2]间接跳转到动态链接器中。动态链接器使用两个栈条目来确定addvec的运行时位置，用这个地址重新GOT[4]，再把控制权传递给addvec。
- 后续再调用addvec时的控制流：
  1. 控制传递到PLT[2]。
  2. 通过GOT[4]的间接跳转会将控制直接转移到addvec。

PLT在ELF文件中以独立的段存放，段名叫".plt"。因为本身是一些地址无关代码，所以可以和代码段等一起合并成同一个可读可指向的“segment”被装载入内存。

## 5. 动态链接相关结构
动态链接情况下，可执行文件的装载与静态链接情况基本一样。首先操作系统会读取可执行文件的头部，检查文件的合法性，然后从头部中的 =="Program Header"== 中读取每个 =="Segment"== 的虚拟地址、文件地址和属性，并将它们映射到进程虚拟空间的相应位置，这些步骤和静态链接情况下的装载基本无异。在静态链接情况下，操作系统接着就可以把控制权交给可执行文件的入口地址，然后程序开始执行。

可是在动态链接情况下，操作系统还不能在装载完可执行文件之后就把控制权交给可执行文件，因为可执行文件==依赖于很多共享对象==。这时，可执行文件对于很多 ==外部符号的引用还处于无效地址的状态==，既还没有和相应的==共享对象中的实际位置链接起来==。所以在映射完可执行文件后，操作系统会先启动一个==动态链接器（Dynamic Linker）==。

在Linux下，==动态链接器ld.so实际上是一个共享对象==，操作系统统一通过映射的方式将它加载到进程的地址空间中。操作系统在加载完动态链接器后，将控制权交给==动态链接器的入口地址（与可执行文件一样，共享对象也有入口地址）==。当动态链接器获得控制权后，便开始一系列自身的初始化操作，然后根据当前的环境参数，开始对可执行文件进行动态链接工作。当所有动态链接工作完成后，动态链接器会将控制权交给可执行文件的入口地址，程序开始正式执行。

### 5.1 ".interp"段
> [!note]
> 动态链接去的位置既不是由系统配置指定，也不是由环境参数决定，而是由ELF可执行文件决定。在动态链接的ELF可执行文件中，有一个专门的段叫做==".interp"段("interp"是"interpreter"(解释器)的缩写)==。
> ".interp"里面保持的就是一个字符串，这个字符串就是可执行文件所需要的动态链接器的路径。在Linux中，操作系统在对可执行文件==进行加载时==，会去寻址装载该可执行文件所需要相应的动态链接器，既".interp"段所指定的路径的共享对象。

使用objdump工具查看"interp"内容：
```c
// test.c
#include <stdio.h>

int main() {
  printf("hello world!\n");
  return 0;
}
// gcc test.c -o test
```
```bash
$> objdump -s test

test:     file format elf64-x86-64

Contents of section .interp:
 0318 2f6c6962 36342f6c 642d6c69 6e75782d  /lib64/ld-linux-
  0328 7838362d 36342e73 6f2e3200           x86-64.so.2.
```
也可以使用readelf查看：
```bash
$> readelf -l test|grep .interpreter
      [Requesting program interpreter: /lib64/ld-linux-x86-64.so.2]
```

### 5.2 ".dynamic"段
> [!important]
> 动态链接ELF中最重要的结构".dynamic"段，这个段保存了动态链接所需要的基本信息，==比如依赖于那些共享对象、动态链接符号表的位置、动态链接重定位表的位置、共享对象初始化代码的地址等==。

".dynamic"段的结构：
```c
/* Dynamic section entry.  */

// 32位
typedef struct {
  Elf32_Sword   d_tag;                /* Dynamic entry type */
  union {
    Elf32_Word d_val;                 /* Integer value */
    Elf32_Addr d_ptr;                 /* Address value */
  } d_un;
} Elf32_Dyn;

// 64位
typedef struct {
  Elf64_Sxword  d_tag;                /* Dynamic entry type */
  union {
    Elf64_Xword d_val;                /* Integer value */
    Elf64_Addr d_ptr;                 /* Address value */
  } d_un;
} Elf64_Dyn;
```
|d_tag类型|d_un的含义|
|---|---|
|DT_SYMTAB|动态链接符号表的地址，d_ptr表示".dynsym"的地址|
|DT_STRTAB|动态链接字符串表地址，d_ptr表示".dynstr"的地址|
|DT_STRSZ|动态链接字符串表大小，d_val表示大小|
|DT_HASH|动态链接哈希表地址，d_ptr表示".hash"的地址|
|DT_SONAME|本共享对象的"SO-NAME"|
|DT_RPATH|动态链接共享对象搜索路径|
|DT_INIT|初始化代码地址|
|DT_FINIT|结束代码地址|
|DT_NEED|依赖的共享对象文件，d_ptr表示所依赖的共享对象文件名|
|DT_REL/DT_RELA|动态链接重定位表地址|
|DT_RELENT/DT_RELAENT|动态重定位表入口函数|

使用readelf工具查看".dynaminc"段的内容：
```bash
$> readelf -d test

Dynamic section at offset 0x2dc8 contains 27 entries:
  Tag        Type                         Name/Value
  0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]
  0x000000000000000c (INIT)               0x1000
  0x000000000000000d (FINI)               0x1168
  0x0000000000000019 (INIT_ARRAY)         0x3db8
  0x000000000000001b (INIT_ARRAYSZ)       8 (bytes)
  0x000000000000001a (FINI_ARRAY)         0x3dc0
  0x000000000000001c (FINI_ARRAYSZ)       8 (bytes)
  0x000000006ffffef5 (GNU_HASH)           0x3b0
  0x0000000000000005 (STRTAB)             0x480
  0x0000000000000006 (SYMTAB)             0x3d8
  0x000000000000000a (STRSZ)              141 (bytes)
  0x000000000000000b (SYMENT)             24 (bytes)
  0x0000000000000015 (DEBUG)              0x0
  0x0000000000000003 (PLTGOT)             0x3fb8
  0x0000000000000002 (PLTRELSZ)           24 (bytes)
  0x0000000000000014 (PLTREL)             RELA
  0x0000000000000017 (JMPREL)             0x610
  0x0000000000000007 (RELA)               0x550
  0x0000000000000008 (RELASZ)             192 (bytes)
  0x0000000000000009 (RELAENT)            24 (bytes)
  0x000000000000001e (FLAGS)              BIND_NOW
  0x000000006ffffffb (FLAGS_1)            Flags: NOW PIE
  0x000000006ffffffe (VERNEED)            0x520
  0x000000006fffffff (VERNEEDNUM)         1
  0x000000006ffffff0 (VERSYM)             0x50e
  0x000000006ffffff9 (RELACOUNT)          3
  0x0000000000000000 (NULL)               0x0
```

使用ldd查看一个程序主模块或一个共享库依赖于哪些共享库：
```bash
$> ldd test
        linux-vdso.so.1 (0x00007ffe6d77b000)
        libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f7967314000)
        /lib64/ld-linux-x86-64.so.2 (0x00007f796754d000)
```

### 5.3 动态符号表
```c
// test.c
#include <stdio.h>

int main() {
  printf("hello world!\n");
  return 0;
}
//gcc test.c -o test
```
> [!note]
> 为了完成动态链接，最关键的还是所依赖的符号和相关文件的信息。我们知道在静态链接中，有一个专门的段叫符号表".symtab"(symbol table)，里面保存了所有关于该目标文件的符号的定义和引用。动态链接的符号表示和静态链接十分相似。比如test程序依赖于libc.so.6，引用了里面的printf()函数。那么对于test来说，我们称test ==导入(import)== 了printf()函数，printf()是test的 ==导入函数(import function)== ；而libc.so.6它实际上是定义了printf()函数，并且提供给其它模块使用，我们称libc.so.6 ==导出(export)== 了printf()函数，printf()是libc.so.6的 ==导出函数(export function)== 。对比于这种导入导出函数，在静态链接中就相当于普通函数的定义和引用。
> ELF中表示这种模块间符号导入导出的关系就叫做 ==动态符号表(Dynamic Symbol Table)==，对应的段名就是 ==".dynsym"(Dynamic Symbol)==。与".symbol"不同的是，".dynsym"只保存了与==动态链接相关的符号==，对于模块内部的符号，比如是私有变量则不保存。很多时候动态链接的模块同时拥有".dynsym"和".symtab"两个表，".symtab"中往往保存了所有符号，包括".dynsym"中的符号。
> 和".symtab"类似，动态符号表也需要一些辅助表，比如用于保存符号名的字符串表。今天链接时叫做字符串表==".strtab"(string table)==，在这里就是==动态符号字符串表".dynstr"(Dynamic String Table)==；由于动态链接下，我们需要在程序运行时查找符号，为了加快符号的查找过程，往往还有辅助的==符号哈希表(".hash")==。

使用readelf查看ELF文件的动态符号表以及哈希表：
```c
// lib.c
#include <stdio.h>

void foobar(int i) {
  printf("Printing form lib.so %d\n", i);
}
// gcc -shared -fPIC lib.c -o lib.so
```
```bash
$>  readelf -sD Lib.so

Symbol table for image contains 7 entries:
  Num:    Value          Size Type    Bind   Vis      Ndx Name
    0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND
    1: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_deregisterT[...]
    2: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND [...]@GLIBC_2.2.5 (2)
    3: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND __gmon_start__
    4: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_registerTMC[...]
    5: 0000000000000000     0 FUNC    WEAK   DEFAULT  UND [...]@GLIBC_2.2.5 (2)
    6: 0000000000001119    43 FUNC    GLOBAL DEFAULT   14 foobar
# .hash似乎已经不用了
```

### 5.4 动态链接重定位表
**动态链接重定位相关结构**
共享对象的重定位与静态链接的目标文件的重定位十分类似。唯一有区别的是目标文件的重定位是目标文件的重定位是在今天链接时完成，而共享对象的重定位则是在装载时完成的。在静态链接中，目标文件里面包含有专门由于表示重定位信息的重定位表，比如 ==".rel.text"== 表示代码段的重定位表，==".rel.data"== 是数据段的重定位表。
动态链接的文件中，重定位表分别叫做".rel.dyn"和".rel.plt"，他们分别相当于".rel.text"和".rel.data"。 ==".rel.dyn"== 实际上是对数据引用的修正，它所修正的位置位于==".got"以及数据段==；而 ==".rel.plt"== 是对函数引用的修正，它所修正的位置位于 ==".got.plt"== 。

使用readelf查看文件的重定位表：
```bash
$> readelf -r Lib.so

Relocation section '.rela.dyn' at offset 0x468 contains 7 entries:
  Offset          Info           Type           Sym. Value    Sym. Name + Addend
  000000003e10  000000000008 R_X86_64_RELATIVE                    1110
  000000003e18  000000000008 R_X86_64_RELATIVE                    10d0
  000000004020  000000000008 R_X86_64_RELATIVE                    4020
  000000003fe0  000100000006 R_X86_64_GLOB_DAT 0000000000000000 _ITM_deregisterTM[...] + 0
  000000003fe8  000300000006 R_X86_64_GLOB_DAT 0000000000000000 __gmon_start__ + 0
  000000003ff0  000400000006 R_X86_64_GLOB_DAT 0000000000000000 _ITM_registerTMCl[...] + 0
  000000003ff8  000500000006 R_X86_64_GLOB_DAT 0000000000000000 __cxa_finalize@GLIBC_2.2.5 + 0

  Relocation section '.rela.plt' at offset 0x510 contains 1 entry:
    Offset          Info           Type           Sym. Value    Sym. Name + Addend
    000000004018  000200000007 R_X86_64_JUMP_SLO 0000000000000000 printf@GLIBC_2.2.5 
$> readelf -S Lib.so
...
  [22] .got              PROGBITS         0000000000003fe0  00002fe0
       0000000000000020  0000000000000008  WA       0     0     8
  [23] .got.plt          PROGBITS         0000000000004000  00003000
       0000000000000020  0000000000000008  WA       0     0     8
  [24] .data             PROGBITS         0000000000004020  00003020
       0000000000000008  0000000000000000  WA       0     0     8
...
```

- R_X86_64_JUMP_SLO(对.got.plt的重定位)
  被修正的位置只需要填入符号的地址即可。例如printf这个重定位入口，它的类型为R_X86_64_JUMP_SLO，它的偏移为0x000000004018，它实际上位于".got.plt"中，前三项被系统占据，从第四项开始存放导入函数的地方。0x0000000000004000 + 8 * 3 = 000000004018。
  1. 第一项保存的是".dynamic"段的地址。
  2. 第二项保存的是本模块的ID。
  3. 第三项保存的是_dl_runtime_resolve()的地址。
  当动态链接器需要进行重定位时，先查找"printf"的地址，"printf"位于libc.so.6中。假设链接器在全局符号表里面找到"printf"的地址为0x000008801234，那么链接器就会将这个地址填入到".got.plt"中的偏移为0x000000004018位置中去，从而实现了地址的重定位，既实现了动态链接最关键的一步。

- R_X86_64_GLOB_DAT(对.got的重定位)
  和R_X86_64_JUMP_SLO一模一样。

## 6. 动态链接的步骤和实现
## 7. HOOK
