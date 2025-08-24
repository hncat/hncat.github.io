---
title: 第十一章 分组数据
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
## 11.1 数据分组
```sql
select count(*) as num_prods from products where vend_id = 1003;
+-----------+
| num_prods |
+-----------+
|         7 |
+-----------+
1 row in set (0.09 sec)

```
> [!note]
> 问题: 但如果要返回每个供应商提供的产品数目怎么办？或者返回只提供单项产品的供应商所提供的产品，或返回提供10个以上产品的供应商怎么办？
> 这时候就只能使用分组了，分组允许把数据分为多个逻辑组，以便能对每个组进行聚集计算。

## 11.2 创建分组
> [!important]
> 分组是在SELECT语句的GROUP BY子句中建立的。

```sql
-- vend_id包含产品供应商的ID，num_prods为计算字段（用COUNT(*)函数建立）。GROUP BY子句指示MySQL按vend_id排序并分组数据。这导致对每个vend_id而不是整个表计算num_prods一次。从输出中可以看到，供应商1001有3个产品，供应商1002有2个产品，供应商1003有7个产品，而供应商1005有2个产品。
select vend_id, count(*) as num_prods from products group by vend_id;
+---------+-----------+
| vend_id | num_prods |
+---------+-----------+
|    1001 |         3 |
|    1002 |         2 |
|    1003 |         7 |
|    1005 |         2 |
+---------+-----------+
4 rows in set (0.03 sec)

```

> [!warning]
> 在具体使用GROUP BY子句前，需要知道一些重要的规定:
> * GROUP BY子句可以包含任意数目的列。这使得能对分组进行嵌套，为数据分组提供更细致的控制。
> * 如果在GROUP BY子句中嵌套了分组，数据将在最后规定的分组上进行汇总。换句话说，在建立分组时，指定的所有列都一起计算（所以不能从个别的列取回数据）。
> * GROUP BY子句中列出的每个列都必须是检索列或有效的表达式（但不能是聚集函数）。如果在SELECT中使用表达式，则必须在GROUP BY子句中指定相同的表达式。不能使用别名。
> * 除聚集计算语句外，SELECT语句中的每个列都必须在GROUP BY子句中给出。
> * 如果分组列中具有NULL值，则NULL将作为一个分组返回。如果列中有多行NULL值，它们将分为一组。
> * GROUP BY子句必须出现在WHERE子句之后，ORDER BY子句之前。

> [!note]
> 使用ROLLUP 使用WITH ROLLUP关键字，可以得到每个分组以及每个分组汇总级别（针对每个分组）的值

```sql
select vend_id, count(*) as num_prods from products group by vend_id with rollup;
+---------+-----------+
| vend_id | num_prods |
+---------+-----------+
|    1001 |         3 |
|    1002 |         2 |
|    1003 |         7 |
|    1005 |         2 |
|    NULL |        14 |
+---------+-----------+
5 rows in set (0.03 sec)

```
## 11.3 过滤分组(HAVING)
> [!important]
> HAVING支持所有WHERE操作符，唯一的差别是WHERE过滤行，而HAVING过滤分组。

```sql
-- 过滤COUNT(*) >=2（两个以上的订单）的那些分组。
select cust_id, count(*) as orders from orders group by cust_id having count(*) >= 2;
+---------+--------+
| cust_id | orders |
+---------+--------+
|   10001 |      2 |
+---------+--------+
1 row in set (0.08 sec)

```

> [!note]
> HAVING和WHERE的差别 这里有另一种理解方法，WHERE在数据分组前进行过滤，HAVING在数据分组后进行过滤。这是一个重要的区别，WHERE排除的行不包括在分组中。这可能会改变计算值，从而影响HAVING子句中基于这些值过滤掉的分组。

```sql
-- WHERE子句过滤所有prod_price至少为10的行。
-- 然后按vend_id分组数据，HAVING子句过滤计数为2或2以上的分组。
select vend_id, count(*) as num_prods from products where prod_price >= 10 group by vend_id having count(*) >= 2;
+---------+-----------+
| vend_id | num_prods |
+---------+-----------+
|    1003 |         4 |
|    1005 |         2 |
+---------+-----------+
2 rows in set (0.03 sec)

-- 如果没有WHERE子句，将会多检索出两行（供应商1002，销售的所有产品价格都在10以下；供应商1001，销售3个产品，但只有一个产品的价格大于等于10）：
select vend_id, count(*) as num_prods from products group by vend_id having count(*) >= 2;
+---------+-----------+
| vend_id | num_prods |
+---------+-----------+
|    1001 |         3 |
|    1002 |         2 |
|    1003 |         7 |
|    1005 |         2 |
+---------+-----------+
4 rows in set (0.01 sec)

select * from products;
+---------+---------+----------------+------------+----------------------------------------------------------------+
| prod_id | vend_id | prod_name      | prod_price | prod_desc                                                      |
+---------+---------+----------------+------------+----------------------------------------------------------------+
| ANV01   |    1001 | .5 ton anvil   |       5.99 | .5 ton anvil, black, complete with handy hook                  |
| ANV02   |    1001 | 1 ton anvil    |       9.99 | 1 ton anvil, black, complete with handy hook and carrying case |
| ANV03   |    1001 | 2 ton anvil    |      14.99 | 2 ton anvil, black, complete with handy hook and carrying case |
| DTNTR   |    1003 | Detonator      |      13.00 | Detonator (plunger powered), fuses not included                |
| FB      |    1003 | Bird seed      |      10.00 | Large bag (suitable for road runners)                          |
| FC      |    1003 | Carrots        |       2.50 | Carrots (rabbit hunting season only)                           |
| FU1     |    1002 | Fuses          |       3.42 | 1 dozen, extra long                                            |
| JP1000  |    1005 | JetPack 1000   |      35.00 | JetPack 1000, intended for single use                          |
| JP2000  |    1005 | JetPack 2000   |      55.00 | JetPack 2000, multi-use                                        |
| OL1     |    1002 | Oil can        |       8.99 | Oil can, red                                                   |
| SAFE    |    1003 | Safe           |      50.00 | Safe with combination lock                                     |
| SLING   |    1003 | Sling          |       4.49 | Sling, one size fits all                                       |
| TNT1    |    1003 | TNT (1 stick)  |       2.50 | TNT, red, single stick                                         |
| TNT2    |    1003 | TNT (5 sticks) |      10.00 | TNT, red, pack of 10 sticks                                    |
+---------+---------+----------------+------------+----------------------------------------------------------------+
14 rows in set (0.00 sec)

```

## 11.4 分组和排序
|order by|group by|
|:-:|:-:|
|排序产生的输出|分组行。但输出可能不是分组的顺序|
|任意列都可以使用（甚至非选择的列也可以使用）|只可能使用选择列或表达式列，而且必须使用每个选择列表达式|
|不一定需要|如果与聚集函数一起使用列（或表达式），则必须使用|
> [!warning]
> 不要忘记ORDER BY 一般在使用GROUP BY子句时，应该也给出ORDER BY子句。这是保证数据正确排序的唯一方法。千万不要仅依赖GROUP BY排序数据。

```sql
select order_num, sum(quantity * item_price) as ordertotal from orderitems group by order_num having sum(quantity * item_price) >= 50;
+-----------+------------+
| order_num | ordertotal |
+-----------+------------+
|     20005 |     149.87 |
|     20006 |      55.00 |
|     20007 |    1000.00 |
|     20008 |     125.00 |
+-----------+------------+
4 rows in set (0.02 sec)

select order_num, sum(quantity * item_price) as ordertotal from orderitems group by order_num having sum(quantity * item_price) >= 50 order by ordertotal;
+-----------+------------+
| order_num | ordertotal |
+-----------+------------+
|     20006 |      55.00 |
|     20008 |     125.00 |
|     20005 |     149.87 |
|     20007 |    1000.00 |
+-----------+------------+
4 rows in set (0.00 sec)

```

## 11.5 select子句顺序
|子句|说明|是否必须使用|
|:-:|:-:|:-:|
|SELECT|要返回的列或表达式|是|
|FROM|从中检索数据的表|仅在从表选择数据时使用|
|WHERE|行级过滤|否|
|GROUP BY|分组说明|仅在按组计算聚集时使用|
|HAVING|组级过滤|否|
|ORDER BY|输出排序顺序|否|
|LIMIT|要检索的行数|否|