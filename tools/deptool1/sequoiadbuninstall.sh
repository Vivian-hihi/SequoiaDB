#!/bin/bash


processes=`ps -ef | grep '\<sdbcm\>'|grep -v '\<grep\>'|awk '{print $2}'`
for process in ${processes}
do
   kill -9 ${process}
done

processes=`ps -ef | grep '\<sdbstart\>'|grep -v '\<grep\>'|awk '{print $2}'`
for process in ${processes}
do
   kill -9 ${process}
done

processes=`ps -ef | grep '\<sdbstop\>'|grep -v '\<grep\>'|awk '{print $2}'`
for process in ${processes}
do
   kill -9 ${process}
done

processes=`ps -ef | grep '\<sdb\>'|grep -v '\<grep\>'|awk '{print $2}'`
for process in ${processes}
do
   kill -9 ${process}
done

processes=`ps -ef | grep '\<sdbbp\>'|grep -v '\<grep\>'|awk '{print $2}'`
for process in ${processes}
do
   kill -9 ${process}
done

processes=`ps -ef | grep '\<sequoiadb\>'|grep -v '\<grep\>'|awk '{print $2}'`
for process in ${processes}
do
   kill -9 ${process}
done

rm -r ${1}
rc=$?
exit 0
