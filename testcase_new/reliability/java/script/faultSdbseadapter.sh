#!/bin/bash
pid=$(ps -ef | grep sdbseadapter | grep -v grep | grep $1 | grep -v faultSdbseadapter.sh | awk '{print $2}')
if [[ -z $pid ]];then
   echo "can not find this sdbseadapter: $1" >&2
   exit 1
fi
command_dir=$(ls -l /proc/$pid/exe | awk '{print $11}')
kill $2 $pid
echo $pid:$command_dir