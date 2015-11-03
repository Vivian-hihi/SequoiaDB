## Java BSON 数据类型##

目前，SequoiaDB 支持多种 BSON 数据类型。详情请查看数据库概念 - 数据库 - 文档一节。

## Java 构造 BSON 数据类型##

* 整数/符浮点数

<pre class="prettyprint lang-javascript">
Java BSON 构造整数/符浮点数类型// {a:123, b:3.14}
BSONObject obj = new BasicBSONObject();
obj.put("a", 123);
obj.put("b", 3.14);</pre>

* 字符串

<pre class="prettyprint lang-javascript">
Java BSON 构造字符串类型// {a:"hi"}
BSONObject obj = new BasicBSONObject();
obj.put("a", "hi");</pre>

* 空类型

<pre class="prettyprint lang-javascript">
Java BSON 构造空类型// {a:null}
BSONObject obj = new BasicBSONObject();
obj.put("a", null);</pre>

* 对象

<pre class="prettyprint lang-javascript">
Java BSON 构造嵌套对象类型// {b:{a:1}}
BSONObject subObj = new BasicBSONObject();
subObj.put("a", 1);
BSONObject obj = new BasicBSONObject();
obj.put("b", subObj);</pre>

* 数组

<pre class="prettyprint lang-javascript">
Java BSON 使用 org.bson.types.BasicBSONList 来构造数组类型// {a:[0,1,2]}
BSONObject obj = new BasicBSONObject();
BSONObject arr = new BasicBSONList();
arr.put("0", 0);
arr.put("1", 1);
arr.put("2", 2);
obj.put("a", arr);</pre>

* 布尔

<pre class="prettyprint lang-javascript">
Java BSON 构造布尔类型// {a:true, b:false}
BSONObject obj = new BasicBSONObject();
obj.put("a", true);
obj.put("b", false);</pre>

* 对象 ID

Java BSON 使用 org.bson.types.ObjectId 来生成每条记录的“\_id”字段内容。Java BSON 12 字节的 ObjectId 与[文档](SdbDoc_Cn/data_model/document.html)一节介绍的对象 ID 略有不同，目前，Java ObjectId 的12字节内容由三部分组成：4字节精确到秒的时间戳，4字节系统（物理机）标示，4字节由随机数起始的序列号。默认情况下，数据库为每条记录生成一个字段名为“\_id”的唯一对象 ID。

<pre class="prettyprint lang-javascript">
BSONObject obj = new BasicBSONObject();
ObjectId id1 = new ObjectId();
ObjectId id2 = new ObjectId("53bb5667c5d061d6f579d0bb");
obj.put("_id", id1);</pre>

* 正则表达式

Java BSON 使用 java.util.regex.Pattern 来构造正则表达式数据类型。

<pre class="prettyprint lang-javascript">
BSONObject matcher = new BasicBSONObject();
Pattern obj = Pattern.compile("^2001",Pattern.CASE_INSENSITIVE);
matcher.put("serial_num", obj);
BSONObject modifier = new BasicBSONObject("$set", new BasicBSONObject("count",1000));
cl.update(matcher, modifier, null);</pre>

以上使用正则表达式构造了一个匹配条件，将序列号以“2001”开头的记录的“count”字段内容改为“1000”。

**Note:**

以上使用 Patten 构造的 bson matcher，当使用 matcher.toString()，内容为：
<pre class="prettyprint lang-diy">
{ "serial_num" : { "$options" : "i" , "$regex" : "^2001"}}</pre>

通过以下 bson 构造方式也可以得到相同的内容：

<pre class="prettyprint lang-javascript">
BSONObject matcher2 = new BasicBSONObject();
BSONObject obj2 = new BasicBSONObject();
obj2.put("$regex","^2001");
obj2.put("$options","i");
matcher2.put("serial_num", obj2);</pre>

但是，通过后者构造出的 matcher2 的数据类型是一个普通的对象嵌套类型，而不是正则表达式类型。

* 日期

Java BSON 使用 java.util.Date 来构造日期类型。

<pre class="prettyprint lang-javascript">
BSONObject obj = new BasicBSONObject();
Date now = new Date();
obj.put("date", now);</pre>

* 二进制

Java BSON 使用 org.bson.types.Binary 来构造二进制类型。

<pre class="prettyprint lang-javascript">
BSONObject obj = new BasicBSONObject();
String str = "hello world";
byte[] arr = str.getBytes();
Binary bindata = new Binary(arr);
obj.put("bindata", bindata);</pre>

* 时间戳

Java BSON 使用 org.bson.types.BSONTimestamp 来构造时间戳类型。

<pre class="prettyprint lang-javascript">
String mydate = "2014-07-01 12:30:30.124232";
String dateStr = mydate.substring(0, mydate.lastIndexOf('.'));
String incStr = mydate.substring(mydate.lastIndexOf('.') + 1);
        
SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
Date date = format.parse(dateStr);
int seconds = (int)(date.getTime()/1000);
int inc = Integer.parseInt(incStr);
BSONTimestamp ts = new BSONTimestamp(seconds, inc);
        
BSONObject obj = new BasicBSONObject();
obj.put("timestamp", ts);</pre>
