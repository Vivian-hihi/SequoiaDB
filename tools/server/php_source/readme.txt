##build the php tool by source

##mkdir /opt/sequoiadb/tools/server/php
mkdir -p /opt/sequoiadb/tools/server/php


##build the zlib first

./configure --prefix=/opt/sequoiadb/tools/server/php/zlib
make
make install

##build the libxml2.so 

tar -zxvf libxml2-2.9.0.tar.gz -C /opt/sequoiadb/tools/server
cd /opt/sequoiadb/tools/server/libxml2-2.9.0
./configure --prefix=/opt/sequoiadb/tools/server/php/libxml2 --with-zlib=/opt/sequoiadb/tools/server/php/zlib/lib
make && make install

##build the php tool
tar -jxvf php-5.4.13.tar.bz2 -C /opt/sequoiadb/tools/server
cd /opt/sequoiadb/tools/server/php-5.4.13
./configure --prefix=/opt/sequoiadb/tools/server/php --with-libxml2-dir=/opt/sequoiadb/tools/server/php/libxml2
make && make install

##cp the php.ini to php build path
cp php.ini /opt/sequoiadb/tools/server/php/lib/php.ini