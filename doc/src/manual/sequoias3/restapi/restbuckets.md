
本章介绍桶相关的接口。

GET Service
----

查询用户创建的所有存储桶。

### 请求语法 ###
```
GET / HTTP/1.1
Host: ip:port
Date: date
Authorization: authorization string
```

### 结果解析 ###
查询结果以XML形式在响应消息体中显示。

**--ListAllMyBucketsResult**

容器，包含Owner和Buckets

**--Owner**

存储桶所有者。

容器，包含ID和DisplayName。

属于：ListAllMyBucketsResult

**--ID**

存储桶所有者的ID。

继承：ListAllMyBucketsResult.Owner

**--DisplayName**

存储桶所有者的名称。

属于：ListAllMyBucketsResult.Owner

**--Buckets**

桶列表。

容器，包含若干Bucket。

属于：ListAllMyBucketsResult

**--Bucket**

存储桶。

容器，包含Name和CreationDate。

属于：ListAllMyBucketsResult.Buckets

**--Name**

存储桶名称。

属于：ListAllMyBucketsResult.Buckets.Bucket

**--CreationDate**

存储桶创建时间。

属于：ListAllMyBucketsResult.Buckets.Bucket

### 样例 ###

响应结果

```
<ListAllMyBucketsResult>
  <Owner>
    <DisplayName>username</DisplayName>
    <ID>34455</ID>
  </Owner>
  <Buckets>
    <Bucket>
      <Name>mybucket</Name>
      <CreationDate>2019-02-03T16:45:09.000Z</CreationDate>
    </Bucket>
    <Bucket>
      <Name>samples</Name>
      <CreationDate>2019-02-03T16:41:58.000Z</CreationDate>
    </Bucket>
  </Buckets>
</ListAllMyBucketsResult>
```

PUT Bucket
----

创建存储桶，存储桶名需在整个系统内唯一，长度在3-63之间。

### 请求元素 ###
在请求消息体中使用XML形式指定存储桶创建的区域，如果不指定，则存储桶创建在默认的区域上。

**--CreateBucketConfiguration**

容器，包含LocationConstraint

**--LocationConstraint**

指定创建存储桶的区域。

类型: String

### 请求语法 ###
```
PUT /bucketname HTTP/1.1
Host: ip:port
Content-Length: length
Date: date
Authorization: authorization string

<CreateBucketConfiguration>
  <LocationConstraint>Region</LocationConstraint>
</CreateBucketConfiguration>
```

### 样例 ###

响应结果

```
HTTP/1.1 200 OK
Location: /bucketName
Content-Length: 0
Date: Fri, 16 Aug 2019 11:11:53 GMT
```

DELETE Bucket
----

删除存储桶。

### 请求语法 ###
```
DELETE /bucketname HTTP/1.1
Host: ip:port
Date: date
Authorization: authorization string
```

### 样例 ###

响应结果

```
HTTP/1.1 204 No Content
Date: Fri, 16 Aug 2019 10:11:53 GMT
```

HEAD Bucket
----

检查一个存储桶是否存在。

### 请求语法 ###
```
HEAD /bucketname HTTP/1.1
Date: date
Authorization: authorization string
Host: ip:port
```

### 样例 ###

响应结果

```
HTTP/1.1 200 OK
Date: Fri, 16 Aug 2019 10:10:53 GMT
```

PUT Bucket versioning
----

修改存储桶的版本控制状态。

### 参数说明 ###

**--versioning**

表示该请求为修改存储桶的版本控制状态。

### 请求元素 ###
在请求消息体中使用XML形式指定存储桶的版本控制状态。

**--VersioningConfiguration**

容器，包含Status

**--Status**

版本控制状态。

有效值：Suspended|Enabled

属于：VersioningConfiguration

### 请求语法 ###
```
PUT /bucketname?versioning HTTP/1.1
Host: ip:port
Content-Length: length
Date: date
Authorization: authorization string

<VersioningConfiguration>
  <Status>VersioningState</Status>
</VersioningConfiguration>
```

### 样例 ###

响应结果

```
HTTP/1.1 200 OK
Date: Wed, 01 Mar  2006 12:00:00 GMT
```

GET Bucket versioning
----

查询桶的版本控制状态。

### 请求语法 ###
```
GET /bucketname?versioning HTTP/1.1
Host: ip:port
Content-Length: length
Date: date
Authorization: authorization string
```

### 结果解析 ###
查询结果以XML形式在响应消息头中显示。

**--VersioningConfiguration**

容器，包含Status

**--Status**

版本控制状态。

有效值：Suspended|Enabled

属于：VersioningConfiguration

### 样例 ###

样例1：打开版本控制开关，查询结果如下：

```
<VersioningConfiguration>
  <Status>Enabled</Status>
</VersioningConfiguration>
```

样例2：禁用版本控制，查询结果如下：

```
<VersioningConfiguration>
  <Status>Suspended</Status>
</VersioningConfiguration>
```

样例3：从未开启或禁用过版本控制，查询结果如下：

```
<VersioningConfiguration/>
```

GET Bucket location
----

查询桶所在的区域。

### 请求语法 ###
```
GET /bucketname?location HTTP/1.1
Host: ip:port
Date: date
Authorization: authorization string
```

### 结果解析 ###
查询结果以XML格式在响应消息体中显示。

**--LocationConstraint**

桶所在的区域。

### 样例 ###
样例1：已经配置了区域的存储桶，查询结果如下：

```
<LocationConstraint>region</LocationConstraint>
```

样例2：未配置区域的存储桶，查询结果如下：

```
<LocationConstraint/>
```

GET Bucket (List Objects) Version 1
----

查询存储桶内对象列表。

### 请求语法 ###
```
GET /bucketname HTTP/1.1
Host: ip:port
Date: date
Authorization: authorization string
```

### 请求参数 ###

**--prefix**

前缀，返回具有前缀的对象列表。

类型：String

**--delimiter**

分隔符，如果指定prefix，则prefix后第一次出现的分隔符之间包含相同字符串的所有键都被分组在一个CommonPrefixes。如果未指定 prefix参数，则子字符串从对象名称的开头开始。

类型：String

**--marker**

指定在存储桶中列出对象要开始的键，返回对象键按照UTF-8二进制顺序从该标记后的键开始按顺序排列。

类型：String

**--max-keys**

设置响应中返回的最大键数。默认值1000，如果要查询返回数量少于1000，可以填写其他值，填写超过1000的值，仍然按照1000条返回。

类型：String

**--encoding-type**

响应结果编码类型，只支持url。由于对象名称可以包含任意字符，但是XML对某些特别的字符无法解析，所以需要对响应中的对象名称进行编码。

### 结果解析 ###
查询结果以XML形式在响应消息体中显示。

**--ListBucketResult**

容器，包含存储桶信息，查询条件和查询的对象信息。

**--Name**

存储桶名称。

**--Prefix**

查询的prefix条件。

**--Delimiter**

查询的delimiter条件。

**--Marker**

查询的marker条件。

**--MaxKeys**

查询的maxKeys条件。

**--Encoding-Type**

查询的encoding-type条件。

**--IsTruncated**

如果该字段为true，说明由于条数限制，本次没有查询完所有符合条件的结果，可以使用NextMarker作为下一次查询的Marker条件继续查询剩余内容。

**--NextMarker**

当IsTruncated为true时，该字段返回的是本次查询的最后一条的记录。

**--CommonPrefixes**

当查询条件指定了Delimter时，Prefix后面第一次出现Delimiter的位置（包括Delimiter）之前的内容作为CommonPrefix，当有多个对象具有相同的CommonPrefix时，只返回一条CommonPrefix，计数1次，对象信息不返回。

**--Prefix**

CommonPrefix包含的前缀。

属于：ListBucketResult.CommonPrefixes

**--Contents**

容器，包含对象的元数据。

**--Key**

对象的名称。

属于：ListBucketResult.Contents

**--LastModified**

创建对象的时间。

属于：ListBucketResult.Contents

**--ETag**

对象的MD5值。

属于：ListBucketResult.Contents

**--Size**

对象的大小，单位：字节。

属于：ListBucketResult.Contents

**--Owner**

存储桶的所有者。

属于：ListBucketResult.Contents

**--ID**

存储桶所有者的ID

属于：ListBucketResult.Contents.Owner

**--DisplayName**

桶所有者的名字。

属于：ListBucketResult.Contents.Owner

### 样例 ###
样例1：本次请求不携带查询条件，查询存储桶内所有记录。

```
GET /bucketname HTTP/1.1
Host: ip:port
Date: date
Authorization: authorization string
```

查询结果：

```
<ListBucketResult>
    <Name>bucketname</Name>
    <Prefix/>
    <Marker/>
    <MaxKeys>1000</MaxKeys>
    <IsTruncated>false</IsTruncated>
    <Contents>
        <Key>my-image.jpg</Key>
        <LastModified>2019-08-12T17:50:30.000Z</LastModified>
        <ETag>&quot;fba9dede5f27731c9771645a39863328&quot;</ETag>
        <Size>434234</Size>
        <Owner>
            <ID>125664</ID>
            <DisplayName>username</DisplayName>
        </Owner>
    </Contents>
    <Contents>
       <Key>my-third-image.jpg</Key>
         <LastModified>2019-08-12T17:51:30.000Z</LastModified>
         <ETag>&quot;1b2cf535f27731c974343645a3985328&quot;</ETag>
         <Size>64994</Size>
         <Owner>
            <ID>125664</ID>
            <DisplayName>username</DisplayName>
        </Owner>
    </Contents>
</ListBucketResult>
```

样例2：本次请求指定prefix为N，起始位置为Ned，并只返回100条记录

```
GET /mybucket?prefix=N&marker=Ned&max-keys=100 HTTP/1.1
Host: iP:port
Date: date
Authorization: authorization string
```

查询结果：

```
<ListBucketResult>
  <Name>mybucket</Name>
  <Prefix>N</Prefix>
  <Marker>Ned</Marker>
  <MaxKeys>100</MaxKeys>
  <IsTruncated>false</IsTruncated>
  <Contents>
    <Key>Nelson</Key>
    <LastModified>2019-08-12T12:00:00.000Z</LastModified>
    <ETag>&quot;828ef3fdfa96f00ad9f27c383fc9ac7f&quot;</ETag>
    <Size>5</Size>
    <Owner>
      <ID>125664</ID>
      <DisplayName>username</DisplayName>
     </Owner>
  </Contents>
  <Contents>
    <Key>Neo</Key>
    <LastModified>2019-08-12T12:01:00.000Z</LastModified>
    <ETag>&quot;828ef3fdfa96f00ad9f27c383fc9ac7f&quot;</ETag>
    <Size>4</Size>
     <Owner>
      <ID>125664</ID>
      <DisplayName>username</DisplayName>
    </Owner>
 </Contents>
</ListBucketResult>
```

样例3：桶内已经有如下对象

```
sample.jpg
photos/2006/January/sample.jpg
photos/2006/February/sample2.jpg
photos/2006/February/sample3.jpg
photos/2006/February/sample4.jpg
```

本次请求携带分隔符/

```
GET /mybucket-2?delimiter=/ HTTP/1.1
Host: ip:port
Date: date
Authorization: authorization string
```

查询结果：

```
<ListBucketResult>
  <Name>mybucket-2</Name>
  <Prefix/>
  <Marker/>
  <MaxKeys>1000</MaxKeys>
  <Delimiter>/</Delimiter>
  <IsTruncated>false</IsTruncated>
  <Contents>
    <Key>sample.jpg</Key>
    <LastModified>2019-08-12T12:01:00.000Z</LastModified>
    <ETag>&quot;bf1d737a4d46a19f3bced6905cc8b902&quot;</ETag>
    <Size>142863</Size>
    <Owner>
      <ID>canonical-user-id</ID>
      <DisplayName>display-name</DisplayName>
    </Owner>
  </Contents>
  <CommonPrefixes>
    <Prefix>photos/</Prefix>
  </CommonPrefixes>
</ListBucketResult>
```

GET Bucket (List Objects) Version 2
---

查询桶内对象列表。

### 请求语法 ###
```
GET /bucketname?list-type=2 HTTP/1.1
Host: ip:port
Date: date
Authorization: authorization string
```

### 请求参数 ###

**--list-type**

固定设置为2，List Objects的第二个版本。

**--prefix**

前缀，返回具有前缀的对象列表。

类型：String

**--delimiter**

分隔符，如果指定prefix，则prefix后第一次出现的分隔符之间包含相同字符串的所有键都被分组在一个CommonPrefixes。如果未指定 prefix参数，则子字符串从对象名称的开头开始。

类型：String

**--start-after**

指定在存储桶中列出对象要开始的键，返回对象键按照UTF-8二进制顺序从该标记后的键开始按顺序排列。

类型：String

**--max-keys**

设置响应中返回的最大键数。默认值1000，如果要查询返回数量少于1000，可以填写其他值，填写超过1000的值，仍然按照1000条返回。

类型：String

**--encoding-type**

响应结果编码类型，只支持url。由于对象名称可以包含任意字符，但是XML对某些特别的字符无法解析，所以需要对响应中的对象名称进行编码。

**--continuation-token**

当响应结果被截断，还有部分未返回时，响应结果中会包含NextContinuationToken，要列出下一组对象，可以使用NextContinuationToken下一个请求中的 元素作为continuation-token。

**--fetch-owner**

默认情况下，结果中不会反悔Owner信息，如果要在响应中包含Owner信息，将该参数置为true。

### 结果解析 ###
查询结果以XML形式在响应消息体中显示。

**--ListBucketResult**

容器，包含存储桶信息，查询条件和查询的对象信息。

**--Name**

存储桶名称。

**--Prefix**

查询的prefix条件。

**--Delimiter**

查询的delimiter条件。

**--StartAfter**

查询的start-after条件。

**--ContinuationToken**

查询的continuation-token条件

**--MaxKeys**

查询的maxKeys条件。

**--Encoding-Type**

查询的encoding-type条件。

**--KeyCount**

本次查询返回的记录数。

**--IsTruncated**

如果该字段为true，说明由于条数限制，本次没有查询完所有符合条件的结果，可以使用NextMarker作为下一次查询的Marker条件继续查询剩余内容。

**--NextContinuationToken**

当IsTruncated为true时，NextContinuationToken记录位置，下一次请求在continuation-token携带该令牌继续查询下一组记录。

**--CommonPrefixes**

当查询条件指定了Delimter时，Prefix后面第一次出现Delimiter的位置（包括Delimiter）之前的内容作为CommonPrefix，当有多个对象具有相同的CommonPrefix时，只返回一条CommonPrefix，计数1次，对象信息不返回。

**--Prefix**

CommonPrefix包含的前缀。

属于：ListBucketResult.CommonPrefixes

**--Contents**

容器，包含对象的元数据。

**--Key**

对象的名称。

属于：ListBucketResult.Contents

**--LastModified**

创建对象的时间。

属于：ListBucketResult.Contents

**--ETag**

对象的MD5值。

属于：ListBucketResult.Contents

**--Size**

对象的大小，单位：字节。

属于：ListBucketResult.Contents

**--Owner**

存储桶的所有者。

属于：ListBucketResult.Contents

**--ID**

存储桶所有者的ID

属于：ListBucketResult.Contents.Owner

**--DisplayName**

桶所有者的名字。

属于：ListBucketResult.Contents.Owner

### 样例 ###
样例1：本次请求不携带查询条件，查询存储桶内所有记录。

```
GET /bucketname?list-type=2 HTTP/1.1
Host: ip:port
Date: Sat, 17 Aug 2019 17:51:00 GMT
Authorization: authorization string
```

查询结果：

```
<ListBucketResult>
    <Name>bucketname</Name>
    <Prefix/>
    <Marker/>
    <KeyCount>205</KeyCount>
    <MaxKeys>1000</MaxKeys>
    <IsTruncated>false</IsTruncated>
    <Contents>
        <Key>my-image.jpg</Key>
        <LastModified>2019-08-12T17:50:30.000Z</LastModified>
        <ETag>&quot;fba9dede5f27731c9771645a39863328&quot;</ETag>
        <Size>434234</Size>
        <Owner>
            <ID>125664</ID>
            <DisplayName>username</DisplayName>
        </Owner>
    </Contents>
    <Contents>
       ...
    </Contents>
    ...
</ListBucketResult>
```

样例2：本次请求指定prefix为N，起始位置为Ned，并只返回100条记录

```
GET /mybucket?list-type=2&prefix=N&start-after=Ned&max-keys=100 HTTP/1.1
Host: iP:port
Date: Sat, 17 Aug 2019 17:45:00 GMT
Authorization: authorization string
```

查询结果，实际查询到两条符合条件的记录：

```
<ListBucketResult>
  <Name>mybucket</Name>
  <Prefix>N</Prefix>
  <Marker>Ned</Marker>
  <KeyCount>2</KeyCount>
  <MaxKeys>100</MaxKeys>
  <IsTruncated>false</IsTruncated>
  <Contents>
    <Key>Nelson</Key>
    <LastModified>2019-08-12T12:00:00.000Z</LastModified>
    <ETag>&quot;828ef3fdfa96f00ad9f27c383fc9ac7f&quot;</ETag>
    <Size>5</Size>
    <Owner>
      <ID>125664</ID>
      <DisplayName>username</DisplayName>
     </Owner>
  </Contents>
  <Contents>
    <Key>Neo</Key>
    <LastModified>2019-08-12T12:01:00.000Z</LastModified>
    <ETag>&quot;828ef3fdfa96f00ad9f27c383fc9ac7f&quot;</ETag>
    <Size>4</Size>
     <Owner>
      <ID>125664</ID>
      <DisplayName>username</DisplayName>
    </Owner>
 </Contents>
</ListBucketResult>
```

样例3：桶内已经有如下对象

```
sample.jpg
photos/2006/January/sample.jpg
photos/2006/February/sample2.jpg
photos/2006/February/sample3.jpg
photos/2006/February/sample4.jpg
```

本次请求携带分隔符/

```
GET /mybucket-2?list-type=2&delimiter=/ HTTP/1.1
Host: ip:port
Date: date
Authorization: authorization string
```

查询结果：

```
<ListBucketResult>
  <Name>mybucket-2</Name>
  <Prefix/>
  <Marker/>
  <KeyCount>2</KeyCount>
  <MaxKeys>1000</MaxKeys>
  <Delimiter>/</Delimiter>
  <IsTruncated>false</IsTruncated>
  <Contents>
    <Key>sample.jpg</Key>
    <LastModified>2019-08-12T12:01:00.000Z</LastModified>
    <ETag>&quot;bf1d737a4d46a19f3bced6905cc8b902&quot;</ETag>
    <Size>142863</Size>
    <Owner>
      <ID>canonical-user-id</ID>
      <DisplayName>display-name</DisplayName>
    </Owner>
  </Contents>
  <CommonPrefixes>
    <Prefix>photos/</Prefix>
  </CommonPrefixes>
</ListBucketResult>
```

GET Bucket Object versions
----

查询桶内对象的所有版本。

### 请求语法 ###
```
GET /bucketname?versions HTTP/1.1
Host: ip:port
Date: date
Authorization: authorization string
```

### 请求参数 ###

**--prefix**

前缀，返回具有前缀的对象列表。

类型：String

**--delimiter**

分隔符，如果指定prefix，则prefix后第一次出现的分隔符之间包含相同字符串的所有键都被分组在一个CommonPrefixes。如果未指定 prefix参数，则子字符串从对象名称的开头开始。

类型：String

**--key-marker**

指定在存储桶中列出对象要开始的键，返回对象键按照UTF-8二进制顺序从该标记后的键开始按顺序排列。

类型：String

**--version-id-marker**

指定起始位置的version，仅在指定了key-marker的情况下有效。

**--max-keys**

设置响应中返回的最大键数。默认值1000，如果要查询返回数量少于1000，可以填写其他值，填写超过1000的值，仍然按照1000条返回。

类型：String

**--encoding-type**

响应结果编码类型，只支持url。由于对象名称可以包含任意字符，但是XML对某些特别的字符无法解析，所以需要对响应中的对象名称进行编码。

### 结果解析 ###
查询结果以XML形式在响应消息体中显示。

**--ListVersionsResult**

容器，包含存储桶信息，查询条件和查询的对象版本信息。

**--Name**

存储桶名称。

**--Prefix**

查询的prefix条件。

**--Delimiter**

查询的delimiter条件。

**--KeyMarker**

查询的key-marker条件。

**--VersionIDMarker**

查询的version-id-marker条件

**--MaxKeys**

查询的maxKeys条件。

**--Encoding-Type**

查询的encoding-type条件。

**--IsTruncated**

如果该字段为true，说明由于条数限制，本次没有查询完所有符合条件的结果，可以使用NextMarker作为下一次查询的Marker条件继续查询剩余内容。

**--NextKeyMarker**

当IsTruncated为true时，NextKeyMarker记录本次返回的最后一个对象或者CommonPrefix。

**--NextVersionIdMarker**

当IsTruncated为true时，NextVersionIdMarker记录本次返回的最后一条记录的version。

**--CommonPrefixes**

当查询条件指定了Delimter时，Prefix后面第一次出现Delimiter的位置（包括Delimiter）之前的内容作为CommonPrefix，当有多个对象具有相同的CommonPrefix时，只返回一条CommonPrefix，计数1次，对象信息不返回。

**--Prefix**

CommonPrefix包含的前缀。

属于：ListVersionsResult.CommonPrefixes

**--Version**

容器，包含对象版本的元数据。

**--DeleteMarker**

容器，包含删除标记。

**--Key**

对象的名称。

属于：ListVersionsResult.Version | ListVersionsResult.DeleteMarker

**--VersionId**

对象的版本号。

属于：ListVersionsResult.Version | ListVersionsResult.DeleteMarker

**--IsLatest**

是否是最新版本。

属于：ListVersionsResult.Version | ListVersionsResult.DeleteMarker

**--LastModified**

创建对象的时间。

属于：ListVersionsResult.Version | ListVersionsResult.DeleteMarker

**--ETag**

对象的MD5值。

属于：ListVersionsResult.Version

**--Size**

对象的大小，单位：字节。

属于：ListVersionsResult.Version

**--Owner**

存储桶的所有者。

属于：ListVersionsResult.Version | ListVersionsResult.DeleteMarker

**--ID**

存储桶所有者的ID

属于：ListVersionsResult.Version.Owner | ListVersionsResult.DeleteMarker.Owner

**--DisplayName**

桶所有者的名字。

属于：ListVersionsResult.Version.Owner | ListVersionsResult.DeleteMarker.Owner

### 样例 ###
样例1：查询存储桶内所有版本

```
GET /bucketname?versions HTTP/1.1
Host: ip:port
Date: date
Authorization: authorization string
```

查询结果：

```
<ListVersionsResult>
    <Name>bucket</Name>
    <Prefix>my</Prefix>
    <KeyMarker/>
    <VersionIdMarker/>
    <MaxKeys>1000</MaxKeys>
    <IsTruncated>false</IsTruncated>
    <Version>
        <Key>my-image.jpg</Key>
        <VersionId>234</VersionId>
        <IsLatest>true</IsLatest>
        <LastModified>2019-08-16T17:50:32.000Z</LastModified>
        <ETag>&quot;fba9dede5f27731c9771645a39863328&quot;</ETag>
        <Size>434234</Size>
        <Owner>
            <ID>125664</ID>
            <DisplayName>username</DisplayName>
        </Owner>
    </Version>
    <DeleteMarker>
        <Key>my-second-image.jpg</Key>
        <VersionId>55566666</VersionId>
        <IsLatest>true</IsLatest>
        <LastModified>2019-08-16T17:50:31.000Z</LastModified>
        <Owner>
            <ID>125664</ID>
            <DisplayName>username</DisplayName>
        </Owner>
    </DeleteMarker>
    <Version>
        <Key>my-second-image.jpg</Key>
        <VersionId>45667</VersionId>
        <IsLatest>false</IsLatest>
        <LastModified>2019-08-16T17:50:30.000Z</LastModified>
        <ETag>&quot;9b2cf535f27731c974343645a3985328&quot;</ETag>
        <Size>166434</Size>
        <Owner>
            <ID>125664</ID>
            <DisplayName>username</DisplayName>
        </Owner>
    </Version>
</ListVersionsResult>
```

样例2：携带分隔符/进行查询

请求样例：

```
GET /mybucket-2?versions&delimiter=/ HTTP/1.1
Host: ip:port
Date: date
Authorization: authorization string
```

响应结果：

```
<ListVersionsResult>
  <Name>mvbucketwithversionon1</Name>
  <Prefix/>
  <KeyMarker/>
  <VersionIdMarker/>
  <MaxKeys>1000</MaxKeys>
  <Delimiter>/</Delimiter>
  <IsTruncated>false</IsTruncated>
  <Version>
    <Key>Sample.jpg</Key>
    <VersionId>toxMzQlBsGyGCz1YuMWMp90cdXLzqOCH</VersionId>
    <IsLatest>true</IsLatest>
    <LastModified>2019-02-02T18:46:20.000Z</LastModified>
    <ETag>&quot;3305f2cfc46c0f04559748bb039d69ae&quot;</ETag>
    <Size>3191</Size>
    <Owner>
        <ID>125664</ID>
        <DisplayName>username</DisplayName>
    </Owner>
  </Version>
  <CommonPrefixes>
    <Prefix>photos/</Prefix>
  </CommonPrefixes>
  <CommonPrefixes>
    <Prefix>videos/</Prefix>
  </CommonPrefixes>
</ListVersionsResult>
```

List Multipart Uploads
----

查询桶内所有已初始化未完成的分段上传请求。

### 请求语法 ###
```
GET /bucketname?uploads HTTP/1.1
Host: ip:port
Date: Date
Authorization: authorization string
```

### 请求参数 ###

**--prefix**

前缀，返回具有前缀的对象列表。

类型：String

**--delimiter**

分隔符，如果指定prefix，则prefix后第一次出现的分隔符之间包含相同字符串的所有键都被分组在一个CommonPrefixes。如果未指定 prefix参数，则子字符串从对象名称的开头开始。

类型：String

**--key-marker**

指定在存储桶中列出对象要开始的键，返回对象键按照UTF-8二进制顺序从该标记后的键开始按顺序排列。

类型：String

**--upload-id-marker**

指定起始位置的uploadId，仅在指定了key-marker的情况下有效。

**--max-uploads**

设置响应中返回的最大键数。默认值1000，如果要查询返回数量少于1000，可以填写其他值，填写超过1000的值，仍然按照1000条返回。

类型：String

**--encoding-type**

对响应内容进行的编码方法，只支持url。由于对象名称可以包含任意字符，但是XML对某些特别的字符无法解析，所以需要对响应中的对象名称进行编码。

### 结果解析 ###
查询结果在响应消息体中以XML形式体现

**--ListMultipartUploadsResult**

容器，包含桶信息，查询条件和未完成的分段上传信息。

**--Bucket**

存储桶名称。

**--Prefix**

查询的prefix条件。

**--Delimiter**

查询的delimiter条件。

**--KeyMarker**

查询的key-marker条件。

**--UploadIdMarker**

查询的upload-id-marker条件

**--MaxUploads**

查询的maxUploads条件。

**--Encoding-Type**

查询的encoding-type条件。

**--IsTruncated**

如果该字段为true，说明由于条数限制，本次没有查询完所有符合条件的结果，可以使用NextMarker作为下一次查询的Marker条件继续查询剩余内容。

**--NextKeyMarker**

当IsTruncated为true时，NextKeyMarker记录本次返回的最后一个对象或者CommonPrefix。

**--NextUploadIdMarker**

当IsTruncated为true时，NextUploadIdMarker记录本次返回的最后一条记录的uploadId。

**--CommonPrefixes**

当查询条件指定了Delimter时，Prefix后面第一次出现Delimiter的位置（包括Delimiter）之前的内容作为CommonPrefix，当有多个对象具有相同的CommonPrefix时，只返回一条CommonPrefix，计数1次，对象信息不返回。

**--Prefix**

CommonPrefix包含的前缀。

属于：ListMultipartUploadsResult.CommonPrefixes

**--Upload**

容器，包含分段上传信息。

**--Key**

对象的名称。

属于：ListMultipartUploadsResult.Upload

**--UploadId**

分段上传的uploadId。

属于：ListMultipartUploadsResult.Upload

**--Initiated**

分段上传的的初始化时间。

属于：ListMultipartUploadsResult.Upload

**--Owner**

存储桶的所有者。

属于：ListMultipartUploadsResult.Upload

**--ID**


存储桶所有者的ID

属于：ListMultipartUploadsResult.Upload.Owner | ListMultipartUploadsResult.Upload.Initiated

**--DisplayName**

桶所有者的名字。

属于：ListMultipartUploadsResult.Upload.Owner |ListMultipartUploadsResult.Upload.Initiated

### 样例 ###
查询uploads，携带分隔符/

```
GET /example-bucket?uploads&delimiter=/ HTTP/1.1
Host: ip:port
Date: Sat, 17 Aug 2019 20:34:56 GMT
Authorization: authorization string
```

查询结果

```
<ListMultipartUploadsResult>
  <Bucket>example-bucket</Bucket>
  <KeyMarker/>
  <UploadIdMarker/>
  <NextKeyMarker>sample.jpg</NextKeyMarker>
  <NextUploadIdMarker>4444</NextUploadIdMarker>
  <Delimiter>/</Delimiter>
  <Prefix/>
  <MaxUploads>1000</MaxUploads>
  <IsTruncated>false</IsTruncated>
  <Upload>
    <Key>sample.jpg</Key>
    <UploadId>4444</UploadId>
    <Initiator>
      <ID>2234</ID>
      <DisplayName>s3-nickname</DisplayName>
    </Initiator>
    <Owner>
      <ID>2234</ID>
      <DisplayName>s3-nickname</DisplayName>
    </Owner>
    <Initiated>2019-08-16T19:24:17.000Z</Initiated>
  </Upload>
  <CommonPrefixes>
    <Prefix>photos/</Prefix>
  </CommonPrefixes>
  <CommonPrefixes>
    <Prefix>videos/</Prefix>
  </CommonPrefixes>
</ListMultipartUploadsResult>
```
