---
title: 第四章 可执行文件的装载
date: 2024-07-03
tags:
    - 计算机基础
    - c/c++
categories: 链接、装载与库
isOriginal: true
order: 4
dir:
    order: 4
---
## 1. 装载的方式
### 1.1 覆盖装入
> [!note]
> 覆盖装入的方法把挖掘内存潜力的任务交给了程序员，程序员在编写程序时必须手工将程序分割成若干块，然后编写一个小的辅助代码工具来管理这些模块何时应该驻留内存何时应该被替换掉。这个小的辅助代码就是所谓的==覆盖管理器(Overlay Manager)==。

### 1.2 页映射
> [!note]
> 建议阅读csapp 第九章 虚拟内存

## 2. 从操作系统角度看可执行文件的装载
### 2.1 进程的建立
- 创建一个独立的虚拟地址空间。
    - 创建虚拟地址空间实际上只是分配一个页目录(Page Directory)就可以了，甚至不需要设置页映射关系。这些映射关系可以等到页错误时再进行映射。这一步中映射的是==虚拟空间到实际物理内存的映射关系==。

- 读取可执行文件头，并且建立虚拟空间与可执行文件的映射关系。
    - 这一步所做的是==虚拟空间与可执行文件的映射关系==。当程序执行发生页错误时，操作系统将从物理内存中分配一个物理页，然后将该“缺页”从磁盘中读取到内存中，再设置缺页的虚拟页和物理页的映射关系，这样程序才得以正确运行。所以当操作系统捕获到也错误发生时，应当知道所需的页在可执行文件中的那一个位置。这就是传统意义的“装载”过程。
    - 假设我们的ELF可执行文件只有一个代码段".text"，它的虚拟地址为0x08048000，它在文件中的大小为0x000e1，对齐为0x1000。由于虚拟存储的也映射都是以页为单位，在32为的Intel IA32下一般为4096字节，所以32位ELF的对齐粒度为0x1000。所以.text段对齐后实际占用的虚拟内存大小为0x1000。一旦该可执行文件被装载，可执行文件与执行该可执行文件进程的虚拟空间映射关系如下：
    ![可执行文件与进程虚拟空间](/image/linker/chapter04/可执行文件与进程虚拟空间.png)
    这种映射关系只是保存在操作系统内部的一个数据结构。linux中将进程虚拟空间中的一个段叫做==虚拟内存区域(VAM, Virtual Memory Area)==。操作系统创建进程后，会在进程相应的数据结构中设置一个.text段的VMA；它在虚拟空间中的地址为0x08040800~0x08049000，它对应ELF文件中偏移为0的.text，它的属性为只读。


- 将CPU的指令寄存器设置成可执行文件的入口地址，启动运行。
    - 从进程的角度看这一步可以简单的认为操作系统执行了一条跳转指令，直接跳转到可执行文件的入口地址(ELF文件头中保存有入口地址)。
### 2.2 页错误
> [!note]
> 进程建立完成后，其实可执行文件的真正指令和数据都没有被装入到内存中。操作系统只是通过可执行文件头部的信息建立起可执行文件和进程虚拟内存之间的映射关系。当CPU开始执行指令时，会发现对应页面是一个空页面，从而触发==页错误(Page Fault)==。CPU将控制权交给操作系统，操作系统处理页错误时，就会使用装载过程的中建立的可执行文件与虚拟内存的映射关系从而找到空页所在的VMA，计算出相应的页面在可执行文件中的偏移，然后在物理内存中分配一个物理页面，将进程与中该虚拟内存页与分配的物理内存页建立映射关系，然后把控制权返还给进程，进程从刚才页错误的位置重新开始执行。

![页错误](/image/linker/chapter04/页错误.png)

## 3. 进程虚拟空间分布
### 3.1 ELF文件链接视图和执行视图
> [!important]
> 从操作系统装载可执行文件的角度看，它只关心一些和装载相关的问题，最主要的时段的权限（可读、可写和可执行）。

- 以代码段为代表的权限为可读可执行段
- 以数据段和bss段位代表的权限为可读可写段
- 以只读数据为代表的权限为只读段

那么我们可以对于相同的段，把它们合并到一起当作一个段进行映射。比如有两个段分别叫".text"和".init"，它们包含的分别是程序的可执行代码和初始化代码，并且它们的权限相同，都是可读可执行。假设.text为4097字节，.init为512字节，这两个段分别映射的话要占用三个页面，但是，如果将它们合并成一起映射的话只需要两个页面。
![](/image/linker/chapter04/elf-segment.png)

> [!note]
> ELF可执行文件引入了一个概念叫做“segment”，一个“segment”包含一个或多个属性类似的“section”。比如上面提到的“.text”段和“.init”段合并在一起看做是一个“segment”，那么装载的时候就可以将它们看作一个整体一起映射，也就是所映射之后在进程虚拟内存中只有一个相对应的vma，而不是两个，这样做的好处是可以明显减少页面内部内存碎片，从而节省内存空间。

```c
// SectionMapping.c
#include <unistd.h>

int main() {
  while (1) {
    sleep(1000);
  }
  return 0;
}
// gcc -static SectionMapping.c -o SectionMapping.elf
```

```bash
$> gcc -static SectionMapping.c -o SectionMapping.elf
$> readelf -S SectionMapping.elf
There are 28 section headers, starting at offset 0xbf510:

Section Headers:
  [Nr] Name              Type             Address           Offset
       Size              EntSize          Flags  Link  Info  Align
  [ 0]                   NULL             0000000000000000  00000000
       0000000000000000  0000000000000000           0     0     0
  [ 1] .note.gnu.pr[...] NOTE             0000000000400270  00000270
       0000000000000030  0000000000000000   A       0     0     8
  [ 2] .note.gnu.bu[...] NOTE             00000000004002a0  000002a0
       0000000000000024  0000000000000000   A       0     0     4
  [ 3] .note.ABI-tag     NOTE             00000000004002c4  000002c4
       0000000000000020  0000000000000000   A       0     0     4
  [ 4] .rela.plt         RELA             00000000004002e8  000002e8
       0000000000000210  0000000000000018  AI       0    20     8
  [ 5] .init             PROGBITS         0000000000401000  00001000
       000000000000001b  0000000000000000  AX       0     0     4
  [ 6] .plt              PROGBITS         0000000000401020  00001020
       0000000000000160  0000000000000000  AX       0     0     16
  [ 7] .text             PROGBITS         0000000000401180  00001180
       000000000007d521  0000000000000000  AX       0     0     64
  [ 8] .fini             PROGBITS         000000000047e6a4  0007e6a4
       000000000000000d  0000000000000000  AX       0     0     4
  [ 9] .rodata           PROGBITS         000000000047f000  0007f000
       000000000001c1a4  0000000000000000   A       0     0     32
  [10] .stapsdt.base     PROGBITS         000000000049b1a4  0009b1a4
       0000000000000001  0000000000000000   A       0     0     1
  [11] rodata.cst32      PROGBITS         000000000049b1c0  0009b1c0
       0000000000000060  0000000000000020  AM       0     0     32
  [12] .eh_frame         PROGBITS         000000000049b220  0009b220
       00000000000096f0  0000000000000000   A       0     0     8
  [13] .gcc_except_table PROGBITS         00000000004a4910  000a4910
       00000000000000dc  0000000000000000   A       0     0     1
  [14] .tdata            PROGBITS         00000000004a5f50  000a4f50
       0000000000000018  0000000000000000 WAT       0     0     8
  [15] .tbss             NOBITS           00000000004a5f68  000a4f68
       0000000000000040  0000000000000000 WAT       0     0     8
  [16] .init_array       INIT_ARRAY       00000000004a5f68  000a4f68
       0000000000000008  0000000000000008  WA       0     0     8
  [17] .fini_array       FINI_ARRAY       00000000004a5f70  000a4f70
       0000000000000010  0000000000000008  WA       0     0     8
  [18] .data.rel.ro      PROGBITS         00000000004a5f80  000a4f80
       0000000000003fc8  0000000000000000  WA       0     0     32
  [19] .got              PROGBITS         00000000004a9f48  000a8f48
       0000000000000090  0000000000000000  WA       0     0     8
  [20] .got.plt          PROGBITS         00000000004a9fe8  000a8fe8
       00000000000000c8  0000000000000008  WA       0     0     8
  [21] .data             PROGBITS         00000000004aa0c0  000a90c0
       0000000000001a08  0000000000000000  WA       0     0     32
  [22] .bss              NOBITS           00000000004abae0  000aaac8
       0000000000005768  0000000000000000  WA       0     0     32
  [23] .comment          PROGBITS         0000000000000000  000aaac8
       0000000000000026  0000000000000001  MS       0     0     1
  [24] .note.stapsdt     NOTE             0000000000000000  000aaaf0
       00000000000015a0  0000000000000000           0     0     4
  [25] .symtab           SYMTAB           0000000000000000  000ac090
       000000000000bc10  0000000000000018          26   711     8
  [26] .strtab           STRTAB           0000000000000000  000b7ca0
       000000000000775f  0000000000000000           0     0     1
  [27] .shstrtab         STRTAB           0000000000000000  000bf3ff
       000000000000010c  0000000000000000           0     0     1
Key to Flags:
  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
  L (link order), O (extra OS processing required), G (group), T (TLS),
  C (compressed), x (unknown), o (OS specific), E (exclude),
  R (retain), D (mbind), l (large), p (processor specific)
```
使用readelf -l指令查看ELF文件的“segment”。正如描述“section”属性的结构叫做段表，描述“segment”的结构叫==程序头(Program Header)==。
```bash
$> readelf -l SectionMapping.elf

Elf file type is EXEC (Executable file)
Entry point 0x401720
There are 10 program headers, starting at offset 64

Program Headers:
  Type           Offset             VirtAddr           PhysAddr
                 FileSiz            MemSiz              Flags  Align
  LOAD           0x0000000000000000 0x0000000000400000 0x0000000000400000
                 0x00000000000004f8 0x00000000000004f8  R      0x1000
  LOAD           0x0000000000001000 0x0000000000401000 0x0000000000401000
                 0x000000000007d6b1 0x000000000007d6b1  R E    0x1000
  LOAD           0x000000000007f000 0x000000000047f000 0x000000000047f000
                 0x00000000000259ec 0x00000000000259ec  R      0x1000
  LOAD           0x00000000000a4f50 0x00000000004a5f50 0x00000000004a5f50
                 0x0000000000005b78 0x000000000000b2f8  RW     0x1000
  NOTE           0x0000000000000270 0x0000000000400270 0x0000000000400270
                 0x0000000000000030 0x0000000000000030  R      0x8
  NOTE           0x00000000000002a0 0x00000000004002a0 0x00000000004002a0
                 0x0000000000000044 0x0000000000000044  R      0x4
  TLS            0x00000000000a4f50 0x00000000004a5f50 0x00000000004a5f50
                 0x0000000000000018 0x0000000000000058  R      0x8
  GNU_PROPERTY   0x0000000000000270 0x0000000000400270 0x0000000000400270
                 0x0000000000000030 0x0000000000000030  R      0x8
  GNU_STACK      0x0000000000000000 0x0000000000000000 0x0000000000000000
                 0x0000000000000000 0x0000000000000000  RW     0x10
  GNU_RELRO      0x00000000000a4f50 0x00000000004a5f50 0x00000000004a5f50
                 0x00000000000040b0 0x00000000000040b0  R      0x1

 Section to Segment mapping:
  Segment Sections...
   00     .note.gnu.property .note.gnu.build-id .note.ABI-tag .rela.plt
   01     .init .plt .text .fini
   02     .rodata .stapsdt.base rodata.cst32 .eh_frame .gcc_except_table
   03     .tdata .init_array .fini_array .data.rel.ro .got .got.plt .data .bss
   04     .note.gnu.property
   05     .note.gnu.build-id .note.ABI-tag
   06     .tdata .tbss
   07     .note.gnu.property
   08
   09     .tdata .init_array .fini_array .data.rel.ro .got
# 这个可执行文件共有10个segment。从装载的角度看，我们只关心“LOAD”类型的segment，因为只有它是需要被映射的，其它类型的都是装载时起辅助作用。
```
“segment”和“section”是从不同角度来划分同一个elf文件。这个在elf中被称为不同的==视图(view)==，从“section”的角度来看elf文件就是==链接视图(linking view)==，从“segment“的角度来看就是==执行视图(exectuion view)==。

![elf可执行文件与进程虚拟空间映射关系](/image/linker/chapter04/mapping.png)

elf可执行文件有一个专门的数据结构叫做==程序头表(program header table)==用来保存”segment“的信息。因为elf目标文件不需要被装载，所以它没有程序头表，而可执行文件和共享库文件都有。和段表结构一样，程序头表也是一个结构体数组。
```c
// elf.h
typedef struct
{
  Elf64_Word	p_type;			/* Segment type */
  Elf64_Word	p_flags;		/* Segment flags */
  Elf64_Off	p_offset;		/* Segment file offset */
  Elf64_Addr	p_vaddr;		/* Segment virtual address */
  Elf64_Addr	p_paddr;		/* Segment physical address */
  Elf64_Xword	p_filesz;		/* Segment size in file */
  Elf64_Xword	p_memsz;		/* Segment size in memory */
  Elf64_Xword	p_align;		/* Segment alignment */
} Elf64_Phdr;
```

![程序头表结构字段](/image/linker/chapter04/programheader.png)
对于“LOAD”类型的“segment”来说，p_memsz的值不可以小于p_filesz，否则就是不合理的。如果p_memsz大于p_filesz，就表示该“segment”在内存中所分配的空间大小超过文件中实际的大小，这部分“多余“的部分则==全部填充为”0“==。因此我们在构造elf可执行文件时==不需要再额外设立bss的”segment“了==，可以把数据”segment“的p_memsz扩大，那些==额外的部分就是bss==。
### 3.2 堆和栈
查看可执行文件的堆栈。
```bash
$> ./SectionMapping.elf &
[1] 669
$> cat /proc/669/maps
00400000-00401000 r--p 00000000 08:10 116178                             /home/far/worker/linker/chapter04/SectionMapping.elf
00401000-0047f000 r-xp 00001000 08:10 116178                             /home/far/worker/linker/chapter04/SectionMapping.elf
0047f000-004a5000 r--p 0007f000 08:10 116178                             /home/far/worker/linker/chapter04/SectionMapping.elf
004a5000-004aa000 r--p 000a4000 08:10 116178                             /home/far/worker/linker/chapter04/SectionMapping.elf
004aa000-004ac000 rw-p 000a9000 08:10 116178                             /home/far/worker/linker/chapter04/SectionMapping.elf
004ac000-004b2000 rw-p 00000000 00:00 0
01ebb000-01edd000 rw-p 00000000 00:00 0                                  [heap]
7ffdee8a2000-7ffdee8c3000 rw-p 00000000 00:00 0                          [stack]
7ffdee99f000-7ffdee9a3000 r--p 00000000 00:00 0                          [vvar]
7ffdee9a3000-7ffdee9a4000 r-xp 00000000 00:00 0                          [vdso]
```
> [!note]
> 第一列是VMA的地址范围；第二列是VMA的权限，“r”表示可读，“w”表示可写，“x”表示可执行，“p”表示私有(COW, Copy on Write)。“s”表示共享。第三列是偏移，表示VMA对应的segment在可执行文件中的偏移；第四列表示可执行文件所在设备的主设备号和次设备号；第五列表示可执行文件的节点号。最后一列是可执行文件的路径。

观察到进程中10个VMA中只有前五个是映射到可执行文件中的两个segment。另外三个所在的设备号和次设备号以及文件节点都是0，则表示它们没有映射到文件中，这种VMA叫做==匿名虚拟内存区域(Anonymous Virtual Memory Area)==。

> [!important]
> 操作系统通过给进程空间划出一个个VMA来管理进程的虚拟空间；基本原则是将相同权限属性的、有相同映像文件的映射成一个VMA。
> - 代码VMA，可读可执行，有映像文件
> - 数据VAM，可读可写可执行，有映像文件
> - 堆VMA，可读可写可执行，无映像文件，匿名，可向上扩展
> - 栈VMA，可读可写、不可执行，无映像文件，匿名，可向下扩展。

![ELF与Linux进程虚拟空间映射关系](/image/linker/chapter04/ELF与Linux进程虚拟空间映射关系.png)
### 3.3 段地址对齐

|段|长度|偏移|权限|
|---|---|---|---|
|seg0|127|34|可读可执行|
|seg1|9899|164|可读可写|
|seg2|1988||只读|

使用最简单的映射将每个段分开映射，对于长度不足一页的按一页占用。所以整个可执行文件只有12014字节却占用了5个页，即20480字节，空间使用率只有58.6%。为了解决这个问题unix系统让那些==各个段接壤部分共享一个物理页，然后将该物理页分别映射两次==。比如seg0和seg1的接壤部分的那个物理页，系统将它们映射两份到虚拟地址空间，一份seg0，另一份seg1，其它的页都按照正常的页进行映射。unix系统将elf的文件头也看做是系统的一个段，将其映射到进程的地址空间，这样做的好处是进程中的某一段区域就是整个elf文件的映射，对于一些必须要访问elf文件头的操作（比如动态链接器就必须读取elf文件头）可以直接通过读写内存地址空间进行。在这种情况下，内存空间得到了充分利用，本来需要5个物理页的现在只需要3个物理页。这种映射情况下对于一个物理页来说可能同时包含多个段。

![物理页普通映射](/image/linker/chapter04/普通映射.png)

![物理页多次映射](/image/linker/chapter04/物理页多次映射.png)
### 3.4 进程栈初始化
假设系统中有两环境变量:
- HOME=/home/user
- PATH=/usr/bin
执行指令
```bash
$> prog 123
```
![Linux进程初始化堆栈](/image/linker/chapter04/栈初始化.png)
栈指针寄存器esp（x86_64应该为rip）指向的位置是初始化以后堆栈的顶部，最前面四个字节表示命令行参数的数量，即"prog"和"123"，紧接的就是指向这两个参数字符串的指针，后面跟一个0，接着是两个指向环境变量的字符串的指针，后面紧跟一个0表示结束。