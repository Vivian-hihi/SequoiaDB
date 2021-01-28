使用 SequoiaPerf 前，用户需要对 SequoiaPerf 进行安装，本文档主要介绍 SequoiaPerf 安装的相关步骤。

##下载安装包##

用户可前往 SequoiaDB 巨杉数据库官网下载中心下载对应版本安装包。

##安装##

###安装前准备###

- 使用 root 用户权限进行 SequoiaPerf 安装
- 如需图形界面安装，应确保 X Server 服务处于运行状态

###安装步骤###

以下安装过程将使用名为 `sequoiaperf-3.4.2-linux_x86_64--enterprise-installer.run` 的 SequoiaPerf 产品包为示例。

>**Note:**
>
> - 用户在安装过程中若输入有误，可按 ctrl+ 退格键进行删除。
> - 安装步骤以命令行方式进行介绍，如果使用图形界面进行安装，可根据图形想到提示完成。

1. 使用 root 用户运行 `sequoiaperf-3.4.2-linux_x86_64--enterprise-installer.run` 包

   ```lang-bash
   # ./sequoiaperf-3.4.2-linux_x86_64--enterprise-installer.run  --mode text
   ```

2. 提示选择向导语言，输入2，选择中文

   ```lang-text
   Language Selection
   
   Please select the installation language
   [1] English - English
   [2] Simplified Chinese - 简体中文
   Please choose an option [1] : 1
   ```

3. 提示指定 SequoiaPerf 安装路径，按回车，选择默认安装路径 `/opt/sequoiaperf`；输入路径后按回车表示选择自定义路径

   ```lang-text
   ----------------------------------------------------------------------------
   Welcome to the SequoiaPerf Server Setup Wizard.
   
   ----------------------------------------------------------------------------
   Please specify the directory where SequoiaPerf Server will be installed.
   
   Installation Directory [/opt/sequoiaperf]: 

4. 提示是否强制安装，按回车，选择不强制安装；输入y后按回车则表示选择强制安装

   ```lang-text
   ----------------------------------------------------------------------------
   Do you want to force the install? SequoiaPerf may not run properly.
   
   Force Installation when SDB doesn't exist [y/N]: y
   ```

5. 提示配置 Linux 用户名和用户组，该用户名用于运行 SequoiaPerf 服务，按回车，选择创建默认的用户名（sdbadmin）和用户组（sdbadmin_group）；输入用户名和用户组后按回车表示选择自定义的用户名和用户组

   ```lang-text
   ----------------------------------------------------------------------------
   Configure user information
   
   Please enter the desired username groupname and password
   
   User Name [sdbadmin]: 
   
   Group Name [sdbadmin_group]: 
   ```

6. 提示配置刚才创建的 Linux 用户密码，按回车，选择使用默认密码（sdbadmin）；输入密码后按回车则表示选择自定义密码

   ```lang-text
   Password [********] :
   Re-enter [********] :
   ```

7. 按回车，确认继续

   ```lang-text
   ----------------------------------------------------------------------------
   Setup is now ready to begin installing SequoiaPerf Server on your computer.
   
   Do you want to continue? [Y/n]: Y
   ```

8. 安装完成
  
   ```lang-text
   ----------------------------------------------------------------------------
   Please wait while Setup installs SequoiaPerf Server on your computer.
   
    Installing
    0% ______________ 50% ______________ 100%
    #########################################
   
   ----------------------------------------------------------------------------
   Setup has finished installing SequoiaPerf Server on your computer.
   ```



[^_^]:
    本文使用的所有引用及链接
