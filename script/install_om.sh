#!/bin/bash

###########################################
# script parameter description:
# $1 for rootPath, ex: /opt/sequoiadb
# $2 for cm port
# $3 om svcName
# $4 om dbPath
###########################################

rootPath=$1
cmPort=$2
svcName=$3
dbPath=$4
restPort=$5
installer_pathname=$6

sdbFile=$1/bin/sdb

# first to start sdbcm
# $rootPath/bin/sdbcmart
# if [  $? != 0  ] ; then
#    echo "Start sdbcm failed"
#    exit 1
# fi

echo "rootPath=" $1
echo "cmPort=" $2
echo "svcName=" $3
echo "dbPath=" $4
echo "restPort=" $5
echo "installer_pathname=" $6

# second to create om
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

#check whether om is ok or not
if [  $? != 0  ] ; 
then
   echo "Create OM failed"
else
   echo "Create OM succeed"
fi

#copy installer file to packet dir
cp $6  $1/packet
