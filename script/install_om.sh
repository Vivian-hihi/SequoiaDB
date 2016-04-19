#!/bin/bash

##########################################################
# script parameter description:
# $1 install mode, ex: normal, ex: upgrade
# $2 root path of sdb, ex: /opt/sequoiadb
# $3 path and name of package, ex:/opt/sequoiadb-2.0.run
# $4 cm port
# $5 om svcName
# $6 om dbPath
# $7 rest port
##########################################################

installMode=$1
rootPath=$2
installer_pathname=$3
cmPort=$4
svcName=$5
dbPath=$6
restPort=$7

sdbFile=$2/bin/sdb

echo "installMode=" $1
echo "rootPath=" $2
echo "installer_pathname=" $3
echo "cmPort=" $4
echo "svcName=" $5
echo "dbPath=" $6
echo "restPort=" $7

# upgrade mode: only copy install package
if [  $installMode == "upgrade"  ] ; then
   packageNum=0
   packageNum=`find $rootPath/packet -name "*.run" | wc -l`
   if [  $packageNum -gt 0  ] ; then
      rm -rf $rootPath/packet/*
      cp $installer_pathname  $rootPath/packet   
   fi
   exit 0
fi

# normal mode: step 1 create om
$sdbFile -s " var _svcName = '${svcName}' ;                                              \
              var _dbPath = '${dbPath}' ;                                                \
              var _restPort = '${restPort}' ;                                            \
              var _cmPort = '${cmPort}' ;                                                \
              var arr = [] ;                                                             \
              var num = 0 ;                                                              \
              var canRemove = true ;                                                     \
              arr = Sdbtool.listNodes( {type:'om', mode:'local', expand:true} ) ;        \
              num = arr.size() ;                                                         \
              if ( num == 0 ) {                                                          \
                 arr = Sdbtool.listNodes( {type:'om', mode:'run', expand:true} ) ;       \
                 num = arr.size() ;                                                      \
              }                                                                          \
              if ( num < 0 || num >= 2 ) {                                               \
                 println( 'Error: there are ' + num + ' sdbom exist in localhost' ) ;    \
                 throw SDB_SYS ;                                                         \
              }                                                                          \
              if ( num == 1 ) {                                                          \
                 var obj = eval( '(' + arr.pos() + ')' ) ;                               \
                 _svcName = obj['svcname'] ;                                             \
                 _dbPath = obj['dbpath'] ;                                               \
                 _restPort = obj['omname'] ;                                             \
                 canRemove = false ;                                                     \
              }                                                                          \
              var oma = new Oma( '127.0.0.1', _cmPort ) ;                                \
              try {                                                                      \
                 oma.createOM( _svcName, _dbPath, {httpname: _restPort} ) ;              \
              } catch( e ) {                                                             \
                 if ( e == SDBCM_NODE_EXISTED ) {                                        \
                    println( 'Warning: sdbom has existed in localhost' ) ;               \
                 }                                                                       \
                 throw e ;                                                               \
              }                                                                          \
              try {                                                                      \
                  oma.startNode( _svcName ) ;                                            \
              } catch( e ) {                                                             \
                 if ( canRemove ) {                                                      \
                    oma.removeOM( _svcName ) ;                                           \
                 }                                                                       \
                 throw e ;                                                               \
              } "

# normal mode: step 2 check whether om is ok or not
if [  $? != 0  ] ; 
then
   echo "Create OM failed"
else
   echo "Create OM succeed"
fi

# normal mode: step 3 copy install package
cp $installer_pathname  $rootPath/packet
