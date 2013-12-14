#!/bin/bash
. ./sequoiadbconfig.sh
. ./sequoiadbfun1.sh
. ./sequoiadbfun2.sh

#分系统检查

#安装文件的大小(MB)
INSTALL_FILE_SIZE=`ls -ld "/tmp/${INSTALL_NAME}" | awk '{print int($5/1048576+10)}'`

#安装后的总大小(MB)
INSTALL_SDB_SIZE=$[${INSTALL_FILE_SIZE}*3]

echo_r "Event" "" 0 'The SubSystem check start'

#检查系统root权限 信任关系 hosts
checkEnv 2
if [ $? -ne 0 ]; then
   exit 1
fi

#检查sdbcm端口 用户组 用户 安装路径 权限 空间
checkLocalEnv $1
if [ $? -ne 0 ]; then
   exit 1
fi

echo_r "Event" "" 0 'The SubSystem check end'

exit 0
