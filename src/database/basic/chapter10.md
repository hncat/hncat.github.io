---
title: 第十章 汇总数据
date: 2025-08-24
tags:
    - 计算机基础
    - 数据库
categories: mysql
isOriginal: true
order: 11
dir:
    order: 11
---
## 10.1 聚集函数
|函数|说明|
|:-:|:-:|
|AVG()|返回某列的平均值|
|COUNT()|返回某列的行数|
|MAX()|返回某列的最大值|
|MIN()|返回某列的最小值|
|SUM()|返回某列值之和|
### 10.1.1 AVG()函数
> [!important]
> AVG()通过对表中行数计数并计算特定列值之和，求得该列的平均值。AVG()可用来返回所有列的平均值，也可以用来返回特定列或行的平均值。

```sql
select avg(prod_price) as avg_price from products;
+-----------+
| avg_price |
+-----------+
| 16.133571 |
+-----------+
1 row in set (0.06 sec)

-- AVG()也可以用来确定特定列或行的平均值。下面的例子返回特定供应商所提供产品的平均价格：
select avg(prod_price) as avg_price from products where vend_id = 1003;
+-----------+
| avg_price |
+-----------+
| 13.212857 |
+-----------+
1 row in set (0.01 sec)

```
> [!warning]
> 只用于单个列 AVG()只能用来确定特定数值列的平均值，而且列名必须作为函数参数给出。为了获得多个列的平均值，必须使用多个AVG()函数。

> [!note]
> NULL值 AVG()函数忽略列值为NULL的行。

### 10.1.2 COUNT()函数
> [!important]
> COUNT()函数进行计数。可利用COUNT()确定表中行的数目或符合特定条件的行的数目。
> 1. 使用COUNT(*)对表中行的数目进行计数，不管表列中包含的是空值（NULL）还是非空值。
> 2. 使用COUNT(column)对特定列中具有值的行进行计数，忽略NULL值。

```sql
select count(*) as num_cust from customers;
+----------+
| num_cust |
+----------+
|        5 |
+----------+
1 row in set (0.05 sec)

select count(cust_email) as num_cust from customers;
+----------+
| num_cust |
+----------+
|        3 |
+----------+
1 row in set (0.00 sec)

```
> [!warning]
> NULL值 如果指定列名，则指定列的值为空的行被COUNT()函数忽略，但如果COUNT()函数中用的是星号（*），则不忽略。

### 10.1.3 MAX()函数
> [!important]
> MAX()返回指定列中的最大值，MAX()要求指定列名。

```sql
-- MAX()返回products表中最贵的物品的价格。
select max(prod_price) as max_price from products;
+-----------+
| max_price |
+-----------+
|     55.00 |
+-----------+
1 row in set (0.00 sec)

```
> [!warning]
> 对非数值数据使用MAX() 虽然MAX()一般用来找出最大的数值或日期值，但MySQL允许将它用来返回任意列中的最大值，包括返回文本列中的最大值。在用于文本数据时，如果数据按相应的列排序，则MAX()返回最后一行。

> [!note]
> NULL值 MAX()函数忽略列值为NULL的行。

### 10.1.4 MIN()函数
> [!important]
> MIN()的功能正好与MAX()功能相反，它返回指定列的最小值。与MAX()一样，MIN()要求指定列名，

```sql
-- 其中MIN()返回products表中最便宜物品的价格
select min(prod_price) as max_prica from products;
+-----------+
| max_prica |
+-----------+
|      2.50 |
+-----------+
1 row in set (0.00 sec)

```
> [!warning]
> 对非数值数据使用MIN() MIN()函数与MAX()函数类似，MySQL允许将它用来返回任意列中的最小值，包括返回文本列中的最小值。在用于文本数据时，如果数据按相应的列排序，则MIN()返回最前面的行。

> [!note]
> NULL值 MIN()函数忽略列值为NULL的行。

### 10.1.5 SUM()函数
> [!important]
> SUM()用来返回指定列值的和（总计）。

```sql
-- 检索所订购物品的总数（所有quantity值之和）
select sum(quantity) as items_ordered from orderitems where order_num = 20005;
+---------------+
| items_ordered |
+---------------+
|            19 |
+---------------+
1 row in set (0.00 sec)

-- 合计每项物品的item_price*quantity，得出总的订单金额
select sum(item_price * quantity) as total_price from orderitems where order_num = 20005;
+-------------+
| total_price |
+-------------+
|      149.87 |
+-------------+
1 row in set (0.00 sec)

```
> [!note]
> NULL值 SUM()函数忽略列值为NULL的行。

## 10.2 聚集不同的值
> [!important]
> 以上5个聚集函数都可以如下使用：
> 1. 对所有的行执行计算，指定ALL参数或不给参数（因为ALL是默认行为）；
> 2. 只包含不同的值，指定DISTINCT参数。

```sql
select avg(distinct prod_price) as avg_price from products where vend_id = 1003;
+-----------+
| avg_price |
+-----------+
| 15.998000 |
+-----------+
1 row in set (0.00 sec)

```

## 10.3 组合聚集函数
```sql
select count(*) as num_items, min(prod_price) as price_min, max(prod_price) as price_max, avg(prod_price) as price_avg from products;
+-----------+-----------+-----------+-----------+
| num_items | price_min | price_max | price_avg |
+-----------+-----------+-----------+-----------+
|        14 |      2.50 |     55.00 | 16.133571 |
+-----------+-----------+-----------+-----------+
1 row in set (0.02 sec)

```