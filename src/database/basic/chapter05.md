---
title: 第五章 数据过滤
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
## 5.1 组合where语句
### 5.1.1 and操作符
```sql
-- 查找由供应商1003制造并且价格小于10$的所有产品的价格和名称。
select prod_id, prod_price, prod_name from products where vend_id = 1003 and prod_price <= 10;
+---------+------------+----------------+
| prod_id | prod_price | prod_name      |
+---------+------------+----------------+
| FB      |      10.00 | Bird seed      |
| FC      |       2.50 | Carrots        |
| SLING   |       4.49 | Sling          |
| TNT1    |       2.50 | TNT (1 stick)  |
| TNT2    |      10.00 | TNT (5 sticks) |
+---------+------------+----------------+
5 rows in set (0.09 sec)

```
### 5.1.2 or操作符
```sql
-- 查找由供应商1003或者1002制造的所有产品的价格和名称。
select vend_id, prod_id, prod_price, prod_name from products where vend_id = 1003 or vend_id = 1002;
+---------+---------+------------+----------------+
| vend_id | prod_id | prod_price | prod_name      |
+---------+---------+------------+----------------+
|    1002 | FU1     |       3.42 | Fuses          |
|    1002 | OL1     |       8.99 | Oil can        |
|    1003 | DTNTR   |      13.00 | Detonator      |
|    1003 | FB      |      10.00 | Bird seed      |
|    1003 | FC      |       2.50 | Carrots        |
|    1003 | SAFE    |      50.00 | Safe           |
|    1003 | SLING   |       4.49 | Sling          |
|    1003 | TNT1    |       2.50 | TNT (1 stick)  |
|    1003 | TNT2    |      10.00 | TNT (5 sticks) |
+---------+---------+------------+----------------+
9 rows in set (0.05 sec)

```
### 5.1.3 计算次序
```sql
-- 要列出价格为10美元（含）以上且由1002或1003制造的所有产品。
select vend_id, prod_id, prod_price, prod_name from products where vend_id = 1003 or vend_id = 1002 and prod_price >= 10;
+---------+---------+------------+----------------+
| vend_id | prod_id | prod_price | prod_name      |
+---------+---------+------------+----------------+
|    1003 | DTNTR   |      13.00 | Detonator      |
|    1003 | FB      |      10.00 | Bird seed      |
|    1003 | FC      |       2.50 | Carrots        |
|    1003 | SAFE    |      50.00 | Safe           |
|    1003 | SLING   |       4.49 | Sling          |
|    1003 | TNT1    |       2.50 | TNT (1 stick)  |
|    1003 | TNT2    |      10.00 | TNT (5 sticks) |
+---------+---------+------------+----------------+
7 rows in set (0.00 sec)

-- 这里已经出问题了因为只限制了1002商家的产品要大于等于10$，和大多数编程语言一样sql也有计算次序问题，所以解决办法如下
select vend_id, prod_id, prod_price, prod_name from products where (vend_id = 1003 or vend_id = 1002) and prod_price >= 10;
+---------+---------+------------+----------------+
| vend_id | prod_id | prod_price | prod_name      |
+---------+---------+------------+----------------+
|    1003 | DTNTR   |      13.00 | Detonator      |
|    1003 | FB      |      10.00 | Bird seed      |
|    1003 | SAFE    |      50.00 | Safe           |
|    1003 | TNT2    |      10.00 | TNT (5 sticks) |
+---------+---------+------------+----------------+
4 rows in set (0.00 sec)

```
## 5.2 in操作符
> [!important]
> IN操作符用来指定条件范围，范围中的每个条件都可以进行匹配。IN取合法值的由逗号分隔的清单，全都括在圆括号中。

```sql
-- 列出由厂商1002，1003生产的所有商品并对商品名称进行升序排序。
select prod_name, prod_price from products where vend_id in(1002, 1003) order by prod_name;
+----------------+------------+
| prod_name      | prod_price |
+----------------+------------+
| Bird seed      |      10.00 |
| Carrots        |       2.50 |
| Detonator      |      13.00 |
| Fuses          |       3.42 |
| Oil can        |       8.99 |
| Safe           |      50.00 |
| Sling          |       4.49 |
| TNT (1 stick)  |       2.50 |
| TNT (5 sticks) |      10.00 |
+----------------+------------+
9 rows in set (0.00 sec)

```
> [!tip]
> IN WHERE子句中用来指定要匹配值的清单的关键字，功能与OR相当。

## 5.3 not操作符
```sql
-- 列出除1002和1003之外的所有供应商制造的产品
select vend_id, prod_name, prod_price from products where vend_id not in(1002, 1003) order by prod_name;
+---------+--------------+------------+
| vend_id | prod_name    | prod_price |
+---------+--------------+------------+
|    1001 | .5 ton anvil |       5.99 |
|    1001 | 1 ton anvil  |       9.99 |
|    1001 | 2 ton anvil  |      14.99 |
|    1005 | JetPack 1000 |      35.00 |
|    1005 | JetPack 2000 |      55.00 |
+---------+--------------+------------+
5 rows in set (0.01 sec)

-- 另一种写法
select vend_id, prod_name, prod_price from products where not vend_id = 1002 and not vend_id = 1003 order by pr
od_name;
+---------+--------------+------------+
| vend_id | prod_name    | prod_price |
+---------+--------------+------------+
|    1001 | .5 ton anvil |       5.99 |
|    1001 | 1 ton anvil  |       9.99 |
|    1001 | 2 ton anvil  |      14.99 |
|    1005 | JetPack 1000 |      35.00 |
|    1005 | JetPack 2000 |      55.00 |
+---------+--------------+------------+
5 rows in set (0.01 sec)

```