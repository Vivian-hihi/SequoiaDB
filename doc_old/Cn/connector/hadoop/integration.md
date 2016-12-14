## 搭建 Hadoop 环境##

我们支持 hadoop 1.x 和 hadoop 2.x。先安装配置好 Hadoop

## 配置连接环境##

与 MapReduce 对接，需要准备 hadoop-connector.jar 和 sequoiadb.jar，这两个 jar 可以在 SequoiaDB 安装目录下面的 hadoop 目录中找到。

因为不同版本的 Hadoop 的 classpath 不一样，所以先查看 hadoop 的 classpath，输入 hadoop classpath，在classpath 中选择一个目录，把 hadoop-connector.jar 和 sequoiadb.jar 放在目录里面，重启 hadoop 集群。

## 编写 MapReduce##

**hadoop-connector.jar 中一些重要的类：**

SequoiadbInputFormat：读取SequoiaDB的数据

SequoiadbOutputFormat：向SequoiaDB中写入数据

BSONWritable：BSONObject 的包装类，实现了 WritableComparable 接口。用于序列化 BSONObject 对象。

**SequoiaDB 和 MapReduce 的配置：**

sequoiadb-hadoop.xml 是配置文件，放在你编写的 MapReduce 工程的源码根目录下面。

sequoiadb.input.url：指定作为输入的 SequoiaDB 的 URL 路径，格式为：hostname1:port1,hostname2:port2,

sequoiadb.input.user：指定输入源的 SequoiaDB 用户，默认为 null。

sequoiadb.input.passwd：指定输入源的 SequoiaDB 连接密码，默认为 null。

sequoiadb.in.collectionspace：指定作为输入的集合空间。

sequoiadb.in.collection：指定作为输入的集合。

sequoiadb.query.json：指定输入源的查询条件，使用 json 结构，默认为 null。

sequoiadb.selector.json：指定输入源的字段筛选，使用 json 结构，默认为 null。

sequoiadb.preferedinstance：指定从 SequoiaDB 中获取数据时，连接哪个数据节点，默认为 anyone，可填值：[slave/master/anyone/node(1-7)]。

sequoiadb.output.url：指定作为输出的 SequoiaDB 的 URL 路径。

sequoiadb.output.user：指定输出源的 SequoiaDB 用户，默认为 null。

sequoiadb.output.passwd：指定输出源的 SequoiaDB 连接密码，默认为 null。

sequoiadb.out.collectionspace：指定作为输出的集合空间。

sequoiadb.out.collection：指定作为输出的集合。

sequoiadb.out.bulknum：指定每次向 SequoiaDB 写入的记录条数，对写入性能进行优化。

## 示例

（1）下面是读取 HDFS 文件，处理后写入到 SequoiaDB 中去：
<pre class="prettyprint lang-javascript">
public class HdfsSequoiadbMR {
    static class MobileMapper extends  Mapper&lt;LongWritable,Text,Text,IntWritable&gt;{
        private static final IntWritable ONE=new IntWritable(1);
        @Override
        protected void map(LongWritable key, Text value, Context context)
                throws IOException, InterruptedException {
            String valueStr=value.toString();

            String mobile_prefix=valueStr.split(",")[3].substring(0,3);
            context.write(new Text(mobile_prefix), ONE);
        }

    }

    static class MobileReducer extends Reducer&lt;Text, IntWritable, NullWritable, BSONWritable&gt;{

        @Override
        protected void reduce(Text key, Iterable&lt;IntWritable&gt; values,Context context)
                throws IOException, InterruptedException {
                Iterator&lt;IntWritable&gt; iterator=values.iterator();
                long sum=0;
                while(iterator.hasNext()){
                    sum+=iterator.next().get();
                }
                BSONObject bson=new BasicBSONObject();
                bson.put("prefix", key.toString());
                bson.put("count", sum);
                context.write(null,new BSONWritable(bson));
        }

    }

    public static void main(String[] args) throws IOException, InterruptedException, ClassNotFoundException {
        if(args.length &lt; 1){
            System.out.print("please set input path ");
            System.exit(1);
        }
        Configuration conf=new Configuration();
        conf.addResource("sequoiadb-hadoop.xml"); //加载配置文件
        Job job=Job.getInstance(conf);
        job.setJarByClass(HdfsSequoiadbMR.class);
        job.setJobName("HdfsSequoiadbMR");
        job.setInputFormatClass(TextInputFormat.class);
        job.setOutputFormatClass(SequoiadbOutputFormat.class); //reduce 输出写入到 SequoiaDB 中
        TextInputFormat.setInputPaths(job, new Path(args[0]));

        job.setMapperClass(MobileMapper.class);
        job.setReducerClass(MobileReducer.class);

        job.setMapOutputKeyClass(Text.class);
        job.setMapOutputValueClass(IntWritable.class);

        job.setOutputKeyClass(NullWritable.class);
        job.setOutputValueClass(BSONWritable.class);

        job.waitForCompletion(true);
    }
}</pre>

（2） 读取 SequoiaDB 中数据处理后写入到 HDFS 中。
<pre class="prettyprint lang-javascript">
public class SequoiadbHdfsMR {
    static class ProvinceMapper extends Mapper&lt;Object, BSONWritable,IntWritable,IntWritable&gt;{
        private static final IntWritable ONE=new IntWritable(1);
        @Override
        protected void map(Object key, BSONWritable value, Context context)
                throws IOException, InterruptedException {
                       BSONObject obj = value.getBson();
            int province=(Integer) obj.get("province_code");
            context.write(new IntWritable(province), ONE);
        }

    }

    static class ProvinceReducer extends Reducer&lt;IntWritable,IntWritable,IntWritable,LongWritable&gt;{

        @Override
        protected void reduce(IntWritable key, Iterable&lt;IntWritable&gt; values,
                Context context)
                throws IOException, InterruptedException {
            Iterator&lt;IntWritable&gt; iterator=values.iterator();
            long sum=0;
            while(iterator.hasNext()){
                sum+=iterator.next().get();
            }
            context.write(key,new LongWritable(sum));
        }

    }


    public static void main(String[] args) throws IOException, InterruptedException, ClassNotFoundException {
        if(args.length&lt;1){
            System.out.print("please set  output path ");
            System.exit(1);
        }
        Configuration conf=new Configuration();
        conf.addResource("sequoiadb-hadoop.xml");
        Job job=Job.getInstance(conf);
        job.setJarByClass(SequoiadbHdfsMR.class);
        job.setJobName("SequoiadbHdfsMR");
        job.setInputFormatClass(SequoiadbInputFormat.class);
        job.setOutputFormatClass(TextOutputFormat.class);


        FileOutputFormat.setOutputPath(job, new Path(args[0]+"/result"));

        job.setMapperClass(ProvinceMapper.class);
        job.setReducerClass(ProvinceReducer.class);

        job.setMapOutputKeyClass(IntWritable.class);
        job.setMapOutputValueClass(IntWritable.class);

        job.setOutputKeyClass(IntWritable.class);
        job.setOutputValueClass(LongWritable.class);

        job.waitForCompletion(true);
    }
}</pre>

**配置信息：**
<pre class="prettyprint lang-diy">
&lt;?xml version="1.0" encoding="UTF-8"?&gt;
&lt;configuration&gt;
  &lt;property&gt;
     &lt;name&gt;sequoiadb.input.url&lt;/name&gt;
     &lt;value&gt;localhost:11810&lt;/value&gt;
  &lt;/property&gt;
  &lt;property&gt;
     &lt;name&gt;sequoiadb.output.url&lt;/name&gt;
     &lt;value&gt;localhost:11810&lt;/value&gt;
  &lt;/property&gt;
  &lt;property&gt;
     &lt;name&gt;sequoiadb.in.collectionspace&lt;/name&gt;
     &lt;value&gt;default&lt;/value&gt;
  &lt;/property&gt;
  &lt;property&gt;
     &lt;name&gt;sequoiadb.in.collect&lt;/name&gt;
     &lt;value&gt;student&lt;/value&gt;
  &lt;/property&gt;
  &lt;property&gt;
     &lt;name&gt;sequoiadb.out.collectionspace&lt;/name&gt;
     &lt;value&gt;default&lt;/value&gt;
  &lt;/property&gt;
  &lt;property&gt;
     &lt;name&gt;sequoiadb.out.collect&lt;/name&gt;
     &lt;value&gt;result&lt;/value&gt;
  &lt;/property&gt;
    &lt;property&gt;
     &lt;name&gt;sequoiadb.out.bulknum&lt;/name&gt;
     &lt;value&gt;10&lt;/value&gt;
  &lt;/property&gt;
&lt;/configuration&gt;</pre>
