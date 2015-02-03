#!/usr/bin/sh
#ip="192.168.20.177 192.168.20.178 192.168.20.176"

#node hostname
ip=("sdbserver2" "sdbserver3" "sdbserver5")

#sdb file path
sdbcmart="/opt/trunk/bin/sdbcmart"
sdbstart="/opt/trunk/bin/sdbstart"
sdbcmtop="/opt/trunk/bin/sdbcmtop"
sdbstop="/opt/trunk/bin/sdbstop"

#config file path
cata_confpath="/opt/trunk/conf/local/11800"
coord_confpath="/opt/trunk/conf/local/11810"
data1_confpath="/opt/trunk/conf/local/11820"
data2_confpath="/opt/trunk/conf/local/11830"

node_count=4
normal_time=10
initial_time=30
abnormal_time=330

function_array1=("testcase1" "testcase2" "testcase3" "testcase4")
function_array2=("testcase5" "testcase6" "testcase7" "testcase8" "testcase9" "testcase10" "testcase11" "testcase12" "testcase13" "testcase14" "testcase15" "testcase16" "testcase17" "testcase18" "testcase19" "testcase20")

#stop all process and sdbcm
function stopAll()
{
   command="ps -ef| grep 'sequoiadb --confpath' | grep -v grep | awk {'print \$2'} | xargs kill -15"
   count=$(ssh "$1" "ps -ef| grep 'sequoiadb --confpath' | grep -v grep | awk '{print \$2}'| wc -l")
   #echo "count=$count"
   if [ $count -ne 0 ]; then
      ssh "$1" "$sdbstop;$command;$sdbcmtop;exit"
   else
      ssh "$1" "$sdbstop;$sdbcmtop;exit"
   fi
}

#stop all process except sdbcm
function stopPro()
{
   command="ps -ef| grep 'sequoiadb --confpath' | grep -v grep | awk {'print \$2'} | xargs kill -15"
   count=$(ssh "$1" "ps -ef| grep 'sequoiadb --confpath' | grep -v grep | awk '{print \$2}'| wc -l")
   #echo "count=$count"
   if [ $count -ne 0 ]; then
      ssh "$1" "$sdbstop;$command;exit"
   else
      ssh "$1" "$sdbstop;exit"
   fi
}

#check the machine has been start successful
function check_machine()
{
   count=$(ssh "$1" "ps -ef | grep  'sequoiadb('|grep -v grep |wc -l;exit")
   #echo "count=$count"
   if [ $count -ne $node_count ]; then
      echo "error,count=$count,ip=$1,line=$2."
      return -1
   fi
   return 0
}


#check the testcase return value
function check_case()
{
   if [ $1 -eq 0 ]; then
      echo "$2 execute success"
   else
      echo "$2 execute fail"
   fi
}

#initialize the envirment before execute a testcase,stop all process and sdbcm
function initializeAll()
{
   for i in ${ip[@]}
   do
      stopAll $i
   done
   echo 'initializeAll success'
}


#initialize the environment before execute a testcase,stop all process expect sdbcm
function initializePro()
{
   for i in ${ip[@]}
   do
      stopPro $i
   done
   echo 'initializePro success'

}

function runAll()
{
   #execute function_array1's function
   for func in ${function_array1[@]}
   do
      initializeAll
      $func
      check_case $? $func
   done

   #execute function_array2's function
   for func in ${function_array2[@]}
   do
      initializePro
      $func
      check_case $? $func
   done

}

function testcase1()
{

   #flag=0 represent this testcase  execute success
   #flag=-1 represent this testcase execute fail
   flag=0

   #login on every machine then start all the nodes
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbcmart" &
      sleep $normal_time
   done


   #time to initialize the coordiate
   sleep $initial_time


   #check every machine has been started successful
   for i in ${ip[@]}
   do
      check_machine $i $LINENO
      if [ $? -ne 0 -a $flag -eq 0 ]; then
         return -1
      fi
   done
   return 0

}

function testcase2()
{
   flag=-1

   for i in ${ip[@]}
   do
      ssh "$i" "$sdbcmart" &
      sleep $abnormal_time
   done

   sleep $initial_time

   for i in ${ip[@]}
   do
      check_machine $i $LINENO
      if [ $? -ne 0 -a $flag -eq  -1 ]; then
         return 0
      fi
   done
   return -1

}

function testcase3()
{
   flag=-1
   j=1
   for i in ${ip[@]}
   do
      if [ $j -eq 2 ]; then
         ssh "$i" "$sdbcmart" &
         sleep $abnormal_time
      else
         ssh "$i" "$sdbcmart" &
         sleep $normal_time
      fi
      j=$[$j+1]
   done

   sleep $initial_time

   for i in ${ip[@]}
   do
      check_machine $i $LINENO
      if [ $? -ne 0 -a $flag -eq -1 ]; then
         return 0
      fi
   done

   return -1

}


function testcase4()
{
   flag=-1
   j=1
   for i in ${ip[@]}
   do
      if [ $j -eq 2 ]; then
         ssh "$i" "$sdbcmart" &
         sleep $normal_time
      else
         ssh "$i" "$sdbcmart" &
         sleep $abnormal_time
      fi
      j=$[$j+1]
   done

   sleep $initial_time

   for i in ${ip[@]}
   do
      check_machine $i $LINENO
      if [ $? -ne 0 -a $flag -eq -1 ]; then
         return 0
      fi
   done

   return -1

}

function testcase5()
{
   flag=0
   #start coord node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $coord_confpath" &
   done

   sleep $abnormal_time

   #start cata node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $cata_confpath" &
   done

   sleep $abnormal_time

   #start data node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $data1_confpath" &
      ssh "$i" "$sdbstart -c $data2_confpath" &
   done

   sleep $initial_time

   #check each machine
   for i in ${ip[@]}
   do
      check_machine $i $LINENO
      if [ $? -ne 0 -a $flag -eq 0 ]; then
         return -1
      fi
   done

   return 0

}

function testcase6()
{
   flag=-1
   #start coord node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $coord_confpath" &
   done

   sleep $abnormal_time

   #start data node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $data1_confpath" &
      ssh "$i" "$sdbstart -c $data2_confpath" &
   done

   sleep $abnormal_time

   #start cata node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $cata_confpath" &
   done


   sleep $initial_time

   #check each machine
   for i in ${ip[@]}
   do
      check_machine $i $LINENO
      if [ $? -ne 0 -a $flag -eq -1 ]; then
         return 0
      fi
   done

   return -1

}

function testcase7()
{
   flag=0
   #start cata node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $cata_confpath" &
   done

   sleep $abnormal_time

   #start coord node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $coord_confpath" &
   done

   sleep $abnormal_time

   #start data node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $data1_confpath" &
      ssh "$i" "$sdbstart -c $data2_confpath" &
   done

   sleep $initial_time

   #check each machine
   for i in ${ip[@]}
   do
      check_machine $i $LINENO
      if [ $? -ne 0 -a $flag -eq 0 ]; then
         return -1
      fi
   done

   return 0

}

function testcase8()
{
   flag=-1
   #start data node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $data1_confpath" &
      ssh "$i" "$sdbstart -c $data2_confpath" &
   done

   sleep $abnormal_time

   #start coord node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $coord_confpath" &
   done

   sleep $abnormal_time

   #start cata node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $cata_confpath" &
   done

   sleep $initial_time

   #check each machine
   for i in ${ip[@]}
   do
      check_machine $i $LINENO
      if [ $? -ne 0 -a $flag -eq -1 ]; then
         return 0
      fi
   done

   return -1

}

function testcase9()
{
   flag=0
   #start coord node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $coord_confpath" &
   done

   sleep $normal_time

   #start cata node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $cata_confpath" &
   done

   sleep $normal_time

   #start data node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $data1_confpath" &
      ssh "$i" "$sdbstart -c $data2_confpath" &
   done

   sleep $initial_time

   #check each machine
   for i in ${ip[@]}
   do
      check_machine $i $LINENO
      if [ $? -ne 0 -a $flag -eq 0 ]; then
         return -1
      fi
   done

   return 0

}

function testcase10()
{
   flag=0
   #start coord node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $coord_confpath" &
   done

   sleep $normal_time

   #start data node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $data1_confpath" &
      ssh "$i" "$sdbstart -c $data2_confpath" &
   done

   sleep $normal_time

   #start cata node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $cata_confpath" &
   done


   sleep $initial_time

   #check each machine
   for i in ${ip[@]}
   do
      check_machine $i $LINENO
      if [ $? -ne 0 -a $flag -eq 0 ]; then
         return -1
      fi
   done

   return 0

}

function testcase11()
{
   flag=0
   #start cata node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $cata_confpath" &
   done

   sleep $normal_time

   #start coord node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $coord_confpath" &
   done

   sleep $normal_time

   #start data node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $data1_confpath" &
      ssh "$i" "$sdbstart -c $data2_confpath" &
   done

   sleep $initial_time

   #check each machine
   for i in ${ip[@]}
   do
      check_machine $i $LINENO
      if [ $? -ne 0 -a $flag -eq 0 ]; then
         return -1
      fi
   done

   return 0

}

function testcase12()
{
   flag=0
   #start data node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $data1_confpath" &
      ssh "$i" "$sdbstart -c $data2_confpath" &
   done

   sleep $normal_time

   #start coord node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $coord_confpath" &
   done

   sleep $normal_time

   #start cata node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $cata_confpath" &
   done

   sleep $initial_time

   #check each machine
   for i in ${ip[@]}
   do
      check_machine $i $LINENO
      if [ $? -ne 0 -a $flag -eq 0 ]; then
         return -1
      fi
   done

   return 0

}


function testcase13()
{
   flag=0
   #start coord node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $coord_confpath" &
   done

   sleep $normal_time

   #start cata node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $cata_confpath" &
   done

   sleep $abnormal_time

   #start data node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $data1_confpath" &
      ssh "$i" "$sdbstart -c $data2_confpath" &
   done

   sleep $initial_time

   #check each machine
   for i in ${ip[@]}
   do
      check_machine $i $LINENO
      if [ $? -ne 0 -a $flag -eq 0 ]; then
         return -1
      fi
   done

   return 0

}

function testcase14()
{
   flag=-1
   #start coord node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $coord_confpath" &
   done

   sleep $normal_time

   #start data node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $data1_confpath" &
      ssh "$i" "$sdbstart -c $data2_confpath" &
   done

   sleep $abnormal_time

   #start cata node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $cata_confpath" &
   done


   sleep $initial_time

   #check each machine
   for i in ${ip[@]}
   do
      check_machine $i $LINENO
      if [ $? -ne 0 -a $flag -eq -1 ]; then
         return 0
      fi
   done

   return -1

}

function testcase15()
{
   flag=0
   #start cata node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $cata_confpath" &
   done

   sleep $normal_time

   #start coord node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $coord_confpath" &
   done

   sleep $abnormal_time

   #start data node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $data1_confpath" &
      ssh "$i" "$sdbstart -c $data2_confpath" &
   done

   sleep $initial_time

   #check each machine
   for i in ${ip[@]}
   do
      check_machine $i $LINENO
      if [ $? -ne 0 -a $flag -eq 0 ]; then
         return -1
      fi
   done

   return 0

}

function testcase16()
{
   flag=-1
   #start data node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $data1_confpath" &
      ssh "$i" "$sdbstart -c $data2_confpath" &
   done

   sleep $normal_time

   #start coord node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $coord_confpath" &
   done

   sleep $abnormal_time

   #start cata node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $cata_confpath" &
   done

   sleep $initial_time

   #check each machine
   for i in ${ip[@]}
   do
      check_machine $i $LINENO
      if [ $? -ne 0 -a $flag -eq -1 ]; then
         return 0
      fi
   done

   return -1

}


function testcase17()
{
   flag=0
   #start coord node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $coord_confpath" &
   done

   sleep $abnormal_time

   #start cata node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $cata_confpath" &
   done

   sleep $normal_time

   #start data node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $data1_confpath" &
      ssh "$i" "$sdbstart -c $data2_confpath" &
   done

   sleep $initial_time

   #check each machine
   for i in ${ip[@]}
   do
      check_machine $i $LINENO
      if [ $? -ne 0 -a $flag -eq 0 ]; then
         return -1
      fi
   done

   return 0

}

function testcase18()
{
   flag=0
   #start coord node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $coord_confpath" &
   done

   sleep $abnormal_time

   #start data node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $data1_confpath" &
      ssh "$i" "$sdbstart -c $data2_confpath" &
   done

   sleep $normal_time

   #start cata node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $cata_confpath" &
   done


   sleep $initial_time

   #check each machine
   for i in ${ip[@]}
   do
      check_machine $i $LINENO
      if [ $? -ne 0 -a $flag -eq 0 ]; then
         return -1
      fi
   done

   return 0

}

function testcase19()
{
   flag=0
   #start cata node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $cata_confpath" &
   done

   sleep $abnormal_time

   #start coord node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $coord_confpath" &
   done

   sleep $normal_time

   #start data node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $data1_confpath" &
      ssh "$i" "$sdbstart -c $data2_confpath" &
   done

   sleep $initial_time

   #check each machine
   for i in ${ip[@]}
   do
      check_machine $i $LINENO
      if [ $? -ne 0 -a $flag -eq 0 ]; then
         return -1
      fi
   done

   return 0

}

function testcase20()
{
   flag=-1
   #start data node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $data1_confpath" &
      ssh "$i" "$sdbstart -c $data2_confpath" &
   done

   sleep $abnormal_time

   #start coord node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $coord_confpath" &
   done

   sleep $normal_time

   #start cata node
   for i in ${ip[@]}
   do
      ssh "$i" "$sdbstart -c $cata_confpath" &
   done

   sleep $initial_time

   #check each machine
   for i in ${ip[@]}
   do
      check_machine $i $LINENO
      if [ $? -ne 0 -a $flag -eq -1 ]; then
         return 0
      fi
   done

   return -1

}

#run all testcase
runAll



