#bash

needUpdate=1
buildStr=""
autoTest=0
isRelease=0
needInstall=0
needCompile=1
startSdb=0
hostName=`hostname`
#curDir=`pwd`
homePath=`pwd`

# common function
function display()
{
   echo "$0 [-c compile] [-nocompile] [-release] [-noup] [-start] [-install] [-test] [-dbpath <db home path>]"
   exit $1
}

function svnUp()
{
   echo "====================Begin to remove auto-gen files========================"
   rm SequoiaDB/engine/include/*Trace.h
   rm SequoiaDB/engine/pd/pdFunctionList.cpp
   rm SequoiaDB/engine/oss/ossErr.cpp
   echo "====================End remove auto-gen fles=============================="

   sleep 1
   echo "====================Begin to update all files============================="
   svn up
   echo "====================End to update all files==============================="
}

function compile()
{
   sleep 1
   compileCmd="scons "
   if [ $isRelease -eq 0 ] ; then
      compileCmd=${compileCmd}" --dd"
   fi
   compileCmd=${compileCmd}" "${buildStr}

   echo "====================Begin to complie====================================="
   echo "CMD: $compileCmd"
   $compileCmd
   ret=$?
   if [ $ret -eq 0 ] ; then
      echo "====================Compile Succeed======================================"
   else
      echo "********************Compile Failed**************************************"
      exit 1
   fi
}

function installSdb()
{
   echo "==========================Start or Install======================"

   # stop cm
   ret=0
   while [ $ret -eq 0 ]
   do
      #outStr=`ps -ef|grep "sdbcm(" |grep -v grep`
      outStr=`bin/sdblist -t cm`
      ret=$?
      if [ $ret -eq 0 ] ; then
         echo "cm is running, stopping sdbcm..."
         #killall -e sdbcmd
         #killall -e sdbcm
         #sleep 4
         bin/sdbcmtop
      fi
   done

   # stop sequoiadb
   ret=0
   while [ $ret -eq 0 ]
   do
      #outStr=`ps -ef|grep "sequoiadb(" |grep -v grep`
      outStr=`bin/sdblist`
      ret=$?
      if [ $ret -eq 0 ] ; then
         echo "sequoiadb is running, stopping..."
         #killall -e sequoiadb
         #sleep 5
         bin/sdbstop
      fi
   done

   rm -f sdb.conf 2>/dev/null
   rm -f nohup 2>/dev/null

   # remove old sdb
   if [ ! -d "conf/local/30000" ] ; then
      needInstall=1
   fi

   if [ $needInstall -ne 0 ] ; then
      echo "Begin to remove sdb...."
      rm -r ${homePath}/17643 2>/dev/null
      rm -r ${homePath}/50000 2>/dev/null
      rm -r ${homePath}/30000 2>/dev/null
      rm -r ${homePath}/30010 2>/dev/null
      rm -r ${homePath}/30020 2>/dev/null
      rm -r ${homePath}/20000 2>/dev/null
      rm -r ${homePath}/40000 2>/dev/null
      rm -r ${homePath}/41000 2>/dev/null
      rm -r ${homePath}/42000 2>/dev/null
      rm -rf conf/local 2>/dev/null
      rm -rf conf/log 2>/dev/null
      mkdir conf/local
      echo "Remove sdb ok"
      sleep 2
   fi

   sleep 2

   # start sdb
   bin/sdbcmart
   if [ $? -eq 0 ] ; then
      echo "sdbcm start succeed"
   else
      echo "***********start sdbcm failed************"
      exit 1
   fi

   # install coord db1 and db2
   if [ $needInstall -ne 0 ] ; then
      # create coord and start coord
      bin/sdb -s " var oma = new Oma() ; oma.createCoord('17643', '${homePath}/17643') ; oma.startNode('17643') ; "
      # check coord
      bin/sdb -s " var db ; for ( var i=0; i < 60; ++i ) { try { db = new Sdb('localhost', '17643') ; break; } catch( e ) { sleep(1000) ; } } "
      if [ $? -eq 0 ] ; then
         echo "Coord is ok"
      else
         echo "Connect coord failed*******"
         exit 1
      fi
      rm -f sdb.conf
      echo "Start creating catalog"
      bin/sdb -s " db.createCataRG('${hostName}', '30000', '${homePath}/30000' ); sleep(5000);"
      echo "Creating catalog complete"
      bin/sdb -s " var dbcat ; for ( var i=0; i < 60; ++i ) { try { dbcat = new Sdb('localhost', '30000'); dbcat.close();  break ; } catch(e) { println( 'Failed: ' + e ) ; sleep(1000) ;} } "
      if [ $? -eq 0 ] ; then
         echo "Create Catalog RG Succeed"
      else
         echo "Create Catalog RG Failed********"
         exit 1
      fi
      echo "Start adding catalog nodes"
      bin/sdb -s " var cataRG = db.getRG('SYSCatalogGroup') ;"
      bin/sdb -s " node1 = cataRG.createNode('${hostName}', '30010', '${homePath}/30010' ) ; sleep(5000) ;"
      bin/sdb -s " node2 = cataRG.createNode('${hostName}', '30020', '${homePath}/30020' ) ; sleep(5000) ;"
      bin/sdb -s " cataRG.start() ; sleep(5000) ;"
      echo "Adding catalog nodes succeed"
      bin/sdb -s " var db ; try { db = new Sdb('localhost', '17643') ; var rg1=db.createRG('db1') ; rg1.createNode('${hostName}', '20000', '${homePath}/20000'); } catch( e) { println('Create db1 failed: ' + e ) ; throw e; } "
      if [ $? -eq 0 ] ; then
         echo "Create group db1 Succeed"
      else
         echo "Create group db1 Failed*********"
         exit 1
      fi
      bin/sdb -s " db.createRG('db2') ; rg2=db.getRG('db2');"
      bin/sdb -s " rg2.createNode('${hostName}', '40000', '${homePath}/40000');"
      bin/sdb -s " rg2.createNode('${hostName}', '41000', '${homePath}/41000');"
      bin/sdb -s " rg2.createNode('${hostName}', '42000', '${homePath}/42000');"
      bin/sdb -s " db.startRG('db1', 'db2') ; "
      if [ $? -eq 0 ] ; then
         echo "Create group db2 Succeed"
      else
         echo "Create group db2 Failed*******"
         exit 1
      fi

      echo "Start adding coordGroup, and remove virtual coord..."
	  bin/sdb -s " var coordRG = db.createCoordRG() ; coordRG.createNode( '${hostName}', '50000', '${homePath}/50000' ) ; "
	  bin/sdb -s " coordRG.start() ; oma.removeCoord( '17643' ) ; db.close() ; "
	  if [ $? -eq 0 ] ; then
	     echo "Create coord group succeed"
      else
	     echo "Create coord group failed*******"
		 exit 1
      fi

      echo "Install sdb succeed."
   else
      sleep 15
   fi

   # insure ok
   bin/sdb -s " var db ; for ( var i=0; i < 100; ++i) { try {db = new Sdb('localhost', '50000'); break ;} catch(e) {sleep(1000);} } db.close();"
   if [ $? -eq 0 ] ; then
      echo "Sequoiadb Start Suceed"
   else
      echo "Sequoiadb Start Failed******************"
      exit 1
   fi
}

function autoTest()
{
   sleep 10
   chmod 777 runtest.sh
   echo "=============================Begin to test usecases============================"
   ./runtest.sh -s 0
   echo "=============================End test usecases================================="
}

readType=0 # 1: compile
# read param
while [ "$1" != "" ]; do
   case $1 in
      -c )                shift
                          buildStr=${buildStr}" "$1
                          ;;
      -noup )             needUpdate=0
                          ;;
      -test )             autoTest=1
                          ;;
      -release )          isRelease=1
                          ;;
      -install )          needInstall=1
                          ;;
      -nocompile )        needCompile=0
                          ;;
      -start )            startSdb=1
                          ;;
      -dbpath )           shift
                          homePath=$1
                          ;;
      * )                 echo "Invalid argument: $p"
                          display 1
   esac
   shift
done
#for p in $@
#do
#   if [ $readType -eq 1 ] ; then
#      buildStr=${buildStr}" "$p
#      readType=0
#   elif [ "$p" == "-c" ] ; then
#      readType=1
#   elif [ "$p" == "-noup" ] ; then
#      needUpdate=0
#   elif [ "$p" == "-test" ] ; then
#      autoTest=1
#   elif [ "$p" == "-release" ] ; then
#      isRelease=1
#   elif [ "$p" == "-install" ] ; then
#      needInstall=1
#   elif [ "$p" == "-nocompile" ] ; then
#      needCompile=0
#   elif [ "$p" == "-start" ] ; then
#      startSdb=1
#   elif [ "$p" == "-p" ] ; then
#      homePath=
#   else
#      echo "Invalid argument: $p"
#      display 1
#   fi
#done

if [ $readType -ne 0 ] ; then
   echo "Invalid argument"
   display 1
fi

if [ "$buildStr" == "" ] ; then
   buildStr="--engine --tool --shell --fmp"
fi

# run entry

if [ $needUpdate -ne 0 ] ; then
   svnUp
fi

if [ $needCompile -ne 0 ] ; then
  compile 
fi

if [ $startSdb -ne 0 -o $needInstall -ne 0 ] ; then
   installSdb
fi

if [ $autoTest -ne 0 ] ; then
   autoTest
fi

