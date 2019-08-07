#!/bin/bash
pid=$(ps -ef | grep elasticsearch | grep -v grep | grep -v faultElasticSearch.sh | awk '{print $2}')
if [[ -z $pid ]];then
   echo "can not find this elasticsearch manager" >&2
   exit 1
fi
command_dir=$(ls -l /proc/$pid/cwd | awk '{print $11}')
kill $1 $pid
echo $pid:$command_dir