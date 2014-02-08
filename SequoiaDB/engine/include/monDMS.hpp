/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = monDMS.hpp

   Descriptive Name = Monitor Data Management Service Header

   When/how to use: this program may be used on binary and text-formatted
   versions of monitoring component. This file contains structure for
   DMS information.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef MONDMS_HPP_
#define MONDMS_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "dms.hpp"
#include "ossUtil.hpp"
#include "../bson/bson.h"
#include "../bson/bsonobj.h"
#include <set>
#include <vector>
using namespace std ;
using namespace bson ;

namespace engine
{
   class _detailedInfo : public SDBObject
   {
   public :
      UINT32 _sequence ;
      UINT32 _numIndexes ;
      UINT16 _blockID ;
      UINT16 _flag ;
      UINT32 _logicID ;

      // stat info
      UINT64 _totalRecords ;
      UINT32 _totalDataPages ;
      UINT32 _totalIndexPages ;
      UINT64 _totalDataFreeSpace ;
      UINT64 _totalIndexFreeSpace ;
      // end

      inline BOOLEAN operator<(const _detailedInfo &r) const
      {
         return _sequence < r._sequence ;
      }

      _detailedInfo ()
      {
         _sequence            = 0 ;
         _numIndexes          = 0 ;
         _blockID             = 0 ;
         _flag                = 0 ;
         _flag                = 0 ;
         _logicID             = 0 ;

         _totalRecords        = 0 ;
         _totalDataPages      = 0 ;
         _totalIndexPages     = 0 ;
         _totalDataFreeSpace  = 0 ;
         _totalIndexFreeSpace = 0 ;
      }
   } ;
   typedef class _detailedInfo detailedInfo ;
   // for list collections
   class _monCollection : public SDBObject
   {
   public :
      CHAR _name [ DMS_COLLECTION_FULL_NAME_SZ + 1 ] ;
      std::set<detailedInfo> _details ;
      inline BOOLEAN operator<(const _monCollection &r) const
      {
         return ossStrncmp( _name, r._name, sizeof(_name))<0 ;
      }
      inline void addDetails ( UINT32 sequence, UINT32 numIndexes,
                               UINT16 blockID, UINT16 flag,
                               UINT32 logicID, UINT64 totalRecords,
                               UINT32 totalDataPages, UINT32 totalIndexPages,
                               UINT64 totalDataFreeSpace,
                               UINT64 totalIndexFreeSpace)
      {
         detailedInfo info ;
         info._sequence = sequence ;
         info._numIndexes = numIndexes ;
         info._blockID = blockID ;
         info._flag = flag ;
         info._logicID = logicID ;

         info._totalRecords        = totalRecords ;
         info._totalDataPages      = totalDataPages ;
         info._totalIndexPages     = totalIndexPages ;
         info._totalDataFreeSpace  = totalDataFreeSpace ;
         info._totalIndexFreeSpace = totalIndexFreeSpace ;

         _details.insert ( info ) ;
      }

   } ;
   typedef class _monCollection monCollection ;

   class _monCollectionSpace : public SDBObject
   {
   public :
      _monCollectionSpace ()
      {
         ossMemset ( _name, 0, sizeof(_name)) ;
         _collections.clear() ;
         _pageSize = 0 ;
      }
      _monCollectionSpace ( const _monCollectionSpace &right )
      {
         vector<CHAR *>::const_iterator it ;
         ossMemcpy ( _name, right._name, sizeof(_name) ) ;
         _pageSize = right._pageSize ;
         try
         {
            for ( it = right._collections.begin();
                  it != right._collections.end(); ++it )
            {
               CHAR *p = (CHAR*)SDB_OSS_MALLOC ( DMS_COLLECTION_NAME_SZ + 1 ) ;
               if ( p )
               {
                  ossMemcpy ( p, *it, DMS_COLLECTION_NAME_SZ + 1 ) ;
                  _collections.push_back ( p ) ;
               }
               else
               {
                  PD_LOG ( PDERROR, "Failed to allocate memory" ) ;
               }
            }
         }
         catch ( std::exception &e )
         {
            PD_LOG ( PDERROR,
                     "Exception happened during monCollectionSpace =: %s",
                     e.what() ) ;
         }
      }
      ~_monCollectionSpace()
      {
         vector<CHAR*>::iterator i ;
         for ( i = _collections.begin(); i != _collections.end(); ++i )
         {
            SDB_OSS_FREE ( *i ) ;
         }
         _collections.clear() ;
      }

      CHAR _name [ DMS_COLLECTION_SPACE_NAME_SZ + 1 ] ;
      vector<CHAR *> _collections ;
      INT32 _pageSize ;

      inline BOOLEAN operator<(const _monCollectionSpace &r) const
      {
         return ossStrncmp( _name, r._name, sizeof(_name))<0 ;
      }
      _monCollectionSpace &operator= (const _monCollectionSpace &right)
      {
         vector<CHAR *>::const_iterator it ;
         ossMemcpy ( _name, right._name, sizeof(_name) ) ;
         _pageSize = right._pageSize ;
         try
         {
            for ( it = right._collections.begin();
                  it != right._collections.end(); ++it )
            {
               CHAR *p = (CHAR*)SDB_OSS_MALLOC ( DMS_COLLECTION_NAME_SZ + 1 ) ;
               if ( p )
               {
                  ossMemcpy ( p, *it, DMS_COLLECTION_NAME_SZ + 1 ) ;
                  _collections.push_back ( p ) ;
               }
               else
               {
                  PD_LOG ( PDERROR, "Failed to allocate memory" ) ;
               }
            }
         }
         catch ( std::exception &e )
         {
            PD_LOG ( PDERROR,
                     "Exception happened during monCollectionSpace =: %s",
                     e.what() ) ;
         }
         return *this ;
      }
   } ;
   typedef class _monCollectionSpace monCollectionSpace ;

   class _monStorageUnit : public SDBObject
   {
   public :
      CHAR _name [ DMS_COLLECTION_SPACE_NAME_SZ + 1 ] ;
      dmsStorageUnitID _CSID ;
      UINT32 _logicalCSID ;
      SINT32 _pageSize ;
      SINT32 _sequence ;
      SINT32 _numCollections ;
      SINT32 _collectionHWM ;
      SINT64 _size ;
      inline BOOLEAN operator<(const _monStorageUnit &r) const
      {
         SINT32 rc = ossStrncmp( _name, r._name, sizeof(_name))<0 ;
         // if two storage unit got same name, let's check sequence
         if ( !rc )
            return _sequence < r._sequence ;
         return rc ;
      }
   } ;
   typedef class _monStorageUnit monStorageUnit ;

   // for indexes
   class _monIndex : public SDBObject
   {
   public:
      UINT16         _indexFlag ;
      CHAR           _version ;
      dmsExtentID    _scanExtLID ;
      BSONObj        _indexDef ;
   } ;
   typedef _monIndex monIndex ;

}

#endif //MONDMS_HPP_

