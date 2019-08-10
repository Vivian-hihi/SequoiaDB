#!/bin/bash
if [ $# -lt 2 ];then
    echo "$0 requires at least 2 parameters" >&2
    exit 1
fi

progname=$1
killrestart=$2

if [ $# -eq 3 ];then
    svcname=$3
    pid=$(ps -ef | grep ${progname} | grep -v grep | grep ${svcname} | grep -v $0 | awk '{print $2}')
else
    pid=$(ps -ef | grep ${progname} | grep -v grep | grep -v $0 | awk '{print $2}')
fi

if [[ -z $pid ]];then
    echo "$0 can not find progname: $1" >&2
    exit 1
fi

case $progname in
    sdbseadapter)    cmddir=$(ls -l /proc/${pid}/exe | awk '{print $11}')
                     ;;
    org.elasticsearch.bootstrap.Elasticsearch)   
                     cmddir=$(ls -l /proc/${pid}/cwd | awk '{print $11}')
                     ;;
    *)               echo "$0 can not match progname[sdbseadapter, elasticsearch]: $1" >&2
                     exit 1
                     ;;
esac

kill $killrestart $pid
echo $pid:$cmddir