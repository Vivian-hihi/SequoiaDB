##获取驱动开发包##

从 [SequoiaDB](http://www.sequoiadb.com) 下载对应操作系统版本的 SequoiaDB 驱动开发包；解压驱动开发包，从 driver/lib/phpliblibsdbphp-x.x.x.so（x.x.x 为版本号，请根据 PHP 版本选择，前两位需相同版本，第三位版本要小于或等于 PHP 的版本）文件。

##数据操作##

-   Linux
    **准备工作：**安装 Apache 和 PHP 环境，PHP 要求5.3.3及以上版本
    **配置步骤：**

    1. 打开 /etc/php5/apache2/php.ini 文件；
      
    2. 在该文件的 [PHP] 配置段中新增如下行：

<pre class="prettyprint lang-diy">
extension=&lt;PATH&gt;/libsdbphp-x.x.x.so </pre>

      其中 PATH 为 libsdbphp-x.x.x.so 文件放置路径。

    3. 保存关闭文件；

    4. 重新启动 apache2 服务；

<pre class="prettyprint lang-javascript">
$ service apache2 restart（SUSE/Redhat） 或  service httpd restart（CentOS）</pre>

    5. 编写包含如下内容 PHP 测试脚本，包存为 test.php 文件，并放在在 Web 服务目录下；

<pre class="prettyprint lang-javascript">
&lt;?php phpinfo(); ?&gt;</pre>

    6. 通过浏览器打开 http://localhost/test.php ，在打开的页面中查看是否包含 SequoiaDB 模块。

-   Windows

    暂未提供 Windows 驱动开发包
