/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = utilAccessDataLocalIO.cpp

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

#include "utilAccessData.hpp"

_utilAccessDataLocalIO::_utilAccessDataLocalIO()
{
}

_utilAccessDataLocalIO::~_utilAccessDataLocalIO()
{
   ossClose ( _fileIO ) ;
}

INT32 _utilAccessDataLocalIO::initialize( void *pParamet )
{
   INT32 rc = SDB_OK ;
   utilAccessParametLocalIO *temp = NULL ;
   if ( !pParamet )
   {
      rc = SDB_SYS ;
      goto error ;
   }
   temp = (utilAccessParametLocalIO*)pParamet ;

   rc = ossOpen ( temp->pFileName,
                  OSS_READONLY | OSS_SHAREREAD,
                  OSS_DEFAULTFILE,
                  _fileIO ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to open input file %s, rc = %d",
               temp->pFileName, rc ) ;
      goto error ;
   }
done:
   return rc ;
error:
   goto done ;
}

INT32 _utilAccessDataLocalIO::readNextBuffer ( CHAR *pBuffer, UINT32 &size )
{
   INT32 rc = SDB_OK ;
   SINT64 iLenRead = 0 ;
   UINT32 sourceSize = 0 ;
   sourceSize = size ;
   SDB_ASSERT ( pBuffer, "pBuffer can't be NULL" ) ;
   while ( size != 0 )
   {
      rc = ossRead ( &_fileIO, pBuffer, size, &iLenRead ) ;
      if ( rc && SDB_INTERRUPT != rc && SDB_EOF != rc )
      {
         PD_LOG ( PDERROR, "Failed to read from file, rc = %d", rc ) ;
         goto error ;
      }
      else if ( rc == SDB_EOF )
      {
         goto done ;
      }
      else
      {
         size -= iLenRead ;
      }
   }
done:
   size = sourceSize - size ;
   return rc ;
error:
   goto done ;
}