#!/bin/bash

#Get the file name of the executable file
#$1 find exec file path
#$2 filename of fuzzy matching( excluding extension )
#$3 extension
function getExecFileName()
{
   local fileList=`ls $1`;
   for fileName in $fileList;
   do
      # it is a file
      if [ -f "$1/$fileName" ]; then
         if [ "$3"x == ""x -a "${fileName%.*}"x == "${fileName##*.}"x ]; then
            # no ext name
            # matching file name
            if [[ "${fileName%.*}" == $2* ]]; then
               echo $fileName;
               break;
            fi
         else
            #matching file ext name
            if [ "${fileName##*.}"x = "$3"x ]; then
               # matching file name
               if [[ "${fileName%.*}" == $2* ]]; then
                  echo $fileName;
                  break;
               fi
            fi
         fi
      fi
   done
}

#Get the process id
#$1 filename
#$2 startup args
function getProcId()
{
   echo $(ps -ef|grep "$1"|grep " $2$\| $2 "|grep -v grep|awk '{print $2}');
}

#Get the process id
#$1 filename
#$2 startup args
function getProcIdTest()
{
   local pid=""
   local procList=$(ps -ef|grep "$1"|grep -v grep);

   IFS_old=$IFS
   IFS=$'\n'

   #echo $procList
   for proc in $procList;
   do
      IFS=$IFS_old
      local i=0

      for info in $proc;
      do
         let i=i+1

         if [ $i -eq 2 ] ; then
            tmp=$info
         fi

         if [ $i -ge 9 -a "$info"x == "$2"x ] ; then
            pid=$tmp
            echo $pid
            break
         fi
      done

      IFS=$'\n'

      if [ "$pid"x != ""x ] ; then
         break;
      fi
   done
   IFS=$IFS_old
}

#Get the common config of the plugin
#$1 shell file path
#$2 key
function getConfig()
{
   local configFile="$1/../../plugin.conf"
   local context=`cat $configFile`
   if [ $? == 0 ]; then
      context=`cat $configFile | grep "$2" | awk -F'=' '{ print $2 }' | sed s/[[:space:]]//g`
      echo "$context";
   fi
}

#startup plugin proc
#$1 works path
#$2 exec file path
#$3 httpname
#$4 other args
function startupPlugin()
{
   cd $1

   nohup $2 --__omhttpname=$3 $4 >/dev/null 2>&1 &
}
