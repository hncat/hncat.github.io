---
title: 第十七章 插入数据
date: 2025-08-24
tags:
    - 计算机基础
    - 数据库
categories: mysql
isOriginal: true
order: 17
dir:
    order: 17
---
## 17.1 数据插入(INSERT)
> [!important]
> INSERT是用来插入（或添加）行到数据库表的。插入可以用几种方式使用：
> 1. 插入完整的行；
> 2. 插入行的一部分；
> 3. 插入多行；
> 4. 插入某些查询的结果。

## 17.2 插入完整的行
```sql
-- 插入之前
select * from customers;
+---------+----------------+---------------------+-----------+------------+----------+--------------+--------------+---------------------+
| cust_id | cust_name      | cust_address        | cust_city | cust_state | cust_zip | cust_country | cust_contact | cust_email          |
+---------+----------------+---------------------+-----------+------------+----------+--------------+--------------+---------------------+
|   10001 | Coyote Inc.    | 200 Maple Lane      | Detroit   | MI         | 44444    | USA          | Y Lee        | ylee@coyote.com     |
|   10002 | Mouse House    | 333 Fromage Lane    | Columbus  | OH         | 43333    | USA          | Jerry Mouse  | NULL                |
|   10003 | Wascals        | 1 Sunny Place       | Muncie    | IN         | 42222    | USA          | Jim Jones    | rabbit@wascally.com |
|   10004 | Yosemite Place | 829 Riverside Drive | Phoenix   | AZ         | 88888    | USA          | Y Sam        | sam@yosemite.com    |
|   10005 | E Fudd         | 4545 53rd Street    | Chicago   | IL         | 54545    | USA          | E Fudd       | NULL                |
+---------+----------------+---------------------+-----------+------------+----------+--------------+--------------+---------------------+
5 rows in set (0.01 sec)

-- 不太安全的做法，和表中列的定义次序相关
insert into customers
values(NULL, 'Pep E. LaPew', '100 Main Street', 'Los Angeles', 'CA', '90046', 'USA', NULL, NULL);
Query OK, 1 row affected (0.03 sec)

-- 插入之后
select * from customers;
+---------+----------------+---------------------+-------------+------------+----------+--------------+--------------+---------------------+
| cust_id | cust_name      | cust_address        | cust_city   | cust_state | cust_zip | cust_country | cust_contact | cust_email          |
+---------+----------------+---------------------+-------------+------------+----------+--------------+--------------+---------------------+
|   10001 | Coyote Inc.    | 200 Maple Lane      | Detroit     | MI         | 44444    | USA          | Y Lee        | ylee@coyote.com     |
|   10002 | Mouse House    | 333 Fromage Lane    | Columbus    | OH         | 43333    | USA          | Jerry Mouse  | NULL                |
|   10003 | Wascals        | 1 Sunny Place       | Muncie      | IN         | 42222    | USA          | Jim Jones    | rabbit@wascally.com |
|   10004 | Yosemite Place | 829 Riverside Drive | Phoenix     | AZ         | 88888    | USA          | Y Sam        | sam@yosemite.com    |
|   10005 | E Fudd         | 4545 53rd Street    | Chicago     | IL         | 54545    | USA          | E Fudd       | NULL                |
|   10006 | Pep E. LaPew   | 100 Main Street     | Los Angeles | CA         | 90046    | USA          | NULL         | NULL                |
+---------+----------------+---------------------+-------------+------------+----------+--------------+--------------+---------------------+
6 rows in set (0.00 sec)

-- 更安全的做法，给出列名
insert into customers(cust_name, cust_address, cust_city, cust_state, cust_zip, cust_country, cust_contact, cust_email)
values('Pep E.Lapew', '100 Main Street', 'LosAngeles', 'CA', '90046', 'USA', NULL, NULL);
Query OK, 1 row affected (0.03 sec)

-- 插入之后
select * from customers;
+---------+----------------+---------------------+-------------+------------+----------+--------------+--------------+---------------------+
| cust_id | cust_name      | cust_address        | cust_city   | cust_state | cust_zip | cust_country | cust_contact | cust_email          |
+---------+----------------+---------------------+-------------+------------+----------+--------------+--------------+---------------------+
|   10001 | Coyote Inc.    | 200 Maple Lane      | Detroit     | MI         | 44444    | USA          | Y Lee        | ylee@coyote.com     |
|   10002 | Mouse House    | 333 Fromage Lane    | Columbus    | OH         | 43333    | USA          | Jerry Mouse  | NULL                |
|   10003 | Wascals        | 1 Sunny Place       | Muncie      | IN         | 42222    | USA          | Jim Jones    | rabbit@wascally.com |
|   10004 | Yosemite Place | 829 Riverside Drive | Phoenix     | AZ         | 88888    | USA          | Y Sam        | sam@yosemite.com    |
|   10005 | E Fudd         | 4545 53rd Street    | Chicago     | IL         | 54545    | USA          | E Fudd       | NULL                |
|   10006 | Pep E. LaPew   | 100 Main Street     | Los Angeles | CA         | 90046    | USA          | NULL         | NULL                |
|   10007 | Pep E.Lapew    | 100 Main Street     | LosAngeles  | CA         | 90046    | USA          | NULL         | NULL                |
+---------+----------------+---------------------+-------------+------------+----------+--------------+--------------+---------------------+
7 rows in set (0.00 sec)

```
> [!tip]
> 总是使用列的列表 一般不要使用没有明确给出列的列表的INSERT语句。使用列的列表能使SQL代码继续发挥作用，即使表结构发生了变化。

> [!warning]
> 不管使用哪种INSERT语法，都必须给出VALUES的正确数目。如果不提供列名，则必须给每个表列提供一个值。如果提供列名，则必须对每个列出的列给出一个值。如果不这样，将产生一条错误消息，相应的行插入不成功。

> [!warning]
> 省略列 如果表的定义允许，则可以在INSERT操作中省略某些列。省略的列必须满足以下某个条件。
> - 该列定义为允许NULL值（无值或空值）。
> - 在表定义中给出默认值。这表示如果不给出值，将使用默认值。
> 如果对表中不允许NULL值且没有默认值的列不给出值，则MySQL将产生一条错误消息，并且相应的行插入不成功。

> [!tip]
> 提高整体性能 数据库经常被多个客户访问，对处理什么请求以及用什么次序处理进行管理是MySQL的任务。INSERT操作可能很耗时（特别是有很多索引需要更新时），而且它可能降低等待处理的SELECT语句的性能。 如果数据检索是最重要的（通常是这样），则你可以通过在INSERT和INTO之间添加关键字LOW_PRIORITY，指示MySQL降低INSERT语句的优先级，如下所示：INSERT LOW PRIORITY INOT 顺便说一下，这也适用于下一章介绍的UPDATE和DELETE语句。

## 17.3 插入多行
```sql
insert into customers(cust_name, cust_address, cust_city, cust_state, cust_zip, cust_country)
values('Pep E. LaPew', '100 Main Street', 'Los Angeles', 'CA', '90046', 'USA'),('M. Martian', '42 Galaxy Way', 'New York', 'NY', '11213', 'USA');
Query OK, 2 rows affected (0.09 sec)
Records: 2  Duplicates: 0  Warnings: 0

```
> [!tip]
> 提高INSERT的性能 此技术可以提高数据库处理的性能，因为MySQL用单条INSERT语句处理多个插入比使用多条INSERT语句快。

## 17.4 插入检索出的数据
```sql
insert into customers(cust_id, cust_contact, cust_email, cust_name, cust_address, cust_city, cust_state, cust_zip, cust_country)
select cust_id, cust_contact, cust_email, cust_name, cust_address, cust_city, cust_state, cust_zip, cust_country
from custnew;
```
