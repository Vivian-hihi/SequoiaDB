#!/bin/bash

#sdbdiag.log file to SDBSNAPS 
function sdbPortGather()
{
   HOST=$1
   DBPATH=$2
   PORT=$3
   installpath=$4

   if [ "$PORT" != "" ] ; then
      mkdir -p SDBNODES/
   else
      return 0
   fi
   #get sequoiadb config path
   confpath=$installpath/conf/local
   #ls -R $confpath >> SDBNODES/$HOST.$PORT.info 2>&1
   ls $DBPATH/diaglog/ > diagfile
   lognum=`cat diagfile|wc -l`
   if [ $lognum -gt 1 ] ; then
      tar -zcvf $DBPATH/$HOST.$PORT.tar.gz $DBPATH/diaglog/
      cp -r $DBPATH/$HOST.$PORT.tar.gz SDBNODES/
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
   installpath=$3

   if [ "$PORT" != "" ] ; then
      mkdir -p SDBSNAPS/
   else
      return 0
   fi
   SDB=$installpath/bin/sdb
   $SDB "var db=new Sdb('localhost',$PORT)" >>sdbsupport.log 2>&1
   rc=$?
   if [ $rc -ne 0 ] ; then
      echo "the sequoiadb is not run! "
   else
      $SDB "db.snapshot(SDB_SNAP_CATALOG)" >> SDBSNAPS/snapshot_catalog.$HOST.$PORT
   fi
   $SDB "quit"
}

#snapshot of sequoiadb to SDBSNAPS
function sdbSnapShot()
{
   HOST=$1
   PORT=$2
   installpath=$3

   if [ "$PORT" != "" ] ; then
      mkdir -p SDBSNAPS/
   else
      return 0
   fi

   SDB=$installpath/bin/sdb
   $SDB "var db=new Sdb('localhost',$PORT)" >>sdbsupport.log 2>&1
   rc=$?
   if [ $rc -ne 0 ] ; then
      echo "Failed to collect snapshot.Please check over the node $PORT is run or not !"
   else
      $SDB "db.listReplicaGroups()" >> SDBSNAPS/listShards.$HOST.$PORT
      $SDB "db.snapshot(SDB_SNAP_CONTEXTS)" >> SDBSNAPS/snapshot_contests.$HOST.$PORT
      $SDB "db.snapshot(SDB_SNAP_SESSIONS)" >> SDBSNAPS/snapshot_sessions.$HOST.$PORT
      $SDB "db.snapshot(SDB_SNAP_COLLECTIONS)" >> SDBSNAPS/snapshot_collections.$HOST.$PORT
      $SDB "db.snapshot(SDB_SNAP_COLLECTIONSPACES)" >> SDBSNAPS/snapshot_collectionspace.$HOST.$PORT
      $SDB "db.snapshot(SDB_SNAP_DATABASE)" >> SDBSNAPS/snapshot_database.$HOST.$PORT
      $SDB "db.snapshot(SDB_SNAP_SYSTEM)" >> SDBSNAPS/snapshot_system.$HOST.$PORT
   fi
   $SDB "quit"
}

#sdbSnapShot Single Extract
function sdbSnapShotExtract()
{
   HOST=$1
   PORT=$2

   group=$3
   context=$4
   session=$5
   collection=$6
   collectionspace=$7
   database=$8
   system=$9
   installpath=${10}

   if [ "$PORT" != "" ] || [ "$group" == "true" ] || [ "$context" == "true" ] || [ "$session" == "true" ] || [ "$collection" == "true" ] || [ "$collectionspace" == "true" ] || [ "$database" == "true" ] || [ "$system" == "true" ] ; then
      mkdir -p SDBSNAPS/
   else
      return 0
   fi

   SDB=$installpath/bin/sdb
   $SDB "var db=new Sdb('localhost',$PORT)" >>sdbsupport.log 2>&1
   if [ $? -ne 0 ] ; then
      echo "Failed to collect snapshot.Please check over the node $PORT is run or not !"
   else
      if [ "$group" == "true" ] ; then
         $SDB "db.listReplicaGroups()" >> SDBSNAPS/listShards.$HOST.$PORT
      fi
      if [ "$context" == "true" ] ; then
         $SDB "db.snapshot(SDB_SNAP_CONTEXTS)" >> SDBSNAPS/snapshot_contests.$HOST.$PORT
      fi
      if [ "$session" == "true" ] ; then
         $SDB "db.snapshot(SDB_SNAP_SESSIONS)" >> SDBSNAPS/snapshot_sessions.$HOST.$PORT
      fi
      if [ "$collection" == "true" ] ; then
         $SDB "db.snapshot(SDB_SNAP_COLLECTIONS)" >> SDBSNAPS/snapshot_collections.$HOST.$PORT
      fi
      if [ "$collectionspace" == "true" ] ; then
         $SDB "db.snapshot(SDB_SNAP_COLLECTIONSPACES)" >> SDBSNAPS/snapshot_collectionspace.$HOST.$PORT
      fi
      if [ "$database" == "true" ] ; then
         $SDB "db.snapshot(SDB_SNAP_DATABASE)" >> SDBSNAPS/snapshot_database.$HOST.$PORT
      fi
      if [ "$system" == "true" ] ; then
         $SDB "db.snapshot(SDB_SNAP_SYSTEM)" >> SDBSNAPS/snapshot_system.$HOST.$PORT
      fi
   fi
   $SDB "quit"
}

#collect all hardware infomation
function sdbHardwareInfoAll()
{
   HOST=$1

   mkdir -p HARDINFO/
	
	sdbCpu
	sdbMemory
	sdbDisk
	sdbNetcard
	sdbMainboard

	#if OSINFO folder don't have file ,then delete it
	lsfold=`ls HARDINFO/` >>/dev/null 2>&1
	if [ "$lsfold" == "" ] ; then
		rm -rf HARDINFO/ 
	fi
}

function sdbCpu()
{
	echo "######>lscpu" >> HARDINFO/$HOST.cpu.info 2>&1 
	lscpu >> HARDINFO/$HOST.cpu.info 2>&1
	rc=$?
   echo "######>cat /proc/cpuinfo" >> HARDINFO/$HOST.cpu.info 2>&1
	cat /proc/cpuinfo >> HARDINFO/$HOST.cpu.info 2>&1
	rc1=$?
	if [ $rc -ne 0 ] && [ $rc1 -ne 0 ] ; then
		echo "Failed to collec cpu information."
		rm HARDINFO/$HOST.cpu.info
	fi
}

function sdbMemory()
{
   echo "######>free -m" >> HARDINFO/$HOST.memory.info 2>&1
	free -m >> HARDINFO/$HOST.memory.info 2>&1
	rc=$?
	echo "######>cat /proc/meminfo" >> HARDINFO/$HOST.memory.info 2>&1
	cat /proc/meminfo >> HARDINFO/$HOST.memory.info 2>&1
	rc1=$?
	if [ $rc -ne 0 ] && [ $rc1 -ne 0 ] ; then
		echo "Failed to collect memory information."
		rm HARDINFO/$HOST.memory.info	
	fi
}

function sdbDisk()
{
   echo "######>lsblk" >> HARDINFO/$HOST.disk.info 2>&1
	lsblk >> HARDINFO/$HOST.disk.info 2>&1
   rc=$?
   echo "######>df -h" >> HARDINFO/$HOST.disk.info 2>&1
	df -h >> HARDINFO/$HOST.disk.info 2>&1
	rc1=$?
	if [ $rc -ne 0 ] && [ $rc1 -ne 0 ] ; then
		echo "Failed to collect disk information"
      rm HARDINFO/$HOST.disk.info
	fi
}

function sdbNetcard()
{
   echo "######>lspci|grep -i 'eth'" >> HARDINFO/$HOST.netcard.info 2>&1
	lspci|grep -i 'eth' >> HARDINFO/$HOST.netcard.info 2>&1
	rc=$?
	if [ $rc -ne 0 ] ; then
		echo "Failed to collect netcard information"	
		rm HARDINFO/$HOST.netcard.info
	fi
}

function sdbMainboard()
{
   echo "######>lspci" >> HARDINFO/$HOST.mainboard.info 2>&1
	lspci >> HARDINFO/$HOST.mainboard.info 2>&1
	rc=$?
   echo "lspci -vv" >> HARDINFO/$HOST.mainboard.info 2>&1
	lspci -vv >> HARDINFO/$HOST.mainboard.info 2>&1
	rc1=$?
	if [ $rc -ne 0 ] && [ $rc1 -ne 0 ] ; then
      echo "Failed to collect mainboard information"
		rm HARDINFO/$HOST.mainboard.info 
	fi

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

   if [ "$cpu" == "true" ] || [ "$memory" == "true" ] || [ "$disk" == "true" ] || [ "$netcard" == "true" ] || [ "$mainboard" == "true" ] ; then
      mkdir -p HARDINFO/
   else
      return 0
   fi

   if [ "$cpu" == "true" ] ; then
		sdbCpu
	fi

   if [ "$memory" == "true" ] ; then
		sdbMemory
	fi

   if [ "$disk" == "true" ] ; then
		sdbDisk
	fi

   if [ "$netcard" == "true" ] ; then
		sdbNetcard
	fi

   if [ "$mainboard" == "true" ] ; then
		sdbMainboard
	fi

   #if OSINFO folder don't have file ,then delete it
	lsfold=`ls HARDINFO/` >>/dev/null 2>&1
	if [ "$lsfold" == "" ] ; then
		rm -rf HARDINFO/
	fi
}

#collect operating system information all
function sdbSystemInfoAll()
{
   HOST=$1

   mkdir -p OSINFO/

	sdbDiskManage
	sdbSystemOS
	sdbModules
	sdbEnvVar
	sdbNetworkInfo
	sdbProcess
	sdbLogin
	sdbLimit
	sdbVmstat

	#if OSINFO folder don't have file ,then delete it
	lsfold=`ls OSINFO/` >>/dev/null 2>&1
	if [ "$lsfold" == "" ] ; then
		rm -rf OSINFO/
	fi

}
##collect disk manage information
function sdbDiskManage()
{
   echo "######>df ./   " >> OSINFO/$HOST.diskmanage.sys
   df ./ >> OSINFO/$HOST.diskmanage.sys 2>&1
	rc=$?
	echo "######>mount   " >> OSINFO/$HOST.diskmanage.sys
   mount >> OSINFO/$HOST.diskmanage.sys 2>&1
	rc1=$?
	if [ $rc -ne 0 ] && [ $rc1 -ne 0 ] ; then 
		echo "Failed to collect disk manage information"
		rm OSINFO/$HOST.diskmanage.sys
	fi
}
##collect system information 
function sdbSystemOS()
{
   echo "######>head -n 1 /etc/issue" >> OSINFO/$HOST.system.sys 2>&1
   head -n 1 /etc/issue >> OSINFO/$HOST.system.sys 2>&1
	rc=$?
	echo "######>cat /proc/version" >> OSINFO/$HOST.system.sys 2>&1
	cat /proc/version >> OSINFO/$HOST.system.sys 2>&1
	rc1=$?
	echo "######>hostname" >> OSINFO/$HOST.system.sys 2>&1
   hostname >> OSINFO/$HOST.system.sys 2>&1
   rc2=$?
	echo "######>getconf LONG_BIT" >> OSINFO/$HOST.system.sys 2>&1
	getconf LONG_BIT >> OSINFO/$HOST.system.sys 2>&1
	rc3=$?
	echo "######>ulimit -a" >> OSINFO/$HOST.system.sys 2>&1
   ulimit -a >> OSINFO/$HOST.system.sys 2>&1
	rc4=$?
	echo "######>lsb_release -a" >> OSINFO/$HOST.system.sys 2>&1
	lsb_release -a >> OSINFO/$HOST.system.sys 2>&1
	rc5=$?
	if [ $rc -ne 0 ] && [ $rc1 -ne 0 ] && [$rc2 -ne 0 ] && [ $rc3 -ne 0 ] && [ $rc4 -ne 0 ] && [ $rc5 -ne 0 ] ; then 
		echo "Failed to collect system information "
		rm OSINFO/$HOST.system.sys
	fi
}
##collect modules information
function sdbModules()
{
	echo "######>lsmod" >> OSINFO/$HOST.modules.sys 2>&1
	lsmod >> OSINFO/$HOST.modules.sys 2>&1
	rc=$?
	if [ $rc -ne 0 ] ; then
		echo "Failed to collect modules information"
		rm $HOST.modules.sys
	fi
}
##collect environment variable
function sdbEnvVar()
{
	echo "######>env" >> OSINFO/$HOST.environmentvar.sys 2>&1
	env >> OSINFO/$HOST.environmentvar.sys 2>&1
	rc=$?
	if [ $rc -ne 0 ] ; then
      echo "Failed to collect environment variable"
	   rm OSINFO/$HOST.environmentvar.sys
	fi
}
##collect network information
function sdbNetworkInfo()
{	
   echo "######>netstat -s" >> OSINFO/$HOST.networkinfo.sys 2>&1
   netstat -s >> OSINFO/$HOST.networkinfo.sys 2>&1
	rc=$?
	echo "######>netstat" >> OSINFO/$HOST.networkinfo.sys 2>&1
   netstat >> OSINFO/$HOST.networkinfo.sys 2>&1
	rc1=$?
	if [ $rc -ne 0 ] && [ $rc1 -ne 0 ] ; then
		echo "Failed to collect network information"
		rm OSINFO/$HOST.networkinfo.sys
	fi

}
##collet process information
function sdbProcess()
{
   echo "######>ps -elf|sort -rn" >> OSINFO/$HOST.process.sys 2>&1
	ps -elf|sort -rn >> OSINFO/$HOST.process.sys 2>&1
   rc=$?
	echo "######>ps aux" >> OSINFO/$HOST.process.sys 2>&1
	ps aux >> OSINFO/$HOST.process.sys 2>&1
   rc1=$?
   if [ $rc -ne 0 ] && [ $rc1 -ne 0 ] ; then
		echo "Failed to collect process information"
		rm OSINFO/$HOST.process.sys
	fi
}
##collect login information
function sdbLogin()
{
   echo "######>last" >> OSINFO/$HOST.logininfo.sys 2>&1
	last >> OSINFO/$HOST.logininfo.sys 2>&1
   rc=$? 
	echo "######>history" >> OSINFO/$HOST.logininfo.sys 2>&1
	history >> OSINFO/$HOST.logininfo.sys 2>&1
   rc1=$?
	if [ $rc -ne 0 ] && [ $rc1 -ne 0 ] ; then
		echo "Failed to collect login information"
		rm OSINFO/$HOST.logininfo.sys
	fi
}
##collect limit information
function sdbLimit()
{
   echo "######>ulimit -a" >> OSINFO/$HOST.ulimit.sys 2>&1
	ulimit -a >> OSINFO/$HOST.ulimit.sys 2>&1
	rc=$?
	if [ $rc -ne 0 ] ; then
		echo "Failed to collect system limit information"
		rm OSINFO/$HOST.ulimit.sys
	fi
}
##collect vmstat information
function sdbVmstat()
{
   echo "######>vmstat" >> OSINFO/$HOST.vmstate.sys 2>&1
	vmstat >> OSINFO/$HOST.vmstate.sys 2>&1
   rc=$?
	if [ $rc -ne 0 ] ; then
	   echo "Failed to collect vmstat information"
		rm OSINFO/$HOST.vmstate.sys
	fi

}

#collect part of operating system information ,front half
function sdbSystemInfoPartFore()
{
   HOST=$1

   diskmanage=$2
   osystem=$3
   kermode=$4
   env=$5
   network=$6

   if [ "$diskmanage" == "true" ] || [ "$osystem" == "true" ] || [ "$kermode" == "true" ] || [ "$env" == "true" ] || [ "$network" == "true" ] ; then
      mkdir -p OSINFO/
   else
      return 0
   fi

   if [ "$diskmanage" == "true" ] ; then
      sdbDiskManage
	fi

   if [ "$osystem" == "true" ] ; then
      sdbSystemOS 
   fi

   if [ "$kermode" == "true" ] ; then
      sdbModules 
   fi

   if [ "$env" == "true" ] ; then
      sdbEnvVar 
   fi

   if [ "$network" == "true" ] ; then
      sdbNetworkInfo 
   fi

   #if OSINFO folder don't have file ,then delete it
	lsfold=`ls OSINFO/` >>/dev/null 2>&1
	if [ "$lsfold" == "" ] ; then
		rm -rf OSINFO/
	fi

}

#collect end half part of Operating System information 
function sdbSystemInfoPartEnd()
{
   HOST=$1

   process=$2
   login=$3
   limit=$4
   vmstate=$5

   if [ "$process" == "true" ] || [ "$login" == "true" ] || [ "$limit" == "true" ] || [ "$vmstate" == "true" ] ; then
      mkdir -p OSINFO/
   else
      return 0
   fi

   if [ "$process" == "true" ] ; then
      sdbProcess 
   fi

   if [ "$login" == "true" ] ; then
      sdbLogin 
   fi

   if [ "$limit" == "true" ] ; then
      sdbLimit 
   fi

   if [ "$vmstate" == "true" ] ; then
      sdbVmstat 
   fi

	#if OSINFO folder don't have file ,then delete it 
	lsfold=`ls OSINFO/` >>/dev/null 2>&1
   if [ "$lsfold" == "" ] ; then
      rm -rf OSINFO/
   fi
}


