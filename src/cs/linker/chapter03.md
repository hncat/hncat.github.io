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

![按序叠加内存分配](/image/chapter03/按序叠加内存分配.png)

### 1.2 相似段合并
> [!note]
> 什么是相似段合并？
> 就是将相同性质的段进行合并，比如将所有输入文件的".text"合并到输出文件的".text"段，接着是".data"段，".bss"段等。

![相似段合并](/image/chapter03/相似段合并.png)

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


### 2.3 符号解析
### 2.4 指令修正方式
## 3. COMMON块
## 4. c++相关问题
## 5. 静态库链接
## 6. 链接过程控制
## 7. BFD库