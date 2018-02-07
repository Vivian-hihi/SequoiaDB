#!/bin/bash

#init variable
currentPath=$(cd "$(dirname "$0")"; pwd)

#config parameter
binName="ssqloltp-plugin"
extName="jar"
javaPath="$currentPath/../../../java/jdk/bin/java"

#include common.sh
source ./common.sh

binFileName=`getExecFileName $currentPath $binName $extName`
if [ "$binFileName"x == ""x ]; then
   echo "Error: executable program not found.";
   exit 1;
fi

#echo $binFileName

omhttpname=`getConfig $currentPath "omhttpname"`
if [ "$omhttpname"x == ""x ]; then
   omhttpname="8000"
fi

#echo $omhttpname

pid=`getProcId $binFileName "--__omhttpname=$omhttpname"`
if [ "$pid"x == ""x ]; then

   startupPlugin $currentPath "$javaPath -jar $currentPath/$binFileName" $omhttpname ""

   pid=`getProcId $binFileName "--__omhttpname=$omhttpname"`
   if [ "$pid"x == ""x ]; then
      echo "Error: failed to start $binFileName" ;
      exit 1
   else
      echo "Success: $binFileName is successfully started ($pid)" ;
      exit 0
   fi

else
   echo "$binFileName is already started ($pid)";
   exit 0;
fi

