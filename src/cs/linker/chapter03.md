---
title: 第三章 静态链接
date: 2024-06-29
tags:
    - 计算机基础
    - c/c++
categories: 链接、装载与库
isOriginal: true
order: 3
dir:
    order: 3
---
```c
// a.c
extern int shared;

int main() {
  int a = 100;
  swap(&a, &shared);
  return 0;
}

// b.c
int shared = 1;

void swap(int *a, int *b) {
  *a ^= *b ^= *a ^= *b;
}
```
```bash
$> gcc -c a.c b.c -O2
```
## 1. 空间与地址分配
> [!note]
> 链接的过程就是将几个输入目标文件加工合并成一个输出文件的过程。

### 1.1 按序叠加
> [!note]
> 什么是按序叠加？
> 就是将各个目标文件依次合并。但是这样会有些问题，当输入文件很多时，如果每个段都分别有.text段、.data段和.bss段，那最后输出文件将会有成百上千零散的段。这种做法是很浪费内存空间的（主要原因是内存对齐），对于x86的硬件来说，段的装载地址和空间的对齐单位是页，也就是4096字节。就算一个段只有1字节的大小，那这个段也要占据4096字节的内存大小。这会导致大量的内存碎片。

![按序叠加内存分配](/image/linker/chapter03/按序叠加内存分配.png)

### 1.2 相似段合并
> [!note]
> 什么是相似段合并？
> 就是将相同性质的段进行合并，比如将所有输入文件的".text"合并到输出文件的".text"段，接着是".data"段，".bss"段等。

![相似段合并](/image/linker/chapter03/相似段合并.png)

> [!important]
> .bss段在目标文件和可执行文件中不占用文件的空间，但是它在装载时占用地址空间。所以链接器在合并各个段的同时，也将".bss"段合并，同时分配虚拟空间。
> 关于地址和空间的两个含义：
> 1. 输出的可执行文件中的空间。
> 2. 装载后的虚拟地址中的虚拟地址空间。
> 对于".text"和".data"，它们在文件中和虚拟地址都要分配空间，对于".bss"这样的段，分配空间只局限与虚拟地址空间，因为它在文件中并没有内容。

> [!important]
> 两步链接
> 1. ==空间与地址分配== 扫描所有的输入目标文件，并且获得它们的各个段的长度、属性和位置，并且将输入目标文件中的符号表中所有的符号定义和符号引用收集起来，统一放到一个全局符号表。这一步连接器将能够获得所有输入目标文件的段长度，并且将它们合并，计算输出文件中各个段合并后的长度和位置，并建立映射关系。
> 2. ==符号解析与重定位== 使用上一步中收集到的所有信息，读取输入文件中段的数据、重定位信息，并且进行符号解析与重定位、调整代码中的地址等。

使用objdump指令查看链接前后的地址分配情况。
```bash
# VMA: 虚拟地址 LMA: 加载地址
$> objdump -h a.o

a.o:     file format elf64-x86-64

Sections:
Idx Name          Size      VMA               LMA               File off  Algn
  0 .text         00000058  0000000000000000  0000000000000000  00000040  2**0
                  CONTENTS, ALLOC, LOAD, RELOC, READONLY, CODE
  1 .data         00000000  0000000000000000  0000000000000000  00000098  2**0
                  CONTENTS, ALLOC, LOAD, DATA
  2 .bss          00000000  0000000000000000  0000000000000000  00000098  2**0
                  ALLOC
  3 .comment      00000027  0000000000000000  0000000000000000  00000098  2**0
                  CONTENTS, READONLY
  4 .note.GNU-stack 00000000  0000000000000000  0000000000000000  000000bf  2**0
                  CONTENTS, READONLY
  5 .note.gnu.property 00000020  0000000000000000  0000000000000000  000000c0  2**3
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  6 .eh_frame     00000038  0000000000000000  0000000000000000  000000e0  2**3
                  CONTENTS, ALLOC, LOAD, RELOC, READONLY, DATA
$> objdump -h b.o

b.o:     file format elf64-x86-64

Sections:
Idx Name          Size      VMA               LMA               File off  Algn
  0 .text         0000004f  0000000000000000  0000000000000000  00000040  2**0
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 .data         00000004  0000000000000000  0000000000000000  00000090  2**2
                  CONTENTS, ALLOC, LOAD, DATA
  2 .bss          00000000  0000000000000000  0000000000000000  00000094  2**0
                  ALLOC
  3 .comment      00000027  0000000000000000  0000000000000000  00000094  2**0
                  CONTENTS, READONLY
  4 .note.GNU-stack 00000000  0000000000000000  0000000000000000  000000bb  2**0
                  CONTENTS, READONLY
  5 .note.gnu.property 00000020  0000000000000000  0000000000000000  000000c0  2**3
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  6 .eh_frame     00000038  0000000000000000  0000000000000000  000000e0  2**3
                  CONTENTS, ALLOC, LOAD, RELOC, READONLY, DATA
$> objdump -h ab

ab:     file format elf64-x86-64

Sections:
Idx Name          Size      VMA               LMA               File off  Algn
  0 .interp       0000001c  0000000000000318  0000000000000318  00000318  2**0
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  1 .note.gnu.property 00000030  0000000000000338  0000000000000338  00000338  2**3
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  2 .note.gnu.build-id 00000024  0000000000000368  0000000000000368  00000368  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  3 .note.ABI-tag 00000020  000000000000038c  000000000000038c  0000038c  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  4 .gnu.hash     00000024  00000000000003b0  00000000000003b0  000003b0  2**3
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  5 .dynsym       000000a8  00000000000003d8  00000000000003d8  000003d8  2**3
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  6 .dynstr       000000a3  0000000000000480  0000000000000480  00000480  2**0
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  7 .gnu.version  0000000e  0000000000000524  0000000000000524  00000524  2**1
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  8 .gnu.version_r 00000040  0000000000000538  0000000000000538  00000538  2**3
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  9 .rela.dyn     000000c0  0000000000000578  0000000000000578  00000578  2**3
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
 10 .rela.plt     00000018  0000000000000638  0000000000000638  00000638  2**3
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
 11 .init         0000001b  0000000000001000  0000000000001000  00001000  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
 12 .plt          00000020  0000000000001020  0000000000001020  00001020  2**4
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
 13 .plt.got      00000010  0000000000001040  0000000000001040  00001040  2**4
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
 14 .plt.sec      00000010  0000000000001050  0000000000001050  00001050  2**4
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
 15 .text         00000190  0000000000001060  0000000000001060  00001060  2**4
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
 16 .fini         0000000d  00000000000011f0  00000000000011f0  000011f0  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
 17 .rodata       00000004  0000000000002000  0000000000002000  00002000  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
 18 .eh_frame_hdr 0000003c  0000000000002004  0000000000002004  00002004  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
 19 .eh_frame     000000cc  0000000000002040  0000000000002040  00002040  2**3
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
 20 .init_array   00000008  0000000000003db8  0000000000003db8  00002db8  2**3
                  CONTENTS, ALLOC, LOAD, DATA
 21 .fini_array   00000008  0000000000003dc0  0000000000003dc0  00002dc0  2**3
                  CONTENTS, ALLOC, LOAD, DATA
 22 .dynamic      000001f0  0000000000003dc8  0000000000003dc8  00002dc8  2**3
                  CONTENTS, ALLOC, LOAD, DATA
 23 .got          00000048  0000000000003fb8  0000000000003fb8  00002fb8  2**3
                  CONTENTS, ALLOC, LOAD, DATA
 24 .data         00000014  0000000000004000  0000000000004000  00003000  2**3
                  CONTENTS, ALLOC, LOAD, DATA
 25 .bss          00000004  0000000000004014  0000000000004014  00003014  2**0
                  ALLOC
 26 .comment      00000026  0000000000000000  0000000000000000  00003014  2**0
                  CONTENTS, READONLY
```

### 1.3 符号地址的确定
> [!note]
> 在第一步的扫描和空间分配阶段，各个段在链接后的虚拟地址空间已经确定。比如".text"段的起始地址为0x0000000000001060，".data"段的起始地址为0x0000000000004000。因为各个符号在段内的相对位置时固定的，所以这时"main"、"shared"和"swap"的地址也已经确定了，连接器通过给每个符号加上一个偏移量，使它们能够调整到正确的虚拟地址。比如我们假设"a.o"中的"main"函数相对于"a.o"的".text"段的偏移量为X，但是经过链接合并后，"a.o"的".text"段位于虚拟地址空间0x0000000000001060(这里是链接成可执行文件ab后在可执行文件ab中的偏移量)，那么"main"函数的虚拟地址应该是0x0000000000001060+X。

## 2. 符号解析与重定位
### 2.1 重定位
> [!note]
> 使用objdump -d查看反汇编结果
> ```bash
> $> objdump -d a.o
> 
> a.o:     file format elf64-x86-64
> 
> 
> Disassembly of section .text.startup:
> 
> 0000000000000000 <main>:
>    0:   f3 0f 1e fa             endbr64
>    4:   48 83 ec 18             sub    $0x18,%rsp
>    8:   48 8d 35 00 00 00 00    lea    0x0(%rip),%rsi        # f <main+0xf> 将shared地址写入第二个参数
>    f:   64 48 8b 04 25 28 00    mov    %fs:0x28,%rax
>   16:   00 00
>   18:   48 89 44 24 08          mov    %rax,0x8(%rsp)
>   1d:   31 c0                   xor    %eax,%eax
>   1f:   48 8d 7c 24 04          lea    0x4(%rsp),%rdi
>   24:   c7 44 24 04 64 00 00    movl   $0x64,0x4(%rsp)
>   2b:   00
>   2c:   e8 00 00 00 00          call   31 <main+0x31>
>   31:   48 8b 44 24 08          mov    0x8(%rsp),%rax
>   36:   64 48 2b 04 25 28 00    sub    %fs:0x28,%rax
>   3d:   00 00
>   3f:   75 07                   jne    48 <main+0x48>
>   41:   31 c0                   xor    %eax,%eax
>   43:   48 83 c4 18             add    $0x18,%rsp
>   47:   c3                      ret
>   48:   e8 00 00 00 00          call   4d <main+0x4d>
> ```
> 由于此时还并没有进行链接（即没有执行链接的第一步：==虚拟地址内存分配==）所以main的起始地址为0。
> shared地址在反汇编代码的0x08字节处标记为0(8:48 8d 35 ==00 00 00 00== lea 0x0(%rip),%rsi)后面四字节就是该指令的==下一条指令的偏移量==。
> swap调用处的地址在反汇编代码的0x2c字节处同样被标记为0(2c:e8 ==00 00 00 00== call 31 <main+0x31>)后面四个字节就是被调用函数的相对于调用指令的==下一条指令的偏移量==。
> 此时的指令地址时暂时的，真正的地址计算工作留给了连接器。连接器在完成第一步的虚拟内存分配后就可以确定所有符号的虚拟地址了。

#### 修正后的地址
> [!important]
> ```bash
> $> objdump -d ab
> ab:     file format elf64-x86-64
> ... # 截取了部分
> Disassembly of section .text:
> 
> 0000000000001060 <main>:
>     1060:       f3 0f 1e fa             endbr64
>     1064:       48 83 ec 18             sub    $0x18,%rsp
>     1068:       48 8d 35 a1 2f 00 00    lea    0x2fa1(%rip),%rsi        # 4010 <shared>
>     106f:       64 48 8b 04 25 28 00    mov    %fs:0x28,%rax
>     1076:       00 00
>     1078:       48 89 44 24 08          mov    %rax,0x8(%rsp)
>     107d:       31 c0                   xor    %eax,%eax
>     107f:       48 8d 7c 24 04          lea    0x4(%rsp),%rdi
>     1084:       c7 44 24 04 64 00 00    movl   $0x64,0x4(%rsp)
>     108b:       00
>     108c:       e8 0f 01 00 00          call   11a0 <swap>
>     1091:       48 8b 44 24 08          mov    0x8(%rsp),%rax
>     1096:       64 48 2b 04 25 28 00    sub    %fs:0x28,%rax
>     109d:       00 00
>     109f:       75 07                   jne    10a8 <main+0x48>
>     10a1:       31 c0                   xor    %eax,%eax
>     10a3:       48 83 c4 18             add    $0x18,%rsp
>     10a7:       c3                      ret
>     10a8:       e8 a3 ff ff ff          call   1050 <__stack_chk_fail@plt>
>     10ad:       0f 1f 00                nopl   (%rax)
> ...
> ```
>
> 经过修正后"shared"和"swap"地址分别为0x4010和0x11a0
> ```txt
> shared的地址: 0x106f + 0x2fa1 = 0x4010
>     1068:       48 8d 35 a1 2f 00 00    lea    0x2fa1(%rip),%rsi
>     106f:       64 48 8b 04 25 28 00    mov    %fs:0x28,%rax
> swap的地址: 0x1091 + 0x010f = 0x11a0
>     108c:       e8 0f 01 00 00          call   11a0 <swap>
>     1091:       48 8b 44 24 08          mov    0x8(%rsp),%rax
> ```

### 2.2 重定位表
> [!note]
> 使用objdump/readelf指令可以查看重定位文件。因为重定位表其实就是elf文件中的一个段，因此又被称为重定位段，比如代码段".text"如果有要重定位的地方，那么会有一个相应的叫".rel.text"的段保存了代码段的重定位表，同理".data"也会有一个叫".rel.data"的段。

```bash
# a.o中所有要重定位的地方，既“a.o”所有引用到的外部符号。
$> bjdump -r a.o

a.o:     file format elf64-x86-64

RELOCATION RECORDS FOR [.text.startup]:
OFFSET           TYPE              VALUE
000000000000000b R_X86_64_PC32     shared-0x0000000000000004 # pc（下一条指令） - 0x04
000000000000002d R_X86_64_PLT32    swap-0x0000000000000004
0000000000000049 R_X86_64_PLT32    __stack_chk_fail-0x0000000000000004


RELOCATION RECORDS FOR [.eh_frame]:
OFFSET           TYPE              VALUE
0000000000000020 R_X86_64_PC32     .text.startup
$> readelf -r a.o

Relocation section '.rela.text.startup' at offset 0x1e8 contains 3 entries:
  Offset          Info           Type           Sym. Value    Sym. Name + Addend
  00000000000b  000400000002 R_X86_64_PC32     0000000000000000 shared - 4
  00000000002d  000500000004 R_X86_64_PLT32    0000000000000000 swap - 4
  000000000049  000600000004 R_X86_64_PLT32    0000000000000000 __stack_chk_fail - 4

  Relocation section '.rela.eh_frame' at offset 0x230 contains 1 entry:
    Offset          Info           Type           Sym. Value    Sym. Name + Addend
    000000000020  000200000002 R_X86_64_PC32     0000000000000000 .text.startup + 0
```
每一个被重定位的地方叫一个==重定位入口(Relocation Entry)==，其中每个重定位表包含了一下信息：
1. 重定位入口的==偏移(offset)==，表示该入口在要被重定位段中的位置。
2. 该重定位表作用的elf文件中的那个段==RELOCATION RECORDS FOR \[.text.startup\](比如这个就是代码段)==。
```c
// 重定位表的结构
typedef struct
{
  Elf64_Addr	r_offset;		/* Address */
  Elf64_Xword	r_info;			/* Relocation type and symbol index */
} Elf64_Rel;
```
![重定位表结构](/image/linker/chapter03/重定位表结构.png)

### 2.3 符号解析
```bash
# 符号未定义错误 (未正确链接目标文件或者库文件导致)
$> gcc a.c
a.c: In function ‘main’:
a.c:5:3: warning: implicit declaration of function ‘swap’ [-Wimplicit-function-declaration]
    5 |   swap(&a, &shared);
      |   ^~~~
/usr/bin/ld: /tmp/ccwmM6Jl.o: warning: relocation against `shared' in read-only section `.text'
/usr/bin/ld: /tmp/ccwmM6Jl.o: in function `main':
a.c:(.text+0x29): undefined reference to `shared'
/usr/bin/ld: a.c:(.text+0x39): undefined reference to `swap'
/usr/bin/ld: warning: creating DT_TEXTREL in a PIE
collect2: error: ld returned 1 exit status
```

> [!note]
> 重定位的过程也伴随着符号解析的过程。每个目标文件都可能定义一些符号，也可能引用其它目标文件的符号。重定位过程中，每个重定位入口都是对一个符号的引用，那么当连接器需要对某个符号的引用进行重定位时，就要确定这个符号的目的地址。这时候连接器就会查找所有输入目标文件的符号表组成的全局符号表，找到相应的符号后进行重定位。

> [!note]
> 需要补充的一点是linux链接器在符号解析阶段，链接器是从左到右按照它们在编译器取得程序命令行上出现的顺序来扫描可重定位目标文件和存档文件(静态库)的。链接器维护一个可重定位目标文件的集合E（这个集合中的文件会被合并起来形成可执行文件），一个未解析的符号集合U（即引用了但尚未定义的符号），以及一个在前面输入文件中已定义的符号集合D。
>
> ·对于命令行上的每个输入文件 f, 链接器会判断f是一个目标文件还是一个存档文件。如果f是一个目标文件，那么链接器把f添加到E, 修改U和D来反映f中的符号定义和引用，并继续下一个输入文件。
>
> ·如果f是一个存档文件，那么链接器就尝试匹配U中未解析的符号和由存档文件成员定义的符号。如果某个存档文件成员m, 定义了一个符号来解析U中的一个引用，那将m加到么就E中，并且链接器修改U和D来反映m中的符号定义和引用。对存档文件中所有的成员目标文件都依次进行这个过程，直到U和D都不再发生变化。此时，任何不包含在E中的成员目标文件都简单地被丢弃，而链接器将继续处理下一个输入文件。
>
> ·如果当链接器完成对命令行上输入文件的扫描后，U是非空的，那么链接器就会输出一个错误并终止。否则，它会合并和重定位E中的目标文件，构建输出的可执行文件。
>
> 这样的解析通常会伴随一个顺序问题，比如文件foo.c依赖与liba.a。当输入指令如下时就会出现符号未定义错误。
> ```bash
> # 错误的
> $> gcc -static ./liba.a foo.c
> # 正确的
> $> gcc -static foo.c ./liba.a
> ```
> 因为链接器时从左到右开始扫描的，而静态库文件（存档文件）liba.a先于foo.c文件所以就会执行上述的存档文件的操作导致foo.c依赖的文件不会并入集合中。最终导致符号未定义行为，特别是出现相互依赖的库文件时更容易出现这种错误。
> foo.c 依赖 liba.a, liba.a 依赖 libb.a, libb.a 依赖 liba.a
> ```bash
> # 错误的
> $> gcc -static foo.c ./liba.a ./libb.a
> # 正确的
> $> gcc -static foo.c ./liba.a ./libb.a ./liba.a
> ```
> ==csapp 第七章 7.6.3==

使用readelf -s指令查看a.o的符号表:
```bash
$>  readelf -s a.o

Symbol table '.symtab' contains 7 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND
     1: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS a.c
     2: 0000000000000000     0 SECTION LOCAL  DEFAULT    1 .text
     3: 0000000000000000    88 FUNC    GLOBAL DEFAULT    1 main
     4: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND shared
     5: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND swap
     6: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND __stack_chk_fail
```
> [!important]
> ==GLOBAL==类型的符号，除了"main"函数是定义在代码段之外，其它几个"shared"，"swap"，"__stack_chk_fail"都是==UND（undefined）未定义的类型==，这种未定义的符号都是该目标文件的==重定位项==。所以在链接器扫描完所有的输入目标文件后，所有这些未定义的符号都应该能在==全局符号表==中找到，否则链接就会报符号==未定义错误==。

### 2.4 指令修正方式(参考csapp，原书第七章 7.7)
```c
// csapp提到的另一种重定位结构
// r_addend: 有符号常数，一些类型的重定位要使用它对被修改引用的值做偏移调整。
typedef struct
{
  Elf64_Addr	r_offset;		/* Address */
  Elf64_Xword	r_info;			/* Relocation type and symbol index */
  Elf64_Sxword	r_addend;		/* Addend */
} Elf64_Rela;
```

elf定义了32种不同的重定位类型。我们只关心其中两种最基本的重定位类型：
- R_X86_64_PC32。重定位一个使用32位PC相对地址的引用。一个PC相对地址就是距程序计数器(PC)的当前运行时值的偏移量。CPU执行一条使用PC相对寻址的指令时，它就将在指令中编码的32位值加上PC的当前运行时值，得到有效地址，PC值通常是下一条指令在内存中的值。
- R_X86_64_32。重定位一个使用32位绝对地址的引用。CPU直接使用在指令中编码的32值作为有效地址，不需要进一步修改。

> [!important]
> 这两种重定位类型支持x86-64小型代码模型(small code model)，该模型假设可执行目标文件中的代码和数据的总体大小小于2GB，因此在运行时可以用32位PC相对地址来访问。GCC默认使用小型代码模型。大于2GB的程序可以使用-mcmodel=medium(中型代码模型)和-mcmodel=large(大型代码模型)标志来编译。

```
foreach section s {
  foreach relocation entry r {
    refptr = s + r.offset; // 需要修改的重定位地址
    if (r.type == R_X86_64_PC32) {
      refaddr = ADDR(s) + r.offset; // 运行时引用符号的地址
      *refptr = (unsigned)(ADDR(r.symbol) + r.addend - refaddr); // 被引用的符号地址 + 修正值 - 运行时引用符号地址。
    }
    if (r.type == R_X86_64_32) {
      *refptr = (unsigned)(ADDR(r.symbol) + r.addend);
    }
  }
}
```

## 3. COMMON块
> [!note]
> COMMON块的机制来源于Fortran，早期的Fortran没有动态分配空间的机制，所以必须事先声明它所需的临时使用空间的大小。Fortran把这种空间叫COMMON块，当不同的目标文件需要的COMMON块空间大小不一致时，以最大的那块为准。
> 现代的链接机制在处理弱符号时，采用的就是与COMMON块一样的机制。当然COMMON块的链接规则仅仅是针对弱符号的，如果其中有一个符号为强符号时，那么最终输出结果中的符号所占空间与强符号相同。
> 值得注意的是，如果链接过程中有弱符号大于强符号所使用的内存大小，那么链接器通常会给出警告。
> /usr/bin/ld: warning: alignment 4 of symbol `global' in /tmp/ccjFgL1Q.o is smaller than 8 in /tmp/ccVEFqSm.o

另外值得注意的是可能有些编译器并不是将弱符号使用COMMON块，而是使用的bss段，因此会报重定义错误。此时可以给编译选项加上--common选项即可。

## 4. 静态库链接
> [!note]
> 静态库可以简单的看成一组目标文件的集合，既很多目标文件经过压缩打包后形成的一个文件。

使用"ar"工具查看这个文件包含了那些目标文件：
```bash
$> ar -t /usr/lib/x86_64-linux-gnu/libc.a
```
使用objdump指令查看某个符号在那个文件中
```bash
$> objdump -t /usr/lib/x86_64-linux-gnu/libc.a | grep vprintf
vprintf.o:     file format elf64-x86-64
0000000000000000 g     F .text  0000000000000018 __vprintf
0000000000000000 g     F .text  0000000000000018 vprintf
0000000000000160 g     F .text  00000000000001c9 .hidden __obstack_vprintf_internal
0000000000000330 g     F .text  00000000000001c4 __obstack_vprintf
0000000000000330  w    F .text  00000000000001c4 obstack_vprintf
vprintf_chk.o:     file format elf64-x86-64
0000000000000000 g     F .text  0000000000000019 ___vprintf_chk
0000000000000000 g     F .text  0000000000000019 __vprintf_chk
0000000000000000         *UND*  0000000000000000 .hidden __obstack_vprintf_internal
0000000000000000 g     F .text  000000000000001c __obstack_vprintf_chk
0000000000000000         *UND*  0000000000000000 .hidden __obstack_vprintf_interna
```
