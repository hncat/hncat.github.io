---
title: 第二十一章 存储过程
date: 2025-08-24
tags:
    - 计算机基础
    - 数据库
categories: mysql
isOriginal: true
order: 21
dir:
    order: 21
---
## 21.1 说明是存储过程
> [!note]
> 存储过程简单来说，就是为以后的使用而保存的一条或多条MySQL语句的集合。可将其视为批文件，虽然它们的作用不仅限于批处理。
> 类比于编程语言的function

## 21.2 为什么要使用存储过程
- 通过把处理封装在容易使用的单元中，简化复杂的操作。
- 由于不要求反复建立一系列处理步骤，这保证了数据的完整性。如果所有开发人员和应用程序都使用同一（试验和测试）存储过程，则所使用的代码都是相同的。这一点的延伸就是防止错误。需要执行的步骤越多，出错的可能性就越大。防止错误保证了数据的一致性。
- 简化对变动的管理。如果表名、列名或业务逻辑（或别的内容）有变化，只需要更改存储过程的代码。使用它的人员甚至不需要知道这些变化。这一点的延伸就是安全性。通过存储过程限制对基础数据的访问减少了数据讹误（无意识的或别的原因所导致的数据讹误）的机会。
- 提高性能。因为使用存储过程比使用单独的SQL语句要快。
- 存在一些只能用在单个请求中的MySQL元素和特性，存储过程可以使用它们来编写功能更强更灵活的代码。

缺陷:
- 一般来说，存储过程的编写比基本SQL语句复杂，编写存储过程需要更高的技能，更丰富的经验。
- 你可能没有创建存储过程的安全访问权限。许多数据库管理员限制存储过程的创建权限，允许用户使用存储过程，但不允许他们创建存储过程。

## 21.3 使用存储过程
### 21.3.1 执行存储过程
> [!important]
> MySQL称存储过程的执行为调用，因此MySQL执行存储过程的语句为CALL。CALL接受存储过程的名字以及需要传递给它的任意参数。
> CALL \[procedure_name\](@argv1, ...);

```sql
CALL productpricing(@pricelow, @pricehigh, @priceaverage);
```

### 21.3.2 创建存储过程
> [!important]
> 用CREATE PROCEDURE \[procedure_name\];创建存储过程。

```sql
CREATE PROCEDURE productpricing()
BEGIN
  SELECT AVG(prod_price) AS priceaverage
  FROM products;
END;

CALL productpricing();
+--------------+
| priceaverage |
+--------------+
|    16.133571 |
+--------------+
1 row in set (0.01 sec)

Query OK, 0 rows affected (0.01 sec)

```

### 21.3.3 删除存储过程
> [!important]
> 使用DROP PROCEDURE \[procedure_name\];删除存储过程。

```sql
DROP PROCEDURE productpricing;
Query OK, 0 rows affected (0.01 sec)

```

### 21.3.4 使用参数
> [!note]
> 变量（variable）: 内存中一个特定的位置，用来临时存储数据。

> [!importatn]
> 关键字OUT指出相应的参数用来从存储过程传出一个值（返回给调用者）。MySQL支持IN（传递给存储过程）、OUT（从存储过程传出，如这里所用）和INOUT（对存储过程传入和传出）类型的参数。存储过程的代码位于BEGIN和END语句内，如下所见，它们是一系列SELECT语句，用来检索值，然后保存到相应的变量（通过指定INTO关键字）。
> 变量名: 所有MySQL变量都必须以@开始。

```sql
CREATE PROCEDURE productpricing(
  OUT pl DECIMAL(8, 2),
  OUT ph DECIMAL(8, 2),
  OUT pa DECIMAL(8, 2)
)
BEGIN
  SELECT MIN(prod_price)
  INTO pl
  FROM products;
  SELECT MAX(prod_price)
  INTO ph
  FROM products;
  SELECT AVG(prod_price)
  INTO pa
  FROM products;
END;
Query OK, 0 rows affected (0.04 sec)

-- 为调用此修改过的存储过程，必须指定3个变量名，如下所示：
CALL productpricing(@pricelow, @pricehigh, @priceaverage);
Query OK, 1 row affected, 1 warning (0.02 sec)

-- 为了显示检索出的产品平均价格，可如下进行：
SELECT @priceaverage;
+---------------+
| @priceaverage |
+---------------+
|         16.13 |
+---------------+
1 row in set (0.01 sec)

-- 为了获得3个值，可使用以下语句：
SELECT @pricelow, @pricehigh, @priceaverage;
+-----------+------------+---------------+
| @pricelow | @pricehigh | @priceaverage |
+-----------+------------+---------------+
|      2.50 |      55.00 |         16.13 |
+-----------+------------+---------------+
1 row in set (0.00 sec)

CREATE PROCEDURE ordertotal(
  IN onumber INT,
  OUT ototal DECIMAL(8, 2)
)
BEGIN
  SELECT SUM(item_price * quantity)
  FROM orderitems
  WHERE order_num = onumber
  INTO ototal;
END;
Query OK, 0 rows affected (0.11 sec)

CALL ordertotal(20005, @total);
Query OK, 1 row affected (0.03 sec)

SELECT @total;
+--------+
| @total |
+--------+
| 149.87 |
+--------+
1 row in set (0.00 sec)

```

### 21.3.5 建立智能存储过程
> [!important]
> 用DECLARE语句定义了两个局部变量。DECLARE要求指定变量名和数据类型，它也支持可选的默认值。

```sql
CREATE PROCEDURE ordertotal(
  IN onumber INT,
  IN taxable BOOLEAN,
  OUT ototal DECIMAL(8, 2)
) COMMENT 'Obtain order total, optionally adding tax'
BEGIN
  -- Declare variable for total
  DECLARE total DECIMAL(8, 2);
  -- Declare tax percentage
  DECLARE taxrate INT DEFAULT 6;

  -- Get the order total
  SELECT SUM(item_price * quantity)
  FROM orderitems
  WHERE order_num = onumber
  INTO total;

  -- IF语句检查taxable是否为真，如果为真，则用另一SELECT语句增加营业税到局部变量total。最后，用另一SELECT语句将total（它增加或许不增加营业税）保存到ototal。
  -- Is this taxable?
  IF taxable THEN
    -- Yes, so add taxrate to the total
    SELECT total + (total/100*taxrate) INTO total;
  END IF;
    -- And finally, save to out variable
    SELECT total INTO ototal;
  END;
END;
```
> [!tip]
> IF语句 这个例子给出了MySQL的IF语句的基本用法。IF语句还支持ELSEIF和ELSE子句（前者还使用THEN子句，后者不使用）。

### 21.3.6 检查存储过程
```sql
SHOW CREATE PROCEDURE ordertotal;
+------------+-----------------------------------------------------------------------------------------------------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+----------------------+----------------------+--------------------+
| Procedure  | sql_mode                                                                                                              | Create Procedure                                                                                                                                                                                                    | character_set_client | collation_connection | Database Collation |
+------------+-----------------------------------------------------------------------------------------------------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+----------------------+----------------------+--------------------+
| ordertotal | ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION | CREATE DEFINER=`root`@`localhost` PROCEDURE `ordertotal`(
  IN onumber INT,
  OUT ototal DECIMAL(8, 2)
)
BEGIN
  SELECT SUM(item_price * quantity)
  FROM orderitems
  WHERE order_num = onumber
  INTO ototal;
END | utf8mb4              | utf8mb4_0900_ai_ci   | utf8mb4_0900_ai_ci |
+------------+-----------------------------------------------------------------------------------------------------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+----------------------+----------------------+--------------------+
1 row in set (0.03 sec)

```