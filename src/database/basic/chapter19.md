---
title: 第十九章 创建和操作表
date: 2025-08-24
tags:
    - 计算机基础
    - 数据库
categories: mysql
isOriginal: true
order: 19
dir:
    order: 19
---
## 19.1 创建表
### 19.1.1 表创建基础
为利用CREATE TABLE创建表，必须给出下列信息：
1. 新表的名字，在关键字CREATE TABLE之后给出；
2. 表列的名字和定义，用逗号分隔。

```sql
CREATE TABLE customers
(
  cust_id      int       NOT NULL AUTO_INCREMENT,
  cust_name    char(50)  NOT NULL ,
  cust_address char(50)  NULL ,
  cust_city    char(50)  NULL ,
  cust_state   char(5)   NULL ,
  cust_zip     char(10)  NULL ,
  cust_country char(50)  NULL ,
  cust_contact char(50)  NULL ,
  cust_email   char(255) NULL ,
  PRIMARY KEY (cust_id)
) ENGINE=InnoDB;
```
### 19.1.2 使用NULL值
允许NULL值的列也允许在插入行时不给出该列的值。不允许NULL值的列不接受该列没有值的行，换句话说，在插入或更新行时，该列必须有值。
```sql
CREATE TABLE orders
(
  order_num  int      NOT NULL AUTO_INCREMENT, -- 订单号
  order_date datetime NOT NULL ,               -- 订单日期
  cust_id    int      NOT NULL ,               -- 客户ID
  PRIMARY KEY (order_num)
) ENGINE=InnoDB;

CREATE TABLE vendors
(
  vend_id      int      NOT NULL AUTO_INCREMENT,
  vend_name    char(50) NOT NULL ,
  vend_address char(50) NULL ,
  vend_city    char(50) NULL ,
  vend_state   char(5)  NULL ,
  vend_zip     char(10) NULL ,
  vend_country char(50) NULL ,
  PRIMARY KEY (vend_id)
) ENGINE=InnoDB;
```
> [!warning]
> 理解NULL 不要把NULL值与空串相混淆。NULL值是没有值，它不是空串。如果指定''（两个单引号，其间没有字符），这在NOT NULL列中是允许的。空串是一个有效的值，它不是无值。NULL值用关键字NULL而不是空串指定。

### 19.1.3 主键
> [!important]
> 主键值必须唯一。即，表中的每个行必须具有唯一的主键值。如果主键使用单个列，则它的值必须唯一。如果使用多个列，则这些列的组合值必须唯一。

```sql
CREATE TABLE orderitems
(
  order_num  int          NOT NULL , -- 订单号
  order_item int          NOT NULL , -- 订单物品
  prod_id    char(10)     NOT NULL ,
  quantity   int          NOT NULL ,
  item_price decimal(8,2) NOT NULL ,
  PRIMARY KEY (order_num, order_item) -- 组合主键
) ENGINE=InnoDB;
```
> [!tip]
> 主键和NULL值: 主键为其值唯一标识表中每个行的列。主键中只能使用不允许NULL值的列。允许NULL值的列不能作为唯一标识。

### 19.1.4 使用AUTO_INCREMENT
> [!important]
> 1. AUTO_INCREMENT告诉MySQL，本列每当增加一行时自动增量。每次执行一个INSERT操作时，MySQL自动对该列增量（从而才有这个关键字AUTO_INCREMENT），给该列赋予下一个可用的值。
> 2. 每个表只允许一个AUTO_INCREMENT列，而且它必须被索引（比如它成为主键）。
```sql
CREATE TABLE customers
(
  cust_id      int       NOT NULL AUTO_INCREMENT, -- 自增主键
  cust_name    char(50)  NOT NULL ,
  cust_address char(50)  NULL ,
  cust_city    char(50)  NULL ,
  cust_state   char(5)   NULL ,
  cust_zip     char(10)  NULL ,
  cust_country char(50)  NULL ,
  cust_contact char(50)  NULL ,
  cust_email   char(255) NULL ,
  PRIMARY KEY (cust_id)
) ENGINE=InnoDB;
```
> [!note]
> 覆盖AUTO_INCREMENT: 如果一个列被指定为AUTO_INCREMENT，则它需要使用特殊的值吗？你可以简单地在INSERT语句中指定一个值，只要它是唯一的（至今尚未使用过）即可，该值将被用来替代自动生成的值。后续的增量将开始使用该手工插入的值。

> [!tip]
> 确定AUTO_INCREMENT值: 让MySQL生成（通过自动增量）主键的一个缺点是你不知道这些值都是谁。
> 考虑这个场景：你正在增加一个新订单。这要求在orders表中创建一行，然后在orderitms表中对订购的每项物品创建一行。order_num在orderitems表中与订单细节一起存储。这就是为什么orders表和orderitems表为相互关联的表的原因。这显然要求你在插入orders行之后，插入orderitems行之前知道生成的order_num。
> 那么，如何在使用AUTO_INCREMENT列时获得这个值呢？可使用last_insert_id()函数获得这个值，如下所示：SELECT last_inser_id()此语句返回最后一个AUTO_INCREMENT值，然后可以将它用于后续的MySQL语句。

### 19.1.5 指定默认值
> [!important]
> 如果在插入行时没有给出值，MySQL允许指定此时使用的默认值。默认值用CREATE TABLE语句的列定义中的DEFAULT关键字指定。

```sql
CREATE TABLE orderitems
(
  order_num  int          NOT NULL ,
  order_item int          NOT NULL ,
  prod_id    char(10)     NOT NULL ,
  quantity   int          NOT NULL ,
  item_price decimal(8,2) NOT NULL DEFAULT 1, -- 默认数量1
  PRIMARY KEY (order_num, order_item)
) ENGINE=InnoDB;
```
### 19.1.6 引擎类型
你可能已经注意到，迄今为止使用的CREATE TABLE语句全都以ENGINE=InnoDB语句结束。

与其他DBMS一样，MySQL有一个具体管理和处理数据的内部引擎。在你使用CREATE TABLE语句时，该引擎具体创建表，而在你使用SELECT语句或进行其他数据库处理时，该引擎在内部处理你的请求。多数时候，此引擎都隐藏在DBMS内，不需要过多关注它。

但MySQL与其他DBMS不一样，它具有多种引擎。它打包多个引擎，这些引擎都隐藏在MySQL服务器内，全都能执行CREATE TABLE和SELECT等命令。

当然，你完全可以忽略这些数据库引擎。如果省略ENGINE=语句，则使用默认引擎（很可能是MyISAM），多数SQL语句都会默认使用它。但并不是所有语句都默认使用它，这就是为什么ENGINE=语句很重要的原因

> [!important]
> 以下是几个需要知道的引擎：
> - InnoDB是一个可靠的事务处理引擎，它不支持全文本搜索；
> - MEMORY在功能等同于MyISAM，但由于数据存储在内存（不是磁盘）中，速度很快（特别适合于临时表）
> - MyISAM是一个性能极高的引擎，它支持全文本搜索，但不支持事务处理。
> 引擎类型可以混用。

> [!warning]
> 外键不能跨引擎 混用引擎类型有一个大缺陷。外键（用于强制实施引用完整性，如第1章所述）不能跨引擎，即使用一个引擎的表不能引用具有使用不同引擎的表的外键。

## 19.2 更新表
> [!tip]
> 为更新表定义，可使用ALTER TABLE语句。但是，理想状态下，当表中存储数据以后，该表就不应该再被更新。在表的设计过程中需要花费大量时间来考虑，以便后期不对该表进行大的改动。

为了使用ALTER TABLE更改表结构，必须给出下面的信息：
- 在ALTER TABLE之后给出要更改的表名（该表必须存在，否则将出错）；
- 所做更改的列表。
```sql
-- 为vendors表增加一个名为vend_phone的列，类型为char(20)
ALTER TABLE vendors
ADD VEND_PHONE char(20);

-- 删除刚刚添加的列，可以这样做：
ALTER TABLE vendors
DROP COLUMN vend_phone;

-- 定义外键
ALTER TABLE orderitems
ADD CONSTRAINT fk_orderitems_orders FOREIGN KEY (order_num)
REFERENCES orders (order_num);

ALTER TABLE orderitems
ADD CONSTRAINT fk_orderitems_products FOREIGN KEY (prod_id)
REFERENCES products (prod_id);

ALTER TABLE orders
ADD CONSTRAINT fk_orders_customers FOREIGN KEY (cust_id)
REFERENCES customers (cust_id);

ALTER TABLE products
ADD CONSTRAINT fk_products_vendors FOREIGN KEY (vend_id)
REFERENCES vendors (vend_id);
```

复杂的表结构更改一般需要手动删除过程，它涉及以下步骤：
1. 用新的列布局创建一个新表；
2. 使用INSERT SELECT语句从旧表复制数据到新表。如果有必要，可使用转换函数和计算字段；
3. 检验包含所需数据的新表；
4. 重命名旧表（如果确定，可以删除它）；
5. 用旧表原来的名字重命名新表；
6. 根据需要，重新创建触发器、存储过程、索引和外键。

> [!warning]
> 小心使用ALTER TABLE 使用ALTER TABLE要极为小心，应该在进行改动前做一个完整的备份（模式和数据的备份）。数据库表的更改不能撤销，如果增加了不需要的列，可能不能删除它们。类似地，如果删除了不应该删除的列，可能会丢失该列中的所有数据。

## 19.3 删除表
删除表（删除整个表而不是其内容）非常简单，使用DROP TABLE语句即可：
```sql
DROP TABLE customers;
```
## 19.4 重命名表
使用RENAME TABLE语句可以重命名一个表：
```sql
-- 重命名一个表
RENAME TABLE customers2 TO customers;

-- 重命名多个表
RENAME TABLE table1 TO table_1,
             table2 TO table_2,
             table3 TO table_3,
```