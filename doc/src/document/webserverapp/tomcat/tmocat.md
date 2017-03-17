
##环境准备##

安装pg，参考[pg部署](connector\postgresql\deployment.md)

##安装配置##

  
  1.下载[tomcat](http://tomcat.apache.org/download-70.cgi)安装包

  2.使用root用户，将tomcat安装包放在/opt目录下，并解压安装包

```lang-javascript
#tar -zxvf apache-tomcat-7.0.68.tar.gz
```
将解压后的文件拷贝到/usr/local下

```lang-javascript
#cp -R /opt/apache-tomcat-7.0.68 /usr/local/
```

  3.打开/usr/local/apache-tomcat-7.0.68/bin/catalina.sh文件，增加如下配置项配置内存大小（根据项目实际需求进行修改）： 

```lang-javascript
#vim /usr/local/apache-tomcat-7.0.68/bin/catalina.sh
```
```
JAVA_OPTS="-server -Xms800m -Xmx800m -XX:PermSize=64M -XX:MaxNewSize=256m -XX:MaxPermSize=128m -Djava.awt.headless=true"
```

  4.查看tomcat端口是否被占用（默认是8080）

```lang-javascript
#netstat -lnpt | grep 8080
```
如果端口被占用，可修改/usr/local/apache-tomcat-7.0.68/conf/server.xml文件的port参数的值，如下：

 ```
<Connector port="8080" protocol="HTTP/1.1"
               connectionTimeout="20000"
               redirectPort="8443" />
```


  5.启动tomcat服务器

```lang-javascript
#/usr/local/apache-tomcat-7.0.68/bin/startup.sh
```

  6.验证tmocat服务是否启动成功，访问http://ip:port,启动成功页面显示如下：


![tomcathome](webserverapp/tomcat/tomcathome.jpg)


##数据库连接配置##

  1.部署web应用使用的JNDI数据源，将pg对应的驱动jar包放到tomcat服务器的/usr/local/apache-tomcat-7.0.68/lib目录下，可以去官网下载对应版本 [http://jdbc.postgresql.org/download.html](http://jdbc.postgresql.org/download.html)

  2.配置JNDI,在/usr/local/apache-tomcat-7.0.68/conf/context.xml文件中新增内容如下：

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

>**Note:** 
>
> name：表示以后要查找的名称。通过此名称可以找到DataSource，此名称任意更换，但是程序中最终要查找的就是此名称，为了不与其他的名称混淆，所以使用jdbc/pg，现在配置的是一个jdbc的关于pg的命名服务。<br/>
  auth：由容器进行授权及管理，指的用户名和密码是否可以在容器上生效。<br/>
  type：此名称所代表的类型，现在为javax.sql.DataSource。<br/>
  maxActive：表示一个数据库在此服务器上所能打开的最大连接数。<br/>
  maxIdle：表示一个数据库在此服务器上维持的最小连接数。<br/>
  maxWait：最大等待时间。10000毫秒。<br/>
  username：数据库连接的用户名。<br/>
  password：数据库连接的密码。<br/>
  driverClassName：数据库连接的驱动程序。<br/>
  url：数据库连接的地址。<br/>

 3.重启tomcat使配置参数生效

```lang-javascript
#/usr/local/apache-tomcat-7.0.68/bin/shutdown.sh
#/usr/local/apache-tomcat-7.0.68/bin/startup.sh
```


##安装Web应用##


1.将待发布的web应用war包放入tomcat服务器的/usr/local/apache-tomcat-7.0.68/webapps目录下，例如将实现连接pg获取pg版本的test应用打成test.war放入该目录下

2.重启tomcat使web应用加载成功

```lang-javascript
#/usr/local/apache-tomcat-7.0.68/bin/shutdown.sh
#/usr/local/apache-tomcat-7.0.68/bin/startup.sh
```

3.在浏览器上输入http://ip:port/项目名，验证web应用是否发布成功，例如test应用发布成功时界面会显示pg数据库版本信息，如下图：

![](webserverapp/tomcat/webtest.jpg)

