本章介绍使用JAVA S3的样例，用户可以下载 AWS 的开发工具包，利用工具包中的接口更快捷地发送S3请求。

SequoiaS3 安装路径下的 sample 目录中的 java 工程，能够实现基本读写操作。解压该工程后修改 endPoint 地址端口即可运行。

初始化客户端
----
生成一个与SequoiaS3的连接。

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