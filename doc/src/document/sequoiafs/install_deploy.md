
##安装fuselib##
###安装前准备###
- 使用root用户权限安装fuselib库
- 下载fuselib的源码包  
https://github.com/libfuse/libfuse/releases/download/fuse_2_9_4/fuse-2.9.4.tar.gz  

###编译安装步骤###
（1）解压源码包  
  
  ```lang-javascript
 $tar -xzvf fuse-2.9.4.tar.gz  
  ```    
（2）进行编译安装    

  ```lang-javascript 
 $./configure  
 $make   
 $make install
  ```   