#!/bin/bash

TYPE="unknown"
CLEAN_PG=0
CLEAN_DB=0
CLEAN_MYSQL=0

function clean_db()
{
   local install_dir=$1
   local filter=$2
   
   local datadir_list=`$install_dir/bin/sdblist -l | grep -v "Total"|awk 'NR>1{print $NF}'`
   echo "begin to uninstall $name"
   echo "$install_dir/uninstall --mode unattended"
   `$install_dir/uninstall --mode unattended`
   echo "ok"
   for datadir in $datadir_list
   do
      echo "rm -rf $datadir"
      rm -rf $datadir
   done
   echo "begin to clean install dir"
   echo "rm -rf $install_dir"
   rm -rf $install_dir
   `sed -i '/'$filter'/d' /etc/default/setup.list`
   echo "ok"
   return 0
}

function clean_sql()
{
   local install_dir=$1
   local filter=$2
      
   local datadir_list=`$install_dir/bin/sdb_sql_ctl listinst | grep -v "Total"|awk 'NR>1{print $2 " " $3}'`
   echo "begin to uninstall $name"
   echo "$install_dir/uninstall --mode unattended"
   `$install_dir/uninstall --mode unattended`
   echo "ok"
   for datadir in $datadir_list
   do
      echo "rm -rf $datadir"
      rm -rf $datadir
   done
   echo "begin to clean install dir"
   echo "rm -rf $install_dir"
   rm -rf $install_dir
   `sed -i '/'$filter'/d' /etc/default/setup.list`
   echo "ok"
   return 0
}

function ask_user()
{
   local install_dir=$1
   local name=$2
   local filter=$3
   read -p "clean $install_dir $name Y/n: " choice
   [ -z $choice ] && choice="Y"
   if [[ "$choice" == "Y" || "$choice" == "y" ]];
   then
      case $name in
         "sequoiadb")
                       echo "begin to clean db data"
                       clean_db $install_dir $filter
                       shift
                       ;;
         "sequoiasql-mysql" | "sequoiasql-postgresql")
                       echo "begin to clean sql data"
                       clean_sql $install_dir $filter
                       shift
                       ;;
         *)            echo "Internal error!"
                       exit 64
                       ;;
      esac
   fi
}

function clean_install()
{
   local name=$1
   local install_dir=$2
   local clean_all=$3
   local filter=$4
   
   if [ $clean_all == true ]; 
   then      
      case $name in
         "sequoiadb")
                       echo "begin to clean data"
                       clean_db $install_dir $filter
                       shift
                       ;;
         "sequoiasql-mysql" | "sequoiasql-postgresql")
                       clean_sql $install_dir $filter
                       shift
                       ;;
         *)            echo "Internal error!"
                       exit 64
                       ;;
      esac
      
   else
      case $name in
         "sequoiadb")
                       ask_user $install_dir $name $filter
                       shift
                       ;;
         "sequoiasql-mysql" | "sequoiasql-postgresql")
                       ask_user $install_dir $name $filter
                       shift
                       ;;
         *)            echo "Internal error!"
                       exit 64
                       ;;
      esac
   fi
   
}

function clean_install_by_name()
{
   local name=$1
   local clean_all=$2
   for installInfo in `cat /etc/default/setup.list | grep "$name"`
   do
      local file=`echo $installInfo |awk -F, '{print $1}'`
      local md5=`echo $installInfo |awk -F, '{print $2}'`
      local filter=`echo $installInfo |awk -F / '{print $4}'`
      . $file
      if [ $MD5 == $md5 ]; then
         clean_install $name $INSTALL_DIR $clean_all $filter
      fi
   done
}

function clean_all()
{
   clean_install_by_name "sequoiadb" true
   clean_install_by_name "sequoiasql-mysql" true
   clean_install_by_name "sequoiasql-postgresql" true
}

function build_help()
{
   echo ""
   echo "Usage:"
   echo "  --sdb        clean sequoiadb"
   echo "  --pg         clean sequoiasql-postgresql"
   echo "  --mysql      clean sequoiasql-mysql"
}

#Parse command line parameters
#test $# -eq 0 && { build_help && exit 64; }

ARGS=`getopt -o h --long help,sdb,pg,mysql -n 'test' -- "$@"`
ret=$?
test $ret -ne 0 && exit $ret

eval set -- "${ARGS}"

while true
do
   case "$1" in
      --sdb )          INSTALL_DB=1
                       TYPE="sequoiadb"
                       clean_install_by_name $TYPE false
                       shift
                       ;;
      --pg )           INSTALL_PG=1
                       TYPE="sequoiasql-postgresql"
                       clean_install_by_name $TYPE false
                       shift
                       ;;
      --mysql )        INSTALL_MYSQL=1
                       TYPE="sequoiasql-mysql"
                       clean_install_by_name $TYPE false
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

case "$TYPE" in
   unknown)             clean_all; shift 1;;
         *)             shift 1;;
esac