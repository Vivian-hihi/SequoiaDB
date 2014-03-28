#!/bin/bash

#check over the password is right or wrong
function sdbCheckPassword()
{
   HOST=$1
   PASSWD=$2

   /usr/local/bin/expect -c "
      set timeout 10 ;
      spawn ssh sdbadmin@$HOST ;
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
                        "

}

#ssh host and run sdbsupport
function sdbExpectSshHosts()
{
   HOST=$1
   PASSWD=$2
   localPath=$3
   sdbsupport=$4
   timeout=$5
#   echo "timeout:$timeout"

   endflag="echo \"Too much time\""
   /usr/local/bin/expect -c   "
      set timeout $timeout ;
      spawn ssh sdbadmin@$HOST ;
      expect {
         \"*yes/no*\";{send \"yes\n\";exp_continue}
         \"*assword\";{send \"$PASSWD\n\";exp_continue}
         \"*login*\";{send \"cd $localPath\r\n\";send \"chmod +x sdbsupport.sh\r\n\";send \"$sdbsupport\r\n\";send \"exit\r\n\";exp_continue}
         timeout ;{exit 4;}
         eof
         {
           send_user \"eof\n\";
         }
      }
                              " >>sdbsupport.log

   rc=$?
   if [ "$rc" == "4" ] ; then
      echo "Run time out,please take too much time in host : $HOST"
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

   echo "sdbtar gz pack"

   Folder="$HOST-$date"

   mkdir -p $Folder/
   if [ $? -ne 0 ] ; then
      echo "Failed to create foler !"
      exit 1
   fi

   if ls HARDINFO/ 2>/dev/null
   then
      hard="false"
      mv HARDINFO/ ./$Folder/
   fi

   if ls SDBNODES/ 2>/dev/null
   then
      sdbnode="false"
      mv SDBNODES/ ./$Folder/
   fi

   if ls OSINFO/ 2>/dev/null
   then
      osinfo="false"
      mv OSINFO/ ./$Folder/
   fi

   if ls SDBSNAPS/ 2>/dev/null
   then
      sdbsnap="false"
      mv SDBSNAPS/ ./$Folder/
   fi

   echo "here"

   if [ "$hard" == "true" ] && [ "$sdbnode" == "true" ] && [ "$osinfo" == "true" ] && [ "$sdbsnap" == "true" ] ; then
      echo "Error,Failed to collect information "
      exit 1
   fi
   echo "here"
   if [ $? -ne 0 ] ; then
      echo "Failed to move the collected information to folder"
      exit 1
   fi

   tar -zcvf $Folder.tar.gz ./$Folder/

   if [ $? -ne 0 ] ; then
      echo "Failed to packaging and compression"
      exit 1
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
      set timeout 50 ;
      spawn scp -r sdbadmin@$HOST:$localPath/*$HOST*.tar.gz ./ ;
      expect {
         \"*yes/no*\";{send \"yes\n\";exp_continue}
         \"*assword\";{send \"$PASSWD\n\";exp_continue}
         eof
         {
            send_user \"eof\n\";
         }
      }
                              " >>sdbsupport.log


}

function sdbSSHRemove()
{
   HOST=$1
   PASSWD=$2
   localPath=$3

   /usr/local/bin/expect -c"
      set timeout 50 ;
      spawn ssh sdbadmin@$HOST ;
      expect {
         \"*yes/no*\";{send \"yes\n\";exp_continue}
         \"*assword\";{send \"$PASSWD\n\";exp_continue}
         \"*login*\";{send \"cd $localPath\r\n\";send \"rm *$HOST*.tar.gz\r\n\";send \"\r\";send \"exit\r\" ;}
         eof
         {
           send_user \"eof\n\";
         }
      }

                           " >>sdbsupport.log

}
