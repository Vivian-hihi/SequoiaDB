1. 在 **部署 - 主机** 可以看到主机状态都是正常的。

   ![更新主机信息](sac/deployment/update_host_1.jpg)

2. 演示的主机 ubuntu-test-03 的 IP 改变了，导致访问 ubuntu-test-03 主机失败。

   ![更新主机信息](sac/deployment/update_host_2.jpg)

3. ubuntu-test-03 的 IP 从192.168.3.233 变成 192.168.3.234。

   ![更新主机信息](sac/deployment/update_host_3.jpg)

4. 选择需要更新 IP 的主机，点击 **主机操作 - 更新主机信息**。

   ![更新主机信息](sac/deployment/update_host_4.jpg)

5. 在窗口修改 ubuntu-test-03 的 IP 地址为 192.168.3.234，点击 **确定**。

   ![更新主机信息](sac/deployment/update_host_5.jpg)

6. 更新完成，关闭窗口。

   ![更新主机信息](sac/deployment/update_host_6.jpg)

7. ubuntu-test-03 主机恢复正常。

   ![更新主机信息](sac/deployment/update_host_7.jpg)

> **Note:**  
> 更新成功后，如果主机状态仍然无法连接，可能是dns缓存没有更新导致。  
> 如果主机用户 sdbadmin 的密码修改了，会更新失败。

####更新dns缓存####

登录SAC页面对应的主机，演示的主机是 ubuntu-test-01 。

```lang-javascript
$ service nscd restart
```
