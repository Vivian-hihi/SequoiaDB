

# 1. 环境准备
>**note:**
>
>数据库与服务器的对接，其实就是在服务器上配置数据源，在早期JDBC还未流行时，Java提供的JNDI在服务器数据源配置方面很发挥了很大的作用，JIDI用来作为配置数据源的工具，可以减少客户端对数据库驱动的依赖。Jboss作为一个web容器，它支持配置多个数据源，用户可以根据自己的需求来使用数据源。在Jboss上配置数据源的方式无非就是修改配置文件或是傻瓜式的用web界面增加或删除数据源，在分布式环境中用户可以考虑使用JNDI来配置数据源的访问接口，这样有助于系统的分布式环境系统的设计，能够降低多个数据源之间交互的复杂度，有利于整个系统性能的提高。
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
postgresql | [Postgresql9.3.4.tar.gz]( https://www.postgresql.org/download/)
postgresql-JDBC |[ postgresql-42.0.0.jre7.jar]( https://jdbc.postgresql.org//)
jboss | [JBoss-7.1.1.Final.zip](http://jbossas.jboss.org/downloads/)
JDK |[ JDK1.7](https://www.oracle.com/index.html/)

---

# 2. 安装配置

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

:%s/127.0.0.1/192.168.31.8/g
```
> **Note:**
>之所以将所有的127.0.0.1改成192.168.31.8是由于Jboss作为一个Web容器，不仅自身实现一些Java EE的规范，比如JMX，JMS ,JTA，EJB等。它的主要作用是对外提供服务，比如OSGI功能等。所以建议将所有的地址配置在非127的网段里，而且一般情况下，在生产环境中我们不会经常接触到运行Jboss的服务器，一般用的最多的是Jboss的web管理的功能。所以最好将所有的地址设置为网卡的地址。

3). 启动数据库

进入Jboss安装目录下的bin里，执行standalone\.sh文件


```lang-javascript

$ cd /opt/jboss/bin
$ ./standalone.sh

```

>**Note:**
>执行standalone.sh脚本后，Jboss会启动，所有的日志信息会输出到屏幕上，在开发环境中可以选择直接运行standlone \.sh。在生产环境不能简单的运行standalone .sh,建议配置日志文件，根据日志的级别来选择日志是持久化还是重定向到/dev/null中。本次安装选择直接启动standalone .sh,目的是安装过程中能够及时的发现错误，启动成功后可以根据自己需求来处理日志。

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
# 3. 数据库连接配置

>**Note:**
>Jboss中数据连接配置其实就是配置数据源，在Jboss中配置数据源的方式有两种 A:根据创建模型来创建数据源 B,通过热部署来创建数据源(和部署web应用相似)，本次配置数据源用的方法就是热部署。因为此方法方便快捷，且比较灵活。


1.复制数据库驱动

  将/opt/jboss-postgresql/postgresql-42.0.0.jre7.jar 复制到/opt/jboss/standalone/deployments/下


```lang-javascript
$ cp postgresql-42.0.0.jre7.jar /opt/jboss/standalone/deployments/
$ ls /opt/jboss/standalone/deployments/
  postgresql-42.0.0.jre7.jar README.txt

```



2.登录后台管理界面

>**note:**
>
>   1 打开浏览器，地址为jboos绑定的IP，访问端口默认9990。
>
>    例如 http://192.168.31.8:9990

>    登录用户名默认是前面创建的Jboss创后台用户

>    点击 登录 按钮



  进入 **Runtime** 的 **deployments**界面中，可以看到部署了但未启用的驱动，手动点击驱动后的 **enable** 启用。

![image](webserverapp/jboss/web-3.png)

 --


1-新增数驱动
点击 **Add content**添加数据库 点击 **Enable**启用数据库驱动，
![image](webserverapp/jboss/web-3.png)


--
2-创建JNDI


进入Profile下的DataSource点击 **Add** 创建JNDI

![image](webserverapp/jboss/ds-2.png)
![image](webserverapp/jboss/ds-3.png)

>**note:**

>在Jboss中JNDI的名字要以Java:jboss/ 开头

--

3-选择驱动

如果选择的驱动有多个，那根据自己的需求去选择,本次安装采用的是postgresql-42.0.0.jre7.jar

![image](webserverapp/jboss/ds-4.png)



--




4-创建数据源地址

![image](webserverapp/jboss/ds-5.png)

>**note:**
>
>Connectiion url的格式为jdbc:postgresql://host:port/DBName
用户一定要严格按照此格式书写，该例中的foo可能不存在，所以用户要查看自己的数据库后写相应的数据库名字和数据库访问的账号和密码，后续的步骤中修改账号密码或是数据库名，本例就是在后续的步骤中修改数据库名字。

--
5-启用postDS数据源

点击 Enable就可以启动数据源

![image](webserverapp/jboss/ds-6.png)

--

6-部署成功
 在selection下的 connection中点击 test connnection,可以测试数据源是否成功配。
![pic](webserverapp/jboss/ds-7.png)

-----
# 4. 安装web应用

1.eclipse中创建Java web项目 HelloEJB

![image](webserverapp/jboss/web-0.png)

--
2.将HelloEJB导出成war包，并上传到jboss服务器

![image](webserverapp/jboss/web-2.png)

--

3.项目上传到Jboss服务器后,启用HelloEJB 点击 Enable

![image](webserverapp/jboss/web-3.png)

--
4.浏览器中访问HelloEJB

![image](driver/jboss/web-6.png)

--

网页上输出数据库的主板本号说明成功部署。

# 5. 样例源码
该样例用来输出数据库的主板本号
