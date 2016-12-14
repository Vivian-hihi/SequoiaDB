此部分是相关 [Python API](api/python/html/index.html) 文档。

## 历史更新情况：##

**Version 1.10**

（1） 新增接口类 lob：

-   close，关闭创建的lob对象，用以刷新数据
-   read，可从lob对象中读取数据
-   write，可把数据写入lob
-   seek，可跳转到到指定数据位置
-   get_oid，可获取lob对象的oid
-   get_size，可获取lob对象的大小(bytes)
-   get_create_time，可获取lob对象的创建时间

（2） collection 新增接口：

-   create_lob，可在当前的collection中创建一个lob对象
-   remove_lob，可在当前的collection中删除指定lob对象
-   get_lob，可获取当前collection中指定oid的lob对象
-   list_lobs，可列出当前collection中所有的lob

详情请查看相关 [Python API](api/python/html/index.html)。
