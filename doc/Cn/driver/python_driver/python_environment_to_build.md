## 获取驱动开发包##

从 [SequoiaDB](http://www.sequoiadb.com) 下载对应操作系统版本的 SequoiaDB 驱动开发包。

## 配置开发环境##

* Linux

编译：

编译需要 scons 环境，中确保 scons 已经安装后，切换到 driver/python 目录，执行 scons。scons 执行完毕，会自动打包生成 **pysequoiadb.tar.gz** 压缩包。

安装：

（1） 解压下来的驱动开发包；得到 python 目录。

（2） 将 python 目录下的 bson 和 pysequoiadb 目录拷贝到开发工程目录中（建议放在第三方库目录下）。

* Windows

暂未推出 Windows 驱动开发包。
