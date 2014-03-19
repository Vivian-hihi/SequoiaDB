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
   HostPara=$1
   PASSWD=$2
   localPath=$3
   sdbsupport=$4

   /usr/local/bin/expect -c   "
      set timeout 10 ;
      spawn ssh sdbadmin@$HostPara ;
      expect {
         \"*yes/no*\";{send \"yes\n\";exp_continue}
         \"*assword\";{send \"$PASSWD\n\";exp_continue}
         \"*login*\";{send \"cd $localPath\n\";send \"chmod +x sdbsupport.sh\n\";send \"$sdbsupport\n\";exp_continue}
         eof
         {
           send_user \"eof\n\";
         }
      }
                              "
}


function sdbTarGzPack()
{
   HOST=$1
   date=`date '+%m%d%y-%H%M%S'`

   Folder="$HOST-$date"

   mkdir -p $Folder/
   if [ $? -ne 0 ] ; then
      echo "Failed to create foler !"
      exit 1
   fi
   mv HARDINFO/ SDBNODES/ OSINFO/ SDBSNAPS/ ./$Folder/
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
      spawn scp -r sdbadmin@$HOST:$localPath/$HOST.tar.gz ./ ;
      expect {
         \"*yes/no*\";{send \"yes\n\";exp_continue}
         \"*assword\";{send \"$PASSWD\n\";exp_continue}
         eof
         {
            send_user \"eof\n\";
         }
      }
                              "

}


