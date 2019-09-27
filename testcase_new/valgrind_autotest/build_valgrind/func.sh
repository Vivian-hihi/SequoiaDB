#!/bin/bash

source ${curpath}/build.conf

host_address=(${host_address//,/ })
testcase_dir=(${testcase_dir//,/ })
runtype=(${runtype//,/ })

function help()
{
    echo ""
    echo "Command options:"
    echo "  -h [ --help ]    help"
}

function check_host()
{
    if [ ${#host_address[@]} -eq 0 ];then
        echo "host num cannot be ${host_address[*]}"
    fi

    regex="^192\.168\.[0-9]{1,3}\.[0-9]{1,3}$"
    for host in ${host_address[*]}
    do
        if [[ ${host} =~ $regex ]];then
            tmp_host=(${host//\./})
            [[ ${tmp_host[2]} -le 255 && ${tmp_host[3]} -le 255 ]]
            ret=$?
            if [ $ret -ne 0 ];then
                echo "host ${host} not the correct ip spelling"
                exit $ret
            fi
        else
            echo "host ${host} not the correct ip spelling"
            exit 4
        fi
    done
}

function ssh_copy_id()
{
    if [ ! -f ~/.ssh/id_rsa ];then
        ssh-keygen -t rsa -f ~/.ssh/id_rsa -P ""
    fi
    for host in ${host_address[*]}
    do
        echo "${host} request password:"
        ssh-copy-id -i ~/.ssh/id_rsa ${USER}@${host} >> /dev/null 2>&1
    done
}

function check_package()
{
    package=$(cd ${curpath}/package;ls sequoiadb-*-linux_x86_64-enterprise-installer* | sed '2,$d' 2>>/dev/null; cd ${curpath})
    if [ -z "${package}" ];then
        echo "cannot match any sequoiadb install package"
        exit 4
    fi
    if [ ! -f "${curpath}/package/${package}" ];then
        echo "${package} is not a file"
        exit 4
    fi
}

function cp_package()
{
    if [[ "${needcover}"M = "true"M || "${needinstall}"M = "true"M ]];then
        check_package
        package=$(echo "${package}" | sed 's/(/\\(/g' | sed 's/)/\\)/g' | sed 's/ /\\ /g')
        echo "prepare install package ${package}"
        eval chmod u+x ${curpath}/package/${package}
        for host in ${host_address[*]}
        do
            ssh ${USER}@${host} "mkdir -p ~/build_package;"
            ret=$(ssh ${USER}@${host} "test -f ~/build_package/${package} && echo true")
            if [ "${ret}"M != "true"M ];then
                eval scp ${curpath}/package/${package} ${USER}@${host}:~/build_package/ >>/dev/null 2>&1
            fi
        done
    else
        echo "do not need to install sequoiadb"
    fi
}

function get_sdb_install_dir()
{
    host=$1
    path=$(ssh ${USER}@${host} "cat /etc/default/sequoiadb" 2>>/dev/null | grep INSTALL_DIR | sed 's/INSTALL_DIR=//g')
    echo ${path}
}

function uninstall_sdb()
{
    path=$1
    host=$2
    echo -e "Y\n\n" | ssh ${USER}@${host} "${path}/uninstall" >> /dev/null
}

function rm_sdb()
{
    for host in ${host_address[*]}
    do
        sdbpath=$(get_sdb_install_dir ${host})
        if [ -z ${sdbpath} ];then
            echo "${host} cannot find sdb install dir"
        else
            echo "${host} sdb install dir ${sdbpath}"
            uninstall_sdb ${sdbpath} ${host}
            ssh ${USER}@${host} "rm -r ${sdbpath}"
        fi
    done
}

function install_sdb()
{
    if [[ "${needcover}"M = "true"M ]];then
        rm_sdb
    fi
    
    for host in ${host_address[*]}
    do
        ssh ${USER}@${host} "~/build_package/${package} --mode unattended" >>/dev/null
        sdbpath=$(get_sdb_install_dir ${host})
        if [ -z ${sdbpath} ];then
            echo "${host} install failed, can not find /etc/default/sequoiadb file"
        else
            echo "${host} install successed sdb info:"
            echo "------------------------------------"
            ssh ${USER}@${host} "${sdbpath}/bin/sdb -v"
            test $? -ne 0 && exit $?
            echo "------------------------------------"
        fi
    done
    check_need_deploy
}

function check_need_deploy()
{
    for host in ${host_address[*]}
    do
        sdbpath=$(get_sdb_install_dir ${host})
        nodenum=$(ssh ${USER}@${host} "${sdbpath}/bin/sdblist | grep Total | sed 's/Total:\ //g'")
        if [ 0 -ne ${nodenum} ];then
            echo "${host} has running nodes, total num: ${nodenum}"
            return
        else
            getnodeaddrs ${host}
            if [ -n ${nodeaddrs[*]}"" ];then
                echo "${host} has nodes, total info: ${nodeaddrs[*]}"
                return
            fi
        fi
    done
    stop_valgrind
    deploy
    check_sdb_ok
}

function deploy()
{
    hosts=""
    for host in ${host_address[*]}
    do
        hosts=${hosts}\"${host}\"\,
    done
    host=${host_address[0]}
    mvneedbins
    cmd=${curpath}/bin/sdb" -f "${curpath}/deploy.js" -e "\'"var sdb=true; var hosts=[${hosts}]"\'
    echo "deploy sdb commond: ${cmd}"
    eval ${cmd}
    test $? -ne 0 && exit $?
}

function getnodeaddrs()
{
    host=$1
    sdbpath=$(get_sdb_install_dir ${host})
    nodeaddrs=()
    count=0
    # nodeaddrs=$(ssh ${USER}@${host} "${sdbpath}/bin/sdblist -l" | sed '1d' | sed '$d' | awk '{print $2":"$3}')
    nodesconf=($(ssh ${USER}@${host} "ls ${sdbpath}/conf/local"))
    for nodeconf in ${nodesconf[*]}
    do
        nodeinfo=$(ssh ${USER}@${host} "cat ${sdbpath}/conf/local/${nodeconf}/sdb.conf | sed 's/ //g'")
        svcname=$(echo "${nodeinfo}" | grep svcname | sed 's/svcname=//g')
        nodetype=$(echo "${nodeinfo}" | grep role | sed 's/role=//g')
        nodeaddrs[${count}]=${svcname}":"${nodetype}
        count=$[${count}+1]
    done
}

function startvalgrind()
{
    stop_valgrind
    for host in ${host_address[*]}
    do
        getnodeaddrs ${host}
        nodeaddrs=(${nodeaddrs[*]})
        if [ 0 -eq ${#nodeaddrs[@]} ];then
            echo "${host} start node number is 0"
            exit 4
        fi
        sdbpath=$(get_sdb_install_dir ${host})
        ssh ${USER}@${host} "${sdbpath}/bin/sdbstop" >> /dev/null
        ret=""
        addrscount=${#nodeaddrs[@]}
        until [[ "true"M = "${ret}"M ]]
        do
            ret=$(check_valgrind_ok ${host} ${addrscount} "${nodeaddrs[*]}")
            echo "${host} start valgrind ${ret}"
            if [[ "true"M != "${ret}"M ]];then
                nodeaddrs=($ret)
            else
                break
            fi
            for addr in ${nodeaddrs[*]}
            do
                addr=(${addr//:/ })
                svcname=${addr[0]}
                nodetype=${addr[1]}
                ssh ${USER}@${host} "valgrind --error-limit=no --tool=memcheck --leak-check=full --log-file=${sdbpath}/database/${nodetype}/${svcname}/diaglog/valgrind.txt ${sdbpath}/bin/sequoiadb -c ${sdbpath}/conf/local/${svcname}/ >> /dev/null 2>&1 & disown"
            done
        done
    done
    check_sdb_ok
}

function check_valgrind_ok()
{
    host=$1
    expcount=$2
    expaddrs=($3)
    actaddrs=($(ssh ${USER}@${host} "ps -ef | grep valgrind" | grep -v grep | sed 's/^.*local\///g' | sed 's/\///g'))
    actcount=${#actaddrs[@]}
    if [ ${actcount} -eq ${expcount} ];then
        echo "true"
    else
        count=0
        for (( i=0; i<${#expaddrs[@]}; i++ ))
        do
            expaddrs=(${expaddrs[*]})
            tmpaddr=(${expaddrs[${i}]//:/ })
            svcname=${tmpaddr[0]}
            ret=$(array_contain_elem "${actaddrs[*]}" ${svcname})
            if [ "${ret}"M = "true"M ];then
                unset expaddrs[${i}]
                i=$[${i}-1]
            fi
        done
        echo "${expaddrs[*]}"
    fi
}

function array_contain_elem()
{
    array=($1)
    elem=$2
    for tmpelem in ${array[*]}
    do
        if [ "${elem}"M = "${tmpelem}"M ];then
            echo "true"
            return
        fi
    done
    echo "false"
}

function stop_valgrind()
{
    for host in ${host_address[*]}
    do
        valpids=($(ssh ${USER}@${host} "ps -ef | grep valgrind | grep -v grep" | awk '{print $2}'))
        for pid in ${valpids[*]}
        do
            echo "${host} stop valgrind node pid ${pid}"
            ssh ${USER}@${host} "kill -15 ${pid}"
        done
    done
}

function check_sdb_ok()
{
    mvneedbins
    for host in ${host_address}
    do
        cmd=${curpath}/bin/sdb" -f "${curpath}/checksdb.js" -e \"var host='${host}'\""
        eval ${cmd}
    done
}

function runtestcase()
{
    if [ -n ${runtype}"" ];then
        mvneedbins
    fi
    for run in ${runtype[*]}
    do
        if [ ${run}M = "js"M ];then
            run_jstest
        fi
        if [ ${run}M = "java"M ];then
            run_javatest
        fi
    done
}

function mvneedbins()
{
    host=${host_address[0]}
    sdbpath=$(get_sdb_install_dir ${host})
    mkdir -p ${curpath}/bin
    if [ ! -f ${curpath}/bin/sdb ];then
        scp ${USER}@${host}:${sdbpath}/bin/sdb ${curpath}/bin/ >> /dev/null
        chmod u+x ${curpath}/bin/sdb
    fi
    if [ ! -f ${curpath}/bin/sdbbp ];then
        scp ${USER}@${host}:${sdbpath}/bin/sdbbp ${curpath}/bin/ >> /dev/null
        chmod u+x ${curpath}/bin/sdbbp
    fi
}

function run_jstest()
{
    host=${host_address[0]}
    cd ${curpath}/js/
    reportdir=$(ls ${curpath}/js/ | grep "report" | grep "local")
    if [ -n ${reportdir}"" ];then
        if [ -d ${curpath}/js/${reportdir} ];then
            rm -r ${curpath}/js/${reportdir}
        fi
    fi
    if [ -f ${curpath}/js/sdbbp.log ];then
        rm ${curpath}/js/sdbbp.log
    fi
    for testdir in ${testcase_dir[*]}
    do
        if [ ${testdir:0:1}"" != "/" ];then
            testdir=$(cd ${curpath}/${testdir}; pwd)
        else
            testdir=$(cd ${testdir}; pwd)
        fi
        if [ -f ${curpath}/js/console.log ];then
            rm ${curpath}/js/console.log
        fi
        source ${curpath}/js/runtest.sh -p ${testdir} -s 0 -n 11810 -h ${host} -c 11820 >> console.log 2>&1
    done
}

function run_javatest()
{
    cd ${javapath}
    driver=$(get_sdbdriver)
    host=${host_address[0]}
    driver_version=$(echo ${driver} | sed 's/sequoiadb-driver-//g' | sed 's/.jar//g')
    if [ -f ${curpath}/java/console.log ];then
        rm ${curpath}/java/console.log
    fi
    mvn surefire-report:report -DxmlFileName=${testngxml} -Dsdbdriver=${driver_version} -DsdbdriverDir=${curpath}/java/ -DreportDir=${curpath}/java/output -DHOSTNAME=${host} -DSVCNAME=11810 -DESHOSTNAME=${host} -DESSVCNAME=9200 -DthreadexecutorDir=${curpath}/java/ -DSCRIPTDIR=${javapath}/script/ >> ${curpath}/java/console.log 2>&1
}

function get_sdbdriver()
{
    host=${host_address[0]}
    sdbpath=$(get_sdb_install_dir ${host})
    driver=($(ssh ${USER}@${host} "ls ${sdbpath}/java | grep sequoiadb-driver"))
    if [ -z ${driver} ];then
        echo "${host} do not have java driver"
        exit 4
    fi
    driver=${driver[0]}
    if [ -f ${sdbpath}/java/${driver} ];then
        scp ${USER}@${host}:${sdbpath}/java/${driver} ${curpath}/java/ >> /dev/null
    else
        echo "${host} ${sdbpath}/java/${driver} is not a file"
        exit 4
    fi
    echo ${driver}
}

function get_result()
{
    if [ -d ${curpath}/result ];then
        rm -r ${curpath}/result
    fi
    mkdir -p ${curpath}/result
    for host in ${host_address[*]}
    do
        sdbpath=$(get_sdb_install_dir ${host})
        nodetypes=($(ssh ${USER}@${host} "ls ${sdbpath}/database"))
        for nodetype in ${nodetypes[*]}
        do
            svcnames=($(ssh ${USER}@${host} "ls ${sdbpath}/database/${nodetype}"))
            for svcname in ${svcnames[*]}
            do
                ret=$(ssh ${USER}@${host} "if [ -f ${sdbpath}/database/${nodetype}/${svcname}/diaglog/valgrind.txt ];then echo true; fi")
                if [ "$ret"M = "true"M ];then
                    scp ${USER}@${host}:${sdbpath}/database/${nodetype}/${svcname}/diaglog/valgrind.txt ${curpath}/result/${host}_${nodetype}_${svcname}_valgrind.txt >> /dev/null
                fi
            done
        done
    done
}
