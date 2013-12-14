/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = oss.hpp

   Descriptive Name = Operating System Services Header

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains functions for OSS operations.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef OSS_HPP_
#define OSS_HPP_
#pragma warning( disable: 4290 )
#include "oss.h"
#include "ossMem.hpp"

// we should NEVER AND EVER put any virtual function in this object
// and we should never cast an object to SDBObject and delete.
class SDBObject
{
protected :
   // never instantiate SDBObject
   SDBObject () {}
public :
   // do NOT put virtual destructor because it will change the inherited object
   // size

   // regular new
   void * operator new ( size_t size ) throw ( const char * )
   {
      void *p = SDB_OSS_MALLOC(size) ;
      if ( !p ) throw "allocation failure" ;
      return p ;
   }

   void * operator new[] ( size_t size ) throw ( const char * )
   {
      void *p = SDB_OSS_MALLOC(size) ;
      if ( !p ) throw "allocation failure" ;
      return p ;
   }

   void operator delete ( void *p )
   {
      SDB_OSS_FREE(p) ;
   }

   void operator delete[] ( void *p )
   {
      SDB_OSS_FREE(p) ;
   }

   // new with file/line number
   void * operator new ( size_t size, const CHAR *pFile, UINT32 line )
         throw ( const char * )
   {
      void *p = SDB_OSS_MALLOC3(size, pFile, line ) ;
      if ( !p ) throw "allocation failure" ;
      return p ;
   }

   void * operator new[] ( size_t size, const CHAR *pFile, UINT32 line )
         throw ( const char * )
   {
      void *p = SDB_OSS_MALLOC3(size, pFile, line ) ;
      if ( !p ) throw "allocation failure" ;
      return p ;
   }

   void operator delete ( void *p, const CHAR *pFile, UINT32 line )
   {
      SDB_OSS_FREE(p) ;
   }

   void operator delete[] ( void *p, const CHAR *pFile, UINT32 line )
   {
      SDB_OSS_FREE(p) ;
   }

   // no throw
   void * operator new ( size_t size, const std::nothrow_t & )
   {
      return SDB_OSS_MALLOC(size) ;
   }

   void * operator new[] ( size_t size, const std::nothrow_t & )
   {
      return SDB_OSS_MALLOC(size) ;
   }

   void operator delete ( void *p, const std::nothrow_t & )
   {
      SDB_OSS_FREE(p) ;
   }

   void operator delete[] ( void *p, const std::nothrow_t & )
   {
      SDB_OSS_FREE(p) ;
   }

   // no throw with line number
   void * operator new ( size_t size, const CHAR *pFile,
                         UINT32 line, const std::nothrow_t & )
   {
      return SDB_OSS_MALLOC3(size, pFile, line ) ;
   }

   void * operator new[] ( size_t size, const CHAR *pFile,
                         UINT32 line, const std::nothrow_t & )
   {
      return SDB_OSS_MALLOC3(size, pFile, line ) ;
   }

   void operator delete ( void *p, const CHAR *pFile,
                          UINT32 line, const std::nothrow_t & )
   {
      SDB_OSS_FREE(p) ;
   }

   void operator delete[] ( void *p, const CHAR *pFile,
                            UINT32 line, const std::nothrow_t & )
   {
      SDB_OSS_FREE(p) ;
   }
} ;
typedef class SDBObject SDBObject ;

#endif // OSS_HPP_
