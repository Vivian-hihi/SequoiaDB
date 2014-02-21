#!/bin/bash

#sdbdiag.log file to SDBSNAPS 
function sdbPortGather()
{
	HOST=$1 
	DBPATH=$2
	PORT=$3 

	mkdir -p SDBNODES/$HOST/
	#get sequoiadb config path
	installpath=`grep "InstallPath" sdbsupport.conf|cut -d "=" -f 2`
	confpath=$installpath/sequoiadb/conf/local 
	ssh $HOST "ls -R $confpath" >> SDBNODES/$HOST/$HOST.$PORT.info 2>&1
	ssh $HOST "ls $DBPATH/diaglog/" >diagfile
	lognum=`cat diagfile|wc -l` 
	if [[ $lognum -gt 1 ]] ; then 
		ssh $HOST "tar -zcvf $DBPATH/$HOST.$PORT.tar.gz $DBPATH/diaglog/" 
	#	echo "ssh $HOST \"tar -zcvf $HOST.$PORT.tar.gz $DBPATH/diaglog/\"" 
		scp $HOST:$DBPATH/$HOST.$PORT.tar.gz SDBNODES/$HOST/
	else
		#copy diaglog for every node
	        scp $HOST:$DBPATH/diaglog/sdbdiag.log SDBNODES/$HOST/$HOST.$PORT.diaglog
	fi  
	scp $HOST:$confpath/$PORT/sdb.conf SDBNODES/$HOST/$HOST.$PORT.conf 
}

#collect catalog information snapshot 
function sdbSnapShotCataLog()
{
	SDB=""
        HOST=$1
	PORT=$2

	mkdir -p SDBSNAPS/catalog/
	installpath=`grep "InstallPath" sdbsupport.conf|cut -d "=" -f 2`
        SDB=$installpath/sequoiadb/bin/sdb
       
	 $SDB "var db=new Sdb('localhost',$PORT)"
        if [[ $? -ne 0 ]] ; then
                echo "the sequoiadb is not run!"
		exit 1 
       	fi 
	$SDB "db.snapshot(SDB_SNAP_CATALOG)" >> SDBSNAPS/catalog/snapshot_catalog.$HOST.$PORT 2>&1		
}

#snapshot of sequoiadb to SDBSNAPS
function sdbSnapShot() 
{
        SDB=""
        HOST=$1
        PORT=$2

	mkdir -p SDBSNAPS/$HOST.$PORT/
        #get sequoiadb config path
        installpath=`grep "InstallPath" sdbsupport.conf|cut -d "=" -f 2`
        SDB=$installpath/sequoiadb/bin/sdb
        $SDB "var db=new Sdb('localhost',$PORT)"
        if [[ $? -ne 0 ]] ; then
                echo "Network error,Please Cheak it!"
       	else  
		$SDB "db.listReplicaGroups()" >> SDBSNAPS/$HOST.$PORT/listShards.$HOST.$PORT 2>&1
		$SDB "db.snapshot(SDB_SNAP_CONTEXTS)" >> SDBSNAPS/$HOST.$PORT/snapshot_contests.$HOST.$PORT 2>&1
		$SDB "db.snapshot(SDB_SNAP_SESSIONS)" >> SDBSNAPS/$HOST.$PORT/snapshot_sessions.$HOST.$PORT 2>&1
		$SDB "db.snapshot(SDB_SNAP_COLLECTIONS)" >> SDBSNAPS/$HOST.$PORT/snapshot_collections.$HOST.$PORT 2>&1
		$SDB "db.snapshot(SDB_SNAP_COLLECTIONSPACES)" >> SDBSNAPS/$HOST.$PORT/snapshot_collectionspace.$HOST.$PORT 2>&1
		$SDB "db.snapshot(SDB_SNAP_DATABASE)" >> SDBSNAPS/$HOST.$PORT/snapshot_database.$HOST.$PORT 2>&1
		$SDB "db.snapshot(SDB_SNAP_SYSTEM)" >> SDBSNAPS/$HOST.$PORT/snapshot_system.$HOST.$PORT 2>&1
	fi
}

#sdbSnapShot Single Extract 
function sdbSnapShotExtract()
{
	SDB=""
        HOST=$1
        PORT=$2

	group=$3
	context=$4
	session=$5
	collection=$6
	collectionspace=$7
	database=$8
	system=$9

        mkdir -p SDBSNAPS/$HOST.$PORT/
        #get sequoiadb config path
        installpath=`grep "InstallPath" sdbsupport.conf|cut -d "=" -f 2`
        SDB=$installpath/sequoiadb/bin/sdb
        $SDB "var db=new Sdb('localhost',$PORT)"
        if [[ $? -ne 0 ]] ; then
                echo "Network error,Please Cheak it!"
        else
		if [[ $group = "true" ]] ; then 
	        	$SDB "db.listReplicaGroups()" >> SDBSNAPS/$HOST.$PORT/listShards.$HOST.$PORT 2>&1
               	fi
		if [[ $context = "true" ]] ; then  
			$SDB "db.snapshot(SDB_SNAP_CONTEXTS)" >> SDBSNAPS/$HOST.$PORT/snapshot_contests.$HOST.$PORT 2>&1
               	fi
		if [[ $session = "true" ]] ; then  
			$SDB "db.snapshot(SDB_SNAP_SESSIONS)" >> SDBSNAPS/$HOST.$PORT/snapshot_sessions.$HOST.$PORT 2>&1
                fi
		if [[ $collection = "true" ]] ; then 	
			$SDB "db.snapshot(SDB_SNAP_COLLECTIONS)" >> SDBSNAPS/$HOST.$PORT/snapshot_collections.$HOST.$PORT 2>&1
               	fi
		if [[ $collectionspace = "true" ]] ; then  
			$SDB "db.snapshot(SDB_SNAP_COLLECTIONSPACES)" >> SDBSNAPS/$HOST.$PORT/snapshot_collectionspace.$HOST.$PORT 2>&1
               	fi 
		if [[ $database = "true" ]] ; then 	
			$SDB "db.snapshot(SDB_SNAP_DATABASE)" >> SDBSNAPS/$HOST.$PORT/snapshot_database.$HOST.$PORT 2>&1
                fi	
		if [[ $system = "true" ]] ; then 	
			$SDB "db.snapshot(SDB_SNAP_SYSTEM)" >> SDBSNAPS/$HOST.$PORT/snapshot_system.$HOST.$PORT 2>&1
		fi
        fi	
}

#collect all hardware infomation
function sdbHardwareInfoAll()
{
	HOST=$1

	mkdir -p HARDINFO/$HOST/
	ssh $HOST "lscpu" >> HARDINFO/$HOST/$HOST.cpu.info 2>&1
	ssh $HOST "cat /proc/cpuinfo" >> HARDINFO/$HOST/$HOST.cpu.info 2>&1

	ssh $HOST "free -m" >> HARDINFO/$HOST/$HOST.memory1.info 2>&1
	ssh $HOST "cat /proc/meminfo" >> HARDINFO/$HOST/$HOST.memory2.info 2>&1
	ssh $HOST "dmidecode -t memory" >> HARDINFO/$HOST/$HOST.memory3.info 2>&1

	ssh $HOST "lsblk" >> HARDINFO/$HOST/$HOST.disk.info 2>&1
	ssh $HOST "fdisk -l" >> HARDINFO/$HOST/$HOST.disk.info 2>&1
	ssh $HOST "df -h" >> HARDINFO/$HOST/$HOST.disk.info 2>&1

	ssh $HOST "lspci|grep -i 'eth'" >> HARDINFO/$HOST/$HOST.netcard.info 2>&1
	ssh $HOST "ifconfig" >> HARDINFO/$HOST/$HOST.netcard.info 2>&1

	ssh $HOST "lspci" >> HARDINFO/$HOST/$HOST.mainboard1.info 2>&1
	ssh $HOST "lspci -vv" >> HARDINFO/$HOST/$HOST.mainboard2.info 2>&1

	ssh $HOST "dmidecode -t bios" >> HARDINFO/$HOST/$HOST.bios1.info 2>&1
	ssh $HOST "dmidecode -q" >> HARDINFO/$HOST/$HOST.bios2.info 2>&1

}

#collect part of hardware information  
function sdbHardwareInfoPart()
{
	HOST=$1
	cpu=$2
	memory=$3
	disk=$4
	netcard=$5	
	mainboard=$6
	bios=$7

	mkdir -p HARDINFO/$HOST/
	if [[ $cpu = "true" ]] ; then 
		ssh $HOST "lscpu" >> HARDINFO/$HOST/$HOST.cpu.info 2>&1 
		ssh $HOST "cat /proc/cpuinfo" >> HARDINFO/$HOST/$HOST.cpu.info 2>&1
	fi

	if [[ $memory = "true" ]] ; then 
		ssh $HOST "free -m" >> HARDINFO/$HOST/$HOST.memory1.info 2>&1
		ssh $HOST "cat /proc/meminfo" >> HARDINFO/$HOST/$HOST.memory2.info 2>&1
		ssh $HOST "dmidecode -t memory" >> HARDINFO/$HOST/$HOST.memory3.info 2>&1
	fi

	if [[ $disk = "true" ]] ; then 
		ssh $HOST "lsblk" >> HARDINFO/$HOST/$HOST.disk.info 2>&1	
		ssh $HOST "fdisk -l" >> HARDINFO/$HOST/$HOST.disk.info 2>&1
		ssh $HOST "df -h" >> HARDINFO/$HOST/$HOST.disk.info 2>&1
	fi

	if [[ $netcard = "true" ]] ; then 
		ssh $HOST "lspci|grep -i 'eth'" >> HARDINFO/$HOST/$HOST.netcard.info 2>&1
		ssh $HOST "ifconfig" >> HARDINFO/$HOST/$HOST.netcard.info 2>&1
	fi
	
	if [[ $mainboard = "true" ]] ; then 
		ssh $HOST "lspci" >> HARDINFO/$HOST/$HOST.mainboard1.info 2>&1
		ssh $HOST "lspci -vv" >> HARDINFO/$HOST/$HOST.mainboard2.info 2>&1
	fi

	if [[ $bios = "true" ]] ; then 
		ssh $HOST "dmidecode -t bios" >> HARDINFO/$HOST/$HOST.bios1.info 2>&1
		ssh $HOST "dmidecode -q" >> HARDINFO/$HOST/$HOST.bios2.info 2>&1
	fi 
}

#collect operating system information all 
function sdbSystemInfoAll()
{
	HOST=$1 
	mkdir -p OSINFO/$HOST/ 
	ssh $HOST "uptime" >> OSINFO/$HOST/$HOST.diskmanage.sys 2>&1
	echo ">>>>>disk manage information" >> OSINFO/$HOST/$HOST.diskmanage.sys 2>&1
	ssh $HOST "df ./" >> OSINFO/$HOST/$HOST.diskmanage.sys 2>&1
	echo ">>>>>mount information" >> OSINFO/$HOST/$HOST.diskmanage.sys 2>&1
	ssh $HOST "mount" >> OSINFO/$HOST/$HOST.diskmanage.sys 2>&1

	ssh $HOST "uptime" >> OSINFO/$HOST/$HOST.system.sys 2>&1
	echo ">>>>>operating system version" >> OSINFO/$HOST/$HOST.system.sys 2>&1 
	ssh $HOST "head -n 1 /etc/issue" >> OSINFO/$HOST/$HOST.system.sys 2>&1
	ssh $HOST "cat /proc/version" >> OSINFO/$HOST/$HOST.system.sys 2>&1
	echo ">>>>>host name" >> OSINFO/$HOST/$HOST.system.sys 2>&1
	ssh $HOST "hostname" >> OSINFO/$HOST/$HOST.system.sys 2>&1
	echo ">>>>>long bit of system" >> OSINFO/$HOST/$HOST.system.sys 2>&1
	ssh $HOST "getconf LONG_BIT" >> OSINFO/$HOST/$HOST.system.sys 2>&1	
	echo ">>>>>lsb_release -a" >> OSINFO/$HOST/$HOST.system.sys 2>&1
	ssh $HOST "ulimit -a" >> OSINFO/$HOST/$HOST.system.sys 2>&1
	ssh $HOST "lsb_release -a" >> OSINFO/$HOST/$HOST.system.sys 2>&1

	ssh $HOST "uptime" >> OSINFO/$HOST/$HOST.mode.sys 2>&1
	ssh $HOST "lsmod" >> OSINFO/$HOST/$HOST.mode.sys 2>&1

	ssh $HOST "uptime" >> OSINFO/$HOST/$HOST.environmentvar.sys 2>&1 
	ssh $HOST "env" >> OSINFO/$HOST/$HOST.environmentvar.sys 2>&1

	ssh $HOST "uptime" >> OSINFO/$HOST/$HOST.IDE.sys 2>&1
	ssh $HOST "dmesg|grep IDE" >> OSINFO/$HOST/$HOST.IDE.sys 2>&1

	ssh $HOST "uptime" >> OSINFO/$HOST/$HOST.networkinfo.sys 2>&1
	echo ">>>>>network state information" >> OSINFO/$HOST/$HOST.networkinfo.sys 2>&1
	ssh $HOST "netstat -s" >> OSINFO/$HOST/$HOST.networkinfo.sys 2>&1
	echo ">>>>>ifconfig information" >> OSINFO/$HOST/$HOST.networkinfo.sys 2>&1
	ssh $HOST "ifconfig" >> OSINFO/$HOST/$HOST.networkinfo.sys 2>&1
	echo ">>>>>detail netstats information" >> OSINFO/$HOST/$HOST.networkinfo.sys 2>&1
	ssh $HOST "netstat" >> OSINFO/$HOST/$HOST.networkinfo.sys 2>&1
	
	ssh $HOST "uptime" >> OSINFO/$HOST/$HOST.nfsstate.sys 2>&1 
	ssh $HOST "nfsstat" >> OSINFO/$HOST/$HOST.nfsstate.sys 2>&1

	ssh $HOST "uptime" >> OSINFO/$HOST/$HOST.progress.sys 2>&1 
	ssh $HOST "ps -elf|sort -rn" >> OSINFO/$HOST/$HOST.progress.sys 2>&1
	ssh $HOST "ps -aux" >> OSINFO/$HOST/$HOST.progress1.sys 2>&1

	ssh $HOST "uptime" >> OSINFO/$HOST/$HOST.logininfo.sys 2>&1 
	ssh $HOST "last" >> OSINFO/$HOST/$HOST.logininfo.sys 2>&1
	ssh $HOST "history" >> OSINFO/$HOST/$HOST.logininfo.sys 2>&1

	ssh $HOST "uptime" >> OSINFO/$HOST/$HOST.swapon.sys 2>&1
	ssh $HOST "swapon -s" >> OSINFO/$HOST/$HOST.swapon.sys 2>&1

	ssh $HOST "uptime" >> OSINFO/$HOST/$HOST.ulimit.sys 2>&1
	ssh $HOST "ulimit -a" >> OSINFO/$HOST/$HOST.ulimit.sys 2>&1
	
	ssh $HOST "uptime" >> OSINFO/$HOST/$HOST.vmstate.sys 2>&1 
	ssh $HOST "vmstat" >> OSINFO/$HOST/$HOST.vmstate.sys 2>&1

}

#collect part of operating system information ,front half  
function sdbSystemInfoPartFore()
{
	HOST=$1
	
	diskmanage=$2
	osystem=$3
	kermode=$4	
	env=$5
	IDE=$6
	network=$7

	mkdir -p OSINFO/$HOST/ 

	if [[ $diskmanage = "true" ]] ; then 
		ssh $HOST "uptime" >> OSINFO/$HOST/$HOST.diskmanage.sys 2>&1
		echo ">>>>>disk manage information" >> OSINFO/$HOST/$HOST.diskmanage.sys 2>&1
		ssh $HOST "df ./" >> OSINFO/$HOST/$HOST.diskmanage.sys 2>&1
		echo ">>>>>mount information" >> OSINFO/$HOST/$HOST.diskmanage.sys 2>&1
		ssh $HOST "mount" >> OSINFO/$HOST/$HOST.diskmanage.sys 2>&1
	fi 

	if [[ $osystem = "true" ]] ; then 
		ssh $HOST "uptime" >> OSINFO/$HOST/$HOST.system.sys 2>&1
		echo ">>>>>operating system version" >> OSINFO/$HOST/$HOST.system.sys 2>&1 
		ssh $HOST "head -n 1 /etc/issue" >> OSINFO/$HOST/$HOST.system.sys 2>&1
		ssh $HOST "cat /proc/version" >> OSINFO/$HOST/$HOST.system.sys 2>&1
		echo ">>>>>host name" >> OSINFO/$HOST/$HOST.system.sys 2>&1
		ssh $HOST "hostname" >> OSINFO/$HOST/$HOST.system.sys 2>&1
		echo ">>>>>long bit of system" >> OSINFO/$HOST/$HOST.system.sys 2>&1
		ssh $HOST "getconf LONG_BIT" >> OSINFO/$HOST/$HOST.system.sys 2>&1	
		echo ">>>>>lsb_release -a" >> OSINFO/$HOST/$HOST.system.sys 2>&1
		ssh $HOST "ulimit -a" >> OSINFO/$HOST/$HOST.system.sys 2>&1
		ssh $HOST "lsb_release -a" >> OSINFO/$HOST/$HOST.system.sys 2>&1
	fi

	if [[ $kermode = "true" ]] ; then 
		ssh $HOST "uptime" >> OSINFO/$HOST/$HOST.mode.sys 2>&1
		ssh $HOST "lsmod" >> OSINFO/$HOST/$HOST.mode.sys 2>&1
	fi

	if [[ $env = "true" ]] ; then 	
		ssh $HOST "uptime" >> OSINFO/$HOST/$HOST.environmentvar.sys 2>&1 
		ssh $HOST "env" >> OSINFO/$HOST/$HOST.environmentvar.sys 2>&1
	fi

	if [[ $IDE = "true" ]] ; then 
		ssh $HOST "uptime" >> OSINFO/$HOST/$HOST.IDE.sys 2>&1
		ssh $HOST "dmesg|grep IDE" >> OSINFO/$HOST/$HOST.IDE.sys 2>&1
	fi

	if [[ $network = "true" ]] ; then  
		ssh $HOST "uptime" >> OSINFO/$HOST/$HOST.networkinfo.sys 2>&1
		echo ">>>>>network state information" >> OSINFO/$HOST/$HOST.networkinfo.sys 2>&1
		ssh $HOST "netstat -s" >> OSINFO/$HOST/$HOST.networkinfo.sys 2>&1
		echo ">>>>>ifconfig information" >> OSINFO/$HOST/$HOST.networkinfo.sys 2>&1
		ssh $HOST "ifconfig" >> OSINFO/$HOST/$HOST.networkinfo.sys 2>&1
		echo ">>>>>detail netstats information" >> OSINFO/$HOST/$HOST.networkinfo.sys 2>&1
		ssh $HOST "netstat" >> OSINFO/$HOST/$HOST.networkinfo.sys 2>&1
	fi 

	if [[ $nfsstat = "true" ]] ; then 	
		ssh $HOST "uptime" >> OSINFO/$HOST/$HOST.nfsstate.sys 2>&1 
		ssh $HOST "nfsstat" >> OSINFO/$HOST/$HOST.nfsstate.sys 2>&1
	fi

}

#collect end half part of Operating System information 
function sdbSystemInfoPartEnd()
{
	HOST=$1
	
	nfsstat=$2
	progress=$3
	login=$4
	swapon=$5
	limit=$6
	vmstate=$7

	mkdir -p OSINFO/$HOST/

	if [[ $progress = "true" ]] ; then 
		ssh $HOST "uptime" >> OSINFO/$HOST/$HOST.progress.sys 2>&1 
		ssh $HOST "ps -elf|sort -rn" >> OSINFO/$HOST/$HOST.progress.sys 2>&1
		ssh $HOST "ps -aux" >> OSINFO/$HOST/$HOST.progress1.sys 2>&1
	fi 

	if [[ $login = "true" ]] ; then 
		ssh $HOST "uptime" >> OSINFO/$HOST/$HOST.logininfo.sys 2>&1 
		ssh $HOST "last" >> OSINFO/$HOST/$HOST.logininfo.sys 2>&1
		ssh $HOST "history" >> OSINFO/$HOST/$HOST.logininfo.sys 2>&1
	fi 

	if [[ $swapon = "true" ]] ; then 
		ssh $HOST "uptime" >> OSINFO/$HOST/$HOST.swapon.sys 2>&1
		ssh $HOST "swapon -s" >> OSINFO/$HOST/$HOST.swapon.sys 2>&1
	fi

	if [[ $limit = "true" ]] ; then 
		ssh $HOST "uptime" >> OSINFO/$HOST/$HOST.ulimit.sys 2>&1
		ssh $HOST "ulimit -a" >> OSINFO/$HOST/$HOST.ulimit.sys 2>&1
	fi

	if [[ $vmstate = "true" ]] ; then 	
		ssh $HOST "uptime" >> OSINFO/$HOST/$HOST.vmstate.sys 2>&1 
		ssh $HOST "vmstat" >> OSINFO/$HOST/$HOST.vmstate.sys 2>&1
	fi
}


