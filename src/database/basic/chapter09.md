---
title: 第九章 使用数据处理函数
date: 2025-08-24
tags:
    - 计算机基础
    - 数据库
categories: mysql
isOriginal: true
order: 10
dir:
    order: 10
---
## 9.1 使用函数
### 9.1.1 文本处理函数
|函数|说明|
|:-:|:-:|
|Left()|返回串左边的字符|
|Length()|返回串的长度|
|Locate()|找出串的一个子串|
|Lower()|将串转换为小写|
|LTrim()|去掉串左边的空格|
|Right()|返回串右边的字符|
|RTrim()|去掉串右边的空格|
|Soundex()|返回串的SOUNDEX值|
|SubString()|返回子串的字符|
|Upper()|将串转换为大写|
```sql
select vend_name, upper(vend_name) as vend_name_upcase from vendors order by vend_name;
+----------------+------------------+
| vend_name      | vend_name_upcase |
+----------------+------------------+
| ACME           | ACME             |
| Anvils R Us    | ANVILS R US      |
| Furball Inc.   | FURBALL INC.     |
| Jet Set        | JET SET          |
| Jouets Et Ours | JOUETS ET OURS   |
| LT Supplies    | LT SUPPLIES      |
+----------------+------------------+
6 rows in set (0.01 sec)

```
> [!node]
> SOUNDEX是一个将任何文本串转换为描述其语音表示的字母数字模式的算法。SOUNDEX考虑了类似的发音字符和音节，使得能对串进行发音比较而不是字母比较。虽然SOUNDEX不是SQL概念，但MySQL（就像多数DBMS一样）都提供对SOUNDEX的支持。

```sql
select cust_name, cust_contact from customers where cust_contact = 'Y. Lie';
Empty set (0.01 sec)

select cust_name, cust_contact from customers where Soundex(cust_contact) = Soundex('Y. Lie');
+-------------+--------------+
| cust_name   | cust_contact |
+-------------+--------------+
| Coyote Inc. | Y Lee        |
+-------------+--------------+
1 row in set (0.01 sec)

```
> 在这个例子中，WHERE子句使用Soundex()函数来转换cust_contact列值和搜索串为它们的SOUNDEX值。因为Y.Lee和Y.Lie发音相似，所以它们的SOUNDEX值匹配，因此WHERE子句正确地过滤出了所需的数据。

### 9.1.2 日期和时间处理函数
|函数|说明|
|:-:|:-:|
|AddDate()|增加一个日期（天、周等）|
|AddTime()|增加一个时间（时、分等）|
|CurDate()|返回当前日期|
|CurTime()|返回当前时间|
|Date()|返回日期时间的日期部分|
|DateDiff()|计算两个日期之差|
|Date_Add()|高度灵活的日期运算函数|
|Date_Format()|返回一个格式化的日期或时间串|
|Day()|返回一个日期的天数部分|
|DayOfWeek()|对于一个日期，返回对应的星期几|
|Hour()|返回一个时间的小时部分|
|Minute()|返回一个时间的分钟部分|
|Month()|返回一个日期的月份部分|
|Now()|返回当前日期和时间|
|Second()|返回一个时间的秒部分|
|Time()|返回一个日期时间的时间部分|
|Year()|返回一个日期的年份部分|
> [!warning]
> 首先需要注意的是MySQL使用的日期格式。不管是插入或更新表值还是用WHERE子句进行过滤，日期必须为格式yyyy-mm-dd。因此，2005年9月1日，给出为2005-09-01。虽然其他的日期格式可能也行，但这是首选的日期格式，因为它排除了多义性（如，04/05/06是2006年5月4日或2006年4月5日或2004年5月6日或……）。

```sql
select cust_id, order_num, order_date from orders where date(order_date) = '2005-09-01';
+---------+-----------+---------------------+
| cust_id | order_num | order_date          |
+---------+-----------+---------------------+
|   10001 |     20005 | 2005-09-01 00:00:00 |
+---------+-----------+---------------------+
1 row in set (0.01 sec)

```
> [!warning]
> 但是，使用WHERE order_date = '2005-09-01'可靠吗？order_ date的数据类型为datetime。这种类型存储日期及时间值。样例表中的值全都具有时间值00:00:00，但实际中很可能并不总是这样。如果用当前日期和时间存储订单日期（因此你不仅知道订单日期，还知道下 订 单 当 天 的 时 间 ）， 怎 么 办 ？ 比 如 ， 存 储 的 order_date 值 为2005-09-01 11:30:05，则WHERE order_date = '2005-09-01'失败。即使给出具有该日期的一行，也不会把它检索出来，因为WHERE匹配失败。

```sql
-- 检索出2005年9月所有订单
select cust_id, order_num, order_date from orders where date(order_date) between '2005-09-01' and '2005-09-30';

+---------+-----------+---------------------+
| cust_id | order_num | order_date          |
+---------+-----------+---------------------+
|   10001 |     20005 | 2005-09-01 00:00:00 |
|   10003 |     20006 | 2005-09-12 00:00:00 |
|   10004 |     20007 | 2005-09-30 00:00:00 |
+---------+-----------+---------------------+
3 rows in set (0.07 sec)

--  方法2: 使用year和month
select cust_id, order_num, order_date from orders where year(order_date) = 2005 and month(order_date) = 9;
+---------+-----------+---------------------+
| cust_id | order_num | order_date          |
+---------+-----------+---------------------+
|   10001 |     20005 | 2005-09-01 00:00:00 |
|   10003 |     20006 | 2005-09-12 00:00:00 |
|   10004 |     20007 | 2005-09-30 00:00:00 |
+---------+-----------+---------------------+
3 rows in set (0.06 sec)

```
### 9.1.3 数值处理函数
|函数|说明|
|:-:|:-:|
|Abs()|返回一个数的绝对值|
|Cos()|返回一个角度的余弦|
|Exp()|返回一个数的指数值|
|Mod()|返回除操作的余数|
|Pi()|返回圆周率|
|Rand()|返回一个随机数|
|Sin()|返回一个角度的正弦|
|Sqrt()|返回一个数的平方根|
|Tan()|返回一个角度的正切|