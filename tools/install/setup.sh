#!/bin/bash

TYPE="unknown"
INSTALL_PG=0
INSTALL_DB=0
INSTALL_MYSQL=0

function install()
{
   local name=$1
   echo "--------------------------begin to install $name-------------------------"
   for file_run in ./$name-*-installer.run;
   do
      echo "$file_run --mode text"
      chmod u+x $file_run
      $file_run --mode text
      test $? -ne 0 && { echo "ERROR: Fail to $file_run --mode text" >&2 && exit 1; }
   done
   echo "----------------------------end install $name----------------------------"
   echo
}

function ask_user()
{
   read -p "Install sequoiadb Y/n: " choice
   [ -z $choice ] && choice="Y"
   [[ "$choice" == "Y" || "$choice" == "y" ]] && install "sequoiadb"

   while :
   do
      read -p "Install 1:sequoiasql-mysql or 2:sequoiasql-postgresql, [1]: " select
      [ -z $select ] && select=1
      [[ "$select" == 1 || "$select" == 2 ]] && break
   done
   [[ "$select" == 1 ]] && install "sequoiasql-mysql" || install "sequoiasql-postgresql"
}

function build_help()
{
   echo ""
   echo "Usage:"
   echo "  --sdb        install sequoiadb"
   echo "  --pg         install sequoiasql-postgresql"
   echo "  --mysql      install sequoiasql-mysql"
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
      --sdb )           INSTALL_BD=1
                       TYPE="sequoiadb"
                       install $TYPE
                       shift
                       ;;
      --pg )           INSTALL_PG=1
                       TYPE="sequoiasql-postgresql"
                       install $TYPE
                       shift
                       ;;
      --mysql )        INSTALL_MYSQL=1
                       TYPE="sequoiasql-mysql"
                       install $TYPE
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
   unknown)           ask_user; shift 1;;
   *)             shift 1;;
esac