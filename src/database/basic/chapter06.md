---
title: 第六章 用通配符进行过滤
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
## 6.1 like操作符
> [!tip]
> 通配符: 用来匹配值的一部分的特殊字符。
> 搜索模式: 由字面值、通配符或两者组合构成的搜索条件。

> 为在搜索子句中使用通配符，必须使用LIKE操作符。LIKE指示MySQL，后跟的搜索模式利用通配符匹配而不是直接相等匹配进行比较。

### 6.1.1 百分号(%)通配符
```sql
-- 找出所有以词jet起头的产品
select prod_id, prod_name from products where prod_name like 'jet%';
+---------+--------------+
| prod_id | prod_name    |
+---------+--------------+
| JP1000  | JetPack 1000 |
| JP2000  | JetPack 2000 |
+---------+--------------+
2 rows in set (0.08 sec)

-- 匹配任何位置包含文本anvil的值
select prod_id, prod_name from products where prod_name like '%anvil%';
+---------+--------------+
| prod_id | prod_name    |
+---------+--------------+
| ANV01   | .5 ton anvil |
| ANV02   | 1 ton anvil  |
| ANV03   | 2 ton anvil  |
+---------+--------------+
3 rows in set (0.00 sec)

```

> [!important]
> 在搜索串中，%表示任何字符出现任意次数。

注意事项: 
> [!note]
> 区分大小写 根据MySQL的配置方式，搜索可以是区分大小写的。如果区分大小写，'jet%'与JetPack 1000将不匹配。

> [!note]
> 注意尾空格 尾空格可能会干扰通配符匹配。例如，在保存词anvil 时，如果它后面有一个或多个空格，则子句WHERE prod_name LIKE '%anvil'将不会匹配它们，因为在最后的l后有多余的字符。解决这个问题的一个简单的办法是在搜索模式最后附加一个%。一个更好的办法是使用函数去掉首尾空格。

> [!note]
> 注意NULL 虽然似乎%通配符可以匹配任何东西，但有一个例外，即NULL。即使是WHERE prod_name LIKE '%'也不能匹配用值NULL作为产品名的行。

### 6.1.2 下划线(_)通配符
> [!important]
> 下划线的用途与%一样，但下划线只匹配单个字符而不是多个字符。

```sql
-- 使用%匹配
select prod_id, prod_name from products where prod_name like '_ ton anvil';
+---------+-------------+
| prod_id | prod_name   |
+---------+-------------+
| ANV02   | 1 ton anvil |
| ANV03   | 2 ton anvil |
+---------+-------------+
2 rows in set (0.01 sec)

-- 使用_匹配
select prod_id, prod_name from products where prod_name like '% ton anvil';
+---------+--------------+
| prod_id | prod_name    |
+---------+--------------+
| ANV01   | .5 ton anvil |
| ANV02   | 1 ton anvil  |
| ANV03   | 2 ton anvil  |
+---------+--------------+
3 rows in set (0.00 sec)

```
## 6.2 使用通配符的技巧
> [!tip]
> * 不要过度使用通配符。如果其他操作符能达到相同的目的，应该使用其他操作符。
> * 在确实需要使用通配符时，除非绝对有必要，否则不要把它们用在搜索模式的开始处。把通配符置于搜索模式的开始处，搜索起来是最慢的。
> * 仔细注意通配符的位置。如果放错地方，可能不会返回想要的数据。