##环境准备##
#####请参考[WebSphere Application Server detailed system requirements](http://www-01.ibm.com/support/docview.wss?rs=180&uid=swg27006921)，检查是否满足WebSphere安装的软硬件需求。#####
##安装配置##
#####从IBM官网下载WebSphere的试用版，将下载的安装包was.cd.70011.trial.base.opt.linux.ia32.tar.gz拷贝到安装服务器上。#####

#####解压、启动安装:#####

```lang-javascript
suse113-1:~ # cd /opt/web/installpacket/
suse113-1:/opt/web/installpacket # tar -xzvf was.cd.70011.trial.base.opt.linux.ia32.tar.gz
suse113-1:/opt/web/installpacket # cd WAS/
suse113-1:/opt/web/installpacket/WAS # ./install
```
#####弹出如下安装界面#####

![](webserverapp/websphere/install_1.jpg)

#####单击Next#####

![](webserverapp/websphere/install_2.jpg)

#####选择 I accept both the IBM and the non-IBM terms#####

#####单击Next#####

![](webserverapp/websphere/install_3.jpg)

#####单击Next#####

![](webserverapp/websphere/install_5.jpg)

#####勾选所有项,单击Next#####

![](webserverapp/websphere/install_4.jpg)

#####选择安装路径,单击Next#####

![](webserverapp/websphere/install_7.jpg)

#####单击Next#####

![](webserverapp/websphere/install_8.jpg)

#####填写用户名和密码，单击Next#####

![](webserverapp/websphere/install_9.jpg)

#####单击Next#####

![](webserverapp/websphere/install_6.jpg)

#####单击Next#####

![](webserverapp/websphere/install_10.jpg)

#####开始安装，等待完成#####

![](webserverapp/websphere/install_11.jpg)

#####单击Finish#####

![](webserverapp/websphere/install_12.jpg)

#####单击Installation verfiication#####

![](webserverapp/websphere/install_13.jpg)

#####提示校验成功，并且服务自动启动。到这一步安装就算结束了#####

##数据库连接配置##
#####服务启动成功后，通过浏览器登录控制台#####
![](webserverapp/websphere/ds_1.jpg)  
#####输入用户标识和密码，单击“登录”#####
#####创建JDBC提供者#####
![](webserverapp/websphere/ds_2.jpg)
#####选择“资源/JDBC/JDBC提供程序”，“所有作用域”选择框中选择“节点=HOSTNAMENode01”，单击“新建”#####
![](webserverapp/websphere/ds_3.jpg)
#####在步骤1：创建新的JDBC提供程序中，数据库类型选择“用户自定义的”；实现类名输入“org.postgresql.jdbc2.optional.ConnectionPool”，名称修改为“pg JDBC Provider”,单击“下一步”#####
![](webserverapp/websphere/ds_4.jpg)
#####在步骤2：输入数据库类路径信息中输入 “/opt/postgresql-9.3-1102.jdbc41.jar”，在服务器的/opt目录下必须存在该文件，单击“下一步”#####
![](webserverapp/websphere/ds_5.jpg)
#####在步骤3：摘要中，单击“完成”#####
![](webserverapp/websphere/ds_6.jpg)
#####单击pg JDBC Provider，图中红框部分#####
![](webserverapp/websphere/ds_7.jpg)
#####进入pg JDBC Provider页面,单击“数据源”#####
![](webserverapp/websphere/ds_8.jpg)
#####进入数据源页面，单击“新建”#####
![](webserverapp/websphere/ds_9.jpg)
#####在创建数据源，步骤1：输入基本数据源信息中，数据源名填入“pg DataSource”; JNDI名称中输入“jdbc/pg DataSource”，单击“下一步”#####
![](webserverapp/websphere/ds_10.jpg)
#####步骤2：输入数据源的特定于数据库的属性中，保持默认，单击“下一步”#####
![](webserverapp/websphere/ds_11.jpg)
#####步骤3：设置安全性别名中，保持默认，单击“下一步”#####
![](webserverapp/websphere/ds_12.jpg)
#####在步骤4：摘要中，保持默认，单击“完成”#####
![](webserverapp/websphere/ds_13.jpg)
#####单击“pg DataSource”,进入“pg DataSource”页面，单击“JAAS － J2C 认证数据”#####
![](webserverapp/websphere/ds_14.jpg)
#####进入JAAS － J2C 认证数据页面，单击“新建”#####
![](webserverapp/websphere/ds_15.jpg)
#####按照图中提示输入别名、用户标识、密码，单击“确定”#####
![](webserverapp/websphere/ds_16.jpg)
#####单击“pg DataSource”返回上一级页面#####
![](webserverapp/websphere/ds_17.jpg)
#####单击“定制属性”#####
![](webserverapp/websphere/ds_18.jpg)
#####进入定制属性页面#####
![](webserverapp/websphere/ds_19.jpg)
#####在该页面中选择配置“databaseName”、“password”、“portNumber”、“serverName”、“user”#####
![](webserverapp/websphere/ds_20.jpg)
#####单击相应的属性名，可以进入对应属性的配置页面#####
![](webserverapp/websphere/ds_21.jpg)
#####配置databaseName,其它属性按相同方式配置#####
![](webserverapp/websphere/ds_22.jpg)
#####相应的属性在本页面找不到时，需要通过页面底部的下一页图标进行切换#####
![](webserverapp/websphere/ds_23.jpg)
#####配置serverName和user#####
![](webserverapp/websphere/ds_24.jpg)
#####通过页面顶部的“保存”到主配置，保存所做的修改#####
![](webserverapp/websphere/ds_25.jpg)
#####单击“数据源”返回数据源页面，选中新建的数据源，单击“测试连接”#####
![](webserverapp/websphere/ds_26.jpg)
#####在页面顶部提示测试连接成功，则数据源配置成功。#####
![](webserverapp/websphere/ds_27.jpg)

##安装web应用##
####新建应用程序#####

#####1.选择“应用程序/新建应用程序/新建企业应用程序”#####
![](webserverapp/websphere/app_1.jpg)
#####2.选择本地的war包，启动浏览器的客户机上的文件，单击“下一步”#####
![](webserverapp/websphere/app_2.jpg)
#####3.默认当前选择，单击“下一步”#####
![](webserverapp/websphere/app_3.jpg)
#####4.默认当前选择，单击“下一步”#####
![](webserverapp/websphere/app_4.jpg)
#####5.默认当前选择，单击“下一步”#####
![](webserverapp/websphere/app_5.jpg)
#####6.默认当前选择，单击“下一步”#####
![](webserverapp/websphere/app_6.jpg)
#####7.这里追回输入war包的名称，单击“下一步”#####
![](webserverapp/websphere/app_7.jpg)
#####8.默认当前选择，单击“完成”#####
![](webserverapp/websphere/app_8.jpg)
#####9.单击“保存”，将配置保存起来。#####
![](webserverapp/websphere/app_9.jpg)
####启动Web应用程序####

#####1.选择“应用程序/WebSphere 企业应用程序”，勾选对应的应用程序#####
![](webserverapp/websphere/app_10.jpg)
#####2.单击启动#####
![](webserverapp/websphere/app_11.jpg)
#####3.确认对应的应用程序启动成功#####
![](webserverapp/websphere/app_13.jpg)
#####4。输入对应应用程序的URL,确认结果，该示例从SequoiaSQL中获取当前实例中的所有数据库名。#####
![](webserverapp/websphere/app_12.jpg)



