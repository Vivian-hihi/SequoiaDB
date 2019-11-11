本章介绍用户相关接口。

Create User
----

创建用户，需管理员用户权限。系统默认生成一个管理员用户，可用该用户创建其他用户。

### 请求语法 ###
```
POST /users/?Action=CreateUser&UserName=username&role=admin HTTP/1.1 
Host: ip:port
Date: date 
Authorization: authorization string
```

### 参数说明 ###

**--Action**

固定为CreateUser，表示该操作为创建一个用户。

**--UserName**

指定用户名称。

**--Role**

指定用户的角色，可选管理员admin用户或普通normal用户，管理员用户可以管理用户也可以使用S3业务，普通用户可以使用S3业务但不能管理用户。

### 结果解析 ###

创建用户成功后，会收到 AccessKeys，AccessKeys 用于访问 SequoiaS3 系统的用户验证。

**--AccessKeys**

容器，包含用户Key值。

**--AccessKeyID**

用户的Access Key ID，代表用户身份。

**--SecretAccessKey**

新用户的Secret Access Key，类似密码，用于计算签名，和Access Key ID一起在请求消息中携带，用来验证用户身份。

### 样例 ###
创建用户结果。

```
HTTP/1.1 200 OK
Date: Wed, 01 Mar  2006 12:00:00 GMT

<AccessKeys>
  <AccessKeyID>AKIAIC6UQBTBIW7THT5A</AccessKeyID>
  <SecretAccessKey>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</SecretAccessKey>
</AccessKeys>
```

Create AccessKey
----

更新用户的访问秘钥，需管理员用户权限。生成新的秘钥之后，旧秘钥失效。

### 请求语法 ###
```
POST /users/?Action=CreateAccessKey&UserName=username HTTP/1.1 
Host: ip:port
Date: date 
Authorization: authorization string
```

### 参数说明 ###

**--Action**

固定为CreateAccessKey，表示该操作为更新AccessKeys。

**--UserName**

指定用户名称。

### 样例 ###
重新获得AccessKeys的响应结果。
```
HTTP/1.1 200 OK
Date: Wed, 01 Mar  2006 12:00:00 GMT

<AccessKeys>
  <AccessKeyID>AKIAIC6UQBCDGW7TH35T</AccessKeyID>
  <SecretAccessKey>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</SecretAccessKey>
</AccessKeys>
```

Delete User
----

删除一个用户，需管理员用户权限。当该用户还有桶未清理时，不允许删除用户，当请求携带了Force标识时，可以强制删除未清理桶的用户，并将该用户拥有的桶以及桶内对象全部清理。

### 请求语法 ###
```
POST /users/?Action=DeleteUser&UserName=username HTTP/1.1 
Host: ip:port
Date: date 
Authorization: authorization string
```

### 参数说明 ###

**--Action**

固定为DeleteUser，表示该操作为删除用户。

**--UserName**

指定用户名称。

**--Force**

强制删除标记，当请求携带Force参数且值为true时，删除用户，并清理该用户拥有的桶及桶内对象。

### 样例 ###
样例一：删除用户

```
POST /users/?Action=DeleteUser&UserName=user1 HTTP/1.1 
Host: ip:port
Date: date 
Authorization: authorization string
```

响应

```
HTTP/1.1 200 OK
Date: date
```

样例二：强制删除用户

```
POST/users/?Action=DeleteUser&UserName=username&Force=true HTTP/1.1 
Host: ip:port
Date: date 
Authorization: authorization string
```

响应

```
HTTP/1.1 200 OK
Date: date
```

Get AccessKey
----

获取用户的访问秘钥，需管理员用户权限。

### 请求语法 ###
```
POST /users/?Action=GetAccessKey&UserName=username HTTP/1.1 
Host: ip:port
Date: date 
Authorization: authorization string
```

### 参数说明 ###

**--Action**

固定为GetAccessKey，表示该操作为获取用户的访问秘钥。

**--UserName**

指定用户名称。

### 样例 ###
获取user1的访问秘钥请求。

```
POST /users/?Action=GetAccessKey&UserName=user1 HTTP/1.1 
Host: ip:port
Date: date 
Authorization: authorization string
```

响应
```
HTTP/1.1 200 OK
Date: date

<AccessKeys>
  <AccessKeyID>AKIAIC6UQBTBIW7THT5A</AccessKeyID>
  <SecretAccessKey>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</SecretAccessKey>
</AccessKeys>
```