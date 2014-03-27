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

   lscpu >> HARDINFO/$HOST.cpu.info
   cat /proc/cpuinfo >> HARDINFO/$HOST.cpu.info

   free -m >> HARDINFO/$HOST.memory1.info
   cat /proc/meminfo >> HARDINFO/$HOST.memory2.info
   #dmidecode -t memory >> HARDINFO/$HOST.memory3.info 2>&1

   lsblk >> HARDINFO/$HOST.disk.info
   #fdisk -l >> HARDINFO/$HOST.disk.info
   df -h >> HARDINFO/$HOST.disk.info

   lspci|grep -i 'eth' >> HARDINFO/$HOST.netcard.info
   #ifconfig >> HARDINFO/$HOST.netcard.info

   lspci >> HARDINFO/$HOST.mainboard1.info
   lspci -vv >> HARDINFO/$HOST.mainboard2.info

   #dmidecode -t bios >> HARDINFO/$HOST.bios1.info
   #dmidecode -q >> HARDINFO/$HOST.bios2.info

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
      lscpu >> HARDINFO/$HOST.cpu.info
      cat /proc/cpuinfo >> HARDINFO/$HOST.cpu.info
   fi

   if [ "$memory" == "true" ] ; then
      free -m >> HARDINFO/$HOST.memory1.info
      cat /proc/meminfo >> HARDINFO/$HOST.memory2.info
      #dmidecode -t memory >> HARDINFO/$HOST.memory3.info 2>&1
   fi

   if [ "$disk" == "true" ] ; then
      lsblk >> HARDINFO/$HOST.disk.info
      #fdisk -l >> HARDINFO/$HOST.disk.info 2>&1
      df -h >> HARDINFO/$HOST.disk.info
   fi

   if [ "$netcard" == "true" ] ; then
      lspci|grep -i 'eth' >> HARDINFO/$HOST.netcard.info
      #ifconfig >> HARDINFO/$HOST.netcard.info
   fi

   if [ "$mainboard" == "true" ] ; then
      lspci >> HARDINFO/$HOST.mainboard1.info
      lspci -vv >> HARDINFO/$HOST.mainboard2.info
   fi

   #if [[ $bios = true ]] ; then
      #dmidecode -t bios >> HARDINFO/$HOST.bios1.info 2>&1
      #dmidecode -q >> HARDINFO/$HOST.bios2.info 2>&1
   #fi
}

#collect operating system information all
function sdbSystemInfoAll()
{
   HOST=$1

   mkdir -p OSINFO/

   echo "success to create FOLDER"
   uptime >> OSINFO/$HOST.diskmanage.sys
   echo ">df ./   [disk manage information]" >> OSINFO/$HOST.diskmanage.sys
   df ./ >> OSINFO/$HOST.diskmanage.sys
   echo ">mount   [mount information]" >> OSINFO/$HOST.diskmanage.sys
   mount >> OSINFO/$HOST.diskmanage.sys

   uptime >> OSINFO/$HOST.system.sys
   echo ">>>>>operating system version" >> OSINFO/$HOST.system.sys
   head -n 1 /etc/issue >> OSINFO/$HOST.system.sys
   cat /proc/version >> OSINFO/$HOST.system.sys
   echo ">>>>>host name" >> OSINFO/$HOST.system.sys
   hostname >> OSINFO/$HOST.system.sys
   echo ">>>>>long bit of system" >> OSINFO/$HOST.system.sys
   getconf LONG_BIT >> OSINFO/$HOST.system.sys
   echo ">>>>>lsb_release -a" >> OSINFO/$HOST.system.sys
   ulimit -a >> OSINFO/$HOST.system.sys
   lsb_release -a >> OSINFO/$HOST.system.sys


   uptime >> OSINFO/$HOST.mode.sys
   lsmod >> OSINFO/$HOST.mode.sys

   uptime >> OSINFO/$HOST.environmentvar.sys
   env >> OSINFO/$HOST.environmentvar.sys

   uptime >> OSINFO/$HOST.IDE.sys
   dmesg|grep IDE >> OSINFO/$HOST.IDE.sys

   uptime >> OSINFO/$HOST.networkinfo.sys
   echo ">>>>>network state information" >> OSINFO/$HOST.networkinfo.sys
   netstat -s >> OSINFO/$HOST.networkinfo.sys
   echo ">>>>>ifconfig information" >> OSINFO/$HOST.networkinfo.sys
   #ifconfig >> OSINFO/$HOST.networkinfo.sys
   echo ">>>>>detail netstats information" >> OSINFO/$HOST.networkinfo.sys
   netstat >> OSINFO/$HOST.networkinfo.sys

   #uptime >> OSINFO/$HOST.nfsstate.sys 2>&1
   #nfsstat >> OSINFO/$HOST.nfsstate.sys 2>&1
   uptime >> OSINFO/$HOST.progress.sys
   ps -elf|sort -rn >> OSINFO/$HOST.progress.sys
   ps aux >> OSINFO/$HOST.progress1.sys
   uptime >> OSINFO/$HOST.logininfo.sys
   last >> OSINFO/$HOST.logininfo.sys
   history >> OSINFO/$HOST.logininfo.sys

   #uptime >> OSINFO/$HOST.swapon.sys
   #swapon -s >> OSINFO/$HOST.swapon.sys

   uptime >> OSINFO/$HOST.ulimit.sys
   ulimit -a >> OSINFO/$HOST.ulimit.sys

   uptime >> OSINFO/$HOST.vmstate.sys
   vmstat >> OSINFO/$HOST.vmstate.sys

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

   if [ "$diskmanage" == "true" ] || [ "$osystem" == "true" ] || [ "$kermode" == "true" ] || [ "$env" == "true" ] || [ "$IDE" == "true" ] || [ "$network" == "true" ] ; then
      mkdir -p OSINFO/
   else
      return 0
   fi

   if [ "$diskmanage" == "true" ] ; then
      uptime >> OSINFO/$HOST.diskmanage.sys
      echo ">>>>>disk manage information" >> OSINFO/$HOST.diskmanage.sys
      df ./ >> OSINFO/$HOST.diskmanage.sys
      echo ">>>>>mount information" >> OSINFO/$HOST.diskmanage.sys
      mount >> OSINFO/$HOST.diskmanage.sys
   fi

   if [ "$osystem" == "true" ] ; then
      uptime >> OSINFO/$HOST.system.sys
      echo ">>>>>operating system version" >> OSINFO/$HOST.system.sys
      head -n 1 /etc/issue >> OSINFO/$HOST.system.sys
      cat /proc/version >> OSINFO/$HOST.system.sys
      echo ">>>>>host name" >> OSINFO/$HOST.system.sys
      hostname >> OSINFO/$HOST.system.sys
      echo ">>>>>long bit of system" >> OSINFO/$HOST.system.sys
      getconf LONG_BIT >> OSINFO/$HOST.system.sys
      echo ">>>>>lsb_release -a" >> OSINFO/$HOST.system.sys
      ulimit -a >> OSINFO/$HOST.system.sys
      lsb_release -a >> OSINFO/$HOST.system.sys
   fi

   if [ "$kermode" == "true" ] ; then
      uptime >> OSINFO/$HOST.mode.sys
      lsmod >> OSINFO/$HOST.mode.sys
   fi

   if [ "$env" == "true" ] ; then
      uptime >> OSINFO/$HOST.environmentvar.sys
      env >> OSINFO/$HOST.environmentvar.sys
   fi

   if [ "$IDE" == "true" ] ; then
      uptime >> OSINFO/$HOST.IDE.sys
      dmesg|grep IDE >> OSINFO/$HOST.IDE.sys
   fi

   if [ "$network" == "true" ] ; then
      uptime >> OSINFO/$HOST.networkinfo.sys
      echo ">>>>>network state information" >> OSINFO/$HOST.networkinfo.sys
      netstat -s >> OSINFO/$HOST.networkinfo.sys
      echo ">>>>>ifconfig information" >> OSINFO/$HOST.networkinfo.sys
      #ifconfig >> OSINFO/$HOST.networkinfo.sys
      echo ">>>>>detail netstats information" >> OSINFO/$HOST.networkinfo.sys
      netstat >> OSINFO/$HOST.networkinfo.sys
   fi

   #if [[ $nfsstat = true ]] ; then
      #uptime >> OSINFO/$HOST.nfsstate.sys 2>&1 
      #nfsstat >> OSINFO/$HOST.nfsstate.sys 2>&1
   #fi

}

#collect end half part of Operating System information 
function sdbSystemInfoPartEnd()
{
   HOST=$1

   progress=$2
   login=$3
   limit=$4
   vmstate=$5

   if [ "$progress" == "true" ] || [ "$login" == "true" ] || [ "$limit" == "true" ] || [ "$vmstate" == "true" ] ; then
      mkdir -p OSINFO/
   else
      return 0
   fi

   if [ "$progress" == "true" ] ; then
      uptime >> OSINFO/$HOST.progress.sys
      ps -elf|sort -rn >> OSINFO/$HOST.progress.sys
      ps aux >> OSINFO/$HOST.progress1.sys
   fi

   if [ "$login" == "true" ] ; then
      uptime >> OSINFO/$HOST.logininfo.sys
      last >> OSINFO/$HOST.logininfo.sys
      history >> OSINFO/$HOST.logininfo.sys
   fi

   #if [[ $swapon = true ]] ; then 
      #uptime >> OSINFO/$HOST.swapon.sys 2>&1
      #swapon -s >> OSINFO/$HOST.swapon.sys 2>&1
   #fi

   if [ "$limit" == "true" ] ; then
      uptime >> OSINFO/$HOST.ulimit.sys
      ulimit -a >> OSINFO/$HOST.ulimit.sys
   fi

   if [ "$vmstate" == "true" ] ; then
      uptime >> OSINFO/$HOST.vmstate.sys
      vmstat >> OSINFO/$HOST.vmstate.sys
   fi
}


