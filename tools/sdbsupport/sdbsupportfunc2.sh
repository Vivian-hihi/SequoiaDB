#!/bin/bash

#check over the password is right or wrong
function sdbCheckPassword()
{
   HOST=$1
   PASSWD=$2

   /usr/local/bin/expect -c "
      set timeout 10 ;
      spawn ssh $USER@$HOST ;
      expect {
            \"*yes/no*\" ; {send \"yes\r\" ; exp_continue}
            \"assword\" ; {send \"$PASSWD\r\" ;
               expect {
                  \"denied\" ; {exit 5 ;}
                  \"*login*\" ; {send \"exit\r\" ;}
               }
            exp_continue}
            eof
            {
               send_user \"eof\n\" ;
            }
      }
                        " >>/dev/null 2>&1

}

#ssh host and run sdbsupport
function sdbExpectSshHosts()
{
   HOST=$1
   PASSWD=$2
   localPath=$3
   sdbsupport=$4
   timeout=$5
   #echo "timeout:$timeout"

   endflag="echo \"Too much time\""
   /usr/local/bin/expect -c   "
      set timeout $timeout ;
      spawn ssh $USER@$HOST ;
      expect {
         \"*yes/no*\";{send \"yes\n\";exp_continue}
         \"*assword\";{send \"$PASSWD\n\";exp_continue}
         \"*login*\";{send \"cd $localPath\r\n\";send \"chmod +x sdbsupport.sh\r\n\";send \"$sdbsupport\r\n\";send \"\r\n\";send \"mv sdbsupport.log sdbsupport.log.$HOST\r\n\";send \"exit\r\n\";exp_continue}
         timeout ;{exit 4;}
         eof
         {
           send_user \"eof\n\";
         }
      }
                              " >>/dev/null 2>&1

   rc=$?
   if [ "$rc" == "4" ] ; then
      echo "Run time out,please take too much time in host : $HOST"
      sdbEchoLog "ERROR" "$0/$HOST/${FUNCNAME}" "${LINENO}" "Run time out,please take too much time in host : $HOST"
   else
      echo "Success to run sdbsupport.sh in $HOST"
       sdbEchoLog "ERROR" "$0/$HOST/${FUNCNAME}" "${LINENO}" "Success to run sdbsupport"
   fi
}

function sdbTarGzPack()
{
   HOST=$1
   date=`date '+%y%m%d-%H%M%S'`
   hard="true"
   sdbnode="true"
   osinfo="true"
   sdbsnap="true"

   Folder="$HOST-$date"
#echo "Begin to packaging and compression"
   mkdir -p $Folder/
   if [ $? -ne 0 ] ; then
      echo "Failed to create foler !"
      sdbEchoLog "ERROR" "$0/$HOST/${FUNCNAME}" "${LINENO}" "Failed to create foler !"
      exit 1
   fi

   if ls HARDINFO/ >>/dev/null 2>&1
   then
      hard="false"
      mv HARDINFO/ ./$Folder/
   fi

   if ls SDBNODES/ >>/dev/null 2>&1
   then
      sdbnode="false"
      mv SDBNODES/ ./$Folder/
   fi

   if ls OSINFO/ >>/dev/null 2>&1
   then
      osinfo="false"
      mv OSINFO/ ./$Folder/
   fi

   if ls SDBSNAPS/ >>/dev/null 2>&1
   then
      sdbsnap="false"
      mv SDBSNAPS/ ./$Folder/
   fi

   if [ "$hard" == "true" ] && [ "$sdbnode" == "true" ] && [ "$osinfo" == "true" ] && [ "$sdbsnap" == "true" ] ; then
      echo "Error,Failed to collect $HOST information "
      sdbEchoLog "ERROR" "$0/$HOST/${FUNCNAME}" "${LINENO}" "Error,Failed to collect $HOST information "
      rm -rf ./$Folder/
      exit 1
   fi

   if [ $? -ne 0 ] ; then
      echo "Failed to move the collected information to folder"
      sdbEchoLog "ERROR" "$0/$HOST/${FUNCNAME}" "${LINENO}" "Error,Failed to move $HOST information to folder"
      exit 1
   fi

   tar -zcvf $Folder.tar.gz ./$Folder/ >>/dev/null 2>&1

   if [ $? -ne 0 ] ; then
      echo "Failed to packaging and compression"
      sdbEchoLog "ERROR" "$HOST/$0/${FUNCNAME}" "${LINENO}" "Failed to packaging and compression "
      exit 1
   else
      echo "Complete to packaging and compression"
      sdbEchoLog "EVENT" "$HOST/$0/${FUNCNAME}" "${LINENO}" "Success to Complete to packaging and compression"
   fi

   mv $Folder.tar.gz ./log
   if [ $? -ne 0 ] ; then
      echo "Failed to move to log folder."
   fi
   rm -rf ./$Folder/
}

function sdbExpectScpHosts()
{
   HOST=$1
   localPath=$2
   PASSWD=$3

#scp -r root@$HOST:$localPath/$HOST.tar.gz ./

   /usr/local/bin/expect -c"
      set timeout 80 ;
      spawn scp -r $USER@$HOST:$localPath/*$HOST*.tar.gz ./log/ ;
      expect {
         \"*yes/no*\";{send \"yes\n\";exp_continue}
         \"*assword\";{send \"$PASSWD\n\";exp_continue}
         timeout ;{exit 4;}
         eof
         {
            send_user \"eof\n\";
         }
      }
                              " >>/dev/null 2>&1
      if [ "$rc" == "4" ] ; then
         echo "Failed to copy $HOST:$localPath/*$HOST*.tar.gz"
         sdbEchoLog "EVENT" "$HOST/$0/${FUNCNAME}" "${LINENO}" "Failed to scp $USER@$HOST:$localPath/*$HOST*.tar.gz"
      else
         echo "Success to copy information from $HOST"
         sdbEchoLog "EVENT" "$HOST/$0/${FUNCNAME}" "${LINENO}" "Success to copy information from $HOST"
      fi

}

function sdbSupportLog()
{
   HOST=$1
   localPath=$2
   PASSWD=$3

   /usr/local/bin/expect -c"
         set timeout 80 ;
         spawn scp -r $USER@$HOST:$localPath/sdbsupport.log.$HOST ./log/ ;
         expect {
            \"*yes/no*\";{send \"yes\n\";exp_continue}
            \"*assword\";{send \"$PASSWD\n\";exp_continue}
            timeout ;{exit 4;}
            eof
            {
               send_user \"eof\n\";
            }
         }
                           " >>/dev/null 2>&1
   if [ "$rc" == "4" ] ; then
      echo "Failed to copy $HOST:$localPath/sdbsupport.log"
      sdbEchoLog "EVENT" "$HOST/$0/${FUNCNAME}" "${LINENO}" "Failed to scp $USER@$HOST:$localPath/sdbsupport.log"
   else
      echo "Success to copy sdbsupport.log"
      sdbEchoLog "EVENT" "$HOST/$0/${FUNCNAME}" "${LINENO}" "Success to copy sdbsupport.log"
   fi

}

function sdbSSHRemove()
{
   HOST=$1
   PASSWD=$2
   localPath=$3

   /usr/local/bin/expect -c"
      set timeout 50 ;
      spawn ssh $USER@$HOST ;
      expect {
         \"*yes/no*\";{send \"yes\n\";exp_continue}
         \"*assword\";{send \"$PASSWD\n\";exp_continue}
         \"*login*\";{send \"cd $localPath\r\n\";send \"rm *$HOST*.tar.gz sdbsupport.log.$HOST\r\n\";send \"\r\";send \"exit\r\" ;}
         eof
         {
           send_user \"eof\n\";
         }
      }

                           " >>/dev/null 2>&1

}

function sdbEchoLog()
{
   Date=`date +%Y-%m-%d-%H:%M:%S.%N`

   echo "Level: $1"
   echo "Date: $Date"
   echo "File/Function: $2"
   echo "Line: $3"
   echo "Message: "
   echo "$4"
   echo ""
} >> sdbsupport.log
