/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = ossHdfs.hpp

   Descriptive Name =

   When/how to use: parse Data util

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          10/30/2013  JW  Initial Draft

   Last Changed =

******************************************************************************/
#ifndef OSSHDFS_HPP_
#define OSSHDFS_HPP_

#define OSS_HDFS_RDONLY 1
#define OSS_HDFS_WRONLY 2

class _ossHdfs
{
private:
   void* _pHdfsFile ;
   void* _pHdfsFS ;
public:
   void* pFunctionAddress ;
public:
   int ossHdfsConnect( const char *hostName,
                       unsigned short port,
                       const char *user ) ;
   int ossHdfsDisconnect() ;
   int ossHdfsOpenFile( const char *path, int flags, int bufferSize,
                        short replication, int blocksize ) ;
   int ossHdfsCloseFile() ;
   int ossHdfsRead( char *buffer, int size, int &readSize ) ;
   _ossHdfs() ;
   ~_ossHdfs() ;
} ;
typedef class _ossHdfs ossHdfs ;

#endif