如果需要在数据库集群中新增一台主机（物理机或者虚拟机），用于部署编目节点或者数据节点，则必须需要按照如下步骤配置好主机系统：

1.安装好与其他主机相同的操作系统，并配置好IP地址；

2.按照[系统配置需求](SdbDoc_Cn/installation/system.html)一节配置好主机名/内核参数，并将其他的主机名和 IP 对应关系加入到 /etc/hosts 中；

3.修改每台集群主机上 /etc/hosts 文件，将新增的主机IP地址与主机对应关系加入到 /etc/hosts 文件中；

4.按照[系统要求](SdbDoc_Cn/installation/system.html)一节验证配置的正确性；

5.按照 [数据库安装](SdbDoc_Cn/installation/deployment/command_installation/installation.html)一节，安装 SequoiaDB 软件。安装时，注意配置管理服务端口与现有系统的端口保持一致。
