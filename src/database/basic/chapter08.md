---
title: 第八章 创建计算字段
date: 2025-08-24
tags:
    - 计算机基础
    - 数据库
categories: mysql
isOriginal: true
order: 9
dir:
    order: 9
---
## 8.1 拼接字段
> [!note]
> 拼接（concatenate） 将值联结到一起构成单个值。
> 在MySQL的SELECT语句中，可使用concat()函数来拼接两个列。

```sql
-- 使用concat拼接
select concat(vend_name, '(', vend_country, ')') from vendors order by vend_name;
+-------------------------------------------+
| concat(vend_name, '(', vend_country, ')') |
+-------------------------------------------+
| ACME(USA)                                 |
| Anvils R Us(USA)                          |
| Furball Inc.(USA)                         |
| Jet Set(England)                          |
| Jouets Et Ours(France)                    |
| LT Supplies(USA)                          |
+-------------------------------------------+
6 rows in set (0.07 sec)

```
> [!tip]
> 使用rtrim移除右边的空格
> ltrim移除左边的空格
> trim移除左右两边的空格

```sql
select concat(rtrim(vend_name), '(', rtrim(vend_country), ')') from vendors order by vend_name;
+---------------------------------------------------------+
| concat(rtrim(vend_name), '(', rtrim(vend_country), ')') |
+---------------------------------------------------------+
| ACME(USA)                                               |
| Anvils R Us(USA)                                        |
| Furball Inc.(USA)                                       |
| Jet Set(England)                                        |
| Jouets Et Ours(France)                                  |
| LT Supplies(USA)                                        |
+---------------------------------------------------------+
6 rows in set (0.05 sec)

```
### 8.1.1 使用别名(alias)
```sql
select concat(rtrim(vend_name), '(', rtrim(vend_country), ')') as vend_title from vendors order by vend_name;
+------------------------+
| vend_title             |
+------------------------+
| ACME(USA)              |
| Anvils R Us(USA)       |
| Furball Inc.(USA)      |
| Jet Set(England)       |
| Jouets Et Ours(France) |
| LT Supplies(USA)       |
+------------------------+
6 rows in set (0.04 sec)

```
## 8.2  执行算术计算
```sql
-- orders表包含收到的所有订单，orderitems表包含每个订单中的各项物品。
select prod_id, quantity, item_price from orderitems where order_num = 20005;
+---------+----------+------------+
| prod_id | quantity | item_price |
+---------+----------+------------+
| ANV01   |       10 |       5.99 |
| ANV02   |        3 |       9.99 |
| TNT2    |        5 |      10.00 |
| FB      |        1 |      10.00 |
+---------+----------+------------+
4 rows in set (0.04 sec)

-- item_price列包含订单中每项物品的单价。如下汇总物品的价格（单价乘以订购数量）：
select prod_id, quantity, item_price, quantity * item_price as expanded_price
from orderitems
where order_num = 20005;
+---------+----------+------------+----------------+
| prod_id | quantity | item_price | expanded_price |
+---------+----------+------------+----------------+
| ANV01   |       10 |       5.99 |          59.90 |
| ANV02   |        3 |       9.99 |          29.97 |
| TNT2    |        5 |      10.00 |          50.00 |
| FB      |        1 |      10.00 |          10.00 |
+---------+----------+------------+----------------+
4 rows in set (0.00 sec)

```

|操作符|说明|
|:-:|:-:|
|+|加|
|-|减|
|*|乘|
|/|除|