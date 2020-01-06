本章介绍使用 Java S3 的样例，用户可以下载 AWS 的开发工具包，利用工具包中的接口更快捷地发送 S3 请求。

Java的AWS开发工具包链接，建议下载 v1.11.x 版本：[https://aws.amazon.com/cn/sdk-for-java][sdkforjava]

可以通过该链接手动下载开发工具包创建测试工程，也可以通过 maven 方式下载开发工具包。

IDEA 设置 maven 自动下载aws-java-sdk-s3方式：安装 Maven 和 Intelliy IDEA 之后，创建 Maven 类型的 Java 工程，在File->Settings->Build, Execution, Deployment->Build Tools->Maven->Importing，勾选“Import Maven projects automatically”，在 Java 工程的 Pom.xml 中添加对aws-java-sdk-s3的依赖，version 建议设置为 1.11.343 及 1.11.x 中的更高版本。

```
        <dependency>
            <groupId>com.amazonaws</groupId>
            <artifactId>aws-java-sdk-s3</artifactId>
            <version>1.11.343</version>
        </dependency>
```

SequoiaS3 安装路径下的 sample 目录中的压缩包是一个 maven 类型的 Java 工程，能够实现基本读写操作。

解压压缩包，使用 IDEA 或其他工具打开该工程(File->Open->选中解压后文件夹中的Pom.xml->Open as Project->Open Existing Project->New Window)，检查并修改 maven 配置使用自动下载方式，修改 AWSClient.java 中的 endPoint，就可以开始使用 samle 中的样例测试桶和对象的上传，下载，查询等功能。

初始化客户端
----
生成一个与SequoiaS3的连接，此处需要修改endPoint的地址和端口，使之指向SequoiaS3的地址和端口。

```
    AWSCredentials credentials = new BasicAWSCredentials("ABCDEFGHIJKLMNOPQRST","abcdefghijklmnopqrstuvwxyz0123456789ABCD");
    String endPoint = "http://ip:port";
    AwsClientBuilder.EndpointConfiguration endpointConfiguration = new AwsClientBuilder.EndpointConfiguration(endPoint, null);
    AmazonS3 s3 = AmazonS3ClientBuilder.standard()
            .withEndpointConfiguration(endpointConfiguration)
            .withCredentials(new AWSStaticCredentialsProvider(credentials))
            .build();
```

创建存储桶
----

创建一个名为"bucketname"的桶。

```
    s3.createBucket("bucketname");
```

上传对象
----

从本地上传一个名为example.png的文件到"bucketname"的存储桶中，并命名为"objectname"。

```
    PutObjectRequest request = new PutObjectRequest("bucketname","objectname",new File("example.png"));
    s3.putObject(request);
```

获取对象
----

从"bucketname"获得"objectname“对象，并将对象内容存储在本地文件中。

```
    GetObjectRequest request = new GetObjectRequest("bucketname", "objectname");
    S3Object result = s3.getObject(request);

    S3ObjectInputStream s3is = result.getObjectContent();
    FileOutputStream fos = new FileOutputStream(new File(object));
    byte[] read_buf = new byte[1024];
    int read_len = 0;
    while ((read_len = s3is.read(read_buf)) > 0) {
        fos.write(read_buf, 0, read_len);
    }
    s3is.close();
    fos.close();

```

查询桶内对象列表
----

查询名为"bucketname"的桶中所有对象名称。

```
    ListObjectsV2Result result = s3.listObjectsV2("bucketname");
```

删除对象
----

删除"bucketname"桶中名为"objectname"的对象。

```
    s3.deleteObject("bucketname", "objectname");
```

删除桶
----

删除名为"bucketname"的存储桶。

```
    s3.deleteBucket("bucketname");
```



[sdkforjava]:https://aws.amazon.com/cn/sdk-for-java/