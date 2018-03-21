##安装 SequoiaSQL MySQL ##

###安装前准备###

- 使用 root 用户权限来安装 SequoiaSQL MySql
- 检查 SequoiaSQL MySql 产品软件包是否与 SequoiaDB 版本一致 
- 如果需要图形界面模式安装，请确保 X Server 服务正在运行

###安装步骤###

**说明：**

（1）产品包名字以 sequoiasql-mysql-2.9-linux_x86_64-enterprise-installer.run 为例；

（2）步骤以命令行方式进行介绍，图形界面按照图像向导提示完成。

- 运行安装程序  
    
  ```lang-javascript
  $ ./sequoiasql-mysql-2.9-linux_x86_64-enterprise-installer.run --mode text
  ```

  >**Note:**   
  >执行安装包不添加参数--mode，则进入图形界面。  

- 程序提示选择向导语言，输入2，选择中文

  ```
  Language Selection
  Please select the installation language
  [1] English - English
  [2] Simplified Chinese - 简体中文
  Please choose an option [1] :2
  ```

- 输入安装路径后按回车（默认安装在 /opt/sequoiasql/mysql ）

  ```
  ----------------------------------------------------------------------------
  由BitRock InstallBuilder评估本所建立
  
  欢迎来到 SequoiaSQL MySql Server 安装程序

  ------------------------------------------------------------
  请指定 SequoiaSQL MySql Server 将会被安装到的目录
  安装目录 /opt/sequoiasql/mysql
  ```

- 提示输入用户名和用户组（默认创建 sdbadmin 用户和 sdbadmin_group 用户组）

  ```
  ------------------------------------------------------------
  数据库管理用户配置
  配置用于启动 SequoiaSQL-MySql 的用户名、用户组和密码
  用户名 [sdbadmin]:
  用户组 [sdbadmin_group]:
  ```

- 提示输入该用户的密码和确认密码（默认密码为 sdbadmin ）

  ```
  密码 [********]:
  确认密码 [********]:
  ```

- 系统提示开始安装，需要用户确认

  ```
  ------------------------------------------------------------
  设定现在已经准备将 SequoiaSQL MySql Server 安装到您的电脑.
  您确定要继续? [Y/n]: 
  ```
    
- 安装完成

  ```
  正在安装 SequoiaSQL MySql Server 于您的电脑中，请稍候。
  安装中
  0% ______________ 50% ______________ 100%
  ########################################
  
  ------------------------------------------------------------
  安装程序已经完成安装 SequoiaSQL MySql Server 于你的电脑中.
  ```

##部署 SequoiaSQL MySQL ##

1. 添加服务

 ```lang-javascript
 # cp /opt/sequoiasql/mysql/support-files/mysql.server /etc/init.d/mysqld
 # chkconfig --add mysqld
 ```

2. 初始化数据库

 ```lang-javascript
 # /opt/sequoiasql/mysql/bin/mysqld --basedir=/opt/sequoiasql/mysql/ --datadir=/opt/sequoiasql/mysql/data/ --initialize-insecure
 # chown -R mysql:mysql /opt/sequoiasql/mysql/
 ```


3. 安装SequoiaDB插件

 ```lang-javascript
 # cp ha_sequoiadb.so /opt/sequoiasql/mysql/lib/plugin/
 ```

4. 指定MySQL启动用户

  编辑/etc/my.cnf，将如下两行添加至文件中：

 ```
   [mysqld]
   user=sdbadmin
 ```   

5. 启动MySQL

 ```lang-javascript
 # service mysqld start
 ```
