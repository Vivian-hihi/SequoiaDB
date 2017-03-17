#数据库连接配置#
服务启动成功后，通过浏览器登录控制台，输入用户标识和密码，单击“登录”
#####注：URL中的IP必须为实际安装服务器的IP，用户标识和密码对应安装时候设置的用户名和密码#####
![](webserverapp/websphere/ds_1.jpg)  

##创建JDBC提供程序##
1. 选择“资源/JDBC/JDBC提供程序”，“所有作用域”选择框中选择“节点=HOSTNAMENode01”
![](webserverapp/websphere/ds_2.jpg)
2. 单击“新建”
![](webserverapp/websphere/ds_3.jpg)
3. 步骤1：创建新的JDBC提供程序中，数据库类型选择“用户定义的”；实现类名输入“org.postgresql.jdbc2.optional.ConnectionPool”，名称修改为“sdb JDBC Provider”，单击“下一步”
![](webserverapp/websphere/ds_4.jpg)
4. 步骤2：输入数据库类路径信息中输入 “/opt/postgresql-9.3-1102.jdbc41.jar”，在服务器的/opt目录下必须存在该文件，单击“下一步”
![](webserverapp/websphere/ds_5.jpg)
5. 步骤3：摘要中，单击“完成”
![](webserverapp/websphere/ds_6.jpg)

##创建数据源##
1. 单击“sdb JDBC Provider”，图中红框部分
![](webserverapp/websphere/ds_7.jpg)
2. 进入sdb JDBC Provider页面，单击“数据源”
![](webserverapp/websphere/ds_8.jpg)
3. 进入数据源页面，单击“新建”
![](webserverapp/websphere/ds_9.jpg)
4. 步骤1：输入基本数据源信息，数据源名填入“sdb DataSource”； JNDI名称中输入“jdbc/sdb DataSource”，单击“下一步”
![](webserverapp/websphere/ds_10.jpg)
5. 步骤2：输入数据源的特定于数据库的属性，单击“下一步”
![](webserverapp/websphere/ds_11.jpg)
6. 步骤3：设置安全性别名，单击“下一步”
![](webserverapp/websphere/ds_12.jpg)
7. 步骤4：摘要，单击“完成”
![](webserverapp/websphere/ds_13.jpg)
8. 单击“sdb DataSource”，进入“sdb DataSource”页面”
![](webserverapp/websphere/ds_14.jpg)
9. 单击“JAAS － J2C 认证数据，进入JAAS － J2C 认证数据页面，单击“新建”
![](webserverapp/websphere/ds_15.jpg)
10. 输入别名：j2c、用户标识：sdbadmin、密码：*******，单击“确定”

#####注：这里的用户标识和密码是安装SequoiaSQL时的用户名和密码#####
![](webserverapp/websphere/ds_16.jpg)
11. 单击“sdb DataSource”，返回上一级页面”
![](webserverapp/websphere/ds_17.jpg)
12. 单击“定制属性”
![](webserverapp/websphere/ds_19.jpg)
13. 进入定制属性页面，在该页面中选择配置“databaseName”、“password”、“portNumber”
![](webserverapp/websphere/ds_20.jpg)
14. 翻页后，配置“serverName”、“user”
![](webserverapp/websphere/ds_21.jpg)
15. 单击相应属性名进入相应页面修改属性值，修改后显示如下：
![](webserverapp/websphere/ds_22.jpg)
16. 通过页面顶部的“保存”到主配置，保存所做的修改
![](webserverapp/websphere/ds_23.jpg)
17. 单击“数据源”返回数据源页面，选中新建的数据源，单击“测试连接”
![](webserverapp/websphere/ds_24.jpg)
18. 在页面顶部提示测试连接成功，则数据源配置成功。
![](webserverapp/websphere/ds_25.jpg)


