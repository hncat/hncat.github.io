---
title: 第二十章 使用视图
date: 2025-08-24
tags:
    - 计算机基础
    - 数据库
categories: mysql
isOriginal: true
order: 20
dir:
    order: 20
---
## 20.1 视图
> [!important]
> 视图是虚拟的表。与包含数据的表不一样，视图只包含使用时动态检索数据的查询。

比如我有一段SQL如下:
```sql
select cust_name, cust_contact from customers as c, orders as o, orderitems as oi where c.cust_id = o.cust_id and oi.order_num = o.order_num and prod_id = 'TNT2';
+----------------+--------------+
| cust_name      | cust_contact |
+----------------+--------------+
| Coyote Inc.    | Y Lee        |
| Yosemite Place | Y Sam        |
+----------------+--------------+
2 rows in set (0.09 sec)

```
此查询用来检索订购了某个特定产品的客户。任何需要这个数据的人都必须理解相关表的结构，并且知道如何创建查询和对表进行联结。为了检索其他产品（或多个产品）的相同数据，必须修改最后的WHERE子句。

现在，假如可以把整个查询包装成一个名为productcustomers的虚拟表，则可以如下轻松地检索出相同的数据：
```sql
select cust_name, cust_contact
from productcustomers
where prod_id = 'TNT2';
+----------------+--------------+
| cust_name      | cust_contact |
+----------------+--------------+
| Coyote Inc.    | Y Lee        |
| Yosemite Place | Y Sam        |
+----------------+--------------+
2 rows in set (0.09 sec)

```
这就是视图的作用。productcustomers是一个视图，作为视图，它不包含表中应该有的任何列或数据，它包含的是一个SQL查询（与上面用以正确联结表的相同的查询）。
### 20.1.1 为什么使用视图
- 重用SQL语句。
- 简化复杂的SQL操作。在编写查询后，可以方便地重用它而不必知道它的基本查询细节。
- 使用表的组成部分而不是整个表。
- 保护数据。可以给用户授予表的特定部分的访问权限而不是整个表的访问权限。
- 更改数据格式和表示。视图可返回与底层表的表示和格式不同的数据。

> [!important]
> 重要的是知道视图仅仅是用来查看存储在别处的数据的一种设施。视图本身不包含数据，因此它们返回的数据是从其他表中检索出来的。在添加或更改这些表中的数据时，视图将返回改变过的数据。

> [!warning]
> 性能问题 因为视图不包含数据，所以每次使用视图时，都必须处理查询执行时所需的任一个检索。如果你用多个联结和过滤创建了复杂的视图或者嵌套了视图，可能会发现性能下降得很厉害。因此，在部署使用了大量视图的应用前，应该进行测试。

### 20.1.2 视图的规则和限制
- 与表一样，视图必须唯一命名（不能给视图取与别的视图或表相同的名字）。
- 对于可以创建的视图数目没有限制。
- 为了创建视图，必须具有足够的访问权限。这些限制通常由数据库管理人员授予。
- 视图可以嵌套，即可以利用从其他视图中检索数据的查询来构造一个视图。
- ORDER BY可以用在视图中，但如果从该视图检索数据SELECT中也含有ORDER BY，那么该视图中的ORDER BY将被覆盖。
- 视图不能索引，也不能有关联的触发器或默认值。
- 视图可以和表一起使用。例如，编写一条联结表和视图的SELECT语句。

## 20.2 使用视图
- 视图用CREATE VIEW语句来创建。
- 使用SHOW CREATE VIEW viewname；来查看创建视图的语句。
- 用DROP删除视图，其语法为DROP VIEW viewname。
- 更新视图时，可以先用DROP再用CREATE，也可以直接用CREATE OR REPLACE VIEW。如果要更新的视图不存在，则第2条更新语句会创建一个视图；如果要更新的视图存在，则第2条更新语句会替换原有视图。

### 20.2.1 利用视图简化复杂的联结
```sql
create view productcustomers as
  select cust_name, cust_contact, prod_id
  from customers as c, orders as o, orderitems as oi
  where c.cust_id = o.cust_id and oi.order_num = o.order_num;
Query OK, 0 rows affected (0.02 sec)

select * from productcustomers;
+----------------+--------------+---------+
| cust_name      | cust_contact | prod_id |
+----------------+--------------+---------+
| Coyote Inc.    | Y Lee        | ANV01   |
| Coyote Inc.    | Y Lee        | ANV02   |
| Coyote Inc.    | Y Lee        | TNT2    |
| Coyote Inc.    | Y Lee        | FB      |
| Coyote Inc.    | Y Lee        | FB      |
| Coyote Inc.    | Y Lee        | OL1     |
| Coyote Inc.    | Y Lee        | SLING   |
| Coyote Inc.    | Y Lee        | ANV03   |
| Wascals        | Jim Jones    | JP2000  |
| Yosemite Place | Y Sam        | TNT2    |
| The Fudds      | E Fudd       | FC      |
+----------------+--------------+---------+
11 rows in set (0.00 sec)

select * from productcustomers where prod_id = 'TNT2';
+----------------+--------------+---------+
| cust_name      | cust_contact | prod_id |
+----------------+--------------+---------+
| Coyote Inc.    | Y Lee        | TNT2    |
| Yosemite Place | Y Sam        | TNT2    |
+----------------+--------------+---------+
2 rows in set (0.05 sec)

```
> [!tip]
> 创建可重用的视图 创建不受特定数据限制的视图是一种好办法。例如，上面创建的视图返回生产所有产品的客户而不仅仅是生产TNT2的客户。扩展视图的范围不仅使得它能被重用，而且甚至更有用。这样做不需要创建和维护多个类似视图。

### 20.2.2 用视图重新格式化检索出的数据
```sql
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
6 rows in set (0.01 sec)

create view vendorlocations as
  select concat(vend_name, '(', vend_country, ')')
  from vendors order by vend_name;
Query OK, 0 rows affected (0.05 sec)

select * from vendorlocations;
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
6 rows in set (0.01 sec)

```
### 20.2.3 用视图过滤不想要的数据
```sql
create view customeremaillist as
  select cust_id, cust_name, cust_email
  from customers where cust_email is not null;
Query OK, 0 rows affected (0.02 sec)

select * from customeremaillist;
+---------+----------------+---------------------+
| cust_id | cust_name      | cust_email          |
+---------+----------------+---------------------+
|   10001 | Coyote Inc.    | ylee@coyote.com     |
|   10003 | Wascals        | rabbit@wascally.com |
|   10004 | Yosemite Place | sam@yosemite.com    |
+---------+----------------+---------------------+
3 rows in set (0.01 sec)

```
> [!note]
> WHERE子句与WHERE子句 如果从视图检索数据时使用了一条WHERE子句，则两组子句（一组在视图中，另一组是传递给视图的）将自动组合。

### 20.2.4 使用视图与计算字段
```sql
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
4 rows in set (0.01 sec)

create view orderitemsexpanded as
select order_num, prod_id, quantity, item_price, quantity * item_price as expanded_price
from orderitems;
Query OK, 0 rows affected (0.01 sec)

select * from orderitemsexpanded where order_num = 20005;
+-----------+---------+----------+------------+----------------+
| order_num | prod_id | quantity | item_price | expanded_price |
+-----------+---------+----------+------------+----------------+
|     20005 | ANV01   |       10 |       5.99 |          59.90 |
|     20005 | ANV02   |        3 |       9.99 |          29.97 |
|     20005 | TNT2    |        5 |      10.00 |          50.00 |
|     20005 | FB      |        1 |      10.00 |          10.00 |
+-----------+---------+----------+------------+----------------+
4 rows in set (0.00 sec)

```
### 20.2.5 更新视图
> [!important]
> 通常，视图是可更新的（即，可以对它们使用INSERT、UPDATE和DELETE）。更新一个视图将更新其基表（可以回忆一下，视图本身没有数据）。如果你对视图增加或删除行，实际上是对其基表增加或删除行。
> 但是，并非所有视图都是可更新的。基本上可以说，如果MySQL不能正确地确定被更新的基数据，则不允许更新（包括插入和删除）。这实际上意味着，如果视图定义中有以下操作，则不能进行视图的更新：
> - 分组（使用GROUP BY和HAVING）；
> - 联结；
> - 子查询；
> - 并；
> - 聚集函数（Min()、Count()、Sum()等）；
> - DISTINCT；
> - 导出（计算）列。

> [!tip]
> 将视图用于检索 一般，应该将视图用于检索（SELECT语句）而不用于更新（INSERT、UPDATE和DELETE）。