#!/bin/bash
. ./sequoiadbconfig.sh
. ./sequoiadbfun1.sh
. ./sequoiadbfun2.sh

#分系统安装

groupStart $1 $2 $3
if [ $? -ne 0 ]; then
   exit 1
fi

exit 0
