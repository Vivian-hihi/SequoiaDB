# 1. 部署
>**note:**
>
>数据库与服务器的对接，其实就是在服务器上配置数据源，在早期JDBC还未流行时，Java提供的JNDI在服务器数据源配置方面很发挥了很大的作用，其原理是通过JNDI将数据库提供的服务与名字进行逻辑关联，只要用户编写符和JNDI要求的代码，就可以通过名字直接访问服务，JNDI作为一种标准，在数据源配置方面，可以减少客户端对数据库驱动的依赖。Jboss作为一个web容器，它支持配置多个数据源，用户可以根据自己的需求来配置使用数据源。在Jboss上配置数据源的方式无非就是修改配置文件或是傻瓜式的用web界面配置，在分布式环境中用户可以考虑使用JNDI来配置数据源的访问接口，这样有助于分布式系统之间数据交互的设计，有利于整个系统性能的提高。
>

--

    下载软件：
    
    Postgresql9.3.4版本： https://www.postgresql.org/download/
    postgresql-JDBC驱动： https://jdbc.postgresql.org/
	JBoss-7.1.1安装包： http://jbossas.jboss.org/downloads 
	JDK1.7安装包： https://www.oracle.com/index.html   
	
>**Note:**
>
>因为jboss7与jdk1.8不兼容，所以尽量选用jdk-1.7.x的版本，postgresql-JDBC对JDK的驱动依赖很强，不同的JDK版本对应的postgresql-JDBC的版本不一样，下载官网上有详细的说明。

本次安装采用的系统是Ubuntu，且所有使用到的软件存放目录为 /opt/jboss-postgresql

软件| 版本
---|---
[postgresql]( https://www.postgresql.org/download/) | Postgresql9.3.4.tar.gz
[postgresql-JDBC]( https://jdbc.postgresql.org//) | postgresql-42.0.0.jre7.jar
[jboss](http://jbossas.jboss.org/downloads/) | JBoss-as-7.1.1.Final.zip
[JDK](https://www.oracle.com/index.html/) | JDK1.7

---

安装配置

 1.postgresql 的安装部署

参照该文档[**Postgresql的安装和部署**](http://doc.sequoiadb.com/cn/SequoiaDB-cat_id-1432190714-edition_id-0/)，该文档里详细描述了postgresql的安装,部署和配置。



2.JDK环境搭建


```lang-javascript

$ vim /etc/profile
  export JAVA_HOME=/usr/java/jdk1.7.0_67
  export CLASSPATH=${JAVA_HOME}/lib:${JAVA_HOME}/jre/lib
  export PATH=${JAVA_HOME}/bin:${PATH}

$ source /etc/profile

```


3.安装Jboss

1). 进入目录/opt/jboss-postgresql 将Jboss解压到/opt目录下，并修改名字为jboss

```lang-javascript
$ cd /opt/jboss-postgresql
$ unzip  JBoss-7.1.1.Final.zip
$ mv JBoss-7.1.1.Final jboss
$ mv jboss ../ 
```

> **Note:**
>jboss的运行模式有两种，domain和standalone，两种模式的安装和配置是相同的，因为standlone提供的功能相对比较多，比如应用热部署，丰富的web操作界面等，为了降低安装的复杂度，在本次安装中采用的运行模式是standalone。

2).修改配置文件

 a. 修改standlone.conf进入Jboss的安装目录中bin目录中，修改standalone.conf 修改JAVA_HOME的值

```lang-javascript
$ cd /opt/jboss/bin
$ vim standalone.conf

JAVA_HOME=/usr/java/jdk1.7.0_67
```

b.配置standalone的配置文件standlone.xml ，将所有的127.0.0.1修改成本机网卡，该实例的虚拟的网卡的IP是192.168.31.8

```lang-javascript
$ cd /opt/jboss/standalone/configuration/
$ vim standalone.xml

:%s/127.0.0.1/192.168.31.8/g		# 在vim的命令行模式下输入
```
> **Note:**
>之所以将所有的127.0.0.1改成192.168.31.8是由于Jboss作为一个Web容器，不仅自身实现一些Java EE的规范，比如JMX，JMS ,JTA，EJB等。它的主要作用是对外提供服务，比如OSGI功能等。所以建议将所有的地址配置在非127的网段,具体根据实际的业务需求,因为,一般情况下，在生产环境中我们不会经常接触到运行Jboss的服务器，一般用的最多的是Jboss的web管理的功能或是直接调Jboss提供的功能，比如，在本例中如果我们把 Jboss的 地址配置成127的网段，那我们局域网内就不能正常使用jboss提供的web界面。所以最好将所有的地址设置为网卡的IP地址。





3). 启动数据库

进入Jboss安装目录下的bin里，执行standalone\.sh文件


```lang-javascript

$ cd /opt/jboss/bin
$ ./standalone.sh

```

>**Note:**
>执行standalone.sh脚本后，Jboss会启动，所有的日志信息会输出到屏幕上，在开发环境中可以选择直接运行standalone.sh。在生产环境不能直接运行standalone.sh,此时建议配置日>志文件，根据日志的级别来选择日志是持久化还是重定向到/dev/null中。本次安装选择直接启动standalone.sh,目的是安装过程中能够方遍的查看日志.安装部署成功>后可以根据实际情况来处理日志。


4.添加Jboss后台访问用户

进入Jboss安装目录的bin中 执行add_user.sh

```lang-javascript

$ cd /opt/jboss/bin
./add-user.sh
What type of user do you wish to add? 
 a) Management User (mgmt-users.properties) 
 b) Application User (application-users.properties)
(a): 								# 选择要要创建的用户的类型(a，b中选一个)
									  a:管理员账号 b：应用程序账号

Enter the details of the new user to add.
Realm (ManagementRealm) :		 	# 所创建用户的角色
Username : 							# 要创建的用户名
Password : 							# 新创建的用户的密码
Re-enter Password :					# 重复输入新创建用户的密码
 
```
--