#!/bin/bash

function build_help()
{
   echo ""
   echo "Usage:"
   echo "  --sdb         deploy SequoiaDB"
   echo "  --mysql      deploy SequoiaSQL-MySQL"
   echo "  --pg         deploy SequoiaSQL-PostgreSQL"
}

DEPLOY_SDB=0
DEPLOY_MYSQL=0
DEPLOY_PG=0
HAS_PARAMETER=0

#Parse command line parameters
ARGS=`getopt -o h --long help,sdb,pg,mysql -n 'quickDeploy.sh' -- "$@"`
ret=$?
test $ret -ne 0 && exit $ret

eval set -- "${ARGS}"

while true
do
   case "$1" in
      --sdb )          DEPLOY_SDB=1
                       HAS_PARAMETER=1
                       shift
                       ;;
      --pg )           DEPLOY_PG=1
                       HAS_PARAMETER=1
                       shift
                       ;;
      --mysql )        DEPLOY_MYSQL=1
                       HAS_PARAMETER=1
                       shift
                       ;;
      -h | --help )    build_help
                       exit 0
                       ;;
      --)              shift
                       break
                       ;;
      *)               echo "Internal error!"
                       exit 64
                       ;;
   esac
done

if [ $HAS_PARAMETER -eq 0 ]; then
   DEPLOY_SDB=1
   DEPLOY_MYSQL=1
   DEPLOY_PG=1
fi

##################################
#    main entry
##################################

#get this script path
myPath=`dirname $0`
if [[ ${dir_name:0:1} != "/" ]]; then
   myPath=$(pwd)/$myPath  #relative path
else
   myPath=$myPath         #absolute path
fi

#get sdb shell path
sdbShellPath=$myPath/../../bin/sdb

#get quickDeploy.js path
jsFile=$myPath/quickDeploy.js

#generate parameters
jsParameter="'"

if [ $DEPLOY_SDB -eq 1 ]; then
   jsParameter=${jsParameter}"var sdb=true;"
fi

if [ $DEPLOY_MYSQL -eq 1 ]; then
   jsParameter=${jsParameter}"var mysql=true;"
fi

if [ $DEPLOY_PG -eq 1 ]; then
   jsParameter=${jsParameter}"var pg=true;"
fi

jsParameter=${jsParameter}"'"

#execute command
command=$sdbShellPath" -f "$jsFile" -e "$jsParameter
echo "Execute command: "$command
eval $command

