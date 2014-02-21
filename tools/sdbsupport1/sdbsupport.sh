#!/bin/bash
. ./sdbsupportfunc1.sh
. ./sdbsupportmain.sh
#variable in bash shell 
declare -A PORT  	#port 
declare -A DBPATH	#dbpath
declare -A ROLE		#role
declare -A HOST		#host
confpath=""		#config path 
nodenum=""		#row of localnode file,the number of nodes  
hostnum=""			#host number 

#gloable variable
svcPort=""
hostName=""
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
 
#make sure whether have para
tmpPara=$1
thirdPara=$3
localhost=`hostname`
echo $localhost 
function Usage()
{
	echo "Command Options:" ;
	echo "    --help			help information" ;
	echo "    -v [--svcname] arg		database sevice port" ;
	echo "	  -h [--hostname] arg		database host name " ; 
	echo "    -n [--snapshot]		snapshot of sequoiadb database" ; 
	echo "    -o [--osinfo]		operating system information" ;
	echo "    -w [--hardware]		hardware information" ;
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

OPTARG=($(getopt -o v:h:n:o:w:H -l svcname,hostname,snapshot,osinfo,hardware,help,cpu,memory,disk,netcard,mainboard,bios,group,context,session,collection,collectionspace,database,system,diskmanage,basicsys,kermode,env,IDE,network,nfstat,process,login,swapon,limit,vmstate,catalog,all -- "$@")) 

for ((i=0;i<${#OPTARG[@]};i++))
do
	eval opt=${OPTARG[$i]}
	case $opt in
	-v|--svcname)
		svcPort=$2	
		shift
		;;
	-h|--hostname)
		hostName=$2
		shift
		;; 
	-n|--snapshot)
		snapShot="true" 
		shift
		;;
	-o|--osinfo)
		sysInfo="true"
		shift
		;;
	-w|--hardware)
		hardInfo="true"
		shift
		;;
	--cpu)
		cpu="true"
		echo "cpu here"$cpu
		shift 	
		;;
	--memory)
		memory="true" 
		shift 	
		;;
	--disk)
		disk="true" 
		shift 	
		;;
	--netcard)
		netcard="true" 
		shift
		;; 
	--mainboard)
		mainboard="true" 
		shift
		;;
	--bios)
		bios="true" 
		shift
		;;  
	--group)
		group="true"
		shift
		;;
	--context)
		context="true" 
		shift
		;;
	--session)
		session="true"
		shift
		;;
	--collection)
		collection="true"  
		shift
		;;
	--collectionspace)
		collectionspace="true" 
		shift
		;;
	--database)
		database="true" 
		echo "$database"
		shift
		;;
	--system)
		system="true"
		echo $system
		shift
		;;	
	--diskmanage)
		diskmanage="true" 
		shift
		;; 
	--basicsys)
		osystem="true" 
		shift
		;;
	--kermode)
		kermode="true" 
		shift
		;; 
	--env)
		env="true" 
		shift
		;; 
	--IDE)
		IDE="true"
		shift
		;;
	--network)
		network="true"
		shift
		;;
	--nfstat)
		nfsstat="true"
		shift
		;;
	--process)
		progress="true"
		shift
		;;
	--login)
		login="true"
		shift
		;;
	--swapon)
		swapon="true"
		shift
		;;
	--limit)
		limit="true"
		shift
		;;
	--vmstate)
		vmstate="true"
		shift
		;; 	
	--catalog)
		catalog="true"
		shift
		;;
	--all)
		all="true" 
		shift
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
done

#print warning message
echo ""
echo "***************WARNING***************WARNING***************"
echo "This program  should be run only at the request of"
echo "SequoiaDB support.  It can cause significant performance"
echo "degradation, especially on busy systems with a high number"
echo "of active connections !"

# print warning message
echo "This program run mode will collect all configuration and " 
echo "system environment information.Please make sure whether" 
echo "you need !" 
echo "***********************************************************"
echo ""
echo "You have 10 seconds to cancel this cript with Ctrl-C"
echo ""
sleep 10

rm -Rf OSINFO SDBNODES SDBSNAPS HARDINFO  

#create local directory
#directory for hardware information
mkdir HARDINFO >> sdbsupport.log 2>&1 
#directory for Operation System Information
mkdir OSINFO >> sdbsupport.log 2>&1  
#directory for sequoiadb all nodes ,such as coord,cata,data
mkdir SDBNODES >> sdbsupport.log 2>&1
#directory for sequoiadb snapshot
mkdir SDBSNAPS >> sdbsupport.log 2>&1

#get sequoiadb config path 
installpath=`grep "InstallPath" sdbsupport.conf|cut -d "=" -f 2` 
confpath=$installpath/sequoiadb/conf/local 
ls $confpath > localnode

#get port dbpath role host 
nodenum=`cat localnode|wc -l`
for i in $(seq 1 $nodenum)
do
	PORT[$i]=`sed -n ''$i'p' localnode`
	DBPATH[$i]=`grep -E "dbpath" $confpath/${PORT[$i]}/sdb.conf|cut -d '=' -f 2`
	ROLE[$i]=`grep -E "role=" $confpath/${PORT[$i]}/sdb.conf|cut -d '=' -f 2`
	if [[ "data" = ${ROLE[$i]} ]] ; then
		cataddr=`grep -E "catalogaddr" $confpath/${PORT[$i]}/sdb.conf|cut -d '=' -f 2`
	fi
	hostnum=`awk 'BEGIN{print split("'$cataddr'",cateArr,",")}'`
	for j in $(seq 1 $hostnum)
	do
		hostcata[$j]=`awk 'BEGIN{split("'$cataddr'",cateArr,",");print cateArr['$j']'}`
		HOST[$j]=`echo ${hostcata[$j]}|cut -d ":" -f 1 `
		#Parameter : null
		if [[ $tmpPara = "" ]] && [[ $localhost = ${HOST[$j]} ]] ; then 
			sdbPortMain ${HOST[$j]} "00000"
			sdbHardwareInfoAll "${HOST[$j]}"
			sdbSystemInfoAll "${HOST[$j]}"  
		fi 
		#Parameter : --all 	
		if [[ $all = "true" ]] ; then 
			sdbPortMain "${HOST[$j]}" "00000" 
			sdbHardwareInfoAll "${HOST[$j]}"
			sdbSystemInfoAll "${HOST[$j]}"	
		fi
		hostnum=`awk 'BEGIN{print split("'$hostName'",hostArr,":")}'`
		for k in $(seq 1 $hostnum)
		do
			#Parameter : --host only 
			if [[ $all = "false" ]] && [[ $hostName = ${HOST[$j]} ]] && [[ $thirdPara = "" ]] ; then 
				sdbPortMain "${HOST[$j]}" "00000" 
				sdbHardwareInfoAll "${HOST[$j]}"
				sdbSystemInfoAll "${HOST[$j]}"
			fi
			#Parameter : --host and other
			if [[ $all = "false" ]] && [[ $hostName = ${HOST[$j]} ]] && [[ $thirdPara != "" ]] && [[ $svcPort != "" ]] ; then 
				sdbPortMain "${HOST[$j]}" "$svcPort" "$snapShot" "$group" "$context" "$session" "$collection" "$collectionspace" "$database" "$system" "$catalog"
				sdbHardSysInfo "${HOST[$j]}" "$hardInfo" "$sysInfo"
				sdbHardwareInfoPart "${HOST[$j]}" "$cpu" "$memory" "$disk" "$netcard" "$mainboard" "$bios" 
				sdbSystemInfoPartFore "${HOST[$j]}" "$diskmanage" "$osystem" "$kermode" "$env" "$IDE" "$network"
				sdbSystemInfoPartEnd "${HOST[$j]}" "$nfsstat" "$progress" "$login" "$swapon" "$limit" "$vmstate"	
			fi 
			
			if [[ $all = "false" ]] && [[ $hostName = ${HOST[$j]} ]] && [[ $thirdPara != "" ]] && [[ $svcPort = "" ]] ; then 
				sdbPortMain "${HOST[$j]}" "" "$snapShot" "$group" "$context" "$session" "$collection" "$collectionspace" "$database" "$system" "$catalog"
				sdbHardSysInfo "${HOST[$j]}" "$hardInfo" "$sysInfo"
				sdbHardwareInfoPart "${HOST[$j]}" "$cpu" "$memory" "$disk" "$netcard" "$mainboard" "$bios"
				sdbSystemInfoPartFore "${HOST[$j]}" "$diskmanage" "$osystem" "$kermode" "$env" "$IDE" "$network"
				sdbSystemInfoPartEnd "${HOST[$j]}" "$nfsstat" "$progress" "$login" "$swapon" "$limit" "$vmstate"

			fi
		done
		#Parameter : --port 	
		if [[ $tmpPara != "" ]] && [[ $all = "false" ]] && [[ $hostName = "" ]] && [[ $svcPort != "" ]] ; then 
			sdbPortMain "${HOST[$j]}" "$svcPort" "$snapShot" "$group" "$context" "$session" "$collection" "$collectionspace" "$database" "$system" "$catalog" "${HOST[$j]}" "$hardInfo" "$sysInfo" "${HOST[$j]}" "$cpu" "$memory" "$disk" "$netcard" "$mainboard" "$bios" "${HOST[$j]}" "$diskmanage" "$osystem" "$kermode" "$env" "$IDE" "$network" "${HOST[$j]}" "$nfsstat" "$progress" "$login" "$swapon" "$limit" "$vmstate"
		fi 	
		
		if [[ $tmpPara != "" ]] && [[ $all = "false" ]] && [[ $hostName = "" ]] && [[ $svcPort = "" ]] ; then
			sdbPortMain "${HOST[$j]}" "$svcPort" "$snapShot" "$group" "$context" "$session" "$collection" "$collectionspace" "$database" "$system" "$catalog" "$cpu" "$memory" "$disk" "$netcard" "$mainboard" "$bios" "$diskmanage" "$osystem" "$kermode" "$env" "$IDE" "$network" "$nfsstat" "$progress" "$login" "$swapon" "$limit" "$vmstate" "$hardInfo" "$sysInfo" 
		fi 
 
		#Parameter: --sysinfo --hardinfo [no all ,no hostName ,no svcPort] 
		if [[ $tmpPara != "" ]] && [[ $all = "false" ]] && [[ $hostName = "" ]] && [[ $svcPort = "" ]] && [[ $snapShot = "" ]] ; then 
			echo "gooooooooooooooo" 
			sdbHardSysInfo "${HOST[$j]}" "$hardInfo" "$sysInfo"		
			sdbHardwareInfoPart "${HOST[$j]}" "$cpu" "$memory" "$disk" "$netcard" "$mainboard" "$bios"  
			sdbSystemInfoPartFore "${HOST[$j]}" "$diskmanage" "$osystem" "$kermode" "$env" "$IDE" "$network" 
			sdbSystemInfoPartEnd "${HOST[$j]}" "$nfsstat" "$progress" "$login" "$swapon" "$limit" "$vmstate"  
		fi
	done

done

tar -zcvf sdbsupport.tar.gz HARDINFO/ OSINFO/ SDBNODES/ SDBSNAPS/ 
#end rm the directory #
rm -Rf HARDINFO/ OSINFO/ SDBNODES/ SDBSNAPS/ localnode diagfile
 
