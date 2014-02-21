#!/bin/bash

#环境校验的函数

#输出调试信息
#参数1 等级     例如 "Error" "Warning" "Event"
#参数2 函数          $FUNCNAME
#参数3 行数          $LINENO
#参数4 输出信息 例如 "hello world"
function echo_r()
{
   if [ ${IS_PRINGT_DEBUG} -eq 1 ]; then
      printf "\nTime: %s\tLevel: $1\nFunction: $2\tLine: %d\nFile: $0\n%s\n" $(date +%Y-%m-%d-%H.%M.%S) $3 "$4"
   elif [ ${IS_PRINGT_DEBUG} -eq 2 ]; then
      echo "$4"
   fi
}

#检查是否root用户运行脚本
function checkRoot()
{
   if [ `id -u` -ne 0 ]; then
      return 1
   else
      return 0
   fi
}

#检查是否能ping通
#参数1 地址 例如 "192.168.1.1" "ubuntu-test-01"
function checkPing()
{
   filestat=$(ping -c 1 $1|grep "transmitted"|awk '{print $4}')
   if [[ ${filestat} =~ ^[0-9]+$ ]]; then
      if [ ${filestat} = 1 ]; then
         return 0
      else
         return 1
      fi
   else
      return 1
   fi
}

#检查能否ping通,并返回响应时间
#参数1 地址 例如 "192.168.1.1"
#返回 响应时间 -1 连接失败 非负数则是响应时间
function checkPingAndReturn()
{
   num=$(ping -c 1 $1|grep 'time='|awk '{print $7}'|cut -d "=" -f 2)
   if [ -z "${num}" ]; then
      return 1
   else
      echo ${num}
      return 0
   fi
}

#检查信任关系
#参数1 地址 例如 "192.168.1.1"
function checkTrust()
{
   temp=`ssh -o StrictHostKeyChecking=no -o PasswordAuthentication=no root@$1 echo`
   if [ $? -ne 0 ]; then
      return 1
   else
      return 0
   fi
}
#检查本地hostname与实际机器的hostname是否一致
#参数1 hostname 例如 "ubuntu-test-01"
function checkHostname2()
{
   thisHost=`ssh root@$1 hostname`
   if [ ${thisHost} = $1 ];then
      return 0
   else
      return 1
   fi
}

#检查ip与hostname是否一致
#参数1 地址 例如 "192.168.1.1"
function checkHostname()
{
   thisHost=`ssh root@$1 hostname`
   checkPing ${thisHost}
   if [ $? -ne 0 ]; then
      return 1
   fi
   thisIP=`ssh root@${thisHost} ifconfig|grep "inet addr"|awk '{print $2}'|cut -d ":" -f 2|grep -v "127.0.0.1"`
   for subIP in ${thisIP}
   do
      if [ ${subIP} = $1 ];then
         return 0
      fi
   done
   return 1
}

#获取目标的hostname
#参数1 地址 例如 "192.168.1.1"
function getSSHHostname()
{
   thisHost=`ssh root@$1 hostname`
   echo "${thisHost}"
}

#获取目标的ip
#参数1 地址 例如 "192.168.1.1"
function getSSHIP()
{
   thisIP=`ssh root@${1} ifconfig|grep "inet addr"|awk '{print $2}'|cut -d ":" -f 2|grep -v "127.0.0.1"`
   for subIP in ${thisIP}
   do
      echo "${subIP}|"
   done
}

#判断指定的远程目录可用空间是否达到指定大小(MB)
#参数1 地址 例如 "192.168.1.1"
#参数2 路径 例如 "/opt/sequoiadb/"
#参数3 大小 例如 256
function sshCheckAvailable()
{
   available=`ssh root@$1 df -m $2 | tail -n1|awk '{print $4}'`
   if [ ${available} -lt $3 ]; then
      return 1
   else
      return 0
   fi
}

#执行远程系统的环境检查
#参数1 地址 例如 "192.168.1.1"
#参数2 主机列表的元素名
#参数3 安装后大小需求(MB)
function sshCheckEnv()
{
   ssh root@$1 "cd /tmp; ./sequoiadbcheck.sh ${2} ${3}"
   if [ $? -ne 0 ]; then
      return 1
   else
      return 0
   fi
}

#执行远程系统的安装(不用)
#参数1 地址 例如 "192.168.1.1"
#参数2 是否第一次创建 0:是,1:否
#参数3 coord的地址
#参数4 coord的端口
#参数5 节点列表的元素名
function sshInstall()
{
   sshInstallon "${1}" "${5}"
   if [ $? -ne 0 ]; then
      return 1
   fi
   sshNodeStart "${1}" "${2}" "${3}" "${4}" "${5}"
   if [ $? -ne 0 ]; then
      return 1
   fi
   return 0
}

#执行节点启动或创建
#参数1 地址 例如 "192.168.1.1"
#参数2 是否第一次创建 0:是,1:否
#参数3 coord的地址
#参数4 coord的端口
#参数5 节点列表的元素名
function sshNodeStart()
{
   ssh root@$1 "cd /tmp; ./sequoiadbinstall.sh 2 ${2} ${3} ${4} ${5}"
   if [ $? -ne 0 ]; then
      return 1
   fi
   return 0
}

#执行安装包安装
#参数1 地址
#参数2 主机列表的元素名
function sshInstallon()
{
   ssh root@${1} "cd /tmp; ./sequoiadbinstall.sh 1 ${2}"
   if [ $? -ne 0 ]; then
      return 1
   fi
   return 0 ;
}

#执行远程系统的卸载
#参数1 地址
#参数1 数据库安装路径
function sshUninstall()
{
   ssh root@$1 "cd /tmp; ./sequoiadbuninstall.sh ${2} ${3} ${4} ${5} ${6} ${7}"
   if [ $? -ne 0 ]; then
      return 1
   else
      return 0
   fi
}

#执行coord启动所有分区组
#参数1 路径
#参数2 coord的地址
#参数3 coord的端口
function sshGroupStart()
{
   ssh root@${2} "cd /tmp; ./sequoiadbgroupstart.sh 1 ${1} ${2} ${3}"
   if [ $? -ne 0 ]; then
      return 1
   else
      return 0
   fi
}

#执行coord启动指定分区组
#参数1 路径
#参数2 coord的地址
#参数3 coord的端口
#参数4 分区组名
function sshOneGroupStart()
{
   ssh root@${2} "cd /tmp; ./sequoiadbgroupstart.sh 2 ${1} ${2} ${3} ${4}"
   if [ $? -ne 0 ]; then
      return 1
   else
      return 0
   fi
}

#复制文件到其他主机上的/tmp目录
#参数1 文件名 例如 "/opt/sequoiadb/sequoiadb.run"
#参数2 地址   例如 "ubuntu-test-01"
function copyFile2OtherHost()
{
   scp $1 root@$2:/tmp
   if [ $? -ne 0 ]; then
      return 1
   else
      return 0
   fi
}

#步骤1，当检查完所有的系统环境之后运行
function distribution()
{
   target=""
   for array_name in ${LIST_HOST[@]}
   do
      eval "host_array=(\"\${${array_name}[@]}\")"
      target="${host_array[0]}"
      #复制安装文件
      copyFile2OtherHost "${INSTALL_PATH}/${INSTALL_NAME}" ${target}
      if [ $? -ne 0 ]; then
         echo_r "Error" $FUNCNAME $LINENO "Copy ${INSTALL_NAME} file to ${target}:/tmp failed"
         return 1
      fi
      echo_r "Event" $FUNCNAME $LINENO "Copy ${INSTALL_NAME} file to ${target}:/tmp successfully"
   done
   return 0
}

#主系统检查系统环境
#参数1 类型 如果是主系统 1  如果是分系统 2
function checkEnv()
{
   #检查root权限启动
   checkRoot
   if [ $? -ne 0 ]; then
      echo_r "Error" $FUNCNAME $LINENO "Please use the root user"
   fi
   target=""
   #遍历主机列表
   for array_name in ${LIST_HOST[@]}
   do
      eval "child=(\"\${${array_name}[@]}\")"
      target="${child[0]}"

      #检查网络连接是否正常
      checkPing ${target}
      if [ $? -ne 0 ]; then
         echo_r "Error" $FUNCNAME $LINENO "The ${target} is not ping"
         return 1
      fi

      if [ "${1}" -eq 1 ]; then
         #检查信任关系
         checkTrust ${target}
         if [ $? -ne 0 ]; then
            echo_r "Error" $FUNCNAME $LINENO "The current host and ${target} is not a trust relationship"
            return 1
         fi
      fi

      #检查hosts与目标hostname是否匹配
      checkHostname2 ${target}
      if [ $? -ne 0 ]; then
         echo_r "Error" $FUNCNAME $LINENO "The target ${target} host and local hosts is not match"
         return 1
      fi

      #这是主系统执行的
      if [ "${1}" -eq 1 ]; then
         #复制子程序到目标机器
         copyFile2OtherHost "${PWD}/sequoiadbconfig.sh" ${target}
         if [ $? -ne 0 ]; then
            echo_r "Error" $FUNCNAME $LINENO "Copy the sequoiadbconfig.sh file to the ${target} failure"
            return 1
         fi
         copyFile2OtherHost "${PWD}/sequoiadbfun1.sh" ${target}
         if [ $? -ne 0 ]; then
            echo_r "Error" $FUNCNAME $LINENO "Copy the sequoiadbfun1.sh file to the ${target} failure"
            return 1
         fi
         copyFile2OtherHost "${PWD}/sequoiadbfun2.sh" ${target}
         if [ $? -ne 0 ]; then
            echo_r "Error" $FUNCNAME $LINENO "Copy the sequoiadbfun2.sh file to the ${target} failure"
            return 1
         fi
         copyFile2OtherHost "${PWD}/sequoiadbcheck.sh" ${target}
         if [ $? -ne 0 ]; then
            echo_r "Error" $FUNCNAME $LINENO "Copy the sequoiadbcheck.sh file to the ${target} failure"
            return 1
         fi
         copyFile2OtherHost "${PWD}/sequoiadbinstall.sh" ${target}
         if [ $? -ne 0 ]; then
            echo_r "Error" $FUNCNAME $LINENO "Copy the sequoiadbinstall.sh file to the ${target} failure"
            return 1
         fi
         copyFile2OtherHost "${PWD}/sequoiadbuninstall.sh" ${target}
         if [ $? -ne 0 ]; then
            echo_r "Error" $FUNCNAME $LINENO "Copy the sequoiadbinstall.sh file to the ${target} failure"
            return 1
         fi
         copyFile2OtherHost "${PWD}/sequoiadbgroupstart.sh" ${target}
         if [ $? -ne 0 ]; then
            echo_r "Error" $FUNCNAME $LINENO "Copy the sequoiadbinstall.sh file to the ${target} failure"
            return 1
         fi
         #远程控制其他主机进行环境检查
         sshCheckEnv ${target} ${array_name} ${INSTALL_SDB_SIZE}
         if [ $? -ne 0 ]; then
            echo_r "Error" $FUNCNAME $LINENO "The ${target} Environmental inspection error"
            return 1
         fi
         #远程检查/tmp路径可用空间
         sshCheckAvailable ${target} "/tmp" ${INSTALL_FILE_SIZE}
         if [ $? -ne 0 ]; then
            echo_r "Error" $FUNCNAME $LINENO "The ${target} /tmp directory available space is less than ${INSTALL_FILE_SIZE}MB"
            return 1
         fi
      fi
      echo_r "Event" $FUNCNAME $LINENO "The target ${target} clear"
   done
   return 0
}

#检查主机系统环境，指定机器
#参数1 hostname
#参数2 安装文件包的大小
#参数3 安装后大小
#参数4 主机列表元素名
function checkEnvOneHost()
{
   target=${1}
   #检查网络连接是否正常
   checkPing ${target}
   if [ $? -ne 0 ]; then
      echo_r "Error" $FUNCNAME $LINENO "The ${target} is not ping"
      return 1
   fi
   #检查信任关系
   checkTrust ${target}
   if [ $? -ne 0 ]; then
      echo_r "Error" $FUNCNAME $LINENO "The current host and ${target} is not a trust relationship"
      return 1
   fi
   #检查hosts与目标hostname是否匹配
   checkHostname2 ${target}
   if [ $? -ne 0 ]; then
      echo_r "Error" $FUNCNAME $LINENO "The target ${target} host and local hosts is not match"
      return 1
   fi
   #复制子程序到目标机器
   copyFile2OtherHost "${PWD}/sequoiadbconfig.sh" ${target}
   if [ $? -ne 0 ]; then
      echo_r "Error" $FUNCNAME $LINENO "Copy the sequoiadbconfig.sh file to the ${target} failure"
      return 1
   fi
   copyFile2OtherHost "${PWD}/sequoiadbfun1.sh" ${target}
   if [ $? -ne 0 ]; then
      echo_r "Error" $FUNCNAME $LINENO "Copy the sequoiadbfun1.sh file to the ${target} failure"
      return 1
   fi
   copyFile2OtherHost "${PWD}/sequoiadbfun2.sh" ${target}
   if [ $? -ne 0 ]; then
      echo_r "Error" $FUNCNAME $LINENO "Copy the sequoiadbfun2.sh file to the ${target} failure"
      return 1
   fi
   copyFile2OtherHost "${PWD}/sequoiadbcheck.sh" ${target}
   if [ $? -ne 0 ]; then
      echo_r "Error" $FUNCNAME $LINENO "Copy the sequoiadbcheck.sh file to the ${target} failure"
      return 1
   fi
   copyFile2OtherHost "${PWD}/sequoiadbinstall.sh" ${target}
   if [ $? -ne 0 ]; then
      echo_r "Error" $FUNCNAME $LINENO "Copy the sequoiadbinstall.sh file to the ${target} failure"
      return 1
   fi
   copyFile2OtherHost "${PWD}/sequoiadbuninstall.sh" ${target}
   if [ $? -ne 0 ]; then
      echo_r "Error" $FUNCNAME $LINENO "Copy the sequoiadbinstall.sh file to the ${target} failure"
      return 1
   fi
   copyFile2OtherHost "${PWD}/sequoiadbgroupstart.sh" ${target}
   if [ $? -ne 0 ]; then
      echo_r "Error" $FUNCNAME $LINENO "Copy the sequoiadbinstall.sh file to the ${target} failure"
      return 1
   fi
   #远程控制其他主机进行环境检查
   sshCheckEnv "${target}" "${4}" "${3}"
   if [ $? -ne 0 ]; then
      echo_r "Error" $FUNCNAME $LINENO "The ${target} Environmental inspection error"
      return 1
   fi
   #远程检查/tmp路径可用空间
   sshCheckAvailable "${target}" "/tmp" "${2}"
   if [ $? -ne 0 ]; then
      echo_r "Error" $FUNCNAME $LINENO "The ${target} /tmp directory available space is less than ${2}MB"
      return 1
   fi
   echo_r "Event" $FUNCNAME $LINENO "The target ${target} pass"
   return 0
}

#把所有分区组启动
#参数1 sdb路径
#参数2 coord的地址
#参数3 coord的端口
function groupStart()
{
   rc=0

   ${1} "var db = new Sdb('${2}','${3}')"
   rc=$?
   if [ ${rc} -ne 0 ]; then
      echo_r "Error" $FUNCNAME $LINENO "Failed to connect data, rc=${rc}"
      return 1
   fi
   for group_name in ${LIST_GROUP[@]}
   do
      ${1} "db.startRG('${group_name}')"
      echo_r "Event" $FUNCNAME $LINENO "group ${group_name} is start"
   done
   return 0
}

#把所有分区组启动
#参数1 sdb路径
#参数2 coord的地址
#参数3 coord的端口
#参数4 分区组名
function oneGroupStart()
{
   rc=0

   ${1} "var db = new Sdb('${2}','${3}')"
   rc=$?
   if [ ${rc} -ne 0 ]; then
      echo_r "Error" $FUNCNAME $LINENO "Failed to connect coord ${2}:${3}, rc=${rc}"
      return 1
   fi
   ${1} "db.startRG('${4}')"
   rc=$?
   if [ ${rc} -ne 0 ]; then
      echo_r "Error" $FUNCNAME $LINENO "Failed to activation ${4}, rc=${rc}"
      return 1
   fi
   echo_r "Event" $FUNCNAME $LINENO "group ${4} is start"
   return 0
}

#执行远程卸载脚本
function sshUninstallSDB()
{
   target=""
   child=""
   logpath=""
   diagpath=""
   dbpath=""
   indexpath=""
   bkuppath=""
   for array_name in ${LIST_NODE[@]}
   do
      eval "node_array=(\"\${${array_name}[@]}\")"
      eval "host_array=(\"\${${node_array[2]}[@]}\")"
      eval "node_conf=(\"\${${node_array[3]}[@]}\")"

      target="${host_array[0]}"


      installpath="${host_array[1]}"

      num=`get_SDBCONF_num "logpath"`
      logpath="${node_conf[${num}]}"

      num=`get_SDBCONF_num "diagpath"`
      diagpath="${node_conf[${num}]}"

      num=`get_SDBCONF_num "dbpath"`
      dbpath="${node_conf[${num}]}"

      num=`get_SDBCONF_num "indexpath"`
      indexpath="${node_conf[${num}]}"

      num=`get_SDBCONF_num "bkuppath"`
      bkuppath="${node_conf[${num}]}"

      sshUninstall "${target}" "${installpath}" "${logpath}" "${diagpath}" "${dbpath}" "${indexpath}" "${bkuppath}"
      if [ $? -ne 0 ]; then
         echo_r "Error" $FUNCNAME $LINENO "${target} Failed to uninstall"
         return 1
      fi
   done
   echo_r "Event" $FUNCNAME $LINENO "All uninstall"
   return 0
}

#执行远程安装脚本
function sshInstallSDB()
{
   target=""
   coordf=0
   cataf=0
   dataf=0
   coordsdb=""
   coordaddr=""
   coordport=""

   #先执行安装包安装
   for array_name in ${LIST_HOST[@]}
   do
      eval "host_array=(\"\${${array_name}[@]}\")"
      target="${host_array[0]}"
      sshInstallon "${target}" "${array_name}"
      if [ $? -ne 0 ]; then
         return 1
      fi
   done

   #开始启动节点
   for array_name in ${LIST_NODE[@]}
   do
      eval "node_array=(\"\${${array_name}[@]}\")"
      eval "host_array=(\"\${${node_array[2]}[@]}\")"
      eval "node_conf=(\"\${${node_array[3]}[@]}\")"

      target="${host_array[0]}"

      if [ ${node_array[0]} = "coord" ]; then
         if [ ${coordf} -eq 0 ]; then
            coordaddr="${host_array[0]}"
            coordsdb="${host_array[1]}/bin/sdb"
            num=`get_SDBCONF_num "svcname"`
            coordport="${node_conf[${num}]}"
         fi
         sshNodeStart "${target}" "${coordf}" "${coordaddr}" "${coordport}" "${array_name}"
         if [ $? -ne 0 ]; then
            return 1
         fi
         if [ ${coordf} -eq 0 ]; then
            coordf=1
         fi
      elif [ ${node_array[0]} = "cata" ]; then
         sshNodeStart "${target}" "${cataf}" "${coordaddr}" "${coordport}" "${array_name}"
         if [ $? -ne 0 ]; then
            return 1
         fi
         if [ ${cataf} -eq 0 ]; then
            cataf=1
         fi
      else
         sshNodeStart "${target}" "${dataf}" "${coordaddr}" "${coordport}" "${array_name}"
         if [ $? -ne 0 ]; then
            return 1
         fi
         if [ ${dataf} -eq 0 ]; then
            dataf=1
         fi
      fi
   done
   sshGroupStart ${coordsdb} ${coordaddr} ${coordport}
   return 0
}

#检验配置文件端口和路径
function check_conf_advanced()
{
   cursor_arr_name=""
   #sdbcm端口
   port0=""
   #svcname
   port1=""
   #replname
   port2=""
   #shardname
   port3=""
   #catalogname
   port4=""
   #httpname
   port5=""
   #logpath
   path1=""
   for array_name in ${LIST_CONFIG[@]}
   do
      cursor_arr_name=${array_name}
      eval "child=(\"\${${array_name}[@]}\")"
      port0=${child[7]}
      port1=${child[2]}
      if [ -n "${child[10]}" ]; then
         echo ""
      fi
      #port2
   done
}

#校验配置文件基础
function check_conf_base()
{
   cursor_arr_name=""
   sdbcm_port=""
   if [ -n "${INSTALL_NAME}" ]; then
      echo_r "Error" $FUNCNAME $LINENO "INSTALL_NAME can not null"
      return 1
   fi

   for array_name in ${LIST_CONFIG[@]}
   do
      cursor_arr_name=${array_name}
      eval "child=(\"\${${array_name}[@]}\")"
      #检查配置文件hostname和ip是否有一项填写
      if [ -z "${child[0]}" ] && [ -z "${child[1]}" ]; then
         echo_r "Error" $FUNCNAME $LINENO "${cursor_arr_name} Host and IP must fill in one"
         return 1
      fi
      #检查端口是否填写
      if [ -z "${child[2]}" ]; then
         echo_r "Error" $FUNCNAME $LINENO "${cursor_arr_name} port can not null"
         return 1
      fi
      #检查端口是否跟扩展配置的端口一致
      if [ -n "${child[10]}" ]; then
         eval "tempconf=(\"\${${child[10]}[@]}\")"
         if [ -n "${tempconf[7]}" ]; then
            if [ "${child[2]}" != "${tempconf[7]}" ]; then
               echo_r "Error" $FUNCNAME $LINENO "${cursor_arr_name} port does not the same of ${child[10]}"
               return 1
            fi
         fi
      fi
      #检查安装路径是否填写
      if [ -z "${child[3]}" ]; then
         echo_r "Error" $FUNCNAME $LINENO "${cursor_arr_name} install path can not null"
         return 1
      fi
      #检查用户组是否填写
      if [ -z "${child[4]}" ]; then
         echo_r "Error" $FUNCNAME $LINENO "${cursor_arr_name} user group can not null"
         return 1
      fi
      #检查用户名是否填写
      if [ -z "${child[5]}" ]; then
         echo_r "Error" $FUNCNAME $LINENO "${cursor_arr_name} user can not null"
         return 1
      fi
      #检查用户密码是否填写
      if [ -z "${child[6]}" ]; then
         echo_r "Error" $FUNCNAME $LINENO "${cursor_arr_name} password can not null"
         return 1
      fi
      #检查sdbcm端口是否填写
      if [ -z "${child[7]}" ]; then
         echo_r "Error" $FUNCNAME $LINENO "${cursor_arr_name} sdbcm port can not null"
         return 1
      fi
      #检查每个配置的sdbcm是否一致
      if [ -z "${sdbcm_port}" ]; then
         sdbcm_port="${child[7]}"
      else
         if [ "${child[7]}" != "${sdbcm_port}" ]; then
            echo_r "Error" $FUNCNAME $LINENO "${cursor_arr_name} sdbcm port does not the same of ${sdbcm_port}"
         fi
      fi
      #检查角色是否填写
      if [ -z "${child[8]}" ]; then
         echo_r "Error" $FUNCNAME $LINENO "${cursor_arr_name} role can not null"
         return 1
      fi
      #检查数据文件存储路径是否填写
      if [ -z "${child[9]}" ]; then
         echo_r "Error" $FUNCNAME $LINENO "${cursor_arr_name} database path can not null"
         return 1
      fi
      #检查分区组是否填写
      if [ "${child[9]}" = "data" ] && [ -z "${child[11]}" ]; then
         echo_r "Error" $FUNCNAME $LINENO "${cursor_arr_name} group can not null"
         return 1
      fi
      #检查分区组是否存在列表中
      if [ "${child[9]}" = "data" ]; then
         check_group_is_exist "${child[11]}"
         if [ $? -ne 0 ]; then
            echo_r "Error" $FUNCNAME $LINENO "${cursor_arr_name} group ${child[11]} does not exist in the LIST_GROUP"
            return 1
         fi
      fi
   done
}

#判断分区组是否存在列表中
#参数1 分区组名
function check_group_is_exist()
{
   for group_name in ${LIST_GROUP[@]}
   do
      if [ "${1}" = "${group_name}" ] && [ -n "${group_name}" ]; then
         return 0
      fi
   done
   return 1
}
