#!/bin/bash

. ./sdbsupportfunc1.sh
. ./sdbsupportfunc2.sh
#variable in bash shell 
declare -A PORT  	#port 
declare -A DBPATH	#dbpath
declare -A ROLE		#role
declare -A HOST		#host
confpath=""		#config path 
pHostNum=""			#host number 

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
#get the number of parameter and what parameters is 
ParaNum=$#
ParaPass=$@ 

function Usage()
{
	echo "Command Options:" ;
	echo "    --help			help information" ;
	echo "    -N [--hostname] arg		database host name " ;
	echo "    -p [--svcport] arg		database sevice port" ;
	echo "    -s [--snapshot]		snapshot of sequoiadb database" ; 
	echo "    -o [--osinfo]		operating system information" ;
	echo "    -h [--hardware]		hardware information" ;
	echo "    --all 			copy the all information of database" ; 
	echo "    --cpu			host cpu information" ;
	echo "    --memory			host memory information" ;
	echo "    --disk			host disk information" ;
	echo "    --netcard			host netcard information" ; 
	echo "    --mainboard			host mainboard information" ;
	echo "    --bios			host bios information" ;
	echo "    --catalog			catalog snapshot for database" ;  
	echo "    --group			group of dababase information" ;
	echo "    --context			context snapshot" ;
	echo "    --session			session snapshot" ;
	echo "    --collection		collection snapshot" ;
	echo "    --collectionspace		collectionspace snapshot" ;
	echo "    --database			database snapshot" ;
	echo "    --system			system snapshot" ;
	echo "    --diskmanage		operating system disk management information" ;
	echo "    --basicsys			operating system basic information" ;
	echo "    --kermode			loadable kernel modules" ;
	echo "    --env			operating system environment variable" ;
	echo "    --IDE			integrated development environment" ;
	echo "    --network			network information" ;
	echo "    --nfstat			network file system state" ;
	echo "    --process			operating system process" ;
	echo "    --login			operating system users and history" ;
	echo "    --swapon			operating system swap on" ;
	echo "    --limit			ulimit used to limit the resources occupied shell startup process" ;
	echo "    --vmstate			Show the server status value of a given time interval" ;


}

#the parameters can use  
optArg=`getopt -a -o N:p:sohH -l hostname,svcport,snapshot,osinfo,hardware,help,cpu,memory,disk,netcard,mainboard,bios,group,context,session,collection,collectionspace,database,system,diskmanage,basicsys,kermode,env,IDE,network,nfstat,process,login,swapon,limit,vmstate,catalog,all -- "$@"`
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
		;;
	--context)
		context="true"
		;;
	--session)
		session="true"
		;;
	--collection)
		collection="true"
		;;
	--collectionspace)
		collectionspace="true"
		;;
	--database)
		database="true"
		;;
	--system)
		system="true"
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

#Warning message 
#print warning message
echo ""
echo "***************WARNING***************WARNING***************"
echo "* This program  should be run only at the request of"
echo "* SequoiaDB support.  It can cause significant performance"
echo "* degradation, especially on busy systems with a high number"
echo "* of active connections !"
echo "* This program run mode will collect all configuration and "
echo "* system environment information.Please make sure whether"
echo "* you need !"
echo "***********************************************************"
echo ""
echo "You have 10 seconds to cancel this cript with Ctrl-C"
echo ""
sleep 10 


#******************************************************************************
#@Function : Check over environment  
#******************************************************************************

#inspect the environment of sequiaDB
localhost=`hostname`
localPath=`pwd` 
#inspect the install path 
installpath=`grep "InstallPath" sdbsupport.conf|cut -d "=" -f 2`
#config file path 
#echo $installpath
ls $installpath 1>/dev/null
#if [ $? -ne 1 ] ; then
#	echo "Err,Wrong install path ,Please inspect the sdbsupportconf file!$?"
#	exit 1
#fi  
confpath=$installpath/sequoiadb/conf/local  
ls $confpath 1>/dev/null
if [ $? -ne 1 ] ; then 
	echo "Err,Don't have Nodes !$?"
fi

#************************************************************************
#@Function : get quantity of all hosts and local sevic port
#@Var : HostNum	Exp : the number of hosts in the sequoiaDB
#@Var : PortNum	Exp : the number of local host's sevice port
#@Note : Array begin 1 count ,but such as file row begin 1, so use 1 begin 
#************************************************************************
cd $confpath
dataRole=`find -name "*.conf"|xargs grep "role=data"|cut -d "/" -f 2`
cd $localPath
if [ $dataRole = "" ] ; then 
	echo "Don't have data node in the sequoiaDB" 
	exit 1 
fi 
#catadrr : get the cata address and catch hosts 
cataddr=`grep -E "catalogaddr" $confpath/$dataRole/sdb.conf|cut -d '=' -f 2`
HostNum=`awk 'BEGIN{print split("'$cataddr'",cateArr,",")}'`
PortNum=`ls -l $confpath|grep "^d"|wc -l`
#*******************************************************************************
#@Function : get all hosts in sequoiaDB and local host's port/dbpath/role 
#@Var : HOST		Exp : Array variable used to store hosts in sequoiaDB
#@Var : PORT		Exp : Array variable store local host's sevice port
#@Var : DBPATH		Exp : Array variable store local host's dbpath
#@Var : ROLE		Exp : Array variable store local hsot's sevice port's role 
#*******************************************************************************
for i in $(seq 1 $HostNum)
do
	hostcata[$i]=`awk 'BEGIN{split("'$cataddr'",cateArr,",");print cateArr['$i']'}`
	HOST[$i]=`echo ${hostcata[$i]}|cut -d ":" -f 1 `
	if [ ${HOST[$i]} = $localhost ] ; then 	
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
#get the all hosts's config file,?????????????????????????????? 
#for i in $(seq 1 $HostNum)
#do 
#	mkdir -p $localPath/CONF/${HOST[$i]}  
#	echo $confpath
#
#	/usr/local/bin/expect -c	"
#		spawn scp -r root@${HOST[$i]}:$confpath/* $localPath/CONF/${HOST[$i]} ;  
#		expect {
#			\"*yes/no*\" ;{send \"yes\r\" ;exp_continue}
#			\"assword\" ;{send \"${PASWD[$i]}\r\" ; exp_continue}
#			eof
#			{
#				send_user \"eof\n\" ;
#			}
#		}
#				"
#done

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
if [ $pHostNum -eq 0 ] && [ $all = "false"  ] && [ $firstLoc != "" ] ; then 
	echo "Warnig !!!! Please specify hosts!"
	exit 1
fi 
#Check over Host
for i in $(seq 1 $pHostNum)
do
	HostPara[$i]=`awk 'BEGIN{split("'$hostName'",hostarr,":");print hostarr['$i']}'`
	HostNumAdd=$(($HostNum+1))	
	for j in $(seq 1 $HostNumAdd)
	do
		count=0	
		if [ ${HostPara[$i]} = ${HOST[$j]} ] ; then  
			break  	
		fi 	
		if [ $j -gt $HostNum ] ; then 
			echo "WARNIGN,SequoiaDB don't have host:${HostPara[$i]}" "j"$j	
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
		if [ ${PortPara[$i]} = ${PORT[$j]} ] ; then
			DbPath[$i]=${DBPATH[$j]}	
			Role[$i]=${ROLE[$j]}	
			break
		fi
		if [ $j -gt $PortNum ] ; then
			echo "WARNIGN,SequoiaDB don't have host:${PortPara[$i]}" "j"$j
			PortPara[$i]=""
		fi
	done 
done 
#******************************************************************************
#@Function : Create Folder OSINFO/SDBNODES/SDBSNAPS/HARDINFO in local path
#@Fold : OSINFO   Exp : directory for Operation System Information
#@Fold : SDBNODES Exp : directory for sequoiadb all nodes ,such as coord,cata,data
#@Fold : SDBSNAPS Exp : directory for sequoiadb snapshot
#@Fold : HARDINFO Exp : directory for hardware information
#******************************************************************************
if [ $firstLoc = "" ] ; then
	rm -rf HARDINFO/ OSINFO/ SDBNODES/ SDBSNAPS/
	mkdir HARDINFO >> sdbsupport.log 2>&1
	mkdir OSINFO >> sdbsupport.log 2>&1
	mkdir SDBNODES >> sdbsupport.log 2>&1
	mkdir SDBSNAPS >> sdbsupport.log 2>&1
else
	for i in $(seq 1 $pHostNum)
	do
		if [ ${HostPara[$i]} = $localhost ] ; then
			rm -rf HARDINFO/ OSINFO/ SDBNODES/ SDBSNAPS/	
			mkdir HARDINFO >> sdbsupport.log 2>&1
			mkdir OSINFO >> sdbsupport.log 2>&1
			mkdir SDBNODES >> sdbsupport.log 2>&1
			mkdir SDBSNAPS >> sdbsupport.log 2>&1
		fi 
	done 
fi
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
		if [ $firstLoc = "" ] && [ $localhost = ${HOST[$i]} ] ; then
			sdbPortGather ${HOST[$i]} ${DBPATH[$j]} ${PORT[$j]}
			sdbSnapShotCataLog ${HOST[$i]} ${PORT[$j]}
			sdbSnapShot ${HOST[$i]} ${PORT[$j]}
			sdbHardwareInfoAll ${HOST[$i]}
			sdbSystemInfoAll ${HOST[$i]}
		fi
	done
done 

#>2.Parameter : --all
if [ $firstLoc = "--all" ] ; then
	for i in $(seq 1 $HostNum)
	do
		if [ ${HOST[$i]} != "" ] ; then
			{	
			sdbsupport="./sdbsupport.sh -N ${HOST[$i]}"
			#ssh host and collect information 	
			sdbExpectSshHosts "${HOST[$i]}" "" "$localPath" "$sdbsupport" >> sdbsupport.log
			sdbExpectScpHosts "${HOST[$i]}" "$localPath" "Password" >> sdbsupport.log	
			}&	
		fi
	done
fi

#>3.have para but not all : ./sdbsupport.sh -h htest1:htes2 -p 11810:50000
for i in $(seq 1 $pHostNum)
do
	#HostPara[$i]=`awk 'BEGIN{split("'$hostName'",hostarr,":");print hostarr['$i']}'` 
	if [ ${HostPara[$i]} = $localhost ] ; then 
		#only have localhost ./sdbsupport.sh -h htest1 	
		if [ $thirdLoc = "" ] ; then
			for j in $(seq 1 $PortNum)	
			do	
				sdbPortGather "${HostPara[$i]}" "${DBPATH[$j]}" "${PORT[$j]}"
				sdbSnapShotCataLog "${HostPara[$i]}" "${PORT[$j]}"
				sdbSnapShot "${HostPara[$i]}" "${PORT[$j]}"
				sdbHardwareInfoAll "${HostPara[$i]}"
				sdbSystemInfoAll "${HostPara[$i]}"
			done	
		else
			#Para : svcPort ->have this Port [./sdbsupport.sh -h htest1 -p 11810]	
			if [ $pPortNum -ne 0 ] ; then 	
				for k in $(seq 1 $pPortNum)
				do
					sdbPortGather ${HostPara[$i]} ${DbPath[$k]} ${PortPara[$k]}	
					if [ ${Role[$k]} = "coord" ] && [ $catalog = "true" ] ; then
						sdbSnapShotCataLog "${HostPara[$i]}" "${PortPara[$k]}"
					fi
					#snapShot	
					if [ $snapShot = "true" ] ; then
						sdbSnapShot ${HostPara[$i]} ${PortPara[$k]}
					else
						sdbSnapShotExtract ${HostPara[$i]} ${PortPara[$k]} $group $context $session $collection $collectionspace $database $system
					fi
				done 	
			fi
			#Parameter:--sysinfo ; Collect all system information or collect part of system information !
			if [ $sysInfo = "true" ] ; then
				sdbSystemInfoAll ${HOST[$i]}
			else
				sdbSystemInfoPartFore ${HOST[$i]} $diskmanage $osystem $kermode $env $IDE $network
				sdbSystemInfoPartEnd ${HOST[$i]} $nfsstat $progress $login $swapon $limit $vmstate
			fi
			
			#Parameter:--hardinfo ; Collect all hardware information or collect part of system information !
			if [ $hardInfo = "true" ] ; then
				sdbHardwareInfoAll ${HOST[$i]}
			else
				sdbHardwareInfoPart ${HOST[$i]} $cpu $memory $disk $netcard $mainboard $bios
			fi
		fi 
	else
		echo "the host not equal localhost"
		for n in $(seq 1 $ParaNum)
		do
			Para[$n]=`echo $ParaPass|cut -d " " -f $n`	
			if [ ${Para[$n]} = "-N" ] || [ ${Para[$n]} = "--hostname" ] ; then 
				Para[$n]=""
			fi 
			if [ ${Para[$n]} = $hostName ] ; then
				Para[$n]=""
			fi 
		done 
		if [ ${HostPara[$i]} != "" ] ; then 
			{	
				sdbsupport="./sdbsupport.sh -N ${HostPara[$i]} ${Para[@]}"  
				sdbExpectSshHosts "${HostPara[$i]}" "Password" "$localPath" "$sdbsupport" >> sdbsupport.log 
				sdbExpectScpHosts "${HostPara[$i]}" "$localPath" "Password" >> sdbsupport.log 
			}&	
		fi 	
	fi 
done 

#tar the all collect information in a packet  
if [ $firstLoc = "" ] ; then 
	sdbTarGzPack $localhost >/dev/null
else
	for i in $(seq 1 $pHostNum)
	do
		echo ${HostPara[$i]} $localhost $firstLoc 	
		if [ ${HostPara[$i]} = $localhost ] ; then 	
			sdbTarGzPack $localhost >/dev/null
		fi 
	done
fi 
