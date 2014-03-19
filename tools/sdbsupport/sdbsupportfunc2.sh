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
   tar -zcvf $HOST.tar.gz HARDINFO/ SDBNODES/ OSINFO/ SDBSNAPS/
   rm -rf HARDINFO/ SDBNODES/ OSINFO/ SDBSNAPS/
}

function sdbExpectScpHosts()
{
   HOST=$1
   localPath=$2
   PASSWD=$3

#scp -r root@$HOST:$localPath/$HOST.tar.gz ./

   /usr/local/bin/expect -c"
      set timeout 10 ;
      spawn scp -r admin@$HOST:$localPath/$HOST.tar.gz ./ ;
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


