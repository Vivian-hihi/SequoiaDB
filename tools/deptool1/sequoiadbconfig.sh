#!/bin/bash

#是否输出调试信息[1:输出调试信息,2:输出普通信息,3:不输出]
IS_PRINGT_DEBUG=1

#安装文件的路径
INSTALL_PATH="/home/sequoiadb"

#安装文件的文件名
INSTALL_NAME="sequoiadb-1.5-linux_x86_64-installer.run"

#配置文件的变量数组
LIST_CONFIG=(\
"ARRAY_CONFIG_1" \
"ARRAY_CONFIG_2" \
"ARRAY_CONFIG_3" \
"ARRAY_CONFIG_4" \
"ARRAY_CONFIG_5" \
"ARRAY_CONFIG_6" \
"ARRAY_CONFIG_7" \
"ARRAY_CONFIG_8" \
"ARRAY_CONFIG_9" \
"ARRAY_CONFIG_10" \
"ARRAY_CONFIG_11" \
)

#分区组列表
LIST_GROUP=("g1" "g2" "g3")

#每一个数组的实际定义,注意:顺序必须是 [coord] [catalog] [catalog2...] [data] [data2...] [coord2...]
#IP hostName 数据库端口 数据库安装路径 系统用户组 系统用户名 系统密码 sdbcm端口 角色["coord","cata","data"] 数据库存储路径 配置 所属分区组
ARRAY_CONFIG_1=("192.168.1.211" "" "50000" "/opt/sequoiadb1" "sdbadmin_group1" "sdbadmin1" "sequoiadb" "50010" "coord" "/opt/database/coord" "SDBCONF_1" "")
ARRAY_CONFIG_2=("192.168.1.212" "" "30000" "/opt/sequoiadb2" "sdbadmin_group2" "sdbadmin2" "sequoiadb" "50010" "cata" "/opt/sequoiadb2/database/cata1" "" "")
ARRAY_CONFIG_3=("192.168.1.212" "" "31000" "/opt/sequoiadb2" "sdbadmin_group2" "sdbadmin2" "sequoiadb" "50010" "cata" "/opt/sequoiadb2/database/cata2" "" "")
ARRAY_CONFIG_4=("192.168.1.212" "" "32000" "/opt/sequoiadb2_2" "sdbadmin_group2" "sdbadmin2" "sequoiadb" "50010" "cata" "/opt/sequoiadb2/database/cata3" "SDBCONF_3" "")
ARRAY_CONFIG_5=("" "ubuntu-test-03" "51000" "/opt/sequoiadb3" "sdbadmin_group3" "sdbadmin3" "sequoiadb" "50010" "data" "/opt/sequoiadb3/database/51000" "" "g1")
ARRAY_CONFIG_6=("" "ubuntu-test-03" "52000" "/opt/sequoiadb3" "sdbadmin_group3" "sdbadmin3" "sequoiadb" "50010" "data" "/opt/sequoiadb3/database/52000" "" "g1")
ARRAY_CONFIG_7=("" "ubuntu-test-03" "53000" "/opt/sequoiadb3" "sdbadmin_group3" "sdbadmin3" "sequoiadb" "50010" "data" "/opt/sequoiadb3/database/53000" "" "g1")
ARRAY_CONFIG_8=("" "ubuntu-test-03" "54000" "/opt/sequoiadb3" "sdbadmin_group3" "sdbadmin3" "sequoiadb" "50010" "data" "/opt/sequoiadb3/database/54000" "" "g2")
ARRAY_CONFIG_9=("" "ubuntu-test-03" "55000" "/opt/sequoiadb3" "sdbadmin_group3" "sdbadmin3" "sequoiadb" "50010" "data" "/opt/sequoiadb3/database/55000" "" "g2")
ARRAY_CONFIG_10=("192.168.1.213" "" "56000" "/opt/sequoiadb3_6" "sdbadmin_group3" "sdbadmin3" "sequoiadb" "50010" "data" "/opt/sequoiadb3/database/56000" "" "g3")
ARRAY_CONFIG_11=("192.168.1.211" "" "51000" "/opt/sequoiadb1" "sdbadmin_group1" "sdbadmin1" "sequoiadb" "50010" "coord" "/opt/database/coord2" "SDBCONF_2" "")


SDB_CONFIG=(confpath logpath diagpath dbpath indexpath bkuppath maxpool svcname replname shardname catalogname httpname diaglevel role catalogaddr logfilesz logfilenum transactionon numpreload maxprefpool maxsubquery logbuffsize)
SDBCONF_1=("" "" "" "/opt/database/coord" "" "" "0" "50000" "50001" "50002" "50003" "50004" "3" "coord" "" "64" "20" "false" "0" "200" "10" "1024")
SDBCONF_2=("" "" "" "/opt/database/coord2" "" "" "0" "51000" "51001" "51002" "51003" "51004" "3" "coord" "" "64" "20" "false" "0" "200" "10" "1024")
SDBCONF_3=("" "" "" "/opt/sequoiadb2/database/cata3" "" "" "" "32000" "32001" "32002" "32008" "32004" "" "" "" "" "" "" "" "" "" "")
