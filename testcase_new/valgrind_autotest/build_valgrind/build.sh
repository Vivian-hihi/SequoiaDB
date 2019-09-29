#!/bin/bash

curpath=$(cd `dirname $0`; pwd)
source ${curpath}/func.sh

args=`getopt -o h -l help -n 'build' -- "$@"`
test $? -ne 0 && exit $?
eval set -- "${args}"

while true
do
    case "$1" in
        -h | --help )     help
                          exit 0
                          ;;
        -- )              shift
                          break
                          ;;
        * )               echo "Unknown argument: $1"
                          exit 4
                          ;;
    esac
done

# Chechk that the IP address is spelled correctly
check_host
ssh_copy_id

# Send tar packages and install them on each host
if [[ "${needcover}"M = "true"M || "${needinstall}"M = "true"M ]];then
    cp_package
    install_sdb
fi

# Start valgrind and run testcase then stop
start_valgrind
runtestcase
stop_valgrind

# Get run valgrind result
get_result
