#!/bin/bash

#校验步骤1的函数

#检查端口是否被占用
#参数1 端口号 例如 "50000"
function checkLocalPort()
{
   portNum=0
   portNum=`netstat -tln|grep "\<$1\>"| wc -l`
   if [ ${portNum} -eq 0 ]; then
      return 0
   else
      return 1
   fi
}

#检查路径是否存在 返回1是不存在
#参数1 路径 例如 "/opt/sequoiadb/"
function checkPathExist()
{
   if [ ! -d "$1" ]; then
      return 1
   else
      return 0
   fi
}

#创建文件夹
#参数1 文件夹名 例如 "myfile"
function createFolder()
{
   mkdir -p $1
}

#执行指定用户权限的命令
#参数1 用户名   例如 sdbadmin
#参数2 执行命令 例如 "mkdir aa"
function userExec()
{
   su - $1 -c "$2"
}

#创建指定用户权限的文件夹
#参数1 用户名   例如 sdbadmin
#参数2 文件夹名 例如 "/opt/sequoiadb/"
function userCreateFolder()
{
   userExec $1 "mkdir -p $2"
}

#判断指定目录可用空间是否达到指定大小(MB)
#参数1 路径 例如 "/opt/sequoiadb/"
#参数2 大小 例如 256
function checkAvailable()
{
   available=`df -m $1 | tail -n1|awk '{print $4}'`
   if [ ${available} -lt $2 ]; then
      return 1
   else
      return 0
   fi
}

#判断目录是否为空 返回1 空目录 0非空
#参数1 路径 例如 "/opt/sequoiadb"
function checkFileNull()
{
   if [ "`ls -A $1`" = "" ]; then
      return 1
   else
      return 0
   fi
}

#运行安装文件
#参数1 文件名   例如 "sequoiadb.run"
#参数2 安装路径 例如 "/opt/sequoiadb"
#参数3 用户名   例如 "sdbadmin"
#参数4 密码     例如 "sequoiadb"
#参数5 端口     例如 "50010"
function install()
{
   /tmp/$1 --mode unattended --prefix $2 --username $3 --userpasswd $4 --port $5
}

#检查用户组是否存在 返回0 存在 返回1 不存在
#参数1 用户组名 例如 "root"
function checkGroup()
{
   groupNum=0
   groupNum=`cat /etc/group | grep "\<$1\>"|wc -l`
   if [ ${groupNum} -ne 0 ]; then
      return 0
   else
      return 1
   fi
}

#检查用户是否存在 返回0 存在 返回1 不存在
#参数1 用户名 例如 "root"
function checkUser()
{
   userNum=0
   userNum=`cat /etc/passwd | grep "\<$1\>"|wc -l`
   if [ ${userNum} -ne 0 ]; then
      return 0
   else
      return 1
   fi
}

#替换指定文件的字符串
#参数1 文件名
#参数2 查找的字符串
#参数3 替换的字符串
function replaceStr()
{
   su - root -c "sed -i '/${2}/c\\${3}' ${1}"
}

#步骤1 子系统检查本地环境
#参数1 节点端口
function checkLocalEnv()
{
   #获取本地IP和hostname
   thisIP=`ifconfig|grep "inet addr"|awk '{print $2}'|cut -d ":" -f 2|grep -v "127.0.0.1"`
   thisHost=`hostname`
   #找出属于本机的配置记录
   child=""
   target=""
   for array_name in ${LIST_CONFIG[@]}
   do
      eval "child=(\"\${${array_name}[@]}\")"
      if [ -n "${child[0]}" ]; then
         #对比IP
         for subIP in ${thisIP}
         do
            if [[ ${subIP} = ${child[0]} ]] && [[ ${1} = ${child[2]} ]]; then
               target=${child[0]}
               break ;
            fi
         done
         if [ -n "${target}" ]; then
            break ;
         fi
      elif [ -n "${child[1]}" ]; then
         #对比hostname
         if [[ ${thisHost} = ${child[1]} ]] && [[ ${1} = ${child[2]} ]]; then
            target=${child[1]}
            break ;
         fi
      else
         echo_r "Error" $FUNCNAME $LINENO "Host or IP must fill in one"
         return 1
      fi
   done

   #如果用的不是端口而是服务名，要做映射(未做)

   #检查sdbcm用的端口在本地是否被占用
   checkLocalPort ${child[7]}
   if [ $? -ne 0 ]; then
      echo_r "Error" $FUNCNAME $LINENO "${target} sdbcm ${child[7]} port is already in use"
      return 1
   fi

   #检查数据库用的端口在本地是否被占用
   checkLocalPort ${child[2]}
   if [ $? -ne 0 ]; then
      echo_r "Error" $FUNCNAME $LINENO "${target} sdbcm ${child[2]} port is already in use"
      return 1
   fi

   #如果拥有配置设置
   if [ -n "${child[10]}" ]; then
      eval "confarray=(\"\${${child[10]}[@]}\")"
      #检查端口
      checkLocalPort ${confarray[8]}
      if [ $? -ne 0 ]; then
         echo_r "Error" $FUNCNAME $LINENO "${target} sdbcm ${confarray[8]} port is already in use"
         return 1
      fi
      checkLocalPort ${confarray[9]}
      if [ $? -ne 0 ]; then
         echo_r "Error" $FUNCNAME $LINENO "${target} sdbcm ${confarray[9]} port is already in use"
         return 1
      fi
      checkLocalPort ${confarray[10]}
      if [ $? -ne 0 ]; then
         echo_r "Error" $FUNCNAME $LINENO "${target} sdbcm ${confarray[10]} port is already in use"
         return 1
      fi
      checkLocalPort ${confarray[11]}
      if [ $? -ne 0 ]; then
         echo_r "Error" $FUNCNAME $LINENO "${target} sdbcm ${confarray[11]} port is already in use"
         return 1
      fi
   fi

   #检查用户组是否存在
   checkGroup ${child[4]}
   if [ $? -ne 0 ]; then
      #用户组不存在,那么创建
      groupadd ${child[4]}
   fi

   #检查用户是否存在
   checkUser ${child[5]}
   if [ $? -ne 0 ]; then
      #用户不存在,那么创建
      useradd -d "${child[3]}" -g "${child[4]}" -p "${child[6]}" -s "/bin/bash" "${child[5]}"
   else
      #用户存在，那么增加他的附加组
      usermod -G "${child[4]}" -s "/bin/bash" "${child[5]}"
   fi

   #检查安装路径是否存在
   checkPathExist ${child[3]}
   if [ $? -ne 1 ]; then
      #路径已经存在，检查路径目录内是否有文件
      checkPathExist "${child[3]}/bin"
      if [ $? -ne 1 ]; then
         checkFileNull "${child[3]}/bin"
         if [ $? -ne 1 ]; then
            echo_r "Error" $FUNCNAME $LINENO "${target} The installation path ${child[3]} is not empty"
            return 1
         fi
      fi
   else
      #路径不存在，创建目录
      #创建目录
      createFolder ${child[3]}
   fi
   #修改所属用户 和 用户组
   chown ${child[5]}:${child[4]} -R "${child[3]}"
   #检查安装路径是否有足够大的空间
   checkAvailable "${child[3]}" ${INSTALL_SDB_SIZE}
   if [ $? -ne 0 ]; then
      echo_r "Error" $FUNCNAME $LINENO "The ${target} ${child[3]} directory available space is less than ${INSTALL_FILE_SIZE}MB"
      return 1
   fi

   if [ ${child[8]} = "coord" ]; then
      #检查配置文件路径是否存在
      confpath="${child[3]}/conf/local/${child[2]}"
      checkPathExist ${confpath}
      if [ $? -ne 1 ]; then
         #路径已经存在，检查路径目录内是否有文件
         checkFileNull ${confpath}
         if [ $? -ne 1 ]; then
            echo_r "Error" $FUNCNAME $LINENO "${target} The installation path ${confpath} is not empty"
            return 1
         fi
      else
         #路径不存在，创建目录
         #创建目录
         createFolder ${confpath}
      fi
      #修改所属用户 和 用户组
      chown ${child[5]}:${child[4]} -R "${confpath}"
   fi

   #检查数据库存储路径是否存在
   checkPathExist ${child[9]}
   if [ $? -ne 1 ]; then
      #路径已经存在，检查路径目录内是否有文件
      checkFileNull ${child[9]}
      if [ $? -ne 1 ]; then
         echo_r "Error" $FUNCNAME $LINENO "${target} The installation path ${child[9]} is not empty"
         return 1
      fi
   else
      #路径不存在，创建目录
      #创建目录
      createFolder ${child[9]}
   fi
   #修改所属用户 和 用户组
   chown ${child[5]}:${child[4]} -R "${child[9]}"
   #检查数据库存储路径是否有足够256MB的空间
   checkAvailable "${child[9]}" 256
   if [ $? -ne 0 ]; then
      echo_r "Error" $FUNCNAME $LINENO "The ${target} ${child[9]} directory available space is less than 256MB"
      return 1
   fi

   #检查其他路径是否存在(未做)

   echo_r "Event" $FUNCNAME $LINENO "The ${target} all clear"
   return 0
}

#安装数据库文件
#参数1 数据库端口
function installsdb()
{
   #获取本地IP和hostname
   thisIP=`ifconfig|grep "inet addr"|awk '{print $2}'|cut -d ":" -f 2|grep -v "127.0.0.1"`
   thisHost=`hostname`
   #找出属于本机的配置记录
   child=""
   target=""
   for array_name in ${LIST_CONFIG[@]}
   do
      eval "child=(\"\${${array_name}[@]}\")"
      if [ -n "${child[0]}" ]; then
         #对比IP
         for subIP in ${thisIP}
         do
            if [[ ${subIP} = ${child[0]} ]] && [[ ${1} = ${child[2]} ]]; then
               target=${child[0]}
               break ;
            fi
         done
         if [ -n "${target}" ]; then
            break ;
         fi
      elif [ -n "${child[1]}" ]; then
         #对比hostname
         if [[ ${thisHost} = ${child[1]} ]] && [[ ${1} = ${child[2]} ]]; then
            target=${child[1]}
            break ;
         fi
      else
         echo_r "Error" $FUNCNAME $LINENO "Host or IP must fill in one"
         return 1
      fi
   done

   checkPathExist "${child[3]}"
   if [ $? -ne 1 ]; then
      #路径已经存在，检查路径目录内是否有文件
      checkPathExist "${child[3]}/bin"
      if [ $? -ne 1 ]; then
         checkFileNull "${child[3]}/bin"
         if [ $? -ne 1 ]; then
            #如果已经有文件，证明已经安装了
            return 0
         fi
      fi
   fi

   #安装数据库
   install "${INSTALL_NAME}" "${child[3]}" "${child[5]}" "${child[6]}" "${child[7]}"
   if [ $? -ne 0 ]; then
      echo_r "Error" $FUNCNAME $LINENO "${target} Failed to install SequoiaDB"
      return 1
   fi
}

#启动coord节点
#参数1 数据数组
#参数2 IP或hostname
#参数3 是否第一次创建 0:是,1:否
function startCoord()
{
   child=$1
   target=$2

   #复制出配置文件
   samconfpath="${child[3]}/conf/samples/sdb.conf.coord"
   confpath="${child[3]}/conf/local/${child[2]}"
   cp "${samconfpath}" "${confpath}/sdb.conf"

   #修改配置文件
   if [ -n "${child[10]}" ]; then
      eval "coord_array=(\"\${${child[10]}[@]}\")"
      for((i=0;i<${#coord_array[@]};i++))
      do
         if [ -n "${coord_array[${i}]}" ]; then
            replaceStr "${confpath}/sdb.conf" "${SDB_CONFIG[${i}]}=" "${SDB_CONFIG[${i}]}=${coord_array[${i}]}"
         fi
      done
   fi

   #如果不是首个coord
   if [ ${3} -ne 0 ]; then
      catalogaddr=""
      thisHost=""
      isfirst=1
      #那么收集catalog信息
      for array_name in ${LIST_CONFIG[@]}
      do
         eval "tempchild=(\"\${${array_name}[@]}\")"
         if [ ${tempchild[8]} = "cata" ]; then
            if [ -n "${tempchild[0]}" ]; then
               thisHost=`ssh root@${tempchild[0]} hostname`
            else
               thisHost=${tempchild[1]}
            fi
            if [ ${isfirst} -eq 1 ]; then
               isfirst=0
            else
               catalogaddr="${catalogaddr},"
            fi
            thisport=""
            if [ -n "${tempchild[10]}" ]; then
               catalognameNum=0
               for((i=0;i<${#SDB_CONFIG[@]};i++))
               do
                  if [ ${SDB_CONFIG[${i}]} = "catalogname" ]; then
                     catalognameNum=${i}
                     break
                  fi
               done

               eval "tempcataconf=(\"\${${tempchild[10]}[@]}\")"
               thisport=${tempcataconf[${catalognameNum}]}
            else
               thisport=$[${tempchild[2]}+3]
            fi
            catalogaddr="${catalogaddr}${thisHost}:${thisport}"
         fi
      done
      #写入到coord配置文件中
      if [ -n "${catalogaddr}" ]; then
         replaceStr "${confpath}/sdb.conf" "catalogaddr=" "catalogaddr=${catalogaddr}"
      fi
   fi

   userExec "${child[5]}" "${child[3]}/bin/sdbstart -c ${confpath}"
   if [ $? -ne 0 ]; then
      echo_r "Error" $FUNCNAME $LINENO "${target} Failed to start coord"
      return 1
   else
      echo_r "Event" $FUNCNAME $LINENO "${target} ${child[2]} coord is start"
      return 0
   fi
}

#通过sdb启动data
#参数1 数据数组
#参数2 IP或hostname
#参数3 是否第一次创建 0:是,1:否
#参数4 coord的地址
#参数5 coord的端口
#参数6 sdb的路径
function startData()
{
   child=$1
   target=$2

   isfirst=1
   jsonconf=""
   if [ -n "${child[10]}" ]; then
      eval "coord_array=(\"\${${child[10]}[@]}\")"
      for((i=0;i<${#coord_array[@]};i++))
      do
         if [ -n "${coord_array[${i}]}" ]; then
            if [ ${isfirst} -eq 1 ]; then
               isfirst=0
               jsonconf="{"
            else
               jsonconf="${jsonconf},"
            fi
            jsonconf="${jsonconf}${SDB_CONFIG[${i}]}:\"${coord_array[${i}]}\""
         fi
      done
      if [ -n "${jsonconf}" ]; then
         jsonconf=${jsonconf}"}"
      fi
   fi

   #sdb连接coord
   rc=0
   ${6} "var db = new Sdb('${4}','${5}')"
   rc=$?
   if [ ${rc} -ne 0 ]; then
      echo_r "Error" $FUNCNAME $LINENO "${target} Failed to connect data, rc=${rc}"
      return 1
   fi
   if [ ${3} -eq 0 ]; then
      #如果是首次创建数据节点，那么先遍历创建分区组
      for group_name in ${LIST_GROUP[@]}
      do
         ${6} "db.createRG('${group_name}')"
         rc=$?
         if [ ${rc} -ne 0 ]; then
            echo_r "Error" $FUNCNAME $LINENO "${target} Failed to create group ${group_name}, rc=${rc}"
            return 1
         fi
      done
   fi

   #选择分区组
   ${6} "var datarg = db.getRG('${child[11]}')"
   rc=$?
   if [ ${rc} -ne 0 ]; then
      echo_r "Error" $FUNCNAME $LINENO "${target} Failed to get group ${child[11]}, rc=${rc}"
      return 1
   fi
   #创建数据节点
   if [ -n "${jsonconf}" ]; then
      ${6} "datarg.createNode('`hostname`','${child[2]}','${child[9]}','${jsonconf}')"
   else
      ${6} "datarg.createNode('`hostname`','${child[2]}','${child[9]}')"
   fi
   rc=$?
   if [ ${rc} -ne 0 ]; then
      echo_r "Error" $FUNCNAME $LINENO "${target} Failed to create data, rc=${rc}"
      return 1
   fi
   echo_r "Event" $FUNCNAME $LINENO "${target} ${child[2]} data is create"
   return 0
}

#通过sdb启动catalog
#参数1 数据数组
#参数2 IP或hostname
#参数3 是否第一次创建 0:是,1:否
#参数4 coord的地址
#参数5 coord的端口
#参数6 sdb的路径
function startCata()
{
   child=$1
   target=$2

   isfirst=1
   jsonconf=""
   if [ -n "${child[10]}" ]; then
      eval "coord_array=(\"\${${child[10]}[@]}\")"
      for((i=0;i<${#coord_array[@]};i++))
      do
         if [ -n "${coord_array[${i}]}" ]; then
            if [ ${isfirst} -eq 1 ]; then
               isfirst=0
               jsonconf="{"
            else
               jsonconf="${jsonconf},"
            fi
            jsonconf="${jsonconf}${SDB_CONFIG[${i}]}:\"${coord_array[${i}]}\""
         fi
      done
      if [ -n "${jsonconf}" ]; then
         jsonconf=${jsonconf}"}"
         echo "cata配置： ${jsonconf}"
      fi
   fi

   #sdb连接coord
   rc=0
   ${6} "var db = new Sdb('${4}','${5}')"
   rc=$?
   if [ ${rc} -ne 0 ]; then
      echo_r "Error" $FUNCNAME $LINENO "${target} Failed to connect coord, rc=${rc}"
      return 1
   fi

   isstart1=`${6} "db.listReplicaGroups()"`

   localhostname=`hostname`
   if [ ${3} -eq 0 ]; then
      if [ -n "${jsonconf}" ]; then
         ${6} "db.createCataRG('${localhostname}','${child[2]}','${child[9]}','${jsonconf}')"
      else
         ${6} "db.createCataRG('${localhostname}','${child[2]}','${child[9]}')"
      fi
      rc=$?
      if [ ${rc} -ne 0 ]; then
         echo_r "Error" $FUNCNAME $LINENO "${target} Failed to create catalog, rc=${rc}"
         return 1
      fi
   else
      ${6} "var catarg = db.getRG('SYSCatalogGroup')"
      if [ -n "${jsonconf}" ]; then
         ${6} "var node=catarg.createNode('${localhostname}','${child[2]}','${child[9]}','${jsonconf}')"
      else
         ${6} "var node=catarg.createNode('${localhostname}','${child[2]}','${child[9]}')"
      fi
      rc=$?
      if [ ${rc} -ne 0 ]; then
         echo_r "Error" $FUNCNAME $LINENO "${target} Failed to create catalog node, rc=${rc}"
         return 1
      fi
      ${6} "node.start()"
      rc=$?
      if [ ${rc} -ne 0 ]; then
         echo_r "Error" $FUNCNAME $LINENO "${target} Failed to start catalog node, rc=${rc}"
         return 1
      fi
   fi

   isstart2=${isstart1}
   while [ "${isstart2}" = "${isstart1}" ]
   do
      sleep 30
      isstart2=`${6} "db.listReplicaGroups()"`
   done
   echo_r "Event" $FUNCNAME $LINENO "${target} ${child[2]} catalog is start"
   return 0
}

#启动节点
#参数1 是否第一次创建 0:是,1:否
#参数2 coord的地址
#参数3 coord的端口
#参数4 创建节点的端口
function SDBstart()
{
   #获取本地IP和hostname
   thisIP=`ifconfig|grep "inet addr"|awk '{print $2}'|cut -d ":" -f 2|grep -v "127.0.0.1"`
   thisHost=`hostname`
   #找出属于本机的配置记录
   child=""
   target=""
   for array_name in ${LIST_CONFIG[@]}
   do
      eval "child=(\"\${${array_name}[@]}\")"
      if [ -n "${child[0]}" ]; then
         #对比IP
         for subIP in ${thisIP}
         do
            if [[ ${subIP} = ${child[0]} ]] && [[ ${4} = ${child[2]} ]]; then
               target=${child[0]}
               break ;
            fi
         done
         if [ -n "${target}" ]; then
            break ;
         fi
      elif [ -n "${child[1]}" ]; then
         #对比hostname
         if [[ ${thisHost} = ${child[1]} ]] && [[ ${4} = ${child[2]} ]]; then
            target=${child[1]}
            break ;
         fi
      else
         echo_r "Error" $FUNCNAME $LINENO "Host or IP must fill in one"
         return 1
      fi
   done

   if [ ${child[8]} = "coord" ]; then
      startCoord ${child} ${target} ${1}
      if [ $? -ne 0 ]; then
         return 1
      fi
   elif [ ${child[8]} = "cata" ]; then
      startCata "${child}" "${target}" "${1}" "${2}" "${3}" "${child[3]}/bin/sdb"
      if [ $? -ne 0 ]; then
         return 1
      fi
   elif [ ${child[8]} = "data" ]; then
      startData "${child}" "${target}" "${1}" "${2}" "${3}" "${child[3]}/bin/sdb"
      if [ $? -ne 0 ]; then
         return 1
      fi
   fi
}
