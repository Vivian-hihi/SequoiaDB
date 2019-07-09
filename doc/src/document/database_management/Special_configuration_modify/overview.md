##概述##

[数据库配置](database_management/runtime_configuration.md#数据库配置)项中生效类型为“在线生效”和“重启生效的配置可以进行在线修改，而生效类型为空的配置则需要用户对相应的文件进行转移和删除等操作，如dbpath、indexpath、lobpath、lobmetapath等。<br>主节点、从节点在修改上没有差异。如果修改的是主节点，节点关闭后，进入到选主的过程，大约持续几秒钟，选主成功后原来的主节点会变成从节点。

