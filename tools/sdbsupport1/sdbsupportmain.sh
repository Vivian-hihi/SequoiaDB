#!/bin/bash
. ./sdbsupportfunc1.sh
#main shell function here 
function sdbPortMain()
{
	HOST=$1
	svcPORT=$2
	snapShot=$3

	group=$4
	context=$5
	session=$6
	collection=$7
	collectionspace=$8
	database=$9
	system=${10}	
	catalog=${11}

	#hardware information variable
	cpu=${12}
	memory=${13}
	disk=${14}
	netcard=${15}
	mainboard=${16}
	bios=${17}

	#operation system variable
	diskmanage=${18}
	osystem=${19}
	kermode=${20}
	env=${21}
	IDE=${22}
	network=${23}
	nfsstat=${24}
	progress=${25}
	login=${26}
	swapon=${27}
	limit=${28}
	vmstate=${29}
		
	hardInfo=${30}
	sysInfo=${31}
	
	confpath=/opt/sequoiadb/conf/local/
        ssh $HOST "ls $confpath > localnode"
        nodenum=`ssh $HOST "cat localnode|wc -l"`
	portnum=`awk 'BEGIN{print split("'$svcPORT'",svcArr,":")}'`
        for i in $(seq 1 $nodenum)
        do
                PORT[$i]=`ssh $HOST "sed -n ''$i'p' localnode"`
                DBPATH[$i]=`ssh $HOST "grep -E "dbpath" $confpath/${PORT[$i]}/sdb.conf|cut -d '=' -f 2"`
                ROLE[$i]=`ssh $HOST "grep -E "role=" $confpath/${PORT[$i]}/sdb.conf|cut -d '=' -f 2"`
		if [[ ${ROLE[$i]} = "coord" ]] && [[ $catalog = "true" ]] ; then 
			sdbSnapShotCataLog "$HOST" "${PORT[$i]}"
		fi 
		for k in $(seq 1 $portnum)
		do
			PORTPara[$k]=`awk 'BEGIN{split("'$svcPORT'",svcArr,":");print svcArr['$k']}'`	
			#Gather the specify node diaglog
			if [[ ${PORTPara[$k]} -eq ${PORT[$i]} ]] ; then
				sdbPortGather "$HOST" "${DBPATH[$i]}" "${PORT[$i]}"
				if [[ ${ROLE[$i]} = "coord" ]] && [[ $catalog = "true" ]] ; then
       		                         sdbSnapShotCataLog "$HOST" "${PORT[$i]}"
	                        fi
				if [[ $snapShot = "false" ]] ; then
					sdbSnapShotExtract "$HOST" "${PORT[$i]}" "$group" "$context" "$session" "$collection" "$collectionspace" "$database" "$system"
				else
					sdbSnapShot "$HOST" "${PORT[$i]}"
				fi	
				sdbHardSysInfo "${HOST[$j]}" "$hardInfo" "$sysInfo"
				sdbHardwareInfoPart "${HOST[$j]}" "$cpu" "$memory" "$disk" "$netcard" "$mainboard" "$bios"
				sdbSystemInfoPartFore "${HOST[$j]}" "$diskmanage" "$osystem" "$kermode" "$env" "$IDE" "$network"
				sdbSystemInfoPartEnd "${HOST[$j]}" "$nfsstat" "$progress" "$login" "$swapon" "$limit" "$vmstate"
			fi
		done 
 
		#collect all Para:--all  
		if [[ $svcPORT = "00000" ]] ; then
			sdbPortGather "$HOST" "${DBPATH[$i]}" "${PORT[$i]}"
			if [[ ${ROLE[$i]} = "coord" ]] && [[ $catalog = "true" ]] ; then
                        	sdbSnapShotCataLog "$HOST" "${PORT[$i]}"
                	fi 
			sdbSnapShot "$HOST" "${PORT[$i]}" 	
		fi 	

		if [[ $svcPORT = "" ]] ; then
			if [[ ${ROLE[$i]} = "coord" ]] && [[ $catalog = "true" ]] ; then
				 sdbSnapShotCataLog "$HOST" "${PORT[$i]}"
			fi
			if [[ $snapShot = "false" ]] ; then 	
				sdbSnapShotExtract "$HOST" "${PORT[$i]}" "$group" "$context" "$session" "$collection" "$collectionspace" "$database" "$system" 
			else
				sdbSnapShot "$HOST" "${PORT[$i]}"
			fi 
			sdbHardSysInfo "${HOST[$j]}" "$hardInfo" "$sysInfo"
			sdbHardwareInfoPart "${HOST[$j]}" "$cpu" "$memory" "$disk" "$netcard" "$mainboard" "$bios"
			sdbSystemInfoPartFore "${HOST[$j]}" "$diskmanage" "$osystem" "$kermode" "$env" "$IDE" "$network"
			sdbSystemInfoPartEnd "${HOST[$j]}" "$nfsstat" "$progress" "$login" "$swapon" "$limit" "$vmstate"
		fi  
#		echo ${PORT[$i]} ${DBPATH[$i]} ${ROLE[$i]}
        done
}

function sdbHardSysInfo()
{
	HOST=$1
	hardInfo=$2
	sysInfo=$3
	if [[ $hardInfo = "true" ]] ; then 
		sdbHardwareInfoAll "$HOST" 
	fi
	if [[ $sysInfo = "true" ]] ; then 
		sdbSystemInfoAll "$HOST" 	
	fi
	
}

