---
title: 第三章 排序查询数据
date: 2025-08-23
tags:
    - 计算机基础
    - 数据库
categories: mysql
isOriginal: true
order: 4
dir:
    order: 4
---
## 3.1 排序数据
```sql
select prod_name from products order by prod_name;
+----------------+
| prod_name      |
+----------------+
| .5 ton anvil   |
| 1 ton anvil    |
| 2 ton anvil    |
| Bird seed      |
| Carrots        |
| Detonator      |
| Fuses          |
| JetPack 1000   |
| JetPack 2000   |
| Oil can        |
| Safe           |
| Sling          |
| TNT (1 stick)  |
| TNT (5 sticks) |
+----------------+
14 rows in set (0.00 sec)

```
> [!important]
> order by语句的默认是升序排列的

## 3.2 按多个列排序
```sql
-- 在多个行具有相同的prod_price值时，对产品按prod_name进行排序
select prod_id, prod_price, prod_name from products order by prod_price, prod_name;
+---------+------------+----------------+
| prod_id | prod_price | prod_name      |
+---------+------------+----------------+
| FC      |       2.50 | Carrots        |
| TNT1    |       2.50 | TNT (1 stick)  |
| FU1     |       3.42 | Fuses          |
| SLING   |       4.49 | Sling          |
| ANV01   |       5.99 | .5 ton anvil   |
| OL1     |       8.99 | Oil can        |
| ANV02   |       9.99 | 1 ton anvil    |
| FB      |      10.00 | Bird seed      |
| TNT2    |      10.00 | TNT (5 sticks) |
| DTNTR   |      13.00 | Detonator      |
| ANV03   |      14.99 | 2 ton anvil    |
| JP1000  |      35.00 | JetPack 1000   |
| SAFE    |      50.00 | Safe           |
| JP2000  |      55.00 | JetPack 2000   |
+---------+------------+----------------+
14 rows in set (0.01 sec)

```

## 3.3 指定排序方向
```sql
-- 对prod_price进行降序排序
select prod_id, prod_price, prod_name from products order by prod_price desc;
+---------+------------+----------------+
| prod_id | prod_price | prod_name      |
+---------+------------+----------------+
| JP2000  |      55.00 | JetPack 2000   |
| SAFE    |      50.00 | Safe           |
| JP1000  |      35.00 | JetPack 1000   |
| ANV03   |      14.99 | 2 ton anvil    |
| DTNTR   |      13.00 | Detonator      |
| FB      |      10.00 | Bird seed      |
| TNT2    |      10.00 | TNT (5 sticks) |
| ANV02   |       9.99 | 1 ton anvil    |
| OL1     |       8.99 | Oil can        |
| ANV01   |       5.99 | .5 ton anvil   |
| SLING   |       4.49 | Sling          |
| FU1     |       3.42 | Fuses          |
| FC      |       2.50 | Carrots        |
| TNT1    |       2.50 | TNT (1 stick)  |
+---------+------------+----------------+
14 rows in set (0.08 sec)

-- 对prod_price进行降序排序，再对prod_name进行升序排序
select prod_id, prod_price, prod_name from products order by prod_price desc, prod_name;
+---------+------------+----------------+
| prod_id | prod_price | prod_name      |
+---------+------------+----------------+
| JP2000  |      55.00 | JetPack 2000   |
| SAFE    |      50.00 | Safe           |
| JP1000  |      35.00 | JetPack 1000   |
| ANV03   |      14.99 | 2 ton anvil    |
| DTNTR   |      13.00 | Detonator      |
| FB      |      10.00 | Bird seed      |
| TNT2    |      10.00 | TNT (5 sticks) |
| ANV02   |       9.99 | 1 ton anvil    |
| OL1     |       8.99 | Oil can        |
| ANV01   |       5.99 | .5 ton anvil   |
| SLING   |       4.49 | Sling          |
| FU1     |       3.42 | Fuses          |
| FC      |       2.50 | Carrots        |
| TNT1    |       2.50 | TNT (1 stick)  |
+---------+------------+----------------+
14 rows in set (0.04 sec)

```

> [!important]
> desc: 降序
> asc: 升序(默认排序)

### 3.3.1 使用limit和desc获取最贵的物品的价格
```sql
select prod_id, prod_price, prod_name from products order by prod_price desc limit 1 offset 0;
+---------+------------+--------------+
| prod_id | prod_price | prod_name    |
+---------+------------+--------------+
| JP2000  |      55.00 | JetPack 2000 |
+---------+------------+--------------+
1 row in set (0.00 sec)

-- 或者
select prod_id, prod_price, prod_name from products order by prod_price desc limit 1;
+---------+------------+--------------+
| prod_id | prod_price | prod_name    |
+---------+------------+--------------+
| JP2000  |      55.00 | JetPack 2000 |
+---------+------------+--------------+
1 row in set (0.00 sec)

```
> [!tip]
> 在给出order by子句时，应该保证它位于from子句之后。如果使用limit，它必须位于order by之后。