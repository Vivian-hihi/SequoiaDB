/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = dmsStorageLoadExtent.hpp

   Descriptive Name =

   When/how to use: load extent to database

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/10/2013  JW  Initial Draft

   Last Changed =

******************************************************************************/

#ifndef DMSSTORAGE_LOADEXTENT_HPP_
#define DMSSTORAGE_LOADEXTENT_HPP_

#include "dmsStorageUnit.hpp"
#include "pmdEDUMgr.hpp"

namespace engine
{
   class migMaster ;
   class dmsStorageLoadOp : public SDBObject
   {
   private:
      CHAR           *_pCurrentExtent ;
      INT32           _currentExtentSize ;
      dmsExtent      *_currentExtent ;
      dmsStorageUnit *_su ;
      SINT32          _pageSize ;

   private:
      void  _initExtentHeader ( dmsExtent *extAddr, UINT16 numPages ) ;
      INT32 _allocateExtent   ( INT32 requestSize ) ;

   public:
      dmsStorageLoadOp( ) : _pCurrentExtent(NULL),
                            _currentExtentSize(0),
                            _currentExtent(NULL),
                            _su(NULL)
      {
      }

      ~dmsStorageLoadOp()
      {
         SAFE_OSS_FREE ( _pCurrentExtent ) ;
      }

      // Flag Load
      BOOLEAN isFlagLoad ( dmsMB *mb )
      {
         return DMS_IS_MB_LOAD ( mb->_flag ) ;
      }

      void setFlagLoad ( dmsMB *mb )
      {
         DMS_SET_MB_LOAD ( mb->_flag ) ;
      }

      void clearFlagLoad ( dmsMB *mb )
      {
         OSS_BIT_CLEAR ( mb->_flag, DMS_MB_FLAG_LOAD ) ;
      }

      // Flag Load Load
      BOOLEAN isFlagLoadLoad ( dmsMB *mb )
      {
         return DMS_IS_MB_FLAG_LOAD_LOAD ( mb->_flag ) ;
      }

      void setFlagLoadLoad ( dmsMB *mb )
      {
         DMS_SET_MB_FLAG_LOAD_LOAD ( mb->_flag ) ;
      }

      void clearFlagLoadLoad ( dmsMB *mb )
      {
         OSS_BIT_CLEAR ( mb->_flag, DMS_MB_FLAG_LOAD_LOAD ) ;
      }

      // Flag Load Build
      BOOLEAN isFlagLoadBuild ( dmsMB *mb )
      {
         return DMS_IS_MB_FLAG_LOAD_BUILD ( mb->_flag ) ;
      }

      void setFlagLoadBuild ( dmsMB *mb )
      {
         DMS_SET_MB_FLAG_LOAD_BUILD ( mb->_flag ) ;
      }

      void clearFlagLoadBuild ( dmsMB *mb )
      {
         OSS_BIT_CLEAR ( mb->_flag, DMS_MB_FLAG_LOAD_BUILD ) ;
      }

      void init ( dmsStorageUnit *su )
      {
         SDB_ASSERT ( su, "su is NULL" )
         _su = su ;
         _pageSize = _su->getPageSize() ;
      }

      INT32 pushToTempDataBlock ( dmsMBContext *mbContext,
                                  BSONObj &record,
                                  BOOLEAN isLast,
                                  BOOLEAN isAsynchr ) ;

      INT32 loadBuildPhase ( dmsMBContext *mbContext,
                             pmdEDUCB *cb,
                             BOOLEAN isAsynchr = FALSE,
                             migMaster *pMaster = NULL,
                             UINT32 *success = NULL,
                             UINT32 *failure = NULL ) ;

      INT32 loadRollbackPhase ( dmsMBContext *mbContext ) ;

   } ;

}

#endif
