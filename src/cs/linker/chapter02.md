---
title: 第二章 目标文件
date: 2024-06-22 23:52:32
tags:
    - 计算机基础
    - c/c++
categories: 链接、装载与库
---
## 1. 目标文件的格式
Linux下的主流文件存储格式为==ELF==，通过file指令可以查看文件的存储格式。
```bash
$> file foobar.o
foobar.o: ELF 64-bit LSB relocatable, x86-64, version 1 (SYSV), not stripped
$> file /bin/bash
/bin/bash: ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, BuildID[sha1]=2f77b36371c214e11670c7d9d92727e9a49f626b, for GNU/Linux 3.2.0, stripped
file /lib32/ld-linux.so.2
/lib32/ld-linux.so.2: ELF 32-bit LSB shared object, Intel 80386, version 1 (GNU/Linux), static-pie linked, BuildID[sha1]=595f7870a8165b3eb04c9ebda08a8ccba6f742c2, stripped
```
ELF文件标准里采用ELF格式的文件:
- 可重定位文件
    Linux的 *.o文件
    静态链接库
- 可执行文件
- 共享目标文件
    Linux的 *.so文件
- 核心转储文件
    Linux下的 core dump
## 2. 目标文件是什么样的
目标文件将信息按不同的属性进行存储，通常称之为==节(section)或者段(segment)==

> [!tip]
> 源代码编译后的机器指令经常放于代码段(.text)
> 全局变量和局部静态变量经常存放于数据段(.data)

![程序与目标文件](/image/chapter02/程序与目标文件.png)
ELF文件的开头是一个“文件头”，描述了整个文件的文件属性。
- 是否可执行
- 是否静态链接
- 是否动态链接
- 入口地址
- 目标硬件
- 目标操作系统
- 等等

文件头同时包括一个段表
> [!tip]
> 段表用来描述一个文件中各个段的数组。段表描述了文件中各个段在文件中的偏移位置及段的属性等，从段表中可以得到每个的段的所有信息。
> .data section: 已初始化的全局变量和局部静态变量
> .bss section: 未初始化的全局变量和局部静态变量(默认值为0),初始化为0的全局变量和局部静态变量
> .bss段只是为未初始化的全局变量和局部静态变量预留位置而已(记录所有为初始或者初始化为0的全局变量和静态变量的大小总和)，所以在文件中也不占据空间

分段的好处
- [x] 可以防止程序被恶意篡改
- [x] 利用缓存，提高CPU的缓存命中
- [x] 节省内存空间(内存共享比如libc的动态链接库)
## 3. 挖掘SimpleSection.o
```c
int printf(const char *format, ...);
int global_init_var = 84;
int global_uninit_var;

void func1(int i) { printf("%d\n", i); }

int main() {
  static int static_var = 85;
  static int static_var2;
  int a = 1;
  int b;
  func1(static_var + static_var2 + a + b);
  return 0;
}
```
> [!tip]
> 使用objdump -h指令打印ELF文件的各个段的基本信息
> -x 参数可以打印更多信息

```bash
$> objdump -h SimpleSection.o

SimpleSection.o:     file format elf64-x86-64

Sections:
Idx Name          Size      VMA               LMA               File off  Algn
  0 .text         00000064  0000000000000000  0000000000000000  00000040  2**0
                  CONTENTS, ALLOC, LOAD, RELOC, READONLY, CODE
  1 .data         00000008  0000000000000000  0000000000000000  000000a4  2**2
                  CONTENTS, ALLOC, LOAD, DATA
  2 .bss          00000008  0000000000000000  0000000000000000  000000ac  2**2
                  ALLOC
  3 .rodata       00000004  0000000000000000  0000000000000000  000000ac  2**0
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  4 .comment      00000027  0000000000000000  0000000000000000  000000b0  2**0
                  CONTENTS, READONLY
  5 .note.GNU-stack 00000000  0000000000000000  0000000000000000  000000d7  2**0
                  CONTENTS, READONLY
  6 .note.gnu.property 00000020  0000000000000000  0000000000000000  000000d8  2**3
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  7 .eh_frame     00000058  0000000000000000  0000000000000000  000000f8  2**3
                  CONTENTS, ALLOC, LOAD, RELOC, READONLY, DATA
```
> [!note]
> .rodata: 只读数据段
> .comment: 注释信息段
> .note.GNU-stack: 堆栈提示段
> Size: 段的长度
> File Offset: 段所在的位置(段偏移)
> CONTENTS: 该段在文件中存在(bss段没有表示bss段在ELF文件中不存在内容。.note.GNU-stack虽然有但是长度为0，认为它在ELF文件中也不存在)

使用==size==指令查看ELF文件的代码段、数据段和bss段。
```bash
$> size SimpleSection.o
   text    data     bss     dec     hex filename
    224       8       8     240      f0 SimpleSection.o
```
### 3.1 代码段
```bash
$> objdump -s -d SimpleSection.o

SimpleSection.o:     file format elf64-x86-64

Contents of section .text:
 0000 f30f1efa 554889e5 4883ec10 897dfc8b  ....UH..H....}..
 0010 45fc89c6 488d0500 00000048 89c7b800  E...H......H....
 0020 000000e8 00000000 90c9c3f3 0f1efa55  ...............U
 0030 4889e548 83ec10c7 45f80100 00008b15  H..H....E.......
 0040 00000000 8b050000 000001c2 8b45f801  .............E..
 0050 c28b45fc 01d089c7 e8000000 00b80000  ..E.............
 0060 0000c9c3                             ....
Contents of section .data:
 0000 54000000 55000000                    T...U...
Contents of section .rodata:
 0000 25640a00                             %d..
Contents of section .comment:
 0000 00474343 3a202855 62756e74 75203133  .GCC: (Ubuntu 13
 0010 2e322e30 2d323375 62756e74 75342920  .2.0-23ubuntu4)
 0020 31332e32 2e3000                      13.2.0.
Contents of section .note.gnu.property:
 0000 04000000 10000000 05000000 474e5500  ............GNU.
 0010 020000c0 04000000 03000000 00000000  ................
Contents of section .eh_frame:
 0000 14000000 00000000 017a5200 01781001  .........zR..x..
 0010 1b0c0708 90010000 1c000000 1c000000  ................
 0020 00000000 2b000000 00450e10 8602430d  ....+....E....C.
 0030 06620c07 08000000 1c000000 3c000000  .b..........<...
 0040 00000000 39000000 00450e10 8602430d  ....9....E....C.
 0050 06700c07 08000000                    .p......

Disassembly of section .text:

0000000000000000 <func1>:
   0:   f3 0f 1e fa             endbr64
   4:   55                      push   %rbp
   5:   48 89 e5                mov    %rsp,%rbp
   8:   48 83 ec 10             sub    $0x10,%rsp
   c:   89 7d fc                mov    %edi,-0x4(%rbp)
   f:   8b 45 fc                mov    -0x4(%rbp),%eax
  12:   89 c6                   mov    %eax,%esi
  14:   48 8d 05 00 00 00 00    lea    0x0(%rip),%rax        # 1b <func1+0x1b>
  1b:   48 89 c7                mov    %rax,%rdi
  1e:   b8 00 00 00 00          mov    $0x0,%eax
  23:   e8 00 00 00 00          call   28 <func1+0x28>
  28:   90                      nop
  29:   c9                      leave
  2a:   c3                      ret

000000000000002b <main>:
  2b:   f3 0f 1e fa             endbr64
  2f:   55                      push   %rbp
  30:   48 89 e5                mov    %rsp,%rbp
  33:   48 83 ec 10             sub    $0x10,%rsp
  37:   c7 45 f8 01 00 00 00    movl   $0x1,-0x8(%rbp)
  3e:   8b 15 00 00 00 00       mov    0x0(%rip),%edx        # 44 <main+0x19>
  44:   8b 05 00 00 00 00       mov    0x0(%rip),%eax        # 4a <main+0x1f>
  4a:   01 c2                   add    %eax,%edx
  4c:   8b 45 f8                mov    -0x8(%rbp),%eax
  4f:   01 c2                   add    %eax,%edx
  51:   8b 45 fc                mov    -0x4(%rbp),%eax
  54:   01 d0                   add    %edx,%eax
  56:   89 c7                   mov    %eax,%edi
  58:   e8 00 00 00 00          call   5d <main+0x32>
  5d:   b8 00 00 00 00          mov    $0x0,%eax
  62:   c9                      leave
  63:   c3                      ret
```
> [!tip]
> -s: 将所有段的内容以十六进制的方式打印
> -d: 将所有包含指令的段反汇编

### 3.2 数据段和只读数据段
> [!tip]
> .data section: 已初始化的全局变量和局部静态变量

SimpleSection.c代码中一共有两个这样的变量(global_init_var, static_var),所以.data段的大小正好为8个字节。
> [!tip]
> .rodata section: 存放只读数据(const变量,字符串字面量)

SimpleSection.c代码中用在调用printf时用到了字符字面量"%d\n",它是一种只读数据,所以它被放到了.rodata段。所以.rodata段的大小正好为四字节(字符串结尾包含字符串结束符)。
### 3.3 bss段
> [!tip]
> .bss section: 未初始化的全局变量和局部静态变量(默认值为0),初始化为0的全局变量和局部静态变量

SimpleSection.c代码中一共有两个这样的变量(global_uninit_var, static_var2),所以.data段的大小正好为8个字节。
> [!tip]
> 可以给gcc选项加入-fcommon使未初始化的全局变量被定义为一个"COMMON"符号，注意对于初始化为0的全局变量则不行。

```bash
$> gcc -c SimpleSection.c -fcommon
$> readelf -s SimpleSection.o

Symbol table '.symtab' contains 13 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND
     1: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS SimpleSection.c
     2: 0000000000000000     0 SECTION LOCAL  DEFAULT    1 .text
     3: 0000000000000000     0 SECTION LOCAL  DEFAULT    3 .data
     4: 0000000000000000     0 SECTION LOCAL  DEFAULT    4 .bss
     5: 0000000000000000     0 SECTION LOCAL  DEFAULT    5 .rodata
     6: 0000000000000004     4 OBJECT  LOCAL  DEFAULT    3 static_var.1
     7: 0000000000000000     4 OBJECT  LOCAL  DEFAULT    4 static_var2.0
     8: 0000000000000000     4 OBJECT  GLOBAL DEFAULT    3 global_init_var
     9: 0000000000000004     4 OBJECT  GLOBAL DEFAULT  COM global_uninit_var
    10: 0000000000000000    43 FUNC    GLOBAL DEFAULT    1 func1
    11: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND printf
    12: 000000000000002b    57 FUNC    GLOBAL DEFAULT    1 main
```
### 3.3 其它段
![其它段](/image/chapter02/其他段.png)
## 4. ELF文件结构描述
> [!note]
> ELF目标文件的最前部是ELF文件头（ELF Header）,它包含了描述整个文件的基本属性。仅接着是ELF各个段。其中ELF文件中与段有关的重要结构就是段表（Section Header Table），该表描述了ELF文件包含的所有段的信息，比如每个段的段名、段的长度、在文件中的偏移、读写权限及段的其它属性。

![ELF文件结构](/image/chapter02/ELF文件结构.png)
### 4.1 文件头
通过使用==readelf -h==指令即可查看ELF文件头
```bash
$> readelf -h SimpleSection.o
ELF Header:
  Magic:   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00
  Class:                             ELF64
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              REL (Relocatable file)
  Machine:                           Advanced Micro Devices X86-64
  Version:                           0x1
  Entry point address:               0x0
  Start of program headers:          0 (bytes into file)
  Start of section headers:          1032 (bytes into file)
  Flags:                             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           0 (bytes)
  Number of program headers:         0
  Size of section headers:           64 (bytes)
  Number of section headers:         14
  Section header string table index: 13
```
ELF文件头定义了
- ELF魔数
- 文件机器字节长度
- 数据存储方式
- 版本
- 运行平台
- ABI版本
- ELF重定位类型
- 硬件平台
- 硬件平台版本
- 入口地址
- 程序入口和长度
- 段表位置和长度
- 段数量
```c
// /usr/include/elf.h
typedef struct
{
  unsigned char	e_ident[EI_NIDENT];	/* Magic number and other info */
  Elf64_Half	e_type;			/* Object file type */
  Elf64_Half	e_machine;		/* Architecture */
  Elf64_Word	e_version;		/* Object file version */
  Elf64_Addr	e_entry;		/* Entry point virtual address */
  Elf64_Off	e_phoff;		/* Program header table file offset */
  Elf64_Off	e_shoff;		/* Section header table file offset */
  Elf64_Word	e_flags;		/* Processor-specific flags */
  Elf64_Half	e_ehsize;		/* ELF header size in bytes */
  Elf64_Half	e_phentsize;		/* Program header table entry size */
  Elf64_Half	e_phnum;		/* Program header table entry count */
  Elf64_Half	e_shentsize;		/* Section header table entry size */
  Elf64_Half	e_shnum;		/* Section header table entry count */
  Elf64_Half	e_shstrndx;		/* Section header string table index */
} Elf64_Ehdr;
```
![ELF文件头结构成员含义](/image/chapter02/ELF文件头结构成员含义.png)
#### 4.1.2 文件类型
![文件类型](/image/chapter02/文件类型.png)
#### 4.1.3 机器类型
![机器类型](/image/chapter02/机器类型.png)
### 4.2 段表
> [!tip]
> 段表用于保存这些段的基本属性结构。

使用==objdump -h==来查看ELF文件中包含的段，但是只是把ELF文件中关键的段显示了出来。可以使用==readelf -S==指令来显示ELF文件的各种段表结构
```bash
$> readelf -S SimpleSection.o
There are 14 section headers, starting at offset 0x408:

Section Headers:
  [Nr] Name              Type             Address           Offset
       Size              EntSize          Flags  Link  Info  Align
  [ 0]                   NULL             0000000000000000  00000000
       0000000000000000  0000000000000000           0     0     0
  [ 1] .text             PROGBITS         0000000000000000  00000040
       0000000000000064  0000000000000000  AX       0     0     1
  [ 2] .rela.text        RELA             0000000000000000  000002e8
       0000000000000078  0000000000000018   I      11     1     8
  [ 3] .data             PROGBITS         0000000000000000  000000a4
       0000000000000008  0000000000000000  WA       0     0     4
  [ 4] .bss              NOBITS           0000000000000000  000000ac
       0000000000000004  0000000000000000  WA       0     0     4
  [ 5] .rodata           PROGBITS         0000000000000000  000000ac
       0000000000000004  0000000000000000   A       0     0     1
  [ 6] .comment          PROGBITS         0000000000000000  000000b0
       0000000000000027  0000000000000001  MS       0     0     1
  [ 7] .note.GNU-stack   PROGBITS         0000000000000000  000000d7
       0000000000000000  0000000000000000           0     0     1
  [ 8] .note.gnu.pr[...] NOTE             0000000000000000  000000d8
       0000000000000020  0000000000000000   A       0     0     8
  [ 9] .eh_frame         PROGBITS         0000000000000000  000000f8
       0000000000000058  0000000000000000   A       0     0     8
  [10] .rela.eh_frame    RELA             0000000000000000  00000360
       0000000000000030  0000000000000018   I      11     9     8
  [11] .symtab           SYMTAB           0000000000000000  00000150
       0000000000000138  0000000000000018          12     8     8
  [12] .strtab           STRTAB           0000000000000000  00000288
       0000000000000060  0000000000000000           0     0     1
  [13] .shstrtab         STRTAB           0000000000000000  00000390
       0000000000000074  0000000000000000           0     0     1
Key to Flags:
  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
  L (link order), O (extra OS processing required), G (group), T (TLS),
  C (compressed), x (unknown), o (OS specific), E (exclude),
  D (mbind), l (large), p (processor specific)
```

> [!tip]
> 段表的结构以Elf64_Shdr结构的数组形式进行存储,数组元素个数为段个数，每个Elf64_Shdr对应一个段属性描述。所以每个Elf64_Shdr又被称为段描述符。
> 第一个元素是无效段描述符，它的类型为"NULL"，除此之外每个描述符对应一个段。所以SimpleSection.o共有10个有效段。

#### 4.2.1 段描述符的结构
```c
// /usr/include/elf.h
typedef struct
{
  Elf64_Word	sh_name;		/* Section name (string tbl index) */
  Elf64_Word	sh_type;		/* Section type */
  Elf64_Xword	sh_flags;		/* Section flags */
  Elf64_Addr	sh_addr;		/* Section virtual addr at execution */
  Elf64_Off	sh_offset;		/* Section file offset */
  Elf64_Xword	sh_size;		/* Section size in bytes */
  Elf64_Word	sh_link;		/* Link to another section */
  Elf64_Word	sh_info;		/* Additional section information */
  Elf64_Xword	sh_addralign;		/* Section alignment */
  Elf64_Xword	sh_entsize;		/* Entry size if section holds table */
} Elf64_Shdr;
```
![Elf64_Shdr](/image/chapter02/Elf64_Shdr结构含义.png)
段表结构解析
![段结构解析](/image/chapter02/段结构分析.png)
> [!note]
> elf文件头中指出了段表位于文件的偏移(Elf64_Ehdr的e_shoff元素)为1032(0x408)，代码段(.text)位于elf文件头后所以elf文件头的大小正好为代码段的偏移0x40(64)字节，段表的最后一个元素(.shstrtab)的偏移量为0x390文件大小为0x74。而0x408 = 0x390 + 0x74 + 0x04正好是段表在elf文件中的偏移(之所以+0x04是因为内存对齐的原因)。

##### 4.2.1.1 段的类型(sh_type)
> [!tip]
> 对于编译器和链接器来说，主要决定段的属性的是段的类型(sh_type)和段的标志位(sh_flags)

![段的类型](/image/chapter02/段的类型.png)
##### 4.2.1.2 段的标志位(sh_flags)
> [!tip]
> 决定了该段在进程虚拟地址空间中的属性，比如是否可写，是否可执行等。

![段的标志位](/image/chapter02/段的标志位.png)
##### 4.2.1.3 段的链接信息(sh_link、sh_info)
> [!tip]
> 只有段的类型是与链接(动态链接、静态链接、重定位表、符号表等)相关时这两个成员才会有意义。

![段的链接信息](/image/chapter02/段的链接信息.png)
### 4.3 重定位表
> [!tip]
> 在SimpleSection.o中有一个叫做".rela.text"的段, 它的类型(sh_type)为"SH_RELA", 也就是说它是一个重定位表。
> 对于每个需要重定位的段，都会有一个相应的重定位表。比如".rela.text"是".text"的重定位表。
> 一个重定位表同时也是elf的一个段, 那么这个段的类型(sh_type)就是"SHL_REL", 它的"sh_link"表示符号表的下标, "sh_info"表示它作用于那个段。

![重定位表](/image/chapter02/重定位表.png)
### 4.4 字符串表
> [!tip]
> elf文件中用到了很多字符串，比如段名、变量名等。因为字符串的长度往往是不定的，所以用固定的结构来表示它比较困难。一种常见的做法是把字符串集中起来存放到一个表，然后使用字符串在表中的偏移来引用字符串。
> .strtab: 字符串表(string table)
> .shstrtab: 段表字符串表(section header string table)

![字符串表](/image/chapter02/字符串表.png)
![段表字符串表](/image/chapter02/段表字符串表分析.png)
## 5. 链接的接口-符号(Symbol)
> [!tip]
> 链接的过程就是把多个不同的目标文件之间相互衔接的过程。这个相互衔接的过程实际上是目标文件之间对地址的引用。
> 比如文件B用到了目标A中的函数"foo", 那么我们就称文件A`定义(define)`了函数"foo", 称目标文件B`引用(refrence)`了目标文件A中的函数"foo"。这两个概念同样适用于变量。
> 在链接中将函数和变量称为`符号(Symbol)`, 函数名和变量名称为`符号名(Symbol Name)`
> `符号是链接中的粘合剂`, 整个链接的过程基于符号才能正确完成。
> 每一个目标文件都有一个相应的`符号表(Symbol Table)`对符号进行管理, 该表记录了目标文件中所用的所有符号。每个定义的符号都有一个对应的值, 叫做`符号值(Symbol Value)`, 对于`变量和函数`来说, 符号值就是它们的`地址`。除了函数和变量外，还存在其它几种不常用的符号。

- 定义在目标文件的全局符号，可以被其它目标文件引用
- 在本文件中引用的全局符号，却没有在本文件中定义，一般叫做`外部符号(External Symbol)`，也被称为符号引用
- 段名，由编译器生成，它的值就是该段的起始地址。(.text, .data)
- 局部符号，只在编译器内部可见。(SimpleSection.o文件的static_var和static_var2)
- 行号信息，即目标文件指令与源代码中代码行的对应关系，可选。

使用nm指令查看符号结果
```bash
$> nm SimpleSection.o
0000000000000000 T func1
0000000000000000 D global_init_var
0000000000000004 C global_uninit_var
000000000000002b T main
                 U printf
0000000000000004 d static_var.1
0000000000000000 b static_var2.0
```
### 5.1 ELF符号表结构
```c
// /usr/include/elf.h
typedef struct
{
  Elf64_Word	st_name;		/* Symbol name (string tbl index) */
  unsigned char	st_info;		/* Symbol type and binding */
  unsigned char st_other;		/* Symbol visibility */
  Elf64_Section	st_shndx;		/* Section index */
  Elf64_Addr	st_value;		/* Symbol value */
  Elf64_Xword	st_size;		/* Symbol size */
} Elf64_Sym;
```
![ELF符号表结构](/image/chapter02/ELF符号表结构.png)
#### 5.1.1 符号类型和绑定类型(st_info)
> [!tip]
> 低4位表示`符号类型(Symbol Type)`, 高28位表示`符号绑定信息(Symbol Binding)`

![符号类型和绑定](/image/chapter02/符号类型和绑定.png)
#### 5.1.2 符号所在段(st_shndx)
> [!tip]
> 如果符号定义在本目标文件中，那么这个成员表示符号所在的段在段表中的下标，如果符号不是定义在本目标文件中，或者对于有些特殊符号，sh_shndx的值会有些特殊。

![符号所在段](/image/chapter02/符号所在段.png)
#### 5.1.3 符号值(st_value)
> [!tip]
> 每个符号都有一个对应的值，如果这个符号是一个函数或变量的定义，那么符号的值就是这个函数或变量的地址。

- 在目标文件中，如果是符号的定义并且该符号不是`"COMMON块"类型`的，则st_value表示`该符号在段中的偏移。`比如SimpleSection.o中的"func1"、"main"、"global_init_var"。
- 在目标文件中，如果符号是"COMMON块"，st_value表示该符号的对齐属性。比如SimpleSection.o中的"global_uninit_var"。
- 在可执行文件中，st_value表示符号的虚拟地址。
使用readelf -s查看符号表
```bash
# 注意编译时使用了-fcommon编译选项
$> readelf -s SimpleSection.o

Symbol table '.symtab' contains 13 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND
     1: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS SimpleSection.c
     2: 0000000000000000     0 SECTION LOCAL  DEFAULT    1 .text
     3: 0000000000000000     0 SECTION LOCAL  DEFAULT    3 .data
     4: 0000000000000000     0 SECTION LOCAL  DEFAULT    4 .bss
     5: 0000000000000000     0 SECTION LOCAL  DEFAULT    5 .rodata
     6: 0000000000000004     4 OBJECT  LOCAL  DEFAULT    3 static_var.1
     7: 0000000000000000     4 OBJECT  LOCAL  DEFAULT    4 static_var2.0
     8: 0000000000000000     4 OBJECT  GLOBAL DEFAULT    3 global_init_var
     9: 0000000000000004     4 OBJECT  GLOBAL DEFAULT  COM global_uninit_var
    10: 0000000000000000    43 FUNC    GLOBAL DEFAULT    1 func1
    11: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND printf
    12: 000000000000002b    57 FUNC    GLOBAL DEFAULT    1 main
```
### 5.2 特殊符号