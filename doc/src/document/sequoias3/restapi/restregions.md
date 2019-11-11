本章介绍区域相关接口。

Create Region
----

增加一个区域或更新区域配置。
创建区域时可对区域内数据存储位置进行配置，即指定集合空间的生成方式，生成方式分为指定模式和自动创建模式。不能够同时指定两种模式，更新区域配置是也不能修改模式，更新区域配置只能够修改自动创建模式下的集合空间生成规则。

### 请求语法 ###
```
POST /region/?Action=CreateRegion&RegionName={regionname} HTTP/1.1
Host: ip:port
Content-Length: length
Date: date
Authorization: authorization string

<RegionConfiguration>
  <DataCSShardingType>year</DataCSShardingType>
  <DataCLShardingType>month</DataCLShardingType>
  <DataDomain>domain1</DataDomain>
  <MetaDomain>domain2</MetaDomain>
</RegionConfiguration>
```

### 参数说明 ###

**--Action**

固定为CreateRegion，表示该操作为创建一个区域。

**--RegionName**

指定区域名称。

### 请求元素 ###

在请求消息体中使用XML形式指定区域的配置。

**--RegionConfiguration**

容器，包含区域配置内容。

**--DataCSShardingType**

对象数据集合空间的生成规则，按照设定的时间生成指定的集合空间。

类型：String

有效值：year|quarter|month

默认值:year

**--DataCLShardingType**

对象数据集合的生成规则，按照设定的时间生成指定的集合。

类型：String

有效值：year/quarter/month

默认值:quarter

**--DataCSRange**

对象数据集合的生成规则，在设定时间段内能够生成的集合空间数量。

类型：Int

**--DataDomain**

对象数据集合空间所属域，域必须已在SequoiaDB中定义，如果不填写域名称，则对象数据集合空间建立在系统域上。

类型：String

**--DataLobPageSize**

对象数据集合空间的LobPageSize。

有效值：0，4096，8192，16384，32768，65536，131072，262144，524288之一，0即为默认值262144。

默认值：262144

**--DataReplSize**

对象集合的ReplSize，写操作同步的副本数。

有效值：-1， 0， 1-7

默认值：-1.

**--MetaDomain**

元数据集合空间所属域，域必须已在SequoiaDB中定义，若不填写，则元数据集合空间建在系统域上。

类型：String

**--DataLocation**

指定模式：对象数据的集合空间.集合名称，如 CS.CL

类型：String

**--MetaLocation**

指定模式：元数据的集合空间.集合名称，如 CS.CL

类型：String

**--MetaHisLocation**

指定模式：历史元数据的集合空间.集合名称，如 CS.CL

类型：String

### 样例 ###
创建区域的请求，指定对象数据集合空间和元数据集合空间的域，指定对象数据集合空间和对象数据集合的生成规则。

```
POST /region/?Action=CreateRegion&RegionName=region1 HTTP/1.1
Host: ip:port
Content-Length: length
Date: date
Authorization: authorization string

<RegionConfiguration>
  <DataCSShardingType>year</DataCSShardingType>
  <DataCLShardingType>month</DataCLShardingType>
  <DataDomain>domain1</DataDomain>
  <MetaDomain>domain2</MetaDomain>
</RegionConfiguration>
```

响应

```
HTTP/1.1 200 OK
Date: date
Content-Length: 0
```

GetRegion
----

获取一个区域的配置。

### 请求语法 ###
```
POST /region/?Action=GetRegion&RegionName={regionname} HTTP/1.1 
Host: ip:port
Date: date 
Authorization: authorization string 
```

### 参数说明 ###

**--Action**

固定为GetRegion，表示该操作为删除一个区域。

**--RegionName**

指定区域名称。

### 样例 ###
查询一个区域的配置信息。

```
POST /region/?Action=GetRegion&RegionName=region1 HTTP/1.1
Host: ip:port
Date: Wed, 12 Oct 2009 17:50:00 GMT
Authorization: authorization string
```

响应

```
<RegionConfiguration>
  <Name>region1</Name>
  <DataCSShardingType>year</DataCSShardingType>
  <DataCLShardingType>month</DataCLShardingType>
  <DataCSRange>1</DataCSRange>
  <DataDomain>domain1</DataDomain>
  <MetaDomain>domain2</MetaDomain>
  <DataLobPageSize>262144</DataLobPageSize>
  <DataReplSize>-1</DataReplSize>
  <DataLocation/>
  <MetaLocation/>
  <MetaHisLocation/>
  <Buckets>
    <Bucket>bucketname1</Bucket>
    <Bucket>bucketname2</Bucket>
  </Buckets>
</RegionConfiguration>

```

DeleteRegion
----

删除一个区域。

### 请求语法 ###
```
POST /region/?Action=DeleteRegion&RegionName={regionname} HTTP/1.1
Host: ip:port
Date: date 
Authorization: authorization string
```

### 参数说明 ###

**--Action**

固定为DeleteRegion，表示该操作为删除一个区域。

**--RegionName**

指定区域名称。

### 样例 ###
删除一个区域的请求

```
POST /region/?Action=DeleteRegion&RegionName=region1 HTTP/1.1
Host: ip:port
Date: date 
Authorization: authorization string
```

响应

```
HTTP/1.1 204 No Content 
Date: date 
```

ListRegions
----

查询区域列表，可以查询当前系统中所有区域名称。

### 请求语法 ###
```
POST /region/?Action=ListRegions HTTP/1.1 
Host: ip:port
Date: date 
Authorization: authorization string 
```

### 参数说明 ###

**--Action**

固定为ListRegions，表示该操作为查询区域列表。

### 结果解析 ###
查询结果以XML形式在响应消息头中显示。

**--ListAllRegionsResult**

容器，包含一个到多个Region。

**--Region**

区域名称。

### 样例 ###
查询区域列表的响应。

```
<ListAllRegionsResult>
  <Region>region1</Region>
  <Region>region1</Region>
  <Region>region1</Region>
</ListAllRegionsResult>
```


HeadRegion
----

查询区域是否存在

### 请求语法 ###
```
POST /region/?Action=HeadRegion&RegionName={regionname} HTTP/1.1
Host: ip:port
Date: date 
Authorization: authorization string 
```

### 样例 ###

```
HTTP/1.1 200 OK
Date: date
```
