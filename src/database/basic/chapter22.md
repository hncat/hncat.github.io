---
title: 第二十二章 游标
date: 2025-08-24
tags:
    - 计算机基础
    - 数据库
categories: mysql
isOriginal: true
order: 22
dir:
    order: 22
---
## 22.1 游标
> [!important]
> 游标（cursor）是一个存储在MySQL服务器上的数据库查询，它不是一条SELECT语句，而是被该语句检索出来的结果集。在存储了游标之后，应用程序可以根据需要滚动或浏览其中的数据。

> [!node]
> 只能用于存储过程 不像多数DBMS，MySQL游标只能用于存储过程（和函数）。

## 22.2 使用游标
1. 在能够使用游标前，必须声明（定义）它。这个过程实际上没有检索数据，它只是定义要使用的SELECT语句。
2. 一旦声明后，必须打开游标以供使用。这个过程用前面定义的SELECT语句把数据实际检索出来。
3. 对于填有数据的游标，根据需要取出（检索）各行。
4. 在结束游标使用时，必须关闭游标。

### 22.2.1 创建游标
> [!important]
> 游标用DECLARE语句创建。DECLARE命名游标，并定义相应的SELECT语句，根据需要带WHERE和其他子句。

```sql
CREATE PROCEDURE processorders()
BEGIN
  DECLARE ordernumbers CURSOR -- DECLARE语句用来定义和命名游标，这里为ordernumbers。
  FOR
  SELECT order_num FROM orders;
END;
```

### 22.2.2 打开和关闭游标
> [!important]
> 游标用OPEN打开，CLOSE关闭

```sql
OPEN ordernumbers; -- 打开游标

CLOSE ordernumbers; -- 关闭游标

CREATE PROCEDURE processorders()
BEGIN
  DECLARE ordernumbers CURSOR -- DECLARE语句用来定义和命名游标，这里为ordernumbers。
  FOR
  SELECT order_num FROM orders;
  -- Open the cursor
  OPEN ordernumbers;

  -- Close the cursor
  CLOSE ordernumbers;
END;
```

> [!tip]
> 隐含关闭 如果你不明确关闭游标，MySQL将会在到达END语句时自动关闭它。

### 22.2.3 使用游标
> [!important]
> 在一个游标被打开后，可以使用FETCH语句分别访问它的每一行。FETCH指定检索什么数据（所需的列），检索出来的数据存储在什么地方。它还向前移动游标中的内部行指针，使下一条FETCH语句检索下一行（不重复读取同一行）。

```sql
CREATE PROCEDURE processorders()
BEGIN
  -- Declare local variables
  DECLARE done BOOLEAN DEFAULT 0;
  DECLARE o INT;

  DECLARE ordernumbers CURSOR -- DECLARE语句用来定义和命名游标，这里为ordernumbers。
  FOR
  SELECT order_num FROM orders;

  -- Declare continue handler
  -- 这条语句定义了一个CONTINUE HANDLER，它是在条件出现时被执行的代码
  -- 当SQLSTATE '02000'出现时，SET done=1。SQLSTATE '02000'是一个未找到条件，当REPEAT由于没有更多的行供循环而不能继续时，出现这个条件。
  DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' SET done = 1;

  -- Open the cursor
  OPEN ordernumbers;

  -- Loop through all rows
  REPEAT
    -- Get order number
    -- 中FETCH用来检索当前行的order_num列（将自动从第一行开始）到一个名为o的局部声明的变量中。对检索出的数据不做任何处理。
    FETCH ordernumbers INTO o;

  UNTIL done END REPEAT;

  -- Close the cursor
  CLOSE ordernumbers;
END;
```