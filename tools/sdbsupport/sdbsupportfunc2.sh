#!/bin/bash

#check over the password is right or wrong
function sdbCheckPassword()
{
   HOST=$1
   PASSWD=$2

   /usr/local/bin/expect -c   "
      set timeout 4;
      spawn ssh sdbadmin@$HOST;
      expect {
         \"*yes/no*\" ;{send \"yes\r\" ;exp_continue}
         \"assword\" ;
         {send \"$PASSWD\r\" ;
            expect {
               \"denied\" ; {exit 5;}
            }
         exp_continue}
         eof
         {
            send_user \"eof\" ;
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

   /usr/local/bin/expect -c   "
      set timeout 10 ;
      spawn ssh sdbadmin@$HOST ;
      expect {
         \"*yes/no*\";{send \"yes\n\";exp_continue}
         \"*assword\";{send \"$PASSWD\n\";exp_continue}
         \"*login*\";{send \"cd $localPath\n\";
                      send \"chmod +x sdbsupport.sh\n\";
                      send \"$sdbsupport\n\";
         exp_continue}
         eof
         {
           send_user \"eof\n\";
         }
      }
                              " >>sdbsupport.log
   rc=$?
   if [ "$rc" == "4" ] ; then
      echo "No such file or directory"
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
      set timeout 10 ;
      spawn scp -r sdbadmin@$HOST:$localPath/$HOST*.tar.gz ./ ;
      expect {
         \"*yes/no*\";{send \"yes\n\";exp_continue}
         \"*assword\";{send \"$PASSWD\n\";
         exp_continue}
         eof
         {
            send_user \"eof\n\";
         }
      }
                              " >>sdbsupport.log

   rc=$?
   if [ "$rc" == "4" ] ; then
      echo "Failed to collet host:$HOST's information"
   fi

}

function sdbSSHRemove()
{
   HOST=$1
   PASSWD=$2
   localPath=$3

   /usr/local/bin/expect -c"
      set timeout 10 ;
      spawn ssh sdbadmin@$HOST ;
      expect {
         \"*yes/no*\";{send \"yes\n\";exp_continue}
         \"*assword\";{send \"$PASSWD\n\";exp_continue}
         \"*login*\";{send \"cd $localPath\n\";
                      send \"rm  $HOST*.tar.gz\n\";
         exp_continue}
         eof
         {
           send_user \"eof\n\";
         }
      }

                           " >>sdbsupport.log
   rc=$?
   if [ "$rc" == "4" ] ; then
      echo "Failed to clean host:$HOST "
   fi

}
