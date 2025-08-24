---
title: 第七章 用正则表达式进行搜索
date: 2025-08-24
tags:
    - 计算机基础
    - 数据库
categories: mysql
isOriginal: true
order: 7
dir:
    order: 7
---
## 7.1 使用mysql正则表达式
### 7.1.1 基本字符匹配
```sql
-- 检索列prod_name包含文本1000的所有行
select prod_name from products where prod_name regexp '1000' order by prod_name;
+--------------+
| prod_name    |
+--------------+
| JetPack 1000 |
+--------------+
1 row in set (0.10 sec)

```

> [!tip]
> '.'是正则表达式语言中一个特殊的字符。它表示匹配任意字符。

```sql
select prod_name from products where prod_name regexp '.000' order by prod_name;
+--------------+
| prod_name    |
+--------------+
| JetPack 1000 |
| JetPack 2000 |
+--------------+
2 rows in set (0.00 sec)

```

> [!note]
> 匹配不区分大小写 MySQL中的正则表达式匹配（自版本3.23.4后）不区分大小写（即，大写和小写都匹配）。为区分大小写，可使用BINARY关键字，如WHERE prod_name REGEXP BINARY 'JetPack .000'。

### 7.1.2 进行or匹配
> [!tip]
> 为搜索两个串之一（或者为这个串，或者为另一个串），使用|。

> 两个以上的OR条件 可以给出两个以上的OR条件。例如，'1000 | 2000 | 3000'将匹配1000或2000或3000。

```sql
select prod_name from products where prod_name regexp '1000|2000' order by prod_name;
+--------------+
| prod_name    |
+--------------+
| JetPack 1000 |
| JetPack 2000 |
+--------------+
2 rows in set (0.00 sec)

```
### 7.1.3 匹配几个字符之一
> [!tip]
> 通过指定一组用\[和\]括起来的字符来完成

```sql
select prod_name from products where prod_name regexp '[123] ton' order by prod_name;
+-------------+
| prod_name   |
+-------------+
| 1 ton anvil |
| 2 ton anvil |
+-------------+
2 rows in set (0.00 sec)

```
> [!note]
> 正则表达式[123]Ton为[1|2|3]Ton的缩写，也可以使用后者。
> 使用[^123]匹配除这些字符之外的所有字符

```sql
select prod_name from products where prod_name regexp '[^123] ton' order by prod_name;
+--------------+
| prod_name    |
+--------------+
| .5 ton anvil |
+--------------+
1 row in set (0.01 sec)

```

### 7.1.4 匹配范围
> [!note]
> 集合可用来定义要匹配的一个或多个字符。例如，下面的集合将匹配数字0到9：[0123456789]
> 为简化这种类型的集合，可使用-来定义一个范围。下面的式子功能上等同于上述数字列表：[0-9]
> 范围不限于完整的集合，[1-3]和[6-9]也是合法的范围。此外，范围不一定只是数值的，[a-z]匹配任意字母字符。

```sql
-- 使用正则表达式[1-5] Ton
select prod_name from products where prod_name regexp '[1-5] Ton' order by prod_name;
+--------------+
| prod_name    |
+--------------+
| .5 ton anvil |
| 1 ton anvil  |
| 2 ton anvil  |
+--------------+
3 rows in set (0.08 sec)

```

### 7.1.5 匹配特殊字符
> [!tip]
> 为了匹配特殊字符，必须用\\为前导。\\-表示查找-，\\.表示查找.。

```sql
select vend_name from vendors where vend_name regexp '.' order by vend_name;
+----------------+
| vend_name      |
+----------------+
| ACME           |
| Anvils R Us    |
| Furball Inc.   |
| Jet Set        |
| Jouets Et Ours |
| LT Supplies    |
+----------------+
6 rows in set (0.01 sec)

-- 使用\\进行转译
select vend_name from vendors where vend_name regexp '\\.' order by vend_name;
+--------------+
| vend_name    |
+--------------+
| Furball Inc. |
+--------------+
1 row in set (0.00 sec)

```

### 7.1.6 匹配字符类
|类|说明|
|:-:|:-:|
|[:alnum:]|任意字母和数字([a-zA-Z0-9])|
|[:alpha:]|任意符([a-zA-Z])|
|[:blank:]|空格和制表([\\t])|
|[:cntrl:]|ASCII控制字符(ASCII 0到31和127)|
|[:digit:]|任意数字([0-9])|
|[:graph:]|与[:print:]相同，但不包括空格|
|[:lower:]|任意小写字母([a-z])|
|[:print:]|任意可打印字符|
|[:punct:]|既不在[:alnum:]又不在[:cntrl:]中的任意字符|
|[:space:]|包括空格在内的任意空白字符([\\f\\n\\r\\t\\v])|
|[:upper:]|任意大写字母([A-Z])|
|[:xdigit:]|任意十六进制数字(同[a-fA-F0-9])|

### 7.1.7 匹配多个实例

|元字符|说明|
|:-:|:-:|
|*|0个或多个匹配|
|+|1个或多个匹配(等于{1,})|
|?|0个或1个匹配(等于{0,1})|
|\{n\}|指定数目的匹配|
|\{n,\}|不少于指定数目的匹配|
|\{n,m\}|匹配数目的范围(m不超过255)|

```sql
-- \\(匹配)，[0-9]匹配任意数字（这个例子中为1和5），sticks?匹配stick和sticks（s后的?使s可选，因为?匹配它前面的任何字符的0次或1次出现），\\)匹配)。没有?，匹配stick和sticks会非常困难。
select prod_name from products where prod_name regexp '\\([0-9] sticks?\\)' order by prod_name;
+----------------+
| prod_name      |
+----------------+
| TNT (1 stick)  |
| TNT (5 sticks) |
+----------------+
2 rows in set (0.09 sec)

-- [:digit:]匹配任意数字，因而它为数字的一个集合。{4}确切地要求它前面的字符（任意数字）出现4次，所以[[:digit:]]{4}匹配连在一起的任意4位数字。
select prod_name from products where prod_name regexp '[[:digit:]]{4}' order by prod_name;
+--------------+
| prod_name    |
+--------------+
| JetPack 1000 |
| JetPack 2000 |
+--------------+
2 rows in set (0.00 sec)

```

### 7.1.8 定位符
|元字符|说明|
|:-:|:-:|
|^|文本的开始|
|$|文本的结尾|
|[[:<:]]|词的开始|
|[[:>:]]|词的结尾|

```sql
-- ^匹配串的开始。因此，^[0-9\\.]只在.或任意数字为串中第一个字符时才匹配它们。
select prod_name from products where prod_name regexp '^[0-9\\.]' order by prod_name;
+--------------+
| prod_name    |
+--------------+
| .5 ton anvil |
| 1 ton anvil  |
| 2 ton anvil  |
+--------------+
3 rows in set (0.06 sec)

```

> [!note]
> 简单的正则表达式测试 可以在不使用数据库表的情况下用SELECT来测试正则表达式。REGEXP检查总是返回0（没有匹配）或1（匹配）。可以用带文字串的REGEXP来测试表达式，并试验它们。相应的语法如下：这个例子显然将返回0（因为文本hello中没有数字）。
