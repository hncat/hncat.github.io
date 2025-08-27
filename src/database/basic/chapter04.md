---
title: 第四章 过滤数据
date: 2025-08-24
tags:
    - 计算机基础
    - 数据库
categories: mysql
isOriginal: true
order: 5
dir:
    order: 5
---
## 4.1 使用where子句
```sql
select prod_name, prod_price from products where prod_price = 2.50;
+---------------+------------+
| prod_name     | prod_price |
+---------------+------------+
| Carrots       |       2.50 |
| TNT (1 stick) |       2.50 |
+---------------+------------+
2 rows in set (0.08 sec)

```
> [!tip]
> 在同时使用order by和where子句时，应该让order by位于where之后。
> 关于order by的使用请查看[这里](chapter03.md)

## 4.2 where子句操作符
| 操作符 | 说明 |
| :---: | :---: |
| = | 等于 |
| <> | 不等于 |
|!= | 不等于 |
| < | 小于 |
| <= | 小于等于 |
| > | 大于 |
| >= | 大于等于 |
| between | 在指定的两个值之间 |

### 4.2.1 检查单个值
```sql
-- 查找prod_name='fuses'的商品
select prod_name, prod_price from products where prod_name = 'fuses';
+-----------+------------+
| prod_name | prod_price |
+-----------+------------+
| Fuses     |       3.42 |
+-----------+------------+
1 row in set (0.08 sec)

```
> [!tip]
> 需要注意的是mysql在只想匹配时默认是不区分大小写的，所以fuses匹配了Fuses

```sql
-- 列出所有小于10$的商品
select prod_name, prod_price from products where prod_price < 10;
+---------------+------------+
| prod_name     | prod_price |
+---------------+------------+
| .5 ton anvil  |       5.99 |
| 1 ton anvil   |       9.99 |
| Carrots       |       2.50 |
| Fuses         |       3.42 |
| Oil can       |       8.99 |
| Sling         |       4.49 |
| TNT (1 stick) |       2.50 |
+---------------+------------+
7 rows in set (0.00 sec)

-- 列出所有小于等于10$的商品，并对其进行排序
select prod_name, prod_price from products where prod_price <= 10 order by prod_price;
+----------------+------------+
| prod_name      | prod_price |
+----------------+------------+
| Carrots        |       2.50 |
| TNT (1 stick)  |       2.50 |
| Fuses          |       3.42 |
| Sling          |       4.49 |
| .5 ton anvil   |       5.99 |
| Oil can        |       8.99 |
| 1 ton anvil    |       9.99 |
| Bird seed      |      10.00 |
| TNT (5 sticks) |      10.00 |
+----------------+------------+
9 rows in set (0.00 sec)

```

### 4.2.2 不匹配检查
```sql
-- 查找所有vend_id不等于1003的商品，<>和!=效果一样
select vend_id, prod_name from products where vend_id <> 1003;
+---------+--------------+
| vend_id | prod_name    |
+---------+--------------+
|    1001 | .5 ton anvil |
|    1001 | 1 ton anvil  |
|    1001 | 2 ton anvil  |
|    1002 | Fuses        |
|    1002 | Oil can      |
|    1005 | JetPack 1000 |
|    1005 | JetPack 2000 |
+---------+--------------+
7 rows in set (0.03 sec)

```
> [!tip]
> 单引号和双引号都是用于限定字符串的，与数值列进行比较的值不用引号。

### 4.2.3 范围检查
```sql
-- 查找所有价格在5$到10$之间的所有产品
select prod_name, prod_price from products where prod_price between 5 and 10;
+----------------+------------+
| prod_name      | prod_price |
+----------------+------------+
| .5 ton anvil   |       5.99 |
| 1 ton anvil    |       9.99 |
| Bird seed      |      10.00 |
| Oil can        |       8.99 |
| TNT (5 sticks) |      10.00 |
+----------------+------------+
5 rows in set (0.01 sec)

-- 效果一样
select prod_name, prod_price from products where prod_price >= 5 and prod_price <= 10;
+----------------+------------+
| prod_name      | prod_price |
+----------------+------------+
| .5 ton anvil   |       5.99 |
| 1 ton anvil    |       9.99 |
| Bird seed      |      10.00 |
| Oil can        |       8.99 |
| TNT (5 sticks) |      10.00 |
+----------------+------------+
5 rows in set (0.00 sec)

```
### 4.2.4 空值检查
> [!tip]
> 什么是空值: 在创建表时，表设计人员可以指定其中的列是否可以不包含值。在一个列不包含值时，称其为包含空值NULL。

```sql
select prod_name from products where prod_price is null;
Empty set (0.00 sec)

-- 只能使用is null语句，不能使用= null这种语句
select cust_id from customers where cust_email is null;
+---------+
| cust_id |
+---------+
|   10002 |
|   10005 |
+---------+
2 rows in set (0.04 sec)

```
> [!important]
> NULL与不匹配: 在通过过滤选择出不具有特定值的行时，你可能希望返回具有NULL值的行。但是，不行。因为未知具有特殊的含义，数据库不知道它们是否匹配，所以在匹配过滤或不匹配过滤时不返回它们。因此，在过滤数据时，一定要验证返回数据中确实给出了被过滤列具有NULL的行。