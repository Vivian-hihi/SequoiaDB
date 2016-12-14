## 获取驱动开发包##

这里介绍如何使用 PHP 驱动接口编写使用 SequoiaDB 数据库的程序。为了简单起见，下面的示例不全部是完整的代码，只起示例性作用。可到安装目录 /client/samples/php 下获取相应的完整的代码。更多查看 [PHP API](api/php/html/index.html)

## 数据操作##

* 连接数据库

<pre class="prettyprint lang-javascript">
//创建 SequoiaDB 对象
$db = new Sequoiadb();
//连接数据库
$array = $db -> connect("localhost:11810");
//检验连接结果，返回的默认是 php 数组类型，数据是 array(0){"errno"=>0}
//如果 errno 为0，那么连接成功
if($array['errno'] !=0 )
{
  exit();
}</pre>

* 选择集合空间

<pre class="prettyprint lang-javascript">
//选择名称为"foo"的集合空间，如果不存在，则自动创建
//返回 SequoiaCS 对象
$cs = $db -> selectCs("foo");
//检验结果，如果成功返回对象，失败返回 NULL
if( empty($cs) )
{
  exit();
}</pre>

* 选择集合

<pre class="prettyprint lang-javascript">
//选择名称为"big"的集合，如果不存在，则自动创建
//返回 SequoiaCollection 对象
$cl = $cs -> selectCollection("big");
//检验结果，如果成功返回对象，失败返回 NULL
if( empty($cl) )
{
  exit();
}</pre>

* 插入

<pre class="prettyprint lang-javascript">
//插入 json
$arr = $cl -> insert("{test:1}");
//检测结果
if($array['errno'] !=0 )
{
  exit();
}
//插入数组
$arr = $cl -> insert(array("test">=2));
//检测结果
if($arr['errno'] !=0 )
{
  exit();
}</pre>

* 查询

<pre class="prettyprint lang-javascript">
//查询集合中的所有记录
//返回 SequoiaCursor 对象
$cursor = $cl -> find();
//遍历所有记录
while($record = $cursor -> getNext())
{
  var_dump($record);
}</pre>

* 更新

<pre class="prettyprint lang-javascript">
//修改集合中的多有记录，把字段 test 的值修改为0
$arr = $cl -> update("{$set:{test:0}}");
//检测结果
if($arr['errno'] !=0 )
{
  exit();
}</pre>

* 删除

<pre class="prettyprint lang-javascript">
//删除集合中的所有记录
$arr = $cl -> remove();
//检测结果
if($arr['errno'] !=0 )
{
  exit();
}</pre>

## 集群操作##

* 选择组

<pre class="prettyprint lang-javascript">
//选择名称为"group"的组，如果不存在，则自动创建
//返回 SequoiaGroup 对象
$group = $db -> selectGroup("group");
//检验结果，如果成功返回对象，失败返回 NULL
if( empty($group) )
{
  exit();
}</pre>

* 启动分区组

<pre class="prettyprint lang-javascript">
//启动分区组，首次会自动激活
//返回操作信息
$arr = $group -> start() ;
//检查结果
If ( $arr['errno'] != 0 )
{
  Exit() ;
}</pre>

* 选择节点

<pre class="prettyprint lang-javascript">
//获取名称为"node"的节点
//返回 SequoiaNode 对象
$node = $group -> getNode( 'node') ;
//检查对象是否空
If ( empty( $node ) )
{
  Exit() ;
}</pre>
