#!/bin/bash

#sdbdiag.log file to SDBSNAPS 
function sdbPortGather()
{
	HOST=$1 
	DBPATH=$2
	PORT=$3 

	mkdir -p SDBNODES/
	#get sequoiadb config path
	installpath=`grep InstallPath sdbsupport.conf|cut -d = -f 2`
	confpath=$installpath/sequoiadb/conf/local 
	ls -R $confpath >> SDBNODES/$HOST.$PORT.info 2>&1
	ls $DBPATH/diaglog/ > diagfile
	lognum=`cat diagfile|wc -l` 
	if [[ $lognum -gt 1 ]] ; then 
		tar -zcvf $DBPATH/$HOST.$PORT.tar.gz $DBPATH/diaglog/ 
	#	echo ssh $HOST \tar -zcvf $HOST.$PORT.tar.gz $DBPATH/diaglog/\ 
		#copy diaglog for every node
	else 
		cp $DBPATH/diaglog/sdbdiag.log SDBNODES/$HOST.$PORT.diaglog
	fi 
	rm -rf diagfile
}

#collect catalog information snapshot 
function sdbSnapShotCataLog()
{
	SDB=
   HOST=$1
	PORT=$2

	mkdir -p SDBSNAPS/catalog/
	installpath=`grep InstallPath sdbsupport.conf|cut -d = -f 2`
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
	SDB=
	HOST=$1
	PORT=$2
	
	mkdir -p SDBSNAPS/
	#get sequoiadb config path
	installpath=`grep InstallPath sdbsupport.conf|cut -d = -f 2`
	SDB=$installpath/sequoiadb/bin/sdb
	$SDB "var db=new Sdb('localhost',$PORT)"
	if [[ $? -ne 0 ]] ; then
		echo "Network error,Please Cheak it!"
   else  
		$SDB "db.listReplicaGroups()" >> SDBSNAPS/listShards.$HOST.$PORT 2>&1
		$SDB "db.snapshot(SDB_SNAP_CONTEXTS)" >> SDBSNAPS/snapshot_contests.$HOST.$PORT 2>&1
		$SDB "db.snapshot(SDB_SNAP_SESSIONS)" >> SDBSNAPS/snapshot_sessions.$HOST.$PORT 2>&1
		$SDB "db.snapshot(SDB_SNAP_COLLECTIONS)" >> SDBSNAPS/snapshot_collections.$HOST.$PORT 2>&1
		$SDB "db.snapshot(SDB_SNAP_COLLECTIONSPACES)" >> SDBSNAPS/snapshot_collectionspace.$HOST.$PORT 2>&1
		$SDB "db.snapshot(SDB_SNAP_DATABASE)" >> SDBSNAPS/snapshot_database.$HOST.$PORT 2>&1
		$SDB "db.snapshot(SDB_SNAP_SYSTEM)" >> SDBSNAPS/snapshot_system.$HOST.$PORT 2>&1
	fi
}

#sdbSnapShot Single Extract 
function sdbSnapShotExtract()
{
	SDB=
   HOST=$1
   PORT=$2

	group=$3
	context=$4
	session=$5
	collection=$6
	collectionspace=$7
	database=$8
	system=$9
        
	mkdir -p SDBSNAPS/
	#get sequoiadb config path
	installpath=`grep InstallPath sdbsupport.conf|cut -d = -f 2`
	SDB=$installpath/sequoiadb/bin/sdb
	$SDB "var db=new Sdb('localhost',$PORT)"
	if [[ $? -ne 0 ]] ; then
		echo "Network error,Please Cheak it!"
   else
		if [[ $group = true ]] ; then 
			$SDB "db.listReplicaGroups()" >> SDBSNAPS/listShards.$HOST.$PORT 2>&1
      fi
		if [[ $context = true ]] ; then  
			$SDB "db.snapshot(SDB_SNAP_CONTEXTS)" >> SDBSNAPS/snapshot_contests.$HOST.$PORT 2>&1
		fi
		if [[ $session = true ]] ; then  
			$SDB "db.snapshot(SDB_SNAP_SESSIONS)" >> SDBSNAPS/snapshot_sessions.$HOST.$PORT 2>&1
		fi
		if [[ $collection = true ]] ; then 	
			$SDB "db.snapshot(SDB_SNAP_COLLECTIONS)" >> SDBSNAPS/snapshot_collections.$HOST.$PORT 2>&1
      fi
		if [[ $collectionspace = true ]] ; then  
			$SDB "db.snapshot(SDB_SNAP_COLLECTIONSPACES)" >> SDBSNAPS/snapshot_collectionspace.$HOST.$PORT 2>&1
		fi 
		if [[ $database = true ]] ; then 	
			$SDB "db.snapshot(SDB_SNAP_DATABASE)" >> SDBSNAPS/snapshot_database.$HOST.$PORT 2>&1
      fi	
		if [[ $system = true ]] ; then 	
			$SDB "db.snapshot(SDB_SNAP_SYSTEM)" >> SDBSNAPS/snapshot_system.$HOST.$PORT 2>&1
		fi
	fi	
}

#collect all hardware infomation
function sdbHardwareInfoAll()
{
	HOST=$1

	mkdir -p HARDINFO/
	lscpu >> HARDINFO/$HOST.cpu.info 2>&1
	cat /proc/cpuinfo >> HARDINFO/$HOST.cpu.info 2>&1

	free -m >> HARDINFO/$HOST.memory1.info 2>&1
	cat /proc/meminfo >> HARDINFO/$HOST.memory2.info 2>&1
	dmidecode -t memory >> HARDINFO/$HOST.memory3.info 2>&1

	lsblk >> HARDINFO/$HOST.disk.info 2>&1
	fdisk -l >> HARDINFO/$HOST.disk.info 2>&1
	df -h >> HARDINFO/$HOST.disk.info 2>&1

	lspci|grep -i 'eth' >> HARDINFO/$HOST.netcard.info 2>&1
	ifconfig >> HARDINFO/$HOST.netcard.info 2>&1

	lspci >> HARDINFO/$HOST.mainboard1.info 2>&1
	lspci -vv >> HARDINFO/$HOST.mainboard2.info 2>&1

	dmidecode -t bios >> HARDINFO/$HOST.bios1.info 2>&1
	dmidecode -q >> HARDINFO/$HOST.bios2.info 2>&1

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

	mkdir -p HARDINFO/
	if [[ $cpu = true ]] ; then 
		lscpu >> HARDINFO/$HOST.cpu.info 2>&1 
		cat /proc/cpuinfo >> HARDINFO/$HOST.cpu.info 2>&1
	fi

	if [[ $memory = true ]] ; then 
		free -m >> HARDINFO/$HOST.memory1.info 2>&1
		cat /proc/meminfo >> HARDINFO/$HOST.memory2.info 2>&1
		dmidecode -t memory >> HARDINFO/$HOST.memory3.info 2>&1
	fi

	if [[ $disk = true ]] ; then 
		lsblk >> HARDINFO/$HOST.disk.info 2>&1	
		fdisk -l >> HARDINFO/$HOST.disk.info 2>&1
		df -h >> HARDINFO/$HOST.disk.info 2>&1
	fi

	if [[ $netcard = true ]] ; then 
		lspci|grep -i 'eth' >> HARDINFO/$HOST.netcard.info 2>&1
		ifconfig >> HARDINFO/$HOST.netcard.info 2>&1
	fi
	
	if [[ $mainboard = true ]] ; then 
		lspci >> HARDINFO/$HOST.mainboard1.info 2>&1
		lspci -vv >> HARDINFO/$HOST.mainboard2.info 2>&1
	fi

	if [[ $bios = true ]] ; then 
		dmidecode -t bios >> HARDINFO/$HOST.bios1.info 2>&1
		dmidecode -q >> HARDINFO/$HOST.bios2.info 2>&1
	fi 
}

#collect operating system information all 
function sdbSystemInfoAll()
{
	HOST=$1 
	mkdir -p OSINFO/ 
	uptime >> OSINFO/$HOST.diskmanage.sys 2>&1
	echo ">>>>>disk manage information" >> OSINFO/$HOST.diskmanage.sys 2>&1
	df ./ >> OSINFO/$HOST.diskmanage.sys 2>&1
	echo ">>>>>mount information" >> OSINFO/$HOST.diskmanage.sys 2>&1
	mount >> OSINFO/$HOST.diskmanage.sys 2>&1

	uptime >> OSINFO/$HOST.system.sys 2>&1
	echo ">>>>>operating system version" >> OSINFO/$HOST.system.sys 2>&1 
	head -n 1 /etc/issue >> OSINFO/$HOST.system.sys 2>&1
	cat /proc/version >> OSINFO/$HOST.system.sys 2>&1
	echo ">>>>>host name" >> OSINFO/$HOST.system.sys 2>&1
	hostname >> OSINFO/$HOST.system.sys 2>&1
	echo ">>>>>long bit of system" >> OSINFO/$HOST.system.sys 2>&1
	getconf LONG_BIT >> OSINFO/$HOST.system.sys 2>&1	
	echo ">>>>>lsb_release -a" >> OSINFO/$HOST.system.sys 2>&1
	ulimit -a >> OSINFO/$HOST.system.sys 2>&1
	lsb_release -a >> OSINFO/$HOST.system.sys 2>&1

	uptime >> OSINFO/$HOST.mode.sys 2>&1
	lsmod >> OSINFO/$HOST.mode.sys 2>&1

	uptime >> OSINFO/$HOST.environmentvar.sys 2>&1 
	env >> OSINFO/$HOST.environmentvar.sys 2>&1

	uptime >> OSINFO/$HOST.IDE.sys 2>&1
	dmesg|grep IDE >> OSINFO/$HOST.IDE.sys 2>&1

	uptime >> OSINFO/$HOST.networkinfo.sys 2>&1
	echo ">>>>>network state information" >> OSINFO/$HOST.networkinfo.sys 2>&1
	netstat -s >> OSINFO/$HOST.networkinfo.sys 2>&1
	echo ">>>>>ifconfig information" >> OSINFO/$HOST.networkinfo.sys 2>&1
	ifconfig >> OSINFO/$HOST.networkinfo.sys 2>&1
	echo ">>>>>detail netstats information" >> OSINFO/$HOST.networkinfo.sys 2>&1
	netstat >> OSINFO/$HOST.networkinfo.sys 2>&1
	
	uptime >> OSINFO/$HOST.nfsstate.sys 2>&1 
	nfsstat >> OSINFO/$HOST.nfsstate.sys 2>&1

	uptime >> OSINFO/$HOST.progress.sys 2>&1 
	ps -elf|sort -rn >> OSINFO/$HOST.progress.sys 2>&1
	ps -aux >> OSINFO/$HOST.progress1.sys 2>&1

	uptime >> OSINFO/$HOST.logininfo.sys 2>&1 
	last >> OSINFO/$HOST.logininfo.sys 2>&1
	history >> OSINFO/$HOST.logininfo.sys 2>&1

	uptime >> OSINFO/$HOST.swapon.sys 2>&1
	swapon -s >> OSINFO/$HOST.swapon.sys 2>&1

	uptime >> OSINFO/$HOST.ulimit.sys 2>&1
	ulimit -a >> OSINFO/$HOST.ulimit.sys 2>&1
	
	uptime >> OSINFO/$HOST.vmstate.sys 2>&1 
	vmstat >> OSINFO/$HOST.vmstate.sys 2>&1

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

	mkdir -p OSINFO/ 

	if [[ $diskmanage = true ]] ; then 
		uptime >> OSINFO/$HOST.diskmanage.sys 2>&1
		echo ">>>>>disk manage information" >> OSINFO/$HOST.diskmanage.sys 2>&1
		df ./ >> OSINFO/$HOST.diskmanage.sys 2>&1
		echo ">>>>>mount information" >> OSINFO/$HOST.diskmanage.sys 2>&1
		mount >> OSINFO/$HOST.diskmanage.sys 2>&1
	fi 

	if [[ $osystem = true ]] ; then 
		uptime >> OSINFO/$HOST.system.sys 2>&1
		echo ">>>>>operating system version" >> OSINFO/$HOST.system.sys 2>&1 
		head -n 1 /etc/issue >> OSINFO/$HOST.system.sys 2>&1
		cat /proc/version >> OSINFO/$HOST.system.sys 2>&1
		echo ">>>>>host name" >> OSINFO/$HOST.system.sys 2>&1
		hostname >> OSINFO/$HOST.system.sys 2>&1
		echo ">>>>>long bit of system" >> OSINFO/$HOST.system.sys 2>&1
		getconf LONG_BIT >> OSINFO/$HOST.system.sys 2>&1	
		echo ">>>>>lsb_release -a" >> OSINFO/$HOST.system.sys 2>&1
		ulimit -a >> OSINFO/$HOST.system.sys 2>&1
		lsb_release -a >> OSINFO/$HOST.system.sys 2>&1
	fi

	if [[ $kermode = true ]] ; then 
		uptime >> OSINFO/$HOST.mode.sys 2>&1
		lsmod >> OSINFO/$HOST.mode.sys 2>&1
	fi

	if [[ $env = true ]] ; then 	
		uptime >> OSINFO/$HOST.environmentvar.sys 2>&1 
		env >> OSINFO/$HOST.environmentvar.sys 2>&1
	fi

	if [[ $IDE = true ]] ; then 
		uptime >> OSINFO/$HOST.IDE.sys 2>&1
		dmesg|grep IDE >> OSINFO/$HOST.IDE.sys 2>&1
	fi

	if [[ $network = true ]] ; then  
		uptime >> OSINFO/$HOST.networkinfo.sys 2>&1
		echo ">>>>>network state information" >> OSINFO/$HOST.networkinfo.sys 2>&1
		netstat -s >> OSINFO/$HOST.networkinfo.sys 2>&1
		echo ">>>>>ifconfig information" >> OSINFO/$HOST.networkinfo.sys 2>&1
		ifconfig >> OSINFO/$HOST.networkinfo.sys 2>&1
		echo ">>>>>detail netstats information" >> OSINFO/$HOST.networkinfo.sys 2>&1
		netstat >> OSINFO/$HOST.networkinfo.sys 2>&1
	fi 

	if [[ $nfsstat = true ]] ; then 	
		uptime >> OSINFO/$HOST.nfsstate.sys 2>&1 
		nfsstat >> OSINFO/$HOST.nfsstate.sys 2>&1
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

	mkdir -p OSINFO/

	if [[ $progress = true ]] ; then 
		uptime >> OSINFO/$HOST.progress.sys 2>&1 
		ps -elf|sort -rn >> OSINFO/$HOST.progress.sys 2>&1
		ps -aux >> OSINFO/$HOST.progress1.sys 2>&1
	fi 

	if [[ $login = true ]] ; then 
		uptime >> OSINFO/$HOST.logininfo.sys 2>&1 
		last >> OSINFO/$HOST.logininfo.sys 2>&1
		history >> OSINFO/$HOST.logininfo.sys 2>&1
	fi 

	if [[ $swapon = true ]] ; then 
		uptime >> OSINFO/$HOST.swapon.sys 2>&1
		swapon -s >> OSINFO/$HOST.swapon.sys 2>&1
	fi

	if [[ $limit = true ]] ; then 
		uptime >> OSINFO/$HOST.ulimit.sys 2>&1
		ulimit -a >> OSINFO/$HOST.ulimit.sys 2>&1
	fi

	if [[ $vmstate = true ]] ; then 	
		uptime >> OSINFO/$HOST.vmstate.sys 2>&1 
		vmstat >> OSINFO/$HOST.vmstate.sys 2>&1
	fi
}


