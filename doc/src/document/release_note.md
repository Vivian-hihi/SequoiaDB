SequoiaDB 数据库是一款新型企业级分布式非关系型数据库，帮助企业用户降低 IT 成本，并对大数据的存储与分析提供了一个坚实，可靠，高效与灵活的底层平台。

[快速使用SequoiaDB](quickstart.md)

##SequoiaDB version 3.0 版本说明##

**接口变更：**

- 增加 集合空间/集合/域 的Alter系列接口
- 增加 配置 修改、删除接口以及 配置 快照接口
- 增加 LOB 随机读写接口
- 增加 LOB Truncate 接口
- 提供 全文索引 访问接口
- 提供 文件系统 访问接口
- 提供 MySQL 兼容访问接口
- Java驱动的lob接口添加支持以流的方式输入输出的接口
- 单节点集合空间的限制由4096提升到16384
- 提供 系统健康检查 快照接口
- 增加 策略 管控接口

**主要特性：**

- 提供 MySQL 兼容OLTP引擎
- SequoiaDB提供POSIX文件接口访问能力
- SequoiaDB支持全文检索能力:  
    * a) 创建全文索引
    * b) 支持全文索引数据与ElasticSearch自动进行同步
    * c) 支持ElasticSearch语法与DB语法集成进行数据检索
- 提供基于开销估算的访问计划：  
    * a) 基于数据和索引统计模型
	* b) 实现访问计划缓存与参数化等多级别匹配
    * c) 实现主子表共享访问计划 
    * d) 访问计划全路径展示
    *  e) 缓存访问计划展示
- 支持按优先级策略进行系统资源的精细化调度
- 支持对集合空间进行Alter操作  
    * a) 修改数据页大小
    * b) 修改大对象页大小
    * c) 加入/移除域
    * d) 开启/关闭Capped属性
- 支持对集合进行Alter操作  
    * a) 创建/删除ID索引
    * b) 开启/关闭/修改压缩
    * c) 开启/关闭/修改分区
    * d) 修改ReplSize/Size/Max/StrictDataMode等其它属性
- 支持对域进行Alter操作
       a) 添加/移除组
	   b) 修改AutoSplit/AutoRebalance属性
- LOB支持并发、随机读写以及Truncate功能
- 支持在线查看、修改和下发配置
- 协调节点加入集群管理，支持全局快照
- 支持将已有SequoiaDB加入OM
- OM支持SequoiaDB的扩容/减容
- OM支持安装部署SequoiaPostgreSQL业务
- OM支持对SequoiaPostgreSQL的数据操作

**工具优化：**

- 导入工具支持对timestamp类型的字段分别指定精度
- sdbdmsdump工具支持 lob 数据的检测、导出等功能
- 导出工具csv类型导出正无穷负无穷跟json统一成[+-]Infinity
- 导入工具支持NaN的数值
- 内置SQL支持is null和isnot null关键字
- OM提供系统资源授权管理功能
- OM提供针对业务的同步功能，让OM的业务配置与真实业务保持一致
- SDB SHELL的代码中支持导入文件的功能
- SDB SHELL支持粘贴多行代码
- OM提供解除业务绑定和解除主机绑定功能
- 内置SQL支持OID、Date和Timstamp类型
- 引擎提供增加外部连接限制的能力
- 外部会话提供超时以及访问隔离的能力
- Java连接池支持设置会话属性
- SDB SHELL的File类 新增文件 远程copy功能
- SDB SHELL的File类 增加readLine()方法

**性能优化：**

- 优化 Spark-SequoiaDB 连器接性能
- 优化 trace 性能，并提供 trace 分析报告，简化性能分析
- 优化 LOB 缓存并发能力，提升LOB读写性能

**解决重要Bug：**

NA
