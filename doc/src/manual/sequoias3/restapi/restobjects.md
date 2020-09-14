
本章介绍对象相关的接口

PUT Object
----

上传一个对象到桶中，如果已有则覆盖。当开启了版本控制，同一个名称的对象可以在系统中保留多个版本，系统会为每次上传的对象生成一个version ID并保留每个版本。

### 请求语法 ###
```
PUT /bucketname/ObjectName HTTP/1.1
Host: ip:port
Date: date
Authorization: authorization string
```

### 请求头部 ###

**--Cache-Control**

指定请求/响应链中的缓存属性。

**--Content-Disposition**

当获取对象时，该属性提示将对象保存为的文件名。

**--Content-Encoding**

对象的附加编码类型，例如压缩文档使用的gzip类型。

**--Content-MD5**

对象内容（不包含头部）的MD5值经过BASE64编码后的字符串，服务端收到对象后也会做同样的计算，比较Content-MD5和服务端计算得出的结果，可以防止上传的对象内容被篡改或不完整。

**--Content-Type**

请求内容的MIME类型。

**--Expect**

当expect设置为100-continue，发送put object的请求时并不立刻发送对象内容，而是等收到100临时响应或等待超时再发送。

**--Expires**

缓存的超时时间。

**--x-amz-meta-***

自定义元数据

### 结果解析 ###
响应信息通过header返回。

**--ETag**

对象内容的MD5值转换为16进制之后生成的字符串。

**--x-amz-version-id**

版本号，当版本控制状态为Enabled，该字段返回此次上传对象的版本号，当版本控制状态为Suspended，该字段返回“null”，当未开启或禁用版本控制，该字段不返回。

### 样例 ###

上传一个对象。

```
PUT /bucketname/my-image.jpg HTTP/1.1
Host: ip:port
Date: Sat, 17 Aug 2019 17:50:00 GMT
Authorization: authorization string
Content-Type: text/plain
Content-Length: 11434
Expect: 100-continue
[11434 bytes of object data]
```

响应结果

```
HTTP/1.1 100 Continue

HTTP/1.1 200 OK
Date: Sat, 17 Aug 2019 17:50:00 GMT
ETag: "1b2cf535f27731c974343645a3985328"
Content-Length: 0
```

PUT Object - Copy
----

从系统中已有的对象拷贝到目标对象，不需要从本地上传对象内容。

### 请求语法 ###
```
PUT /destinationbucket/destinationObject HTTP/1.1
Host: ip:port
x-amz-copy-source: /source_bucket/sourceObject
x-amz-metadata-directive: metadata_directive
x-amz-copy-source-if-match: etag
x-amz-copy-source-if-none-match: etag
x-amz-copy-source-if-unmodified-since: time_stamp
x-amz-copy-source-if-modified-since: time_stamp
<request metadata>
Authorization: authorization string
```
### 请求头部 ###

**--x-amz-copy-source**

必须携带的头部。

复制对象的源对象地址，包含源存储桶和源对象，例如：/source_bucket/sourceObject ，默认复制源对象的最新版本，如果要指定版本复制，则需要增加版本号，例如：/source_bucket/sourceObject?versionId=3344 。

**--x-amz-metadata-directive**

指定是否从源对象复制元数据到目标对象。当指定为COPY时，从源对象复制元数据到目标对象，当指定为REPLACE时，源对象的元数据都不会复制到目标对象，目标对象使用复制对象请求中携带的元数据。

取值：COPY|REPLACE

默认值：COPY

**--x-amz-copy-if-modified-since**

时间，只有当源对象的创建时间在此时间后才进行复制。

**--x-amz-copy-if-unmodified-since**

时间，只有当源对象的创建时间在此之前才进行复制。

**--x-amz-copy-if-match**

ETag，只有当源对象的ETag与此ETag匹配才进行复制。

**--x-amz-copy-if-none-match**

ETag，只有当源对象的ETag与此ETag不匹配才进行复制。

**--Cache-Control**

指定请求/响应链中的缓存属性。

**--Content-Disposition**

当获取对象时，该属性提示将对象保存为的文件名。

**--Content-Encoding**

对象的附加编码类型，例如压缩文档使用的gzip类型。

**--Content-MD5**

对象内容（不包含头部）计算MD5值后经过BASE64编码后的字符串，服务端收到对象后也会做同样的计算，比较Content-MD5和服务端计算得出的结果是否一致，可以防止上传的对象内容被篡改或不完整。

**--Expires**

缓存的超时时间。

**--x-amz-meta-***

自定义元数据

### 结果解析 ###
版本号在响应header中体现。

**--x-amz-version-id**

复制后生成对象的版本号。

**--x-amz-copy-source-version-id**

源对象的版本号。

复制后对象的ETag和创建时间在消息体中以XML形式体现。

**--CopyObjectResult**

容器，包含ETag和LastModified。

**--ETag**

新对象内容计算MD5值后转换为16进制得到的字符串，与源对象一致。

**--LastModified**

新对象的创建时间。

### 样例 ###
复制指定的版本。

```
PUT /bucketname/my-second-image.jpg HTTP/1.1
Host: ip:port
Date: Sat, 17 Aug 2019 17:50:00 GMT
x-amz-copy-source: /bucketname/my-image.jpg?versionId=3344
Authorization: authorization string
```

响应结果

```
HTTP/1.1 200 OK
x-amz-version-id:5656
x-amz-copy-source-version-id:3344
Date: Sat, 17 Aug 2019 17:50:00 GMT

<CopyObjectResult>
   <LastModified>2019-08-17T17:50:00</LastModified>
   <ETag>"9b2cf535f27731c974343645a3985328"</ETag>
</CopyObjectResult>
```

GET Object
----

获取对象内容。

### 请求语法 ###
```
GET /bucketname/ObjectName HTTP/1.1
Host: ip:port
Date: date
Authorization: authorization string
```

### 请求参数 ###
通过URL参数指定请求参数。

**--versionId**

获取指定版本的对象时通过此参数指定版本号。

**--response-content-type**

指定响应消息中的Content-Type头部值。

**--response-content-language**

指定响应消息中的Content-Language头部值。

**--response-expires**

指定响应消息中的Expires头部值。

**--response-cache-control**

指定响应消息中的Cache-Control头部值。

**--response-content-disposition**

指定响应消息中的Content-Disposition头部值。

**--response-content-encoding**

指定响应消息中的Content-Encoding头部值。

### 请求头部 ###

**--Range**

下载指定位置的字节数。

**--If-Modified-Since**

指定时间，只有在指定时间之后更新过，才返回对象，否则返回304.

**--If-Unmodified-Since**

指定时间，只有在指定时间之前未更新，才返回对象，否则返回412.

**--If-Match**

指定ETag，只有对象的ETag和ETag匹配，才返回对象，否则返回412.

**--If-None-Match**

指定ETag，只有对象的ETag和ETag不匹配，才返回对象，否则返回304.

### 结果解析 ###
响应信息通过header返回。

**--x-amz-version-id**

获取的对象的版本号。

**--x-amz-meta-***

对象的自定义元数据，与上传对象时的设置一致。

**--x-amz-delete-marker**

当获取的对象是一个删除标记，响应中会携带该头部且值为true，当获取的对象不是删除标记，该头部不会携带。

### 样例 ###

样例1：获取一个对象

```
GET /bucketname/ObjectName HTTP/1.1
Host: ip:port
Date: Sat, 17 Aug 2019 17:50:00 GMT
Authorization: authorization string
```

响应结果

```
HTTP/1.1 200 OK
Date: Sat, 17 Aug 2019 17:50:00 GMT
Last-Modified: Sat, 17 Aug 2019 17:40:00 GMT
ETag: "fba9dede5f27731c9771645a39863328"
Content-Length: 434234

[434234 bytes of object data]
```

样例2：指定版本号获取一个对象

```
GET /bucketname/myObject?versionId=4433 HTTP/1.1
Host: ip:port
Date: Sat, 17 Aug 2019 17:50:00 GMT
Authorization: authorization string
```

响应结果

```
HTTP/1.1 200 OK
Date: Sat, 17 Aug 2019 17:50:00 GMT
Last-Modified: Sat, 17 Aug 2019 17:40:00 GMT
x-amz-version-id: 4433
ETag: "fba9dede5f27731c9771645a39863328"
Content-Length: 434234
Content-Type: text/plain

[434234 bytes of object data]
```

HEAD Object
----

获取对象的元数据信息，不获取对象内容。

### 请求语法 ###
```
HEAD /bucketname/ObjectName HTTP/1.1
Host: ip:port
Authorization: authorization string
```

### 请求参数 ###
通过URL参数指定请求参数。

--versionId

获取指定版本的对象时通过此参数指定版本号。

### 请求头部 ###

**--Range**

下载指定位置的字节数。

**--If-Modified-Since**

指定时间，只有在指定时间之后更新过，才返回对象，否则返回304.

**--If-Unmodified-Since**

指定时间，只有在指定时间之前未更新，才返回对象，否则返回412.

**--If-Match**

指定ETag，只有对象的ETag和ETag匹配，才返回对象，否则返回412.

**--If-None-Match**

指定ETag，只有对象的ETag和ETag不匹配，才返回对象，否则返回304.

### 结果解析 ###
响应信息通过header返回。

**--x-amz-version-id**

获取的对象的版本号。

**--x-amz-meta-***

对象的自定义元数据，与上传对象时的设置一致。

### 样例 ###

样例1：获取对象的元数据

```
HEAD /bucketname/my-image.jpg HTTP/1.1
Host: ip:port
Date: Sat, 17 Aug 2019 17:50:00 GMT
Authorization: authorization string
```

响应结果

```
HTTP/1.1 200 OK
x-amz-version-id: 3344
Date: Sat, 17 Aug 2019 17:50:00 GMT
Last-Modified: Sat, 17 Aug 2019 17:40:00 GMT
ETag: "fba9dede5f27731c9771645a39863328"
Content-Length: 434234
Content-Type: text/plain
```

样例2：获取指定版本对象的元数据

```
HEAD /bucketname/my-image.jpg?versionId=3344 HTTP/1.1
Host: ip:port
Date: Sat, 17 Aug 2019 17:55:00 GMT
Authorization: authorization string
```

响应结果

```
HTTP/1.1 200 OK
x-amz-version-id: 3344
Date: Sat, 17 Aug 2019 17:55:00 GMT
Last-Modified: Sat, 17 Aug 2019 17:40:00 GMT
ETag: "fba9dede5f27731c9771645a39863328"
Content-Length: 434234
Content-Type: text/plain
```

DELETE Object
----

删除对象。

当打开了版本控制，删除对象实际会生成一个删除标记，原来的对象还保存在系统中。

当指定版本号删除时，找到对应的版本后会永久删除该版本。

### 请求语法 ###
```
DELETE /bucketname/ObjectName HTTP/1.1
Host: ip:port
Date: date
Authorization: authorization string
```

### 请求参数 ###
通过URL参数指定请求参数。

--versionId

删除指定版本的对象时通过此参数指定版本号。

### 结果解析 ###
响应信息通过header返回。

**--x-amz-delete-marker**

当删除操作生成了一个删除标记，会返回该头部且值为true。

当指定版本号删除时，如果删除的是一个删除标记，会返回该头部且值为true。

**--x-amz-version-id**

当删除操作生成了一个删除标记，该头部记录删除标记的版本号。当指定版本删除对象时，该头部记录被删除的版本号。

### 样例 ###

样例1：删除一个未开启版本控制的桶内的对象

```
DELETE /bucketname/my-second-image.jpg HTTP/1.1
Host: ip:port
Date: Sat, 17 Aug 2019 17:55:00 GMT
Authorization: authorization string
Content-Type: text/plain
```

响应结果

```
HTTP/1.1 204 NoContent
Date: Sat, 17 Aug 2019 17:55:00 GMT
Content-Length: 0
```

样例2：删除指定版本对象

```
DELETE /bucketname/my-third-image.jpg?versionId=4455 HTTP/1.1
Host: ip:port
Date: Sat, 17 Aug 2019 17:58:00 GMT
Authorization: authorization string
```

响应结果

```
HTTP/1.1 204 NoContent
x-amz-version-id: 4455
Date: Sat, 17 Aug 2019 17:58:00 GMT
Content-Length: 0
```

Initiate Multipart Upload
----

初始化分段上传，获得upload ID。

### 请求语法 ###
```
POST /bucketname/ObjectName?uploads HTTP/1.1
Host: ip:port
Date: date
Authorization: authorization string
```

### 请求头部 ###
初始化时携带的元数据，在合并分段上传生成一个完整对象时作为对象的元数据。

**--Cache-Control**

指定请求/响应链中的缓存属性。

**--Content-Disposition**

当获取对象时，该属性提示将对象保存为的文件名。

**--Content-Encoding**

对象的附加编码类型，例如压缩文档使用的gzip类型。

**--Content-Type**

请求内容的MIME类型。

**--Expires**

缓存的超时时间。

**--x-amz-meta-***

自定义元数据

### 结果解析 ###
响应消息体中返回XML形式的结果，包含upload ID。

**--InitiateMultipartUploadResult**

容器，包含初始化分段的结果。

**--Bucket**

初始化分段上传对象所在存储桶。

**--Key**

初始化分段上传的对象名称。

**--UploadId**

初始化分段上传的ID，用来唯一标识一个分段上传请求。

### 样例 ###

响应结果

```
HTTP/1.1 200 OK
Date: Sat, 17 Aug 2019 17:59:00 GMT
Content-Length: 151

<InitiateMultipartUploadResult>
  <Bucket>bucketname</Bucket>
  <Key>ObjectName</Key>
  <UploadId>56778</UploadId>
</InitiateMultipartUploadResult>
```

Upload Part
----

上传分段。

### 请求语法 ###
```
PUT /bucketname/ObjectName?partNumber=PartNumber&uploadId=UploadId HTTP/1.1
Host: ip:port
Date: date
Content-Length: Size
Authorization: authorization string
```

### 请求参数 ###

**--partNumber**

分段编号，有效范围：1-10000

**--uploadId**

upload ID，在Initiate Multipart Upload获得。

### 样例 ###

上传一个分段

```
PUT /bucketname/ObjectName?partNumber=1&uploadId=56778 HTTP/1.1
Host: ip:port
Date: Sat, 17 Aug 2019 18:05:00 GMT
Content-Length: 10485760
Content-MD5: pUNXr/BjKK5G2UKvaRRrOA==
Authorization: authorization string

***part data omitted***
```

响应结果

```
HTTP/1.1 200 OK
Date: Sat, 17 Aug 2019 18:05:00 GMT
ETag: "b54357faf0632cce46e942fa68356b38"
Content-Length: 0
```

List Parts
----

查询分段列表。

### 请求语法 ###
```
GET /bucketname/ObjectName?uploadId=UploadId HTTP/1.1
Host: ip:port
Date: Date
Authorization: authorization string
```

### 请求参数 ###

**--uploadId**

upload ID，在Initiate Multipart Upload获得。

**--max-parts**

一次返回的最大分段数。

**--part-number​-marker**

查询的起始位置。

**--encoding-type**

响应结果编码类型，只支持url。由于对象名称可以包含任意字符，但是XML对某些特别的字符无法解析，所以需要对响应中的对象名称进行编码。

### 结果解析 ###
在消息体中以XML形式返回查询结果。

**--ListPartsResult**

**--Bucket**

分段上传的存储桶名称。

**--Key**

分段上传的对象名称。

**--UploadId**

分段上传请求的upload ID。

**--Initiator**

分段上传的发起者。

**--Owner**

存储桶的拥有者。

**--DisplayName**

用户的名称。

**--ID**

用户的ID。

**--PartNumberMarker**

查询的part-number​-marker条件。

**--MaxParts**

查询的max-parts条件。

**--IsTruncated**

是否被截断。

**--NextPartNumberMarker**

当IsTruncated为true时，该字段记录下一次查询的起始位置。

**--Encoding-Type**

查询的encoding-type条件。

**--Part**

容器，包含分段内容。

**--PartNumber**

分段编号。

**--LastModified**

分段的最新修改时间。

**--ETag**

分段的ETag。

**--Size**

分段的大小。

### 样例 ###

查询upload ID为56778的分段列表，指定max-parts为2，part-number-marker为1。

```
GET /bucketname/ObjectName?uploadId=56778&max-parts=2&part-number-marker=1 HTTP/1.1
Host: ip:port
Date: Sat, 17 Aug 2019 18:10:00 GMT
Authorization: authorization string
```

响应结果

```
HTTP/1.1 200 OK
Date: Sat, 17 Aug 2019 18:10:00 GMT
Content-Length: 838

<ListPartsResult>
  <Bucket>bucketname</Bucket>
  <Key>ObjectName</Key>
  <UploadId>56778</UploadId>
  <Initiator>
    <DisplayName>username</DisplayName>
    <ID>34455</ID>
  </Initiator>
  <Owner>
    <DisplayName>username</DisplayName>
    <ID>34455</ID>
  </Owner>
  <PartNumberMarker>1</PartNumberMarker>
  <NextPartNumberMarker>3</NextPartNumberMarker>
  <MaxParts>2</MaxParts>
  <IsTruncated>true</IsTruncated>
  <Part>
    <PartNumber>2</PartNumber>
    <LastModified>2019-08-1T17:06:06.000Z</LastModified>
    <ETag>"7778aef83f66abc1fa1e8477f296d394"</ETag>
    <Size>10485760</Size>
  </Part>
  <Part>
    <PartNumber>3</PartNumber>
    <LastModified>2019-08-1T17:06:23.000Z</LastModified>
    <ETag>"aaaa18db4cc2f85cedef654fccc4a4x8"</ETag>
    <Size>10485760</Size>
  </Part>
</ListPartsResult>
```

Complete Multipart Upload
----

完成分段上传，合并分段。

### 请求语法 ###
```
POST /bucketname/ObjectName?uploadId=UploadId HTTP/1.1
Host: ip:port
Date: Date
Content-Length: Size
Authorization: authorization string

<CompleteMultipartUpload>
  <Part>
    <PartNumber>PartNumber</PartNumber>
    <ETag>ETag</ETag>
  </Part>
  ...
</CompleteMultipartUpload>
```

### 请求元素 ###

**--CompleteMultipartUpload**

包含所有要合并的的分段信息

**--Part**

一个分段

**--PartNumber**

分段编码

**--ETag**

分段的ETag

### 结果解析 ###
响应消息体中包含XML形式的合并结果，包含合并后对象的ETag。

**--CompleteMultipartUploadResult**

合并分段结果。

**--Location**

合并后对象的地址

**--Bucket**

存储桶名称

**--Key**

对象名称

**--ETag**

合并后对象的ETag，不一定是合并后完整对象的MD5值。

### 样例 ###

```
POST /bucketname/ObjectName?uploadId=56778 HTTP/1.1
Host: ip:port
Date: Sat, 17 Aug 2019 18:10:30 GMT
Content-Length: 391
Authorization: authorization string

<CompleteMultipartUpload>
  <Part>
    <PartNumber>1</PartNumber>
    <ETag>"a54357aff0632cce46d942af68356b38"</ETag>
  </Part>
  <Part>
    <PartNumber>2</PartNumber>
    <ETag>"0c78aef83f66abc1fa1e8477f296d394"</ETag>
  </Part>
  <Part>
    <PartNumber>3</PartNumber>
    <ETag>"acbd18db4cc2f85cedef654fccc4a4d8"</ETag>
  </Part>
</CompleteMultipartUpload>
```

响应结果

```
HTTP/1.1 200 OK
Date: Sat, 17 Aug 2019 18:10:30 GMT

<CompleteMultipartUploadResult>
  <Location>http://ip:port/bucketname/ObjectName</Location>
  <Bucket>bucketname</Bucket>
  <Key>ObjectName</Key>
  <ETag>"3858f62230ac3c915f300c664312c11f-9"</ETag>
</CompleteMultipartUploadResult>
```

Abort Multipart Upload
----

取消分段上传。

### 请求语法 ###

```
DELETE /bucketname/ObjectName?uploadId=UploadId HTTP/1.1
Host: ip:port
Date: Date
Authorization: authorization string
```

### 请求参数 ###

**--uploadId**

待取消的upload ID。

### 样例 ###

响应结果

```
HTTP/1.1 204 OK
Date: Sat, 17 Aug 2019 18:15:30 GMT
Content-Length: 0
```
