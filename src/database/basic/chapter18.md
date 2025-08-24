---
title: 第十八章 更新和删除数据
date: 2025-08-24
tags:
    - 计算机基础
    - 数据库
categories: mysql
isOriginal: true
order: 18
dir:
    order: 18
---
## 18.1 更新数据
为了更新（修改）表中的数据，可使用UPDATE语句。可采用两种方式使用UPDATE：
- 更新表中特定行。
- 更新表中所有行。

> [!wraning]
> 不要省略WHERE子句 在使用UPDATE时一定要注意细心。因为稍不注意，就会更新表中所有行。

```sql
-- UPDATE语句总是以要更新的表的名字开始。
-- SET命令用来将新值赋给被更新的列。
-- UPDATE语句以WHERE子句结束，它告诉MySQL更新哪一行。
-- 没有WHERE子句，MySQL将会用这个电子邮件地址更新customers表中所有行，这不是我们所希望的。
update customers
set cust_email = 'elmer@fuadd.com'
where cust_id = 10005;
Query OK, 1 row affected (0.03 sec)
Rows matched: 1  Changed: 1  Warnings: 0

-- 更新多个列
update customers
set cust_name = 'The Fudds',
    cust_email = 'elmer@fudd.com'
where cust_id = 10005;
Query OK, 1 row affected (0.02 sec)
Rows matched: 1  Changed: 1  Warnings: 0

-- 为了删除某个列的值，可设置它为NULL（假如表定义允许NULL值）。
update customers
set cust_email = NULL
where cust_id = 10005;
Query OK, 1 row affected (0.02 sec)
Rows matched: 1  Changed: 1  Warnings: 0

```
> [!tip]
> IGNORE关键字 如果用UPDATE语句更新多行，并且在更新这些行中的一行或多行时出一个现错误，则整个UPDATE操作被取消（错误发生前更新的所有行被恢复到它们原来的值）。为即使是发生错误，也继续进行更新，可使用IGNORE关键字，如下所示：update igonre customers...

## 18.2 删除数据
为了从一个表中删除（去掉）数据，使用DELETE语句。可以两种方式使用DELETE：
- 从表中删除特定的行；
- 从表中删除所有行。

> [!warning]
> 不要省略WHERE子句 在使用DELETE时一定要注意细心。因为稍不注意，就会错误地删除表中所有行。

```sql
delete from customers where cust_id = 10006;
Query OK, 1 row affected (0.02 sec)

```
> [!tip]
> 更快的删除 如果想从表中删除所有行，不要使用DELETE。可使用TRUNCATE TABLE语句，它完成相同的工作，但速度更快（TRUNCATE实际是删除原来的表并重新创建一个表，而不是逐行删除表中的数据）。

## 18.3 更新和删除的指导原则
- 除非确实打算更新和删除每一行，否则绝对不要使用不带WHERE子句的UPDATE或DELETE语句。
- 保证每个表都有主键，尽可能像WHERE子句那样使用它（可以指定各主键、多个值或值的范围）。
- 在对UPDATE或DELETE语句使用WHERE子句前，应该先用SELECT进行测试，保证它过滤的是正确的记录，以防编写的WHERE子句不正确。
- 使用强制实施引用完整性的数据库，这样MySQL将不允许删除具有与其他表相关联的数据的行。