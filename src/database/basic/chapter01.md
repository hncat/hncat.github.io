---
title: 第一章 使用mysql
date: 2025-08-23
tags:
    - 计算机基础
    - 数据库
categories: mysql
isOriginal: true
order: 1
dir:
    order: 1
---
## 1.1 连接
### mysql的选项和参数
```zsh
~ > mysql -hlocalhost -P3306 -uroot -p'12345678' # mysql指令
mysql: [Warning] Using a password on the command line interface can be insecure.
Welcome to the MySQL monitor.  Commands end with ; or \g.
Your MySQL connection id is 9
Server version: 8.0.43-0ubuntu0.24.04.1 (Ubuntu)

Copyright (c) 2000, 2025, Oracle and/or its affiliates.

Oracle is a registered trademark of Oracle Corporation and/or its
affiliates. Other names may be trademarks of their respective
owners.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

mysql> 
```
> -h: 主机名
> -P: 端口(一般为3306)
> -u: 用户名
> -p: 用户密码
## 1.2 选择数据库
```sql
USE crashcourse;
Database changed
```
## 1.3 了解数据库和表
```sql
SHOW DATABASES; -- 查看数据库
+--------------------+
| Database           |
+--------------------+
| crashcourse        |
| information_schema |
| mysql              |
| performance_schema |
| sys                |
+--------------------+
5 rows in set (0.00 sec)

SHOW TABLES; -- 查看表
+-----------------------+
| Tables_in_crashcourse |
+-----------------------+
| customers             |
| orderitems            |
| orders                |
| productnotes          |
| products              |
| vendors               |
+-----------------------+
6 rows in set (0.00 sec)

show columns from customers; -- 查看表结构
+--------------+-----------+------+-----+---------+----------------+
| Field        | Type      | Null | Key | Default | Extra          |
+--------------+-----------+------+-----+---------+----------------+
| cust_id      | int       | NO   | PRI | NULL    | auto_increment |
| cust_name    | char(50)  | NO   |     | NULL    |                |
| cust_address | char(50)  | YES  |     | NULL    |                |
| cust_city    | char(50)  | YES  |     | NULL    |                |
| cust_state   | char(5)   | YES  |     | NULL    |                |
| cust_zip     | char(10)  | YES  |     | NULL    |                |
| cust_country | char(50)  | YES  |     | NULL    |                |
| cust_contact | char(50)  | YES  |     | NULL    |                |
| cust_email   | char(255) | YES  |     | NULL    |                |
+--------------+-----------+------+-----+---------+----------------+
9 rows in set (0.10 sec)

DESCRIBE customers; -- 和使用 show columns form <表名>一个效果
+--------------+-----------+------+-----+---------+----------------+
| Field        | Type      | Null | Key | Default | Extra          |
+--------------+-----------+------+-----+---------+----------------+
| cust_id      | int       | NO   | PRI | NULL    | auto_increment |
| cust_name    | char(50)  | NO   |     | NULL    |                |
| cust_address | char(50)  | YES  |     | NULL    |                |
| cust_city    | char(50)  | YES  |     | NULL    |                |
| cust_state   | char(5)   | YES  |     | NULL    |                |
| cust_zip     | char(10)  | YES  |     | NULL    |                |
| cust_country | char(50)  | YES  |     | NULL    |                |
| cust_contact | char(50)  | YES  |     | NULL    |                |
| cust_email   | char(255) | YES  |     | NULL    |                |
+--------------+-----------+------+-----+---------+----------------+
9 rows in set (0.00 sec)

```
> 其它mysql的SHOW语句
> SHOW STATUS: 查看服务器状态信息
> SHOW CREATE DATABASES和SHOW CREATE TABLE: 分别用来显示创建特定数据库或表的Mysql语句
> SHOW GRANTS: 用来显示授予用户的安全权限
> SHOW ERRORS和SHOW WARNINGS: 用来显示服务器错误或警告消息
> 使用HELP SHOW: 显示SHOW语句