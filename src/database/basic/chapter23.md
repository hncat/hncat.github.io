---
title: 第二十三章 触发器
date: 2025-08-24
tags:
    - 计算机基础
    - 数据库
categories: mysql
isOriginal: true
order: 23
dir:
    order: 23
---
## 23.1 触发器
> [!important]
> 触发器是MySQL响应以下任意语句而自动执行的一条MySQL语句（或位于BEGIN和END语句之间的一组语句）：
> - DELETE
> - INSERT
> - UPDATE

## 23.2 创建触发器
在创建触发器时，需要给出4条信息：
1. 唯一的触发器名；
2. 触发器关联的表；
3. 触发器应该响应的活动（DELETE、INSERT或UPDATE）；
4. 触发器何时执行（处理之前或之后）。

> [!tip]
> 保持每个数据库的触发器名唯一: 在MySQL 5中，触发器名必须在每个表中唯一，但不是在每个数据库中唯一。这表示同一数据库中的两个表可具有相同名字的触发器。这在其他每个数据库触发器名必须唯一的DBMS中是不允许的，而且以后的MySQL版本很可能会使命名规则更为严格。因此，现在最好是在数据库范围内使用唯一的触发器名。

触发器用CREATE TRIGGER语句创建。下面是一个简单的例子：
```sql
CREATE TRIGGER newproduct AFTER INSERT ON products
FOR EACH ROW SELECT 'Product added';

```
> [!note]
> CREATE TRIGGER用来创建名为newproduct的新触发器。
> 触发器可在一个操作发生之前或之后执行，这里给出了AFTER INSERT，所以此触发器将在INSERT语句成功执行后执行。
> 这个触发器还指定FOREACH ROW，因此代码对每个插入行执行。
> 在这个例子中，文本Product added将对每个插入的行显示一次。

> [!note]
> 仅支持表: 只有表才支持触发器，视图不支持（临时表也不支持）。

> [!warning]
> 触发器按每个表每个事件每次地定义，每个表每个事件每次只允许一个触发器。因此，每个表最多支持6个触发器（每条INSERT、UPDATE和DELETE的之前和之后）。单一触发器不能与多个事件或多个表关联，所以，如果你需要一个对INSERT和UPDATE操作执行的触发器，则应该定义两个触发器。

> [!note]
> 触发器失败 如果BEFORE触发器失败，则MySQL将不执行请求的操作。此外，如果BEFORE触发器或语句本身失败，MySQL将不执行AFTER触发器（如果有的话）。

## 23.3 删除触发器
> [!important]
> 使用ROP TRIGGER \[trigger_name\];语句除触发器

触发器不能更新或覆盖。为了修改一个触发器，必须先删除它，然后再重新创建。
```sql
DROP TRIGGER newproduct;
```

## 23.4 使用触发器
### 23.4.1 INSERT触发器
INSERT触发器在INSERT语句执行之前或之后执行。需要知道以下几点：
1. 在INSERT触发器代码内，可引用一个名为NEW的虚拟表，访问被插入的行；
2. 在BEFORE INSERT触发器中，NEW中的值也可以被更新（允许更改被插入的值）；
3. 对于AUTO_INCREMENT列，NEW在INSERT执行之前包含0，在INSERT执行之后包含新的自动生成值。
```sql
-- 创建一个名为neworder的触发器
-- 它按照AFTER INSERT ON orders执行。
-- 在插入一个新订单到orders表时，MySQL生成一个新订单号并保存到order_num中。
-- 触发器从NEW. order_num取得这个值并返回它。此触发器必须按照AFTER INSERT执行，因为在BEFORE INSERT语句执行之前，新order_num还没有生成。对于orders的每次插入使用这个触发器将总是返回新的订单号。
CREATE TRIGGER neworder AFTER INSERT ON orders
FOR EACH ROW SELECT NEW.order_num;
```

> [!tip]
> BEFORE或AFTER？ 通常，将BEFORE用于数据验证和净化（目的是保证插入表中的数据确实是需要的数据）。本提示也适用于UPDATE触发器。

### 23.4.2 DELETE触发器
DELETE触发器在DELETE语句执行之前或之后执行。需要知道以下两点：
1. 在DELETE触发器代码内，你可以引用一个名为OLD的虚拟表，访问被删除的行；
2. OLD中的值全都是只读的，不能更新。

```sql
-- 在任意订单被删除前将执行此触发器。它使用一条INSERT语句将OLD中的值（要被删除的订单）保存到一个名为archive_orders的存档表中（为实际使用这个例子，你需要用与orders相同的列创建一个名为archive_orders的表）。
CREATE TRIGGER deleteorder BEFORE DELETE ON orders
FOR EACH ROW
BEGIN
  INSERT INTO archive_orders(order_num, order_date, cust_id)
  VALUES(OLD.order_num, OLD.order_date, OLD.cust_id);
END;
```
使用BEFORE DELETE触发器的优点（相对于AFTER DELETE触发器来说）为，如果由于某种原因，订单不能存档，DELETE本身将被放弃。

### 23.4.3 UPDATE触发器
UPDATE触发器在UPDATE语句执行之前或之后执行。需要知道以下几点：
1. 在UPDATE触发器代码中，你可以引用一个名为OLD的虚拟表访问以前（UPDATE语句前）的值，引用一个名为NEW的虚拟表访问新更新的值；
2. 在BEFORE UPDATE触发器中，NEW中的值可能也被更新（允许更改将要用于UPDATE语句中的值）；
3. OLD中的值全都是只读的，不能更新。

```sql
CREATE TRIGGER updatevendor BEFORE UPDATE ON vendors
FOR EACH ROW SET NEW.vend_state = UPPER(NEW.vend_state);

```
显然，任何数据净化都需要在UPDATE语句之前进行，就像这个例子中一样。每次更新一个行时，NEW.vend_state中的值（将用来更新表行的值）都用UPPER(NEW.vend_state)替换。