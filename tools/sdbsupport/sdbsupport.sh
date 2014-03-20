#!/bin/bash

. ./sdbsupportfunc1.sh
. ./sdbsupportfunc2.sh
#variable in bash shell
declare -a PORT
declare -a DBPATH
declare -a ROLE
declare -a HOST
confpath=""
pHostNum=""

#gloable variable
hostName=""
svcPort=""
sysInfo="false"
snapShot="false"
hardInfo="false"

#hardware information variable
cpu="false"
memory="false"
disk="false"
netcard="false"
mainboard="false"
bios="false"

#snapshot variable
rcPort="false"
group="false"
context="false"
session="false"
collection="false"
collectionspace="false"
database="false"
system="false"
catalog="false"

#operation system variable
diskmanage="false"
osystem="false"
kermode="false"
env="false"
IDE="false"
network="false"
nfsstat="false"
progress="false"
login="false"
swapon="false"
limit="false"
vmstate="false"
all="false"

#the parameter get where location
firstLoc=""
firstLoc=$1
thirdLoc=""
thirdLoc=$3

#the number of concurrent threads
thread=10

#get the number of parameter and what parameters is 
ParaNum=$#
ParaPass=$@

#check is local host run
Local=""

function Usage()
{
   echo "Command Options:" ;
   echo "    --help                 help information" ;
   echo "    -N [--hostname] arg    database host name " ;
   echo "    -p [--svcport] arg     database sevice port" ;
   echo "    -t [--thread] arg      number of concurrent threads,default:10" ;
   echo "    -s [--snapshot]        snapshot of sequoiadb database" ;
   echo "    -o [--osinfo]          operating system information" ;
   echo "    -h [--hardware]        hardware information" ;
   echo "    --all                  copy the all information of database" ;
   echo "    --cpu                  host cpu information" ;
   echo "    --memory               host memory information" ;
   echo "    --disk                 host disk information" ;
   echo "    --netcard              host netcard information" ;
   echo "    --mainboard            host mainboard information" ;
   echo "    --bios                 host bios information" ;
   echo "    --catalog              catalog snapshot for database" ;
   echo "    --group                group of dababase information" ;
   echo "    --context              context snapshot" ;
   echo "    --session              session snapshot" ;
   echo "    --collection           collection snapshot" ;
   echo "    --collectionspace      collectionspace snapshot" ;
   echo "    --database             database snapshot" ;
   echo "    --system               system snapshot" ;
   echo "    --diskmanage           operating system disk management information" ;
   echo "    --basicsys             operating system basic information" ;
   echo "    --kermode              loadable kernel modules" ;
   echo "    --env                  operating system environment variable" ;
   echo "    --IDE                  integrated development environment" ;
   echo "    --network              network information" ;
   echo "    --nfstat               network file system state" ;
   echo "    --process              operating system process" ;
   echo "    --login                operating system users and history" ;
   echo "    --swapon               operating system swap on" ;
   echo "    --limit                ulimit used to limit the resources occupied shell startup process" ;
   echo "    --vmstate              Show the server status value of a given time interval" ;


}

#the parameters can use  
optArg=`getopt -a -o N:p:t:sohH -l hostname,svcport,thread,snapshot,osinfo,hardware,help,cpu,memory,disk,netcard,mainboard,bios,group,context,session,collection,collectionspace,database,system,diskmanage,basicsys,kermode,env,IDE,network,nfstat,process,login,swapon,limit,vmstate,catalog,all -- "$@"`

#check over the option of sdbsupport
rc=$?
if [ "$rc" == "1" ] ; then
   echo "The option don't have,please check by use '--help'!"
   exit 1
fi


eval set -- "$optArg"

while true
do
#eval set -- "$optArg"
   case $1 in
   -N|--hostname)
      hostName=$2
      shift
      ;;
   -p|--svcport)
      svcPort=$2
      shift
      ;;
   -t|--thread)
      thread=$2
      shift
      ;;
   -s|--snapshot)
      snapShot="true"
      ;;
   -o|--osinfo)
      sysInfo="true"
      ;;
   -h|--hardware)
      hardInfo="true"
      ;;
   --cpu)
      cpu="true"
      ;;
   --memory)
      memory="true"
      ;;
   --disk)
      disk="true"
      ;;
   --netcard)
      netcard="true"
      ;; 
   --mainboard)
      mainboard="true"
      ;;
   --bios)
      bios="true"
      ;;
   --group)
      group="true"
      rcPort="true"
      ;;
   --context)
      context="true"
      rcPort="true"
      ;;
   --session)
      session="true"
      rcPort="true"
      ;;
   --collection)
      collection="true"
      rcPort="true"
      ;;
   --collectionspace)
      collectionspace="true"
      rcPort="true"
      ;;
   --database)
      database="true"
      rcPort="true"
      ;;
   --system)
      system="true"
      rcPort="true"
      ;;
   --diskmanage)
      diskmanage="true"
      ;;
   --basicsys)
      osystem="true"
      ;;
   --kermode)
      kermode="true"
      ;;
   --env)
      env="true"
      ;;
   --IDE)
      IDE="true"
      ;;
   --network)
      network="true"
      ;;
   --nfstat)
      nfsstat="true"
      ;;
   --process)
      progress="true"
      ;;
   --login)
      login="true"
      ;;
   --swapon)
      swapon="true"
      ;;
   --limit)
      limit="true"
      ;;
   --vmstate)
      vmstate="true"
      ;;
   --catalog)
      catalog="true"
      ;;
   --all)
      all="true"
      ;;
   -H|--help)
      Usage
      exit 1
      ;;
   --)
      shift
      break
      ;;
   esac
shift
done

echo ""
echo "**********************Sdbsupport***************************"
echo "* This program run mode will collect all configuration and "
echo "* system environment information.Please make sure whether"
echo "* you need !"
echo "* Begin ....."
echo "***********************************************************"
echo ""

#******************************************************************************
#@Function : Check over environment
#******************************************************************************
#inspect the environment of sequiaDB
localhost=`hostname`
localPath=`pwd`

cd ../../
if [ $? -ne 0 ] ; then
   echo "Failed to go to install path,please check"
   exit 1
fi
installpath=`pwd`
cd $localPath
if [ $? -ne 0 ] ; then
   echo "Failed to go to sdbsupport path"
   exit 1
fi
#echo $installpath
#pwd

#config file path
#echo $installpath
ls $installpath 1>/dev/null
if [ $? -ne 0 ] ; then
   echo "Wrong install path ,Please inspect the sdbsupport path and install path!$?"
   exit 1
fi
confpath=$installpath/conf/local
ls $confpath 1>/dev/null
if [[ $? -ne 0 ]] ; then
   echo "Err,Don't have Nodes !$?"
   exit 1
fi

#*************************************************************************************************
#@Function: create Number of concurrent threads
#@
#*************************************************************************************************
fifo="/tmp/$$.fiofo"
mkfifo $fifo
exec 6<>$fifo
rm -rf $fifo
for ((i=0;i<$thread;i++))
do
   echo ""
done >&6

#************************************************************************
#@Function : get quantity of all hosts and local sevic port
#@Var : HostNum   Exp : the number of hosts in the sequoiaDB
#@Var : PortNum   Exp : the number of local host's sevice port
#@Note : Array begin 1 count ,but such as file row begin 1, so use 1 begin 
#************************************************************************
cd $confpath
dataRole=`find -name "*.conf"|xargs grep "role=data"|cut -d "/" -f 2`
cd $localPath
if [ "$dataRole" == "" ] ; then
   echo "Don't have data node in the sequoiaDB"
   exit 1
fi
#catadrr : get the cata address and catch hosts
data=`echo $dataRole | cut -d " " -f 1`
cataddr=`grep -E "catalogaddr" $confpath/$data/sdb.conf|cut -d '=' -f 2`
HostNum=`awk 'BEGIN{print split("'$cataddr'",cateArr,",")}'`
PortNum=`ls -l $confpath|grep "^d"|wc -l`

#*******************************************************************************
#@Function : get all hosts in sequoiaDB and local host's port/dbpath/role 
#@Var : HOST      Exp : Array variable used to store hosts in sequoiaDB
#@Var : PORT      Exp : Array variable store local host's sevice port
#@Var : DBPATH    Exp : Array variable store local host's dbpath
#@Var : ROLE      Exp : Array variable store local hsot's sevice port's role 
#*******************************************************************************
for i in $(seq 1 $HostNum)
do
   hostcata[$i]=`awk 'BEGIN{split("'$cataddr'",cateArr,",");print cateArr['$i']'}`
   HOST[$i]=`echo ${hostcata[$i]}|cut -d ":" -f 1 `
   if [ "${HOST[$i]}" == "$localhost" ] ; then
      for j in $(seq 1 $PortNum)
      do
         PortArr=`ls $confpath`
         PORT[$j]=`echo $PortArr|cut -d " " -f $j`
         DBPATH[$j]=`grep -E "dbpath" $confpath/${PORT[$j]}/sdb.conf|cut -d '=' -f 2`
         #delete the space in config file and put in tmpconf
         sed -i 's/\ //g' $confpath/${PORT[$j]}/sdb.conf 
         ROLE[$j]=`grep -E "role=" $confpath/${PORT[$j]}/sdb.conf|cut -d '=' -f 2`
      done
   fi
done

#*************************************************************************************************
#@Function : Get parameter passed in and check over them wether or not correct,if don't have this 
#            Host or Port ,will delete the wrong host and port 
#@Var : pHostNum  Exp : quantity of parameter hosts, such as :--hostname ubunt-dev1:ubunt-dev2:
#@Var : pPortNum  Exp : quantity of parameter sevice port, such as:--svcport 51111:61111
#@Var : HostPara  Exp : Array variable to store hosts parameter
#@Var : PortPara  Exp : Array variable to store local hosts' sevice port
#*************************************************************************************************
pHostNum=`awk 'BEGIN{print split("'$hostName'",hostarr,":")}'`
pPortNum=`awk 'BEGIN{print split("'$svcPort'",portarr,":")}'`
#when have parameter ,but not --all ,we must specify the hosts[--hostname]
if [[ $pHostNum -eq 0 ]] && [[ $all = "false"  ]] && [[ $firstLoc != "" ]] ; then
   echo "Warning !!!! Please specify hosts!"
   exit 1
fi
#Check over Host
for i in $(seq 1 $pHostNum)
do
   HostPara[$i]=`awk 'BEGIN{split("'$hostName'",hostarr,":");print hostarr['$i']}'`
   HostNumAdd=$(($HostNum+1))
   for j in $(seq 1 $HostNumAdd)
   do
      if [[ ${HostPara[$i]} = ${HOST[$j]} ]] ; then
         break
      fi
      if [[ $j -gt $HostNum ]] ; then
         echo "WARNIGN,SequoiaDB don't have host:${HostPara[$i]}"
         HostPara[$i]=""
      fi
   done
done
#Check over Port
for i in $(seq 1 $pPortNum)
do
   PortPara[$i]=`awk 'BEGIN{split("'$svcPort'",portarr,":");print portarr['$i']}'`
   PortNumAdd=$(($PortNum+1))
   for j in $(seq 1 $PortNumAdd)
   do
      if [[ ${PortPara[$i]} = ${PORT[$j]} ]] ; then
         DbPath[$i]=${DBPATH[$j]}
         Role[$i]=${ROLE[$j]}
         break
      fi
      if [[ $j -gt $PortNum ]] ; then
         echo "WARNIGN,SequoiaDB don't have host:${PortPara[$i]}" "j"$j
         PortPara[$i]=""
      fi
   done
done

#***********************************************************************************
#@Function: get password of host that you begin to collect information
#@
#***********************************************************************************
if [ "$all" == "true" ] ; then
   for i in $(seq 1 $HostNum)
   do
      if [ "${HOST[$i]}" != "$localhost" ] ; then
         echo "The host sdbadmin@${HOST[$i]}'s password :"
         read -s PASSWD[$i]
         sdbCheckPassword "${HOST[$i]}" "${PASSWD[$i]}" >> sdbsupport.log 2>&1
         retVal=$?
         while [ "$retVal" == "5" ]
         do
            PASSWD[$i]=""
            echo "Wrong password of host sdbadmin@${HOST[$i]}, please enter again :"
            read -s PASSWD[$i]
            sdbCheckPassword "${HOST[$i]}" "${PASSWD[$i]}" >> sdbsupport.log 2>&1
            retVal=$?
            #echo "until return value"$retVal
         done
         #echo "password:" "${PASSWD[$i]}"
      fi
   done
fi

if [ "$pHostNum" -gt 0 ] && [ "$all" == "false" ] ; then
   for i in $(seq 1 $pHostNum)
   do
      if [ "${HostPara[$i]}" != "" ] && [ "${HostPara[$i]}" != "$localhost" ] ; then
         echo "The host sdbadmin@${HostPara[$i]}'s password :"
         read -s PASSWD[$i]
         sdbCheckPassword "${HostPara[$i]}" "${PASSWD[$i]}" >> sdbsupport.log 2>&1
         retVal=$?
         while [ "$retVal" == "5" ]
         do
            PASSWD[$i]=""
            echo "Wrong password of host sdbadmin@${HostPara[$i]}, please enter again :"
            read -s PASSWD[$i]
            sdbCheckPassword "${HostPara[$i]}" "${PASSWD[$i]}" >> sdbsupport.log 2>&1
            retVal=$?
            #echo "until return value"$retVal
            done
         #echo "password:" "${PASSWD[$i]}"
      fi
   done
fi

#******************************************************************************
#@Function : Create Folder OSINFO/SDBNODES/SDBSNAPS/HARDINFO in local path
#@Fold : OSINFO   Exp : directory for Operation System Information
#@Fold : SDBNODES Exp : directory for sequoiadb all nodes ,such as coord,cata,data
#@Fold : SDBSNAPS Exp : directory for sequoiadb snapshot
#@Fold : HARDINFO Exp : directory for hardware information
#******************************************************************************
   rm -rf HARDINFO/ OSINFO/ SDBNODES/ SDBSNAPS/
   #mkdir HARDINFO >> sdbsupport.log 2>&1
   #mkdir OSINFO >> sdbsupport.log 2>&1
   #mkdir SDBNODES >> sdbsupport.log 2>&1
   #mkdir SDBSNAPS >> sdbsupport.log 2>&1
   #echo "mkdir ok"

#*************************************************************************************************
#@Function : Collect local host information about sequoiadb,such as Dialog,
#				 Conf,Group,Snapshot,Hardware and System information !
#@Var : pHostNum	Exp :	quantity of parameter hosts, such as :--hostname ubunt-dev1:ubunt-dev2: 
#@Var : pPortNum	Exp :	quantity of parameter sevice port, such as:--svcport 51111:61111 
#@Var : HostPara	Exp : Array variable to store hosts parameter
#@Var : PortPara	Exp : Array variable to store local hosts' sevice port
#*************************************************************************************************
#>1.no para here:./sdbsupport.sh      
for i in $(seq 1 $HostNum)
do
   for j in $(seq 1 $PortNum)
   do
      if [ "$firstLoc" == "" ] && [ "$localhost" == "${HOST[$i]}" ] ; then
         #echo "localhost:$localhost"
         sdbPortGather ${HOST[$i]} ${DBPATH[$j]} ${PORT[$j]} $installpath
         sdbSnapShotCataLog ${HOST[$i]} ${PORT[$j]} $installpath
         sdbSnapShot ${HOST[$i]} ${PORT[$j]} $installpath
         sdbHardwareInfoAll ${HOST[$i]} $installpath
         sdbSystemInfoAll ${HOST[$i]} $installpath
      fi
   done
done

#>2.Parameter : --all
if [ "$all" == "true" ] ; then
   for i in $(seq 1 $HostNum)
   do
      if [ "${HOST[$i]}" == "$localhost" ] ; then
         sdbPortGather ${HOST[$i]} ${DBPATH[$j]} ${PORT[$j]} $installpath
         sdbSnapShotCataLog ${HOST[$i]} ${PORT[$j]} $installpath
         sdbSnapShot ${HOST[$i]} ${PORT[$j]} $installpath
         sdbHardwareInfoAll ${HOST[$i]} $installpath
         sdbSystemInfoAll ${HOST[$i]} $installpath
         Local=$localhost
      fi
      #read -u 6
      #{
      if [ "${HOST[$i]}" != "" ] && [ "${HOST[$i]}" != "$localhost" ] ; then
         sdbsupport="./sdbsupport.sh -N ${HOST[$i]}"
         #ssh host and collect information
         sdbExpectSshHosts "${HOST[$i]}" "${PASSWD[$i]}" "$localPath" "$sdbsupport" >> sdbsupport.log
         sdbExpectScpHosts "${HOST[$i]}" "$localPath" "${PASSWD[$i]}" >> sdbsupport.log
         sdbSSHRemove "${HOST[$i]}" "${PASSWD[$i]}" "$localPath" >>sdbsupport.log
         #echo "concurent $i"
         #echo "" >&6
      fi
      #}&
   done
   wait
fi

#>3.have para but not all : ./sdbsupport.sh -h htest1:htes2 -p 11810:50000
for i in $(seq 1 $pHostNum)
do
   if [ $all == "false" ] ; then
      #read -u 6
      #HostPara[$i]=`awk 'BEGIN{split("'$hostName'",hostarr,":");print hostarr['$i']}'` 
      if [[ ${HostPara[$i]} = $localhost ]] ; then
         #only have localhost ./sdbsupport.sh -h htest1
         Local=$localhost
         if [[ $thirdLoc = "" ]] ; then
            for j in $(seq 1 $PortNum)	
            do	
               sdbPortGather "${HostPara[$i]}" "${DBPATH[$j]}" "${PORT[$j]}" "$installpath"
               sdbSnapShotCataLog "${HostPara[$i]}" "${PORT[$j]}" "$installpath"
               sdbSnapShot "${HostPara[$i]}" "${PORT[$j]}" "$installpath"
               sdbHardwareInfoAll "${HostPara[$i]}"
               sdbSystemInfoAll "${HostPara[$i]}"
            done	
         else
            #Para : svcPort ->have this Port [./sdbsupport.sh -h htest1 -p 11810]	
            if [[ $pPortNum -ne 0 ]] ; then 	
               for k in $(seq 1 $pPortNum)
               do
                  sdbPortGather ${HostPara[$i]} ${DbPath[$k]} ${PortPara[$k]} "$installpath"
                  if [[ ${Role[$k]} = "coord" ]] && [[ $catalog = "true" ]] ; then
                     sdbSnapShotCataLog "${HostPara[$i]}" "${PortPara[$k]}" "$installpath"
                  fi
                  #snapShot
                  if [[ $snapShot = "true" ]] ; then
                     sdbSnapShot ${HostPara[$i]} ${PortPara[$k]} "$installpath"
                  fi
                  if [ "$sysInfo" == "false" ] && [ "$rcPort" == "true" ] ; then
                     sdbSnapShotExtract ${HostPara[$i]} ${PortPara[$k]} $group $context $session $collection $collectionspace $database $system "$installpath"
                  fi
               done
            fi
            #Parameter:--sysinfo ; Collect all system information or collect part of system information !
            if [[ $sysInfo = "true" ]] ; then
               sdbSystemInfoAll ${HOST[$i]}
            else
               sdbSystemInfoPartFore ${HOST[$i]} $diskmanage $osystem $kermode $env $IDE $network
               sdbSystemInfoPartEnd ${HOST[$i]} $nfsstat $progress $login $swapon $limit $vmstate
            fi

            #Parameter:--hardinfo ; Collect all hardware information or collect part of system information !
            if [[ $hardInfo = "true" ]] ; then
               sdbHardwareInfoAll ${HOST[$i]}
            else
               sdbHardwareInfoPart ${HOST[$i]} $cpu $memory $disk $netcard $mainboard $bios
            fi
            #echo "" >&6
         fi
      else
         #echo "the host not equal localhost"
         for n in $(seq 1 $ParaNum)
         do
            Para[$n]=`echo $ParaPass|cut -d " " -f $n`
            if [[ ${Para[$n]} = "-N" ]] || [[ ${Para[$n]} = "--hostname" ]] ; then
               Para[$n]=""
            fi
            if [[ ${Para[$n]} = $hostName ]] ; then
               Para[$n]=""
            fi
         done
         if [[ ${HostPara[$i]} != "" ]] ; then
            sdbsupport="./sdbsupport.sh -N ${HostPara[$i]} ${Para[@]}"
            sdbExpectSshHosts "${HostPara[$i]}" "${PASSWD[$i]}" "$localPath" "$sdbsupport" >> sdbsupport.log
            sdbExpectScpHosts "${HostPara[$i]}" "$localPath" "${PASSWD[$i]}" >> sdbsupport.log
            sdbSSHRemove "${HostPara[$i]}" "${PASSWD[$i]}" "$localPath" >>sdbsupport.log
         fi
#echo "" >&6
#}&
      fi
   fi
done
wait

#tar the all collect information in a packet
if [ "$firstLoc" == "" ] || [ "$Local" == "$localhost" ]; then
   sdbTarGzPack $localhost >>sdbsupport.log
fi

#clean environment
exec 6>&-
