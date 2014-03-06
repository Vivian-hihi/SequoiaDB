#!/bin/bash
function sdbExpectSshHosts()
{
	HostPara=$1
	PASSWD=$2
	localPath=$3	
	sdbsupport=$4
	
	/usr/local/bin/expect -c   "
		set timeout 10 ;
		spawn ssh $HostPara ;
		expect {
			\"*yes/no*\";{send \"yes\r\";exp_continue}
			\"*assword\";{send \"$PASSWD\r\";exp_continue}
			\"*login*\";{send \"cd $localPath\r\";send \"chmod +x sdbsupport.sh\r\";send \"$sdbsupport\r\";send \"exit\r\";exp_continue}
			eof
			{
				send_user \"eof\r\";
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

#	scp -r root@$HOST:$localPath/$HOST.tar.gz ./

	/usr/local/bin/expect -c	"
		set timeout 10 ;
		spawn scp -r root@$HOST:$localPath/$HOST.tar.gz ./ ; 
		expect {
			\"*yes/no*\";{send \"yes\r\";exp_continue}
			\"*assword\";{send \"$PASSWD\r\";exp_continue}
			eof
			{
				send_user\"eof\r\";
			}
		}
											"

}


