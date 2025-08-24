---
title: 第十二章 使用子查询
date: 2025-08-24
tags:
    - 计算机基础
    - 数据库
categories: mysql
isOriginal: true
order: 12
dir:
    order: 12
---
## 12.1 利用子查询进行过滤
> [!tip]
> 订单存储在两个表中。
> 1. 对于包含订单号、客户ID、订单日期的每个订单，orders表存储一行。
> 2. 各订单的物品存储在相关的orderitems表中。
> 3. orders表不存储客户信息。它只存储客户的ID。实际的客户信息存储在customers表中。

```sql
-- 1. prod_id为TNT2的所有订单物品，检索其order_num列
select order_num from orderitems where prod_id = 'TNT2';
+-----------+
| order_num |
+-----------+
|     20005 |
|     20007 |
+-----------+
2 rows in set (0.08 sec)

-- 2. 查询具有订单20005和20007的客户ID。
select cust_id from orders where order_num in(20005, 20007);
+---------+
| cust_id |
+---------+
|   10001 |
|   10004 |
+---------+
2 rows in set (0.01 sec)

-- 把第一个查询（返回订单号的那一个）变为子查询组合两个查询。
select cust_id
        from orders
        where order_num in(select order_num
                            from orderitems
                            where prod_id = 'TNT2');
+---------+
| cust_id |
+---------+
|   10001 |
|   10004 |
+---------+
2 rows in set (0.02 sec)

-- 3. 检索这些客户ID的客户信息。
select cust_name, cust_contact from customers where cust_id in(10001,10004);
+----------------+--------------+
| cust_name      | cust_contact |
+----------------+--------------+
| Coyote Inc.    | Y Lee        |
| Yosemite Place | Y Sam        |
+----------------+--------------+
2 rows in set (0.01 sec)

-- 组合所有查询
select cust_name, cust_contact
        from customers
        where cust_id in(select cust_id
                          from orders
                          where order_num in(select order_num
                                              from orderitems
                                              where prod_id = 'TNT2'));
+----------------+--------------+
| cust_name      | cust_contact |
+----------------+--------------+
| Coyote Inc.    | Y Lee        |
| Yosemite Place | Y Sam        |
+----------------+--------------+
2 rows in set (0.02 sec)

```
> [!warning]
> 列必须匹配 在WHERE子句中使用子查询（如这里所示），应该保证SELECT语句具有与WHERE子句中相同数目的列。通常，子查询将返回单个列并且与单个列匹配，但如果需要也可以使用多个列。

## 12.2 作为计算字段使用子查询
显示customers表中每个客户的订单总数。订单与相应的客户ID存储在orders表中。
1. 从customers表中检索客户列表。
2. 对于检索出的每个客户，统计其在orders表中的订单数目。
```sql
select count(*) as orders from orders where cust_id = 10001;
+--------+
| orders |
+--------+
|      2 |
+--------+
1 row in set (0.03 sec)

select cust_name,
        cust_state,
        (select count(*)
          from orders
          where orders.cust_id = customers.cust_id) as orders
        from customers
        order by cust_name;
+----------------+------------+--------+
| cust_name      | cust_state | orders |
+----------------+------------+--------+
| Coyote Inc.    | MI         |      2 |
| E Fudd         | IL         |      1 |
| Mouse House    | OH         |      0 |
| Wascals        | IN         |      1 |
| Yosemite Place | AZ         |      1 |
+----------------+------------+--------+
5 rows in set (0.01 sec)

```
> [!note]
> 相关子查询（correlated subquery） 涉及外部查询的子查询。