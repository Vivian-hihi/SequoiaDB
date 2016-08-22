/*******************************************************************************


   Copyright (C) 2011-2014 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = dmsStatus.hpp

   Descriptive Name = Data Management Service Header

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains structure for
   dms Reccord ID (RID).

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/08/2016  XJH Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef DMS_STATUS_HPP__
#define DMS_STATUS_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "ossIO.hpp"
#include "dms.hpp"
#include <vector>

using namespace std ;

namespace engine
{

   #define DMS_STATUS_FILE_NAME        ".SEQUOIADB_PERSIST"

   /*
      _dmsStatus define
   */
   class _dmsStatus : public SDBObject
   {
      public:
         _dmsStatus() ;
         virtual ~_dmsStatus() ;

      public:
         INT32    open( const CHAR *fileName ) ;
         void     close() ;
         INT32    remove() ;

      protected:
         INT32    _write( UINT32 offset, const CHAR *pData, UINT32 len ) ;
         INT32    _read( UINT32 offset, CHAR *pData, UINT32 len ) ;
   } ;
   typedef _dmsStatus dmsStatus ;

   /*
      Dms Status Item Type define
   */
   enum DMS_SI_TYPE
   {
      DMS_SI_CL      = 0,
      DMS_SI_CS
   } ;

   /*
      _dmsStatusItem define
   */
   struct _dmsStatusItem
   {
      DMS_SI_TYPE    _type ;
      CHAR           _name[ DMS_COLLECTION_FULL_NAME_SZ + 1 ] ;
      UINT32         _commitFlag ;
      UINT64         _commitLSN ;
      UINT64         _commitTime ;

      _dmsStatusItem()
      {
         _type       = DMS_SI_CL ;
         _name[0]    = 0 ;
         _commitFlag = 0 ;
         _commitLSN  = 0 ;
         _commitTime = 0 ;
      }
   } ;
   typedef _dmsStatusItem dmsStatusItem ;

   typedef vector< dmsStatusItem >        VEC_STATUS_ITEM ;

   /*
      _dmsPersistStatus define
   */
   class _dmsPersistStatus : public _dmsStatus
   {
      public:
         _dmsPersistStatus() ;
         virtual ~_dmsPersistStatus() ;

         INT32    removeItem( const CHAR *name ) ;

         INT32    addItem( const CHAR *name,
                           UINT32 commitFlag,
                           UINT64 commitLSN,
                           UINT64 commitTime ) ;

         BOOLEAN  findItem( const CHAR *name,
                            dmsStatusItem &item ) ;

         UINT32   findCLItemsByCS( const CHAR *name,
                                   VEC_STATUS_ITEM &vecItems ) ;

         UINT32   getCount() ;

      protected:
         
   } ;
   typedef _dmsPersistStatus dmsPersistStatus ;

}

#endif /* DMS_STATUS_HPP__ */

