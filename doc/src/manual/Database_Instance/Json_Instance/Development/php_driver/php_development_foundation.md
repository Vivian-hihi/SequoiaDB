本文档主要介绍如何使用 PHP 驱动接口编写使用 SequoiaDB 巨杉数据库的程序。下述为 SequoiaDB 巨杉数据库 PHP 驱动的简单示例，示例中的代码可能不完整，用户可在 `/sequoiadb/samples/PHP` 目录下获取相应的完整代码。

##数据库操作##

* 连接数据库

  ```lang-php
  // 创建 SequoiaDB 对象
  $db = new SequoiaDB();
  // 连接数据库
  $arr = $db -> connect( "localhost:11810" );
  // 打印连接结果，返回的默认是 php 数组类型，返回值是 array(0){"errno"=>0}
  var_dump($arr);
  // 如果 errno 为 0，那么连接成功
  if( $arr['errno'] !=0 )
  {
     exit();
  }
  ```

* 选择集合空间

  ```lang-php
  // 选择集合空间 sample，如果不存在则自动创建
  // 返回 SequoiaCS 对象
  $cs = $db -> selectCS( "sample" );
  // 检验结果，如果成功返回对象，失败返回 NULL
  if( empty($cs) )
  {
     exit();
  }
  ```

* 选择集合

  ```lang-php
  // 选择集合 employee，如果不存在则自动创建
  // 返回 SequoiaCollection 对象
  $cl = $cs -> selectCollection( "employee" );
  // 检验结果，如果成功返回对象，失败返回 NULL
  if( empty($cl) )
  {
     exit();
  }
  ```

* 插入

  ```lang-php
  // json 方式插入
  $arr = $cl -> insert( "{test:1}" );
  // 检测结果
  if( $arr['errno'] !=0 )
  {
     exit();
  }
  // 数组方式插入
  $rec = array ( 'a' => 1, 'b' => new SequoiaDate('2012-12-21') ) ;
  $arr = $cl -> insert( $rec );
  // 检测结果
  if( $arr['errno'] !=0 )
  {
     exit();
  }
  ```

* 查询

  ```lang-php
  // 查询集合中的所有记录
  // 返回 SequoiaCursor 对象
  $cursor = $cl -> find();
  // 遍历所有记录
  while( $record = $cursor -> getNext() )
  {
     var_dump( $record );
  }
  ```

* 更新

  ```lang-php
  // 修改集合中的多有记录，把字段 test 的值修改为 0
  $arr = $cl -> update( '{$set:{test:0}}' );
  // 检测结果
  if( $arr['errno'] !=0 )
  {
     exit();
  }
  ```

* 删除

  ```lang-php
  // 删除集合中的所有记录
  $arr = $cl -> remove();
  // 检测结果
  if( $arr['errno'] !=0 )
  {
     exit();
  }
  ```

## 集群操作##

* 选择组

  ```lang-php
  // 选择名称为"group"的组，如果不存在则自动创建
  // 返回 SequoiaGroup 对象
  $group = $db -> selectGroup("db1");
  // 检验结果，如果成功返回对象，失败返回 NULL
  if( empty($group) )
  {
     exit();
  }
  ```

* 启动复制组

  ```lang-php
  // 启动复制组，首次会自动激活
  // 返回操作信息
  $arr = $group -> start();
  // 检查结果
  if( $arr['errno'] != 0 ) 
  {
     exit();
  }
  ```

* 选择节点

  ```lang-php
  // 获取名称为"node"的节点
  // 返回 SequoiaNode 对象
  $node = $group -> getNode( 'hostname1:11820' );
  // 检查对象是否空
  if( empty( $node ) )
  {
     exit();
  }
  ```
