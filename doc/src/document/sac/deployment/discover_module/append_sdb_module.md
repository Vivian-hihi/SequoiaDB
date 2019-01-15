发现服务可以将SequoiaDB服务添加到OM中。

1. 点击部署首页的发现服务按钮，选择服务类型为SequoiaDB，点击确定。

   ![发现服务](sac/deployment/discover_module/append_1.jpg)

2. 填写coord节点或standalone所在的主机名或者IP地址、服务名，如果sequoiadb设置了鉴权，需要输入用户名和密码，否则留空，点击确定。

   ![发现SequoiaDB](sac/deployment/discover_module/append_2.jpg)  
   * 当发现的SequoiaDB中所有主机已经添加到当前OM中时，开始发现SequoiaDB。  
   * 当发现的SequoiaDB中有主机未添加到当前OM中时，无法发现该SequoiaDB，提示是否安装主机，点击是，进入安装主机步骤。

     ![发现SequoiaDB](sac/deployment/discover_module/append_error_1.jpg)

3. 发现完成，可以查看该SequoiaDB的服务信息。

   ![发现SequoiaDB](sac/deployment/discover_module/append_3.jpg)
