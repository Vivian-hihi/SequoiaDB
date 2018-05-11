sequoiafs文件系统是基于FUSE在linux系统下实现的一套文件系统，支持通用的文件操作API。sequoiafs利用sequoiadb的collection存储文件和目录的属性信息，lob对象存储文件的数据内容，从而实现了类似NFS分布式网络文件系统。用户可以将远程sequoiadb的某个集合通过映射的方式挂载到本地节点，从而在挂载节点的目标目录下可以通过通用文件系统API对文件和目录进行操作。

下面为其基本逻辑结构图：
  
![](sequoiafs/model.png)
     
##sequoiafs支持的fuselib库版本##
FUSE library version:2.9.4  
fusermount version:2.9.4
