使用hint显示地控制执行计划。

##使用方法##

hint的格式为"/\*+hint1 hint2 ...\*/"。但我们希望控制某个SELECT语句时，只需要在这个SELECT语句结尾处增加hint即可。属于同一个SELECT语句的hint使用空格分隔。

##hint列表##

###use_hash###

* 指定关联方式为哈希关联

<pre class="prettyprint lang-javascript">
SELECT t1.a, t2.b FROM foo.bar1 AS t1 INNER JOIN foo.bar2 AS t2 ON t1.a = t2.b /*+use_hash()*/</pre>

###use_index###

* 指定集合的扫描方式

**使用索引"myindex"进行扫描**
<pre class="prettyprint lang-javascript">
SELECT * FROM foo.bar WHERE a = 1 /*+use_index(myindex)*/</pre>

**在关联中指定某个集合使用索引"myindex"进行扫描**
<pre class="prettyprint lang-javascript">
SELECT t1.a, t2.b FROM foo.bar1 AS t1 INNER JOIN foo.bar2 AS t2 ON t1.a = t2.b /*+use_index(t1, myindex)*/</pre>

**在一个SELECT语句中使用多个hint**
<pre class="prettyprint lang-javascript">
SELECT t1.a, t2.b FROM foo.bar1 AS t1 INNER JOIN foo.bar2 AS t2 ON t1.a = t2.b /*+use_index(t1, myindex1) use_index(t2, myindex2) use_hash()*/</pre>

**指定集合不使用索引**
<pre class="prettyprint lang-javascript">
SELECT * FROM foo.bar WHERE a = 1 /*+use_index(NULL)*/</pre>

**在嵌套查询中的不同SELECT语句中使用hint**
<pre class="prettyprint lang-javascript">
SELECT t1.a, t2.b, t2.cnt
FROM foo.bar1 AS t1
     INNER JOIN (
           SELECT t3.b, t3.cnt, t4.c
           FROM(
               SELECT COUNT(a) AS cnt, b
               FROM foo.bar2
               GROUP BY b
           ) AS t3
                   INNER JOIN foo.bar3 AS t4 ON t3.b = t4.c /*+use_hash() use_index(t4, aaa)*/
      ) AS t2 ON t1.a = t2.b /*+use_index(t1, a) */</pre>