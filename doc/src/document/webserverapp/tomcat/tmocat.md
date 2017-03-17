
##环境准备##

1 安装pg，参考[pg部署](connector\postgresql\deployment.md)

##安装配置##

1 安装tomcat
  
  1) 下载tomcat安装包

  2) 登录root用户，解压安装包、拷贝到/usr/local下

```lang-javascript
$tar -zxvf apache-tomcat-7.0.68.tar.gz
```
```lang-javascript
$cp -R /opt/apache-tomcat-7.0.68 /usr/local/
```

  3）修改/usr/local/tomcat/bin/catalina.sh下配置文件，增加配置项： JAVA_OPTS="-server -Xms800m -Xmx800m -XX:PermSize=64M -XX:MaxNewSize=256m -XX:MaxPermSize=128m -Djava.awt.headless=true "配置内存大小（可根据项目实际需求进行修改），修改完成后保存配置

```lang-javascript
$vim /usr/local/tomcat/bin/catalina.sh
```

  4) 查看端口被占用（tomcat默认端口号是8080）

```lang-javascript
$netstat -lnpt | grep 8080
```

>**Note:** 
>
> 端口被占用，可修改conf目录下的server.xml的port值，如下图：

 ![](webserverapp/tomcat/tomcatserver.jpg)


5）启动tomcat服务器

```lang-javascript
$/usr/local/apache-tomcat-7.0.68/bin/startup.sh
```

>**Note:** 
>
> 启动成功访问tmocat服务器会显示tomcat首页，页面如下：


![tomcathome](webserverapp/tomcat/tomcathome.jpg)


##数据库连接配置##

1. 部署web应用使用的JNDI数据源，将pg对应的驱动jar包放到tomcat服务器的lib目录下，可以去官网下载对应版本 [http://jdbc.postgresql.org/download.html](http://jdbc.postgresql.org/download.html)

2. 配置JNDI,配置/usr/local/apache-tomcat-7.0.68/conf下的context.xml，如下：

```
<Resource 
         name="jdbc/pg"
         auth="Container"
         type="javax.sql.DataSource"
         maxActive="100"
         maxId="30"
         maxWait="10000"
         username="sdbadmin"
         password="sdbadmin"
         driverClassName="org.postgresql.Driver"
         url="jdbc:postgresql://localhost:5432/foo"/>
```

3. 重启tomcat使配置参数生效

```lang-javascript
$ /usr/local/apache-tomcat-7.0.68/bin/shutdown.sh
$ /usr/local/apache-tomcat-7.0.68/bin/startup.sh
```

>**Note:** 
>
> name：表示以后要查找的名称。通过此名称可以找到DataSource，此名称任意更换，但是程序中最终要查找的就是此名称，为了不与其他的名称混淆，所以使用jdbc/pg，现在配置的是一个jdbc的关于oracle的命名服务。<br/>
  auth：由容器进行授权及管理，指的用户名和密码是否可以在容器上生效。<br/>
  type：此名称所代表的类型，现在为javax.sql.DataSource。<br/>
  maxActive：表示一个数据库在此服务器上所能打开的最大连接数。<br/>
  maxIdle：表示一个数据库在此服务器上维持的最小连接数。<br/>
  maxWait：最大等待时间。10000毫秒。<br/>
  username：数据库连接的用户名。<br/>
  password：数据库连接的密码。<br/>
  driverClassName：数据库连接的驱动程序。<br/>
  url：数据库连接的地址。<br/>


##安装Web应用##


1. 将待发布的web应用war包放入tomcat服务器的webapps目录下即可/usr/local/apache-tomcat-7.0.68/webapps。

