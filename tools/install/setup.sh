#!/bin/bash

TYPE="unknown"

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
      local SETUP_CONF="/etc/default/setup.list"
      test -f $SETUP_CONF || touch $SETUP_CONF
      chmod 755 $SETUP_CONF
      case $name in
         "sequoiadb" )              . "/etc/default/sequoiadb"
                                    sed -i '/\/etc\/default\/sequoiadb,\s*/d' $SETUP_CONF
                                    echo '/etc/default/sequoiadb,'$MD5'' >> $SETUP_CONF
                                    shift
                                    ;;
         "sequoiasql-mysql" )       file=`ls -l -t /etc/default/sequoiasql-mysq* | awk 'NR<2{print $NF}' | awk -F '/' '{print $4}'`
                                    . "/etc/default/$file"
                                    sed -i '/\/etc\/default\/'$file',\s*/d' $SETUP_CONF
                                    echo '/etc/default/'$file','$MD5'' >> $SETUP_CONF
                                    shift
                                    ;;  
         "sequoiasql-postgresql" )  file=`ls -l -t /etc/default/sequoiasql-postgresql* | awk 'NR<2{print $NF}' | awk -F '/' '{print $4}'`
                                    . "/etc/default/$file"
                                    sed -i '/\/etc\/default\/'$file',\s*/d' $SETUP_CONF
                                    echo '/etc/default/'$file','$MD5'' >> $SETUP_CONF
                                    shift
                                    ;;  
      esac
   done
   echo "----------------------------end install $name----------------------------"
   echo 
}

function ask_user()
{
   read -p "Install sequoiadb Y/n: " choice
   [ -z $choice ] && choice="Y"
   [[ "$choice" == "Y" || "$choice" == "y" ]] && install "sequoiadb"
   
   local file_exist=false
   test -f "/etc/default/sequoiadb" && file_exist=true
   if [ $file_exist == true ]; then
      . "/etc/default/sequoiadb"
      for file_run in ./sequoiasql-*.run;
      do
         cp $file_run $INSTALL_DIR/packet
      done
      chmod u+x $INSTALL_DIR/packet/sequoiasql-*.run
   fi
   
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
      --sdb )          TYPE="sequoiadb"
                       install $TYPE
                       shift
                       ;;
      --pg )           TYPE="sequoiasql-postgresql"
                       install $TYPE
                       shift
                       ;;
      --mysql )        TYPE="sequoiasql-mysql"
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