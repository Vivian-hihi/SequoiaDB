<<<<<<< .mine
##环境准备##
=======
# 1. 部署
>**note:**
>
>数据库与服务器的对接，其实就是在服务器上配置数据源，在早期JDBC还未流行时，Java提供的JNDI在服务器数据源配置方面很发挥了很大的作用，其原理是通过JNDI将数据库提供的服务与名字进行逻辑关联，只要用户编写符和JNDI要求的代码，就可以通过名字直接访问服务，JNDI作为一种标准，在数据源配置方面，可以减少客户端对数据库驱动的依赖。Jboss作为一个web容器，它支持配置多个数据源，用户可以根据自己的需求来配置使用数据源。在Jboss上配置数据源的方式无非就是修改配置文件或是傻瓜式的用web界面配置，在分布式环境中用户可以考虑使用JNDI来配置数据源的访问接口，这样有助于分布式系统之间数据交互的设计，有利于整个系统性能的提高。
>
>>>>>>> .r28012

数据库与服务器的对接，其实就是在服务器上配置数据源，在早期JDBC还未流行时，Java提供的JNDI在服务器数据源配置方面很发挥了很大的作用，JIDI用来作为配置数据源的工具，可以减少客户端对数据库驱动的依赖。Jboss作为一个web容器，它支持配置多个数据源，用户可以根据自己的需求来使用数据源。在Jboss上配置数据源的方式无非就是修改配置文件或是傻瓜式的用web界面增加或删除数据源，在分布式环境中用户可以考虑使用JNDI来配置数据源的访问接口，这样有助于系统的分布式环境系统的设计，能够降低多个数据源之间交互的复杂度，有利于整个系统性能的提高。


首先，下载需要的安装包，对应版本参考如下：

软件| 版本
---|---
[postgresql]( https://www.postgresql.org/download/) | Postgresql9.3.4.tar.gz
[postgresql-JDBC]( https://jdbc.postgresql.org//) | postgresql-42.0.0.jre7.jar
[jboss](http://jbossas.jboss.org/downloads/) | jboss-as-7.1.1.Final.zip
[JDK](https://www.oracle.com/index.html/) | JDK1.7

	
 >**Note:**

 >因为jboss7与jdk1.8不兼容，建议使用jdk-1.7.x的版本，postgresql-JDBC对JDK的驱动依赖很强，不同的JDK版本对应的postgresql-JDBC的版本不一样，请参看下载官网上说明。


##安装配置##

1. postgresql 的安装部署

 参照[Postgresql部署](connector\postgresql\deployment.md)

2. 配置JDK环境变量


 ```lang-javascript
 $ vim /etc/profile
 ```
 增加如下配置项

 ```
   export JAVA_HOME=/usr/java/jdk1.7.0_67
   export CLASSPATH=${JAVA_HOME}/lib:${JAVA_HOME}/jre/lib
   export PATH=${JAVA_HOME}/bin:${PATH}
 ```
 使环境变量生效

 ```lang-javascript
 $ source /etc/profile
 ```

3. 安装Jboss

 1. 将安装包解压到/opt目录下，并修改名字为jboss

      ```lang-javascript 
      $ unzip jboss-as-7.1.1.Final.zip -d /opt
      $ mv JBoss-7.1.1.Final jboss
      ```

     > **Note:**
     >jboss的运行模式有两种，domain和standalone，两种模式的安装和配置是相同的，因为standlone提供的功能相对比较多，比如应用热部署，丰富的web操作界面等，为了降低安装的复杂度，在本次安装中采用的运行模式是standalone。

   2. 修改standlone.conf配置文件

      ```lang-javascript
      $ cd /opt/jboss/bin
      $ vim standalone.conf
      ```
     设置JAVA_HOME的值（根据实际配置）

     ```
     JAVA_HOME=/usr/java/jdk1.7.0_67
     ```

   3. 修改standlone.xml配置文件

<<<<<<< .mine
     ```lang-javascript
     $ cd /opt/jboss/standalone/configuration/
     $ vim standalone.xml
     ```
     将所有的127.0.0.1修改成本机IP，该实例的IP是192.168.31.8，在vim的命令行模式下输入

=======
>>>>>>> .r28012
     ```
     :%s/127.0.0.1/192.168.31.8/g		
     ```
    > **Note:**
    >Jboss作为一个Web容器，不仅自身实现一些Java EE的规范，比如JMX，JMS ,JTA，EJB等。它的主要作用还是对外提供服务，比如OSGI功能等。所以建议将所有的地址配置在非127的网段里。

    4. 启动数据库

        进入bin目录执行standalone\.sh

       ```lang-javascript
       $ cd /opt/jboss/bin
       $ ./standalone.sh
       ```

<<<<<<< .mine
     >**Note:**
     >执行standalone.sh脚本启动Jboss，所有的日志信息会输出到屏幕上；建议配置日志文件，根据日志的级别来选择日志是持久化还是重定向到/dev/null中。本次安装选择直接启动standalone .sh,目的是安装过程中能够及时的发现错误，启动成功后可以根据自己需求来处理日志。
=======
:%s/127.0.0.1/192.168.31.8/g		# 在vim的命令行模式下输入
```
> **Note:**
>之所以将所有的127.0.0.1改成192.168.31.8是由于Jboss作为一个Web容器，不仅自身实现一些Java EE的规范，比如JMX，JMS ,JTA，EJB等。它的主要作用是对外提供服务，比如OSGI功能等。所以建议将所有的地址配置在非127的网段,具体根据实际的业务需求,因为,一般情况下，在生产环境中我们不会经常接触到运行Jboss的服务器，一般用的最多的是Jboss的web管理的功能或是直接调Jboss提供的功能，比如，在本例中如果我们把 Jboss的 地址配置成127的网段，那我们局域网内就不能正常使用jboss提供的web界面。所以最好将所有的地址设置为网卡的IP地址。
>>>>>>> .r28012

4. 添加Jboss后台访问用户

    进入bin目录执行add_user.sh

    ```lang-javascript
    $ cd /opt/jboss/bin
    ./add-user.sh
    ```
    程序提示选择要创建的用户类型，如输入a

<<<<<<< .mine
    ```
    What type of user do you wish to add? 
=======

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
>>>>>>> .r28012
 a) Management User (mgmt-users.properties) 
 b) Application User (application-users.properties)
(a): a    							
    ```
    提示输入创建用户的角色、用户名和密码（默认创建ManagementRealm角色）

    ```
    Enter the details of the new user to add.
Realm (ManagementRealm) : 
Username (jboss) : jboss
Password : 
Re-enter Password : 
About to add user 'jboss' for realm 'ManagementRealm'
Is this correct yes/no? yes
    ```
    提示创建成功

    ```
   Added user 'jboss' to file    '/opt/jboss/standalone/configuration/mgmt-users.properties'
   Added user 'jboss' to file '/opt/jboss/domain/configuration/mgmt-users.properties'
 
   ```
