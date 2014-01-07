#!/bin/bash
. ./sequoiadbconfig.sh
. ./sequoiadbfun1.sh
. ./sequoiadbfun2.sh

#主系统检查

#安装文件的大小(MB)
INSTALL_FILE_SIZE=`ls -ld "${INSTALL_PATH}/${INSTALL_NAME}" | awk '{print int($5/1048576+10)}'`

#安装后的总大小(MB)
INSTALL_SDB_SIZE=$[${INSTALL_FILE_SIZE}*3]

checkEnv 1
if [ $? -ne 0 ]; then
   exit 1
fi

distribution
if [ $? -ne 0 ]; then
   exit 1
fi

sshInstallSDB
if [ $? -ne 0 ]; then
   exit 1
fi

exit 0
