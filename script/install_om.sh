#!/bin/bash

###########################################
# script parameter description:
# $1 for rootPath, ex: /opt/sequoiadb
# $2 for cm port
# $3 om svcname
# $4 om dbpath
###########################################

rootPath=$1
cmPort=$2
svcname=$3
dbpath=$4

sdbFile=$1/bin/sdb

# first to start sdbcm
# $rootPath/bin/sdbcmart
# if [  $? != 0  ] ; then
#    echo "Start sdbcm failed"
#    exit 1
# fi

echo "rootPath=" $1
echo "cmPort=" $2
echo "svcname=" $3
echo "dbpath=" $4

# second to create om
$sdbFile -s " var oma = new Oma('localhost', '${cmPort}' ); \
             try { \
                 oma.createOM( '${svcname}', '${dbpath}' ) ; \
              } catch( e ) { \
                 if ( e != SDBCM_NODE_EXISTED ) { \
                    throw e ; } \
              } \
              try { \
                 oma.startNode( '${svcname}' ) ; \
              } catch( e ) { \
                 oma.removeOM( '${svcname}' ) ; \
                 throw e ; \
              } "
if [  $? != 0  ] ; 
then
   echo "Create OM failed"
   exit 1
else
   echo "Create OM succeed"
   exit 0
fi
