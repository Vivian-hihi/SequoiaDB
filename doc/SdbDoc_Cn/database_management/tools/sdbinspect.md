sdbinspect 是一个 SequoiaDB 数据库的数据节点间数据一致性检测工具。它可以检查节点间数据是否完全一致，并且给出结果报告。

##权限需求##

无

##连接需求##

sdbinsepct 需要与数据库（coord 节点）连接。

##选项##

  参数                缩写   描述
  ------------------- ------ ------------------------------------------------------------------------------------------------------------------------
  --help              -h     返回基本帮助和用法文本
  --version           -v     返回当前工具所附属的数据库的版本
  --action            -a     指定检查数据或对已经存在的中间文件生成 report，inspect 和 report 可选，默认是 inspect
  --coord             -d     指定 coord 节点的 hostname 和服务端口，格式为 hostname:servicename，必须指定
  --loop              -t     指定迭代检查的次数，默认是5（次）
  --group             -g     指定要检查的 group 的名字，若不指定，则检查所有的 group
  --collectionspace   -c     指定检查的集合空间名字，不指定则检查所有集合空间
  --collection        -l     指定检查的集合名字，不指定则检查所有集合，指定集合时，必须制定集合空间
  --file              -f     指定从已存在的（上一次检查的）结果文件开始检查，当指定此选择时，其它选项（除 -o 外）均失效，生效的为文件中保存的 command 选项
  --output            -o     指定输出的文件名，默认是 inspect.bin，报告文件为 inspect.bin.report
  --view              -w     指定生成 report 文件的内容按 group 查看和按 collection 查看，默认为 group

##用法##

在下面的例子，sdbinspect 检查协调节点 ubuntu-dev9:11810 下的全部集群（5次），并将中间文件结果输出到 item.bin 中，同时会解析 item.bin 文件，把文本结果按（默认的）group 划分，输出到 item.bin.report 文件中。

<pre class="prettyprint lang-javascript">
sdbinspect -d ubuntu-dev9:11810 –o item.bin</pre>

在下面的例子，sdbinspect 检查协调节点 ubuntu-dev9:11810 下的全部集群中的集合空间 sports（3次），并将中间文件结果输出到 item.bin 中，同时会解析 item.bin 文件，把文本结果按 collection 划分，输出到
item.bin.report 文件中。

<pre class="prettyprint lang-javascript">
sdbinspect -d ubuntu-dev9:11810 –o item.bin –c sports –w collection –t 3</pre>

在下面的例子，sdbinspect 检查协调节点 ubuntu-dev9:11810 下的 data_group 集群中的名为 sports 的集合空间，名为 item 的集合（5次），并将中间文件结果输出到 inspect.bin 中，同时会解析 inspect.bin 文件，把文本结果按（默认的）group 划分，输出到 inspect.bin.report 文件中。

<pre class="prettyprint lang-javascript">
sdbinspect -d ubuntu-dev9:11810 –g data_group –c sports –l item</pre>
