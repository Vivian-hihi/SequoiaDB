##数据迁移 — 导入##

sdbimprt是SequoiaDB的数据导入工具，它可以将JSON格式或CSV格式的数据导入到SequoiaDB数据库中。

###JSON###

JSON格式的记录必须符合JSON的定义，以左右花括号作为记录的分界符，并且字符串类型的数据必须包含在两个双引号之间，转义字符为反斜杠'\\'。

###CSV###

CSV(Comma Separated Value)格式以逗号分隔数值。默认情况下记录以换行符分隔，字段以逗号分隔。用户能够指定字符串分隔符、字段分隔符以及记录分隔符。

###分隔符###

 | 类型 | 默认值 | 
 | --- | ------ | 
 | 字符串分隔符 | " | 
 | 字段分隔符 | , | 
 | 记录分隔符 | 换行符('\\n') | 

注意：

- 分隔符可以使用ASCII码中的不可见字符，通过转义字符'\\'输入ASCII码的十进制数值（0~127），例如'\\30'。回车符、换行符、制表符、转义字符'\\'可以直接使用'\\r'，'\\n'，'\\t'，'\\\\'。
- 可以使用UTF-8字符作为分隔符。
- 可以使用多个字符作为分隔符。

###CSV类型###

 | 类型 | 别名 | 说明 | 
 | --- | ---- | --- | 
 | int | integer | 十进制整型，取值范围为-2147483648~2147483647 | 
 | long | - | 十进制长整型，取值范围-9223372036854775808~9223372036854775807 | 
 | double | - | 双精度浮点型，取值范围为-1.7E+308~1.7E+308 | 
 | number | - | 数值类型，自动判断数值的具体类型(int, long, double) | 
 | bool | boolean | 布尔型，取值为true/false/t/f/yes/no/y/n，不区分大小写 | 
 | string | - | 字符串 | 
 | null | - | 空值 | 
 | oid | - | OID类型，长度必须为24个字符，不支持类型自动判断 | 
 | date | - | 日期类型，取值范围为-9999-01-01~9999-12-31，不支持类型自动判断 | 
 | timpestamp | - | 时间戳类型，取值范围为1901-12-15-00.00.00.000000~2038-01-18-23.59.59.999999，不支持类型自动判断 | 
 | binary | - | 二进制类型，使用base64编码，不支持类型自动判断 | 
 | regex | - | 正则表达式类型，不支持类型自动判断 | 

注意：

- int、long、double支持以‘#’开头的数字，例如#123.456。
- double支持科学计数法，例如1.23e-4，-1.23E+4。
- date类型支持使用整数，表示自1970-01-01-00.00.00.000000以来的秒数, 取值范围为-377705145943~253402271999。
- timestamp类型支持使用整数，表示自1970-01-01-00.00.00.000000以来的毫秒数，取值范围为-2147414400000~2147443199000。

###CSV类型自动判断

在不指定CSV字段类型时，导入工具会自动判断类型。其中oid、date、timestamp、binary、regex不支持自动类型判断，会被识别为string类型。超出long类型范围的数值会作为double类型处理。

例如：

 | CSV数据 | 判断类型 | 实际数据 | 
 | ------ | ------- | ------- | 
 | 123 | int | 123 | 
 | 123. | int | 123 | 
 | +123 | int | 123 | 
 | -123 | int | -123 | 
 | 0123 | int | 123 | 
 | #-123. | int | -123 | 
 | 2147483648 | long | 2147483648 | 
 | 123.1 | double | 123.1 | 
 | 9223372036854775808 | double | 9.223372036854776e+18 | 
 | true | bool | true | 
 | false | bool | false | 
 | "123" | string | "123" | 
 | 123a | string | "123a" | 
 | "true" | string | "true" | 
 | "false" | string | "false" | 
 | "null" | string | "null" | 
 | null | null | null | 

###CSV类型转换

在指定CSV字段类型时，导入工具会将字段转换为指定的类型。如果字段的实际类型不是指定的类型，则转换可能失败。具体参考下表，最左边一列是指定的类型，Y表示可以转换，N表示不能转换。

 | 指定类型\\实际类型 | int | long | double | bool | string | null | oid | date | timestamp | binary | regex | 
 | - | --- | ---- | ------ | ------ | ---- | ------ | ---- | --- | ---- | --------- | ------ | ----- | 
 | int | Y | 可能溢出 | 可能丢失精度 | Y | 支持数值字符串 | N | N | N | N | N | N | 
 | long | Y | Y | 可能丢失精度 | Y | 支持数值字符串 | N | N | N | N | N | N | 
 | double | Y | Y | Y | N | 支持数值字符串 | N | N | N | N | N | N | 
 | number | Y | Y | Y | Y | 支持数值字符串 | N | N | N | N | N | N | 
 | bool | Y | Y | N | Y | 支持bool字符串 | N | N | N | N | N | N | 
 | string | Y | Y | Y | Y | Y | Y | Y | Y | Y | Y | Y | 
 | null | Y | Y | Y | Y | Y | Y | Y | Y | Y | Y | Y | 
 | oid | N | N | N | N | 支持OID字符串 | N | Y | N | N | N | N | 
 | date | Y | Y | N | N | 支持date字符串 | N | N | Y | Y | N | N | 
 | timestamp | Y | Y | N | N | 支持timestamp字符串 | N | N | Y | Y | N | N | 
 | binary | N | N | N | N | 支持binary字符串 | N | N | N | N | Y | N | 
 | regex | N | N | N | N | 支持regex字符串 | N | N | N | N | N | Y | 

注意：

- 指定类型为bool，实际类型为int、long时，0值转为false，非0值转为true。
- 指定类型为int、long，实际类型为bool时，true/t/yes/y转为1，false/f/no/n转为0。
- 参数--cast可以指定数值转换时是否允许精度损失或数值溢出。

###命令选项

**通用选项**

 | 选项 | 缩写 | 说明 | 
 | --- | ---- | --- | 
 | --help | -h | 显示帮助信息 | 
 | --version | -V | 显示版本号 | 
 | --hosts | - | 指定主机地址(hostname:svcname)，用','分隔多个地址，默认为'localhost:11810' | 
 | --user | -u | 数据库用户名 | 
 | --password | -w | 数据库用户名对应的密码 | 
 | --csname | -c | 集合空间的名字 | 
 | --clname | -l | 集合的名字 | 
 | --errorstop | - | 如果遇到解析错误就停止，默认为false | 
 | --ssl | - | 使用SSL连接，默认为false | 
 | --verbose | -v | 显示详细的执行信息 | 

**输入选项**

 | 选项 | 缩写 | 说明 | 
 | --- | ---- | --- | 
 | --file | - | 要导入的数据文件的名称，使用','分隔多个文件或目录。如果--file和--exec都没有指定，则从标准输入读取数据 | 
 | --exec | - | 执行外部程序来获取数据，外部程序必须将数据输出到标准输出 | 
 | --type | - | 导入数据格式，可以是csv或json，默认为csv | 
 | --linepriority | - | 当前分隔符默认的优先级为：记录分隔符，字符串分隔符，字段分隔符，默认值是true；如果设置为 false，那么分隔符的优先级为：字符串分隔符，记录分隔符，字段分隔符 | 
 | --delrecord | -r | 指定记录分隔符，默认是换行符'\\n' | 
 | --force | - | 如果数据中有非UTF-8的字符，强制导入数据，默认为false | 

注意:

- linepriority参数需要被特别关注，如果设置不当，可能会导入数据失败。当记录中包含“记录分隔符”并且 linepriority为 true 时，工具会优先按照“记录分隔符”解析，而导致导入失败。比如：如果记录为 {"name": "Mike\\n"}，应当设置 linepriority为false。
- 使用file参数指定文件或目录时，重复出现的文件会被忽略。
- 使用hosts指定地址时，重复出现的地址会被忽略。

**CSV选项**

 | 选项 | 缩写 | 说明 | 
 | --- | ---- | --- | 
 | --delchar | -a | 指定字符串分隔符，默认是双引号'"' | 
 | --delfield | -e | 指定字段分隔符,默认是逗号',' | 
 | --fields | - | 指定导入数据的字段名、类型、默认值 | 
 | --datefmt | - | 指定日期格式，默认为YYYY-MM-DD | 
 | --timestampfmt | - | 指定时间戳格式，默认为YYYY-MM-DD-HH.mm.ss.ffffff | 
 | --trim | - | 删除字符串左右两侧的空格，取值可以是[no|right|left|both]， 默认为no | 
 | --headerline | - | 指定导入数据首行是否作为字段名，默认为false | 
 | --sparse | - | 指定导入数据时是否自动添加字段名，默认为true。字段名按"filed1"、"field2"顺序增加 | 
 | --extra | - | 指定导入数据时是否自动添加值，默认为false | 
 | --cast | - | 指定是否允许数值类型转换时丢失精度或数值溢出，默认为false | 

注意：

- fields语法：fieldName [type [default &lt;value>], ...]
	- type支持所有的CSV类型
	- type可不写，由导入工具自动判断
	- 指定字段可以用命令行指定，也可以在导入文件的首行指定。如果在命令行指定了--fields，并且--headerline设为true，导入工具将会优先使用命令行指定字段并且跳过导入文件的首行
	- 字段名不能以'$'开头，中间不能有'.'，不能有不可见字符，包含空格时需要将字段名用单引号或双引号引起来
	- 例如：--fields='name string default "Jack", age int default 18, phone'
- datefmt格式包括年、月、日、通配符以及特定字符
	- 年：YYYY
	- 月：MM
	- 日：DD
	- 通配符：*
	- 特定字符：任意UTF-8字符
	- 其中年、月、日必须是整数，并且符合日期类型的范围
	- 指定通配符时，日期字段上对应的位置可以为任意字符
	- 指定特定字符时，日期字段上对应的位置必须为该指定字符
	- 例如：--datefmt="MM/DD, YYYY"，字段"3/15, 2015"与该格式匹配
- timestamp格式包括年、月、日、时、分、秒、微秒或毫秒、通配符以及特定字符
	- 年：YYYY
	- 月：MM
	- 日：DD
	- 时：HH
	- 分：mm
	- 秒：ss
	- 微秒：ffffff
	- 毫秒：SSS
	- 通配符：*
	- 特定字符：任意UTF-8字符
	- 其中年、月、日、时、分、秒、微秒、毫秒必须是整数，并且符合时间戳类型的范围
	- 微秒和毫秒不能同时出现，只能出现其中一个
	- 指定通配符时，时间戳字段上对应的位置可以为任意字符
	- 指定特定字符时，时间戳字段上对应的位置必须为该指定字符
	- 例如：--timestampfmt="MM/DD/YYYY T mm.ss.SSS"，字段"3/15/2015 T 12.30.123"与该格式匹配

**导入选项**

 | 选项 | 缩写 | 说明 | 
 | --- | ---- | --- | 
 | --insertnum | -n | 指定每次导入的记录数，取值范围为1~100000，默认为100 | 
 | --jobs | -j | 指定导入连接数（每个连接一个线程），取值范围为1~1000，默认为1 | 
 | --coord | - | 指定是否自动查找协调节点，默认为true | 
 | --sharding | - | 指定是否按分区信息重新打包记录，默认为true | 
 | --transaction | - | 指定导入数据时是否开启事务，默认为false。注意此功能需要服务端开启事务。| 

###示例

1. 将数据导入到本地数据库11810中集合空间foo的集合bar，导入格式是csv，数据文件为test.csv，第一行为字段定义

<pre class="prettyprint lang-javascript">
$ sdbimprt --hosts=localhost:11810 --type=csv --file=test.csv -c foo -l bar --headerline=true</pre>

2. 导入格式是csv，文件名是test.csv，导入至集合空间foo的集合bar中

以下是导入文件的内容：

<pre class="prettyprint lang-diy">
"Jack",18,"China"
"Mike",20,"USA"</pre>

导入命令：

<pre class="prettyprint lang-javascript">
$ sdbimprt --hosts=localhost:11810 --type=csv --file=test.csv -c foo -l bar --fields='name string default "Anonymous", age int, country'</pre>

3. 导入格式是csv，文件名是test.csv，导入至集合空间foo的集合bar中

以下是导入文件的内容：

<pre class="prettyprint lang-diy">
name, age, country
"Jack",18,"China"
"Mike",20,"USA"</pre>

文件第一行是字段定义，需要跳过。
导入命令：

<pre class="prettyprint lang-javascript">
$ sdbimprt --hosts=localhost:11810 --type=csv --file=test.csv -c foo -l bar --fields='name string default "Anonymous", age int, country' --headerline=true</pre>

4. 导入格式是csv，导入文件是目录../data中的所有文件，导入至集合空间foo的集合bar中

<pre class="prettyprint lang-javascript">
$ sdbimprt --hosts=localhost:11810 --type=csv --file=../data -c foo -l bar</pre>

5. 导入格式是csv，导入文件是目录../data中的所有文件以及./foo_bar_data.csv，导入至集合空间foo的集合bar中，有11810和11910两个协调节点，记录中时间戳类型的数据类似于"2015-10-01 T 12.31.15.123 T"，使用两个连接同时导入

<pre class="prettyprint lang-javascript">
$ sdbimprt --type=csv --file=../data,./foo_bar_data.csv -c foo -l bar --timestampfmt="YYYY-MM-DD T HH.mm.ss.SSS T" --hosts=localhost:11810,localhost:11910 -j 2</pre>

6. 导入格式是json，通过管道从其它工具other获取数据，导入至集合空间foo的集合bar中，第一行为字段定义

<pre class="prettyprint lang-javascript">
$ other | sdbimprt --hosts=localhost:11810 --type=json -c foo -l bar --headerline=true</pre>

##数据迁移 — 导出##
sdbexprt 是一个实用的工具。它可以从 SequoiaDB 数据库导出一个 JSON 格式或者 CSV 格式的数据存储文件。

###选项###

  参数          缩写   描述
  ------------- ------ -------------------------------------------------------------------------------
  --help        -h     返回基本帮助和用法文本。
  --version            返回版本信息。
  --hostname    -s     从指定主机名的 SequoiaDB 中导出数据。默认情况下 sdbexprt 尝试连接到本地主机。
  --svcname     -p     指定的端口号。默认情况下 sdbexprt 尝试连接到端口号11810的主机。
  --user        -u     数据库用户名。
  --password    -w     数据库密码。
  --delchar     -a     指定字符分隔符。默认是（"），csv 格式有效。
  --delfield    -e     指定字段分隔符。默认是（,），csv 格式有效。
  --delrecord   -r     指定记录分隔符。默认是（\\n）。
  --csname      -c     指定导出数据的集合空间名。
  --clname      -l     指定导出数的集合名。
  --fields             指定一个或多个字段来导出数据，使用逗号分隔多个字段。csv 格式有效。
  --included           指定是否导出字段名到 csv 首行，默认 true，csv 格式有效。
  --file               指定要导出的文件名。
  --type               指定的导出数据格式。默认 csv，数据格式可以是 csv 或 json。
  --errorstop          如果遇到错误就停止，默认 false。
  --ssl                使用 SSL 连接，默认 false。

###返回值###

-   0：成功

-   1：成功但有警告

-   2：失败

-   127：参数错误

###用法###

在下面的例子，sdbexprt 从本地数据库端口11810中导出集合空间 foo 的集合 bar 的数据，导出类型是 csv，导出文件为 contact，导出字段是 field1 和 field2。

<pre class="prettyprint lang-javascript">
$ sdbexprt -s localhost -p 11810 --type csv --file contace --fields field1,field2 -c foo -l bar</pre>
