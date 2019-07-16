##python BSON 数据类型##

目前，SequoiaDB 支持多种 BSON 数据类型。详情请查看 [数据类型](data_model/datatype/datatype.md)一节。

##python 构造 BSON 数据类型##
对于一些特殊类型如$oid，在构造bson数据时需要使用python驱动中对应的类去构造，如$oid类型对应着ObjectId，特殊类详情请查看[Python API](api/python/html/index.html)。以下为构造示例。

* 整数/浮点数

	python BSON 整数/浮点数对象类型，示例数据：{"a": 123,"b":3.14}

  ```
  doc = {"a": 123,"b":3.14}
  ```
* 字符串

	python BSON 字符串对象类型，示例数据：{"key":"hello word"}

  ```
  doc = {"key":"hello word"}
  ```

* 列表

	python BSON 列表对象类型，示例数据：{"key":[1,2,3]}

  ```
  sublist = [1,2,3]
  doc = {"key":sublist }
  ```

* None

	python BSON None类型，示例数据：{"key":null}，None在数据库中会以null值存储。

  ```
  doc = {"key":None }
  ```

* 对象
	python BSON 构造嵌套对象类型，示例数据：{b:{a:1}}

  ```
  subObj = {"a":1}
  doc = {"b": subObj}
  ```
    或是

  ```
  doc = {b:{a:1}}
  ```

* 对象 ID($oid)

	python BSON 构造对象ID类型 ，示例数据：{"_id":{"$oid":"5d035e2bb4d450b04fcd0dff"}}

  ```
  #ObjectId()可以不传入参数，这样会自动生成一个id值,推荐使用这种方式。
  oid = ObjectId()
  doc = {"_oid" : oid}
  ```
  ```
  #传入一个str类型参数
  oid = ObjectId("5d035e2bb4d450b04fcd0dff")
  doc = {"_oid" : oid}
  ```
  ```
  #传入Unicode编码的参数
  oid = ObjectId(u'666f6f2d6261722d71757578') 
  doc = {"_oid" : oid}
  ```


* 二进制数据（$binary）

	python BSON 构造二进制数类型，示例数据：{"rest": {"$binary": "QUJD","$type": "0"}}

  ```lang-Python
  binary = Binary("ABC")
  doc = {"rest" : binary}
  ```

* 高精度数（$decimal）

   pyhton BSON 构造不带精度要求的Decimal类型，示例数据： {a:{"$decimal":"12345.067891234567890123456789"}}

  ```lang-Python
  decimalObj = Decimal("12345.067891234567890123456789"） 
  doc = { 'rest':decimalObj }
  ```

  python BSON 构造一个最多有100位有效数字，其中小数部分最多有30位的Decimal类型，示例数据： {b:{"$decimal":"12345.067891234567890123456789", "$precision":[100, 30]}}

  ```
  decimalObj =  Decimal("12345.067891234567890123456789", 1000, 30)
  doc = { 'rest':decimalObj  }
  ```
 

* 正则表达式

  python BSON 构造正则表达式数据类型，示例数据：{"regex":{"$regex": "\^w","$options": ""}

  ```lang-python
	regexObj = Regex("^w")
	doc = {"regex":regexObj }
  ```

* 日期

  python BSON 使用 datetime的datetime类型来构造日期类型，示例数据：{date:{"$date": "2019-07-12"}}

  ```lang-python
	import datetime
	date = datetime.datetime(2019,07,12)
	doc = {"date":date}
  ```


* 时间戳

  python BSON 使用 Timestamp来构造时间戳类型，示例数据：{"rest": {"$timestamp": "2003-12-01-08.00.00.000000"}}

  ```lang-python
	time = datetime.datetime(2003,01,02,03,04,05)
	timestamp = Timestamp(time,0)
	doc = {"rest":timestamp}
  ```

##注意事项##

* 特殊类型的使用注意

	说明：对于一些特殊类型如$oid，如果是直接指定，如下eg所示的构造方式，在进行dict转bson时会被认为是普通的嵌套类型，而不是$oid类型。

	```
	eg = {"_oid" : {"$oid":5d035e2bb4d450b04fcd0dff}}
	```
	以下是一个根据$oid删除数据错误的使用示例。根据{"_id":	{"$oid":"5d035e2bb4d450b04fcd0dff"}}去删除对应的数据时，	condition会被认为是一个普通的嵌套类型，而导致删除失败。

  	```
	 cl = db.get_collection("foo.bar");  
 	condition = {"_id":{"$oid":"5d035e2bb4d450b04fcd0dff"}}; 
 	cl.delete ( condition=condition ); 
	  ```

	正确使用示例如下，需要使用ObjectId类构建对应的$oid对象，之后再执行删除操作。其他特殊类型的使用与之类似。

	 ```
	from bson import * 
	oid = ObjectId("5d035e2bb4d450b04fcd0dff");	 
	condition = {"_id":oid}; 
	cl.delete ( condition=condition ); 
	 ```

