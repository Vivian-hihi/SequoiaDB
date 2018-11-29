#!/bin/bash

function Usage()
{
    echo  "Usage: sequoias3 <subcommand> [options] [args]"
    echo  "Command options:"
    echo  "  help                      help informaion"
    echo  ""
    echo  "  start                     start sequoias3 with config"
    echo  "  start -p|--port arg       start sequoias3 with specified port and config"
    echo  "  start -c|--confpath arg   start sequoias3 with config in confpath"
    echo  ""
    echo  "  stop  -p|--port arg       stop the sequoias3 process which listen the specified port"
    echo  "  stop  -a|--All            stop all sequoias3 process"
    echo  ""
    echo  "  list                      list all sequoias3 process and listening port"
}

function Start()
{
  package=""
  for file in `ls`
  do
    if [[ $file = sequoia-s3-*.jar ]]; then
      if [[ $file > $package ]]; then
        package=$file
      fi
    fi
  done

  if [ -z $package ]; then
    echo "can not find sequoias3 package"
    exit 1
  fi
  echo "start $package"

  port=$1

  confpath=`pwd`"/config"
  if [ -n "$2" ];then
    confpath=$2
  fi
  configfile=$confpath"/application.properties"
  logback=$confpath"/logback.xml"
  logpath="../log"

  if [ "$port" != "" ]; then
    if [ $port -lt 0  -o  $port -gt 65535 ]; then
      echo -e "\033[31mthe port $port out of range:0-65535\033[0m"
      exit 1
    fi
    portpid=$(lsof -t -i:$port)                                                                                               
    if [ "$portpid" != "" ] ; then
      echo -e "\033[31mthe port $port already be used\033[0m"                                                                                   
      exit 1                                                                                                                          
    fi
    nohup java -jar $package --server.port=$port --spring.config.location=$configfile --logging.config=$logback &
  else
    nohup java -jar $package --spring.config.location=$configfile --logging.config=$logback &
  fi

  pid=$(jobs -l|awk '{print $2}')
  echo "pid:"$pid

  sleep 5 

  listenport=""
  
  loop=0
  while(( $loop < 10 ))
  do
    #echo $loop
    let "loop++"
    listenpid=$( ps -ef |grep $pid |grep -v grep)
    if [ "$listenpid" != "" ]; then
      if [ -n "$(lsof -p $pid |grep LISTEN )" ]; then
        listenport=$(lsof -p $pid |grep LISTEN | awk '{print $9}'|awk -F":" '{print $2}')
        echo "start process:"$pid" port:"$listenport
        break
      else
        sleep 1
        continue
      fi
    else
      echo -e "\033[31mstart failed. please check the application.properties config and re-run\033[0m "
      exit 1
    fi
  done
  
  if [ -z $listenport ]; then
    if [ -z "$( ps -ef |grep $pid |grep -v grep)" ]; then
      echo -e "\033[31mprocess is started, but the LISTEN port is unknown.\033[0m"
    fi
  fi
  
  if [ -z "$(lsof -p $pid |grep ESTABLISHED)"  ]; then
    echo -e "\033[31mprocess is started, but the connection with db doesn't established, please check the application.properties config and db\033[0m"
  fi

  exit 1
}

function Stop()
{
  prefix="sequoia-s3-"

  killall=$1
  port=$2

  if [ -z "$killall" ] && [ -z "$port" ]; then
    echo "stop command must specified -p or -a"
    exit 1
  fi

  if [ -n "$killall" ] && [ -n "$port" ]; then
    echo "can not specified -a and -p at the same time"
    exit 1
  fi

  if [ -n "$killall" ]; then
    pidlist=$(ps -ef|grep $prefix | grep -v grep | awk '{print $2}')
 #   while read pid
    for pid in $pidlist
    do
      kill $pid
      echo "stop pid:$pid"
    done
    exit 0
  fi

  if [ -n "$port" ]; then
    portpid=$(lsof -t -i :$port)
    if [ -z "$portpid" ]; then
      echo "no such port: $port"
      exit 1
    fi

    pidlist=$(ps -ef|grep $prefix | grep -v grep | awk '{print $2}')
    for pid in $pidlist
    do  
      if [ "$pid" == "$portpid" ]; then
          echo "stop process:$pid port:$port"
          kill $pid
          exit 0
      fi
    done
  fi

  echo "do nothing"
  exit 0
}

function List()
{
  prefix="sequoia-s3-"

  pidlist=$( ps -ef|grep $prefix | grep -v grep | awk '{print $2}' )  
  #echo $pidlist
  echo -e "PID \t Port"
  for pid in $pidlist
  do
    port=$(lsof -p $pid | grep LISTEN | awk '{print $9}' | awk -F":" '{print $2}')
    echo -e "$pid \t $port"
  done
}


start=""
stop=""
list=""

killall=""
port=""
configpath=""

if [ "$1" == "start" ]; then
    start="true"
elif [ "$1" == "stop" ]; then
    stop="true"
elif [ "$1" == "list" ]; then
    list="true"
elif [ "$1" == "help" ]; then
    Usage
    exit 0
else
    Usage
    exit 0
fi

shift

while true
do
    case $1 in
    -a|--All)
      killall="true"
      ;;
    -p|--port)
      port=$2
      shift
      ;;
    -c|--confpath)
      configpath=$2
      shift
      ;;
    *)
      break
      ;;
    esac
shift
done

if [ "true" == "$start" ]; then
    Start "$port" "$configpath"
elif [ "true" == "$stop" ]; then
    Stop "$killall" "$port"
elif [ "true" == "$list" ]; then
    List 
else
    echo "do nothing"
fi

exit 0
