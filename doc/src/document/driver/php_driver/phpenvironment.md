##获取驱动开发包##

从[SequoiaDB](http://www.sequoiadb.com)下载对应操作系统版本的SequoiaDB驱动开发包。

解压驱动开发包，选择driver/lib/phplib/libsdbphp-x.x.x.so（x.x.x 为版本号，请根据PHP版本选择，前两位需相同版本，第三位版本要小于或等于PHP的版本）。

##配置开发环境##

* Linux


 **准备工作：**安装Apache和PHP环境，PHP要求5.3.3及以上版本

    
 **配置步骤：**


 **1.** 打开/etc/php5/apache2/php.ini文件；

      
 **2.** 在该文件的[PHP]配置段中新增如下行：


 ```
 extension=<PATH>/libsdbphp-x.x.x.so
 ```

 其中PATH为 libsdbphp-x.x.x.so 文件放置路径。

 **3.** 保存关闭文件；

 **4.** 重新启动apache2服务；

 ```
 $ service apache2 restart（SUSE/Redhat/Ubuntu） 或  service httpd restart（CentOS）
 ```

 **5.** 编写包含如下内容PHP测试脚本，保存为test.php文件，并放在apache的Web服务目录下；

 ```
 <?php phpinfo(); ?>
 ```

 **6.** 通过浏览器打开如下网址

 ```
 http://<IP>/test.php
 ```

 \<IP\>为apache所在的主机IP, 在打开的页面中查看是否包含SequoiaDB模块。

* Windows

 暂未提供Windows驱动开发包
