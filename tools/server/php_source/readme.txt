##build the php tool by source

##make output path
mkdir -p /opt/sequoiadb/tools/server/php


PS: 如果执行configure时，报检查系统识别错误的问题，可以使用config.guess和config.sub替换到configure的目录，然后重新执行

##build the zlib first
tar -jxvf zlib-1.2.5.tar.bz2  -C /opt/sequoiadb/tools/server
cd /opt/sequoiadb/tools/server/zlib-1.2.5
./configure --prefix=/opt/sequoiadb/tools/server/php/zlib
make
make install

##build the libxml2.so 

tar -zxvf libxml2-2.9.0.tar.gz -C /opt/sequoiadb/tools/server
cd /opt/sequoiadb/tools/server/libxml2-2.9.0
./configure --prefix=/opt/sequoiadb/tools/server/php/libxml2 --with-zlib=/opt/sequoiadb/tools/server/php/zlib/lib
make && make install

PS: 如果编译PHP，make的时候报错: libtool: link: `ext/opcache/zend_accelerator_debug.o` is not a valid libtool object
cd到php源码目录下，执行 make clean,然后执行 cp php.ini-production /etc/php.ini

##build the php tool
tar -jxvf php-5.4.13.tar.bz2 -C /opt/sequoiadb/tools/server
cd /opt/sequoiadb/tools/server/php-5.4.13
./configure --prefix=/opt/sequoiadb/tools/server/php --with-libxml-dir=/opt/sequoiadb/tools/server/php/libxml2

##ARM环境下, 编译之前, 检查 Makefile, 看看变量 EXTRA_LIBS 是否少了 -lpthread

make && make install

##cp the php.ini to php build path
cp php.ini /opt/sequoiadb/tools/server/php/lib/php.ini