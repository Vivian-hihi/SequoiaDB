#环境准备#
请参考[WebSphere Application Server detailed system requirements](http://www-01.ibm.com/support/docview.wss?rs=180&uid=swg27006921)，检查是否满足WebSphere安装的软硬件需求。
#安装配置#
从IBM官网下载WebSphere的试用版，将下载的安装包was.cd.70011.trial.base.opt.linux.ia32.tar.gz拷贝到安装服务器上。

1. 解压、启动安装：(这里将安装包拷贝到了/opt/web/installpacket)

   ```lang-javascript
suse113-1:~ # cd /opt/web/installpacket/
suse113-1:/opt/web/installpacket # tar -xzvf was.cd.70011.trial.base.opt.linux.ia32.tar.gz
suse113-1:/opt/web/installpacket # cd WAS/
suse113-1:/opt/web/installpacket/WAS # ./install
```
2. 弹出如下安装界面，单击“Next”

   ![](webserverapp/websphere/install_1.jpg)

3. 选择 “I accept both the IBM and the non-IBM terms”，单击“Next”

![](webserverapp/websphere/install_2.jpg)

4. 单击“Next”

   ![](webserverapp/websphere/install_3.jpg)

5. 勾选所有项，单击“Next”

   ![](webserverapp/websphere/install_4.jpg)

6. 选择安装路径，单击“Next”

   ![](webserverapp/websphere/install_5.jpg)

7. 单击“Next”

   ![](webserverapp/websphere/install_6.jpg)

8. 填写用户名和密码，单击“Next”

   ![](webserverapp/websphere/install_7.jpg)

9. 单击“Next”

   ![](webserverapp/websphere/install_8.jpg)

10. 单击“Next”

   ![](webserverapp/websphere/install_9.jpg)

11. 单击“Next”，开始安装

   ![](webserverapp/websphere/install_10.jpg)

12. 等待完成，单击“Finish”

   ![](webserverapp/websphere/install_11.jpg)

13. 单击“Installation verfiication”

  ![](webserverapp/websphere/install_12.jpg)

14. 提示校验成功，并且服务自动启动。到这一步安装就算结束了

  ![](webserverapp/websphere/install_13.jpg)

