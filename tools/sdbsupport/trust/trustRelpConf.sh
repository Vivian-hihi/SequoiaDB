#!/bin/bash 
#config the trust relationship between the hosts 
#path :sequoiadb/tools/
declare -A PASSWD 
locPath=`pwd` 
roLen=`cat host.conf|wc -l` 
#echo $roLen
echo ""
echo "*******************WARNING***************WARNING********************"
echo "*    This program  should be run only at the request of"
echo "* SequoiaDB support. Please confirm the trust relationship whether"
echo "* you have config them.Otherwise,this script will delete you trust"
echo "* relationship!"
echo "*    This program will configure trust relationship with your"
echo "* specify hosts.Please make sure whether you needed!"
echo "********************************************************************"
echo ""
echo "You have 10 seconds to cancel this cript with Ctrl-C"
echo ""
sleep 10 
#insall expect tools
cd ../expect/ 
chmod a+x install.sh
./install.sh
cd ../trust

#get hosts/users/passwrds/
for i in $(seq 2 $roLen) 
do 
	USER[$i]=`sed -n ''$i'p' host.conf|cut -d ':' -f 2 ` 	
#	PASSWD[$i]=`sed -n ''$i'p' host.conf|cut -d ':' -f 3 ` 
	HOST[$i]=`sed -n ''$i'p' host.conf|cut -d ':' -f 3 ` 
#	echo $i ${USER[$i]} ${PASSWD[$i]} ${HOST[$i]} 
	#collect user's password ,put it in variable 
	echo "The host is ${HOST[$i]}"
	echo "Please input the password of ${HOST[$i]}:"
	read -s PASSWD[$i] 

done

for i in $(seq 2 $roLen)
do
	#create the id_rsa.pub in the every host of group 
	/usr/local/bin/expect -c	"
		set timeout 10 ;
		spawn ssh ${USER[$i]}@${HOST[$i]} ; 
		expect {
			\"*yes/no*\" ;{send \"yes\r\" ;exp_continue}
			\"assword\" ;{send \"${PASSWD[$i]}\r\" ; exp_continue}
			\"*login*\" ;{send \"ssh-keygen -t rsa\r\" ;exp_continue}
			\"*Enter file in which to save the key*\" ;{send \"\r\" ;exp_continue}
			\"Overwrite (y/n)\" ; {send \"y\r\" ;exp_continue}
			\"*Enter passphrase*\" ;{send \"\r\" ;exp_continue}
			\"*Enter same passphrase again*\" ;{send \"\r\" ;exp_continue}
			\"*The key's randomart image is*\" ;{send \"cd .ssh/\r\";send \"\r\" ; send \"mkdir -p cpfolder\r\" ;send \"hostname >cpfolder/hostfile.$i\r\" ;send \"cp id_rsa.pub cpfolder/authorized_keys.$i\r\" ;exp_continue}
			eof
			{
				send_user \"eof\n\" ; 
			}
		}
					"
done

echo "******************************trust relationship certificate OVER*************************************"
echo "******************************Copy authorized_keys to localhost***************************************"

#copy the authorized_keys to localhost
for i in $(seq 2 $roLen)
do
	 /usr/local/bin/expect -c    	"
		set timeout 10 ;
		spawn scp -r ${USER[$i]}@${HOST[$i]}:~/.ssh/cpfolder $locPath ;
		expect {
			\"assword\" ;{send \"${PASSWD[$i]}\n\";exp_continue}
			eof       
			{
				send_user \"eof\n\" ;
			}
		}
					"
	cat $locPath/cpfolder/authorized_keys.* > /root/.ssh/authorized_keys	
done 

echo "******************************Copy authorized_keys to localhost OVER*************************************"
echo "******************************Copy localhost authorized_keys to remote hosts******************************"

#copy the authorized_keys to other hosts 
for i in $(seq 2 $roLen)
do
	/usr/local/bin/expect -c	"
		set timeout 5 ;
		spawn scp -r /root/.ssh/authorized_keys ${USER[$i]}@${HOST[$i]}:~/.ssh/ ; 
		expect {
			\"yes\";{send \"yes\r\";exp_continue}
			\"assword\";{send \"${PASSWD[$i]}\n\";exp_continue}
			eof
			{	
				send_user \"eof\n\" ; 	
			}
		}
					"
done

echo "******************************Copy localhost authorized_keys to remote hosts OVER*************************"
echo "******************************SSH hosts and remove cpfolder***********************************************"

#ssh the hosts and remove cpfolder
for i in $(seq 2 $roLen)
do
	/usr/local/bin/expect -c 	"
		set timeout 5 ;
		spawn ssh ${USER[$i]}@${HOST[$i]} ;  
		expect {
			\"login\";{send \"\r\";send \"cd .ssh/\r\";send \"rm -rf cpfolder/\r\" ; send \"exit\r\";exp_continue}
			eof
			{
				send_user \"eof\n\" ;
			}
			
		}					
					"
	rm -rf ./cpfolder/
done   

echo "******************************SSH hosts and remove cpfolder OVER******************************************"

