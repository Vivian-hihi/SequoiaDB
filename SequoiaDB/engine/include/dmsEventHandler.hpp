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

   Source File Name = dmsEventHandler.hpp

   Descriptive Name = Data Management Service Event Handler Header

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains structure for
   DMS event handler.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =

*******************************************************************************/
#ifndef DMS_EVENTHANDLER_HPP_
#define DMS_EVENTHANDLER_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "dms.hpp"
#include "ossUtil.hpp"
#include "ossMem.hpp"
#include "pmd.hpp"
#include "dpsLogWrapper.hpp"
#include "utilList.hpp"
#include "utilSUCache.hpp"

namespace engine
{

   enum DMS_CACHE_TYPE
   {
      DMS_CACHE_TYPE_STAT = 0,
      DMS_CACHE_TYPE_NUM,
      DMS_CACHE_TYPE_INVALID,
   } ;

   class _IDmsEventHolder ;
   class _IUtilSUCacheHolder ;

   #define DMS_EVENT_MASK_ALL    0xFFFFFFFF
   #define DMS_EVENT_MASK_STAT   0x00000001

   typedef struct _dmsCLItem
   {
      _dmsCLItem ()
      {
         _pCLName = NULL ;
         _mbID = DMS_INVALID_MBID ;
         _clLID = DMS_INVALID_CLID ;
      }

      _dmsCLItem ( const CHAR *pCLName, UINT16 mbID, UINT32 clLID )
      {
         _pCLName = pCLName ;
         _mbID = mbID ;
         clLID = clLID ;
      }

      const CHAR *   _pCLName ;
      UINT16         _mbID ;
      UINT32         _clLID ;
   } dmsCLItem ;

   typedef struct _dmsIdxItem
   {
      _dmsIdxItem ()
      {
         _pIdxName = NULL ;
      }

      _dmsIdxItem ( const CHAR *pIdxName, const BSONObj &boDefine )
      {
         _pIdxName = pIdxName ;
         _boDefine = boDefine ;
      }

      const CHAR *   _pIdxName ;
      BSONObj        _boDefine ;
   } dmsIdxItem ;

   /*
      _IDmsEventHandler
    */
   class _IDmsEventHandler
   {
      public :
         _IDmsEventHandler () {}

         virtual ~_IDmsEventHandler () {}

         OSS_INLINE virtual INT32 onCreateCS ( _IDmsEventHolder *pEventHolder,
                                               _IUtilSUCacheHolder *pCacheHolder,
                                               pmdEDUCB *cb,
                                               SDB_DPSCB *dpsCB )
         {
            return SDB_OK ;
         }

         OSS_INLINE virtual INT32 onLoadCS ( _IDmsEventHolder *pEventHolder,
                                             _IUtilSUCacheHolder *pCacheHolder,
                                             pmdEDUCB *cb,
                                             SDB_DPSCB *dpsCB )
         {
            return SDB_OK ;
         }

         OSS_INLINE virtual INT32 onUnloadCS ( _IDmsEventHolder *pEventHolder,
                                               _IUtilSUCacheHolder *pCacheHolder,
                                               pmdEDUCB *cb,
                                               SDB_DPSCB *dpsCB )
         {
            return SDB_OK ;
         }

         OSS_INLINE virtual INT32 onRenameCS ( _IDmsEventHolder *pEventHolder,
                                               _IUtilSUCacheHolder *pCacheHolder,
                                               const CHAR *pOldCSName,
                                               const CHAR *pNewCSName,
                                               pmdEDUCB *cb,
                                               SDB_DPSCB *dpsCB )
         {
            return SDB_OK ;
         }

         OSS_INLINE virtual INT32 onDropCS ( _IDmsEventHolder *pEventHolder,
                                             _IUtilSUCacheHolder *pCacheHolder,
                                             pmdEDUCB *cb,
                                             SDB_DPSCB *dpsCB )
         {
            return SDB_OK ;
         }

         OSS_INLINE virtual INT32 onCreateCL ( _IDmsEventHolder *pEventHolder,
                                               _IUtilSUCacheHolder *pCacheHolder,
                                               const dmsCLItem &clItem,
                                               pmdEDUCB *cb,
                                               SDB_DPSCB *dpsCB )
         {
            return SDB_OK ;
         }

         OSS_INLINE virtual INT32 onRenameCL ( _IDmsEventHolder *pEventHolder,
                                               _IUtilSUCacheHolder *pCacheHolder,
                                               const dmsCLItem &clItem,
                                               const CHAR *pNewCLName,
                                               pmdEDUCB *cb,
                                               SDB_DPSCB *dpsCB )
         {
            return SDB_OK ;
         }

         OSS_INLINE virtual INT32 onTruncateCL ( _IDmsEventHolder *pEventHolder,
                                                 _IUtilSUCacheHolder *pCacheHolder,
                                                 const dmsCLItem &clItem,
                                                 pmdEDUCB *cb,
                                                 SDB_DPSCB *dpsCB )
         {
            return SDB_OK ;
         }

         OSS_INLINE virtual INT32 onDropCL ( _IDmsEventHolder *pEventHolder,
                                             _IUtilSUCacheHolder *pCacheHolder,
                                             const dmsCLItem &clItem,
                                             pmdEDUCB *cb,
                                             SDB_DPSCB *dpsCB )
         {
            return SDB_OK ;
         }

         OSS_INLINE virtual INT32 onCreateIndex ( _IDmsEventHolder *pEventHolder,
                                                  _IUtilSUCacheHolder *pCacheHolder,
                                                  const dmsCLItem &clItem,
                                                  const dmsIdxItem &idxItem,
                                                  pmdEDUCB *cb,
                                                  SDB_DPSCB *dpsCB )
         {
            return SDB_OK ;
         }

         OSS_INLINE virtual INT32 onDropIndex ( _IDmsEventHolder *pEventHolder,
                                                _IUtilSUCacheHolder *pCacheHolder,
                                                const dmsCLItem &clItem,
                                                const dmsIdxItem &idxItem,
                                                pmdEDUCB *cb,
                                                SDB_DPSCB *dpsCB )
         {
            return SDB_OK ;
         }

         OSS_INLINE virtual INT32 onLinkCL ( _IDmsEventHolder *pEventHolder,
                                             _IUtilSUCacheHolder *pCacheHolder,
                                             const dmsCLItem &clItem,
                                             const CHAR *pMainCLName,
                                             pmdEDUCB *cb, SDB_DPSCB *dpsCB )
         {
            return SDB_OK ;
         }

         OSS_INLINE virtual INT32 onUnlinkCL ( _IDmsEventHolder *pEventHolder,
                                               _IUtilSUCacheHolder *pCacheHolder,
                                               const dmsCLItem &clItem,
                                               const CHAR *pMainCLName,
                                               pmdEDUCB *cb,
                                               SDB_DPSCB *dpsCB )
         {
            return SDB_OK ;
         }

         virtual UINT32 getMask () = 0 ;
   } ;

   /*
      _IDmsEventHolder
    */
   class _IDmsEventHolder
   {
      public :
         _IDmsEventHolder () {}

         virtual ~_IDmsEventHolder () {}

         virtual void regHandler ( _IDmsEventHandler *pHandler ) = 0 ;

         virtual void unregHandler ( _IDmsEventHandler *pHandler ) = 0 ;

         virtual void unregAllHandlers () = 0 ;

         virtual INT32 onCreateCS ( UINT32 mask, pmdEDUCB *cb, SDB_DPSCB *dpsCB ) = 0 ;

         virtual INT32 onLoadCS ( UINT32 mask, pmdEDUCB *cb, SDB_DPSCB *dpsCB ) = 0 ;

         virtual INT32 onUnloadCS ( UINT32 mask, pmdEDUCB *cb, SDB_DPSCB *dpsCB ) = 0 ;

         virtual INT32 onRenameCS ( UINT32 mask, const CHAR *pOldCSName,
                                    const CHAR *pNewCSName,
                                    pmdEDUCB *cb, SDB_DPSCB *dpsCB ) = 0 ;

         virtual INT32 onDropCS ( UINT32 mask, pmdEDUCB *cb, SDB_DPSCB *dpsCB ) = 0 ;

         virtual INT32 onCreateCL ( UINT32 mask, const dmsCLItem &clItem,
                                    pmdEDUCB *cb, SDB_DPSCB *dpsCB ) = 0 ;

         virtual INT32 onRenameCL ( UINT32 mask, const dmsCLItem &clItem,
                                    const CHAR *pNewCLName,
                                    pmdEDUCB *cb, SDB_DPSCB *dpsCB ) = 0 ;

         virtual INT32 onTruncateCL ( UINT32 mask, const dmsCLItem &clItem,
                                      pmdEDUCB *cb, SDB_DPSCB *dpsCB ) = 0 ;

         virtual INT32 onDropCL ( UINT32 mask, const dmsCLItem &clItem,
                                  pmdEDUCB *cb, SDB_DPSCB *dpsCB ) = 0 ;

         virtual INT32 onCreateIndex ( UINT32 mask, const dmsCLItem &clItem,
                                       const dmsIdxItem &idxItem, pmdEDUCB *cb,
                                       SDB_DPSCB *dpsCB ) = 0 ;

         virtual INT32 onDropIndex ( UINT32 mask, const dmsCLItem &clItem,
                                     const dmsIdxItem &idxItem, pmdEDUCB *cb,
                                     SDB_DPSCB *dpsCB ) = 0 ;

         virtual INT32 onLinkCL ( UINT32 mask, const dmsCLItem &clItem,
                                  const CHAR *pMainCLName, pmdEDUCB *cb,
                                  SDB_DPSCB *dpsCB ) = 0 ;

         virtual INT32 onUnlinkCL ( UINT32 mask, const dmsCLItem &clItem,
                                    const CHAR *pMainCLName, pmdEDUCB *cb,
                                    SDB_DPSCB *dpsCB ) = 0 ;

         virtual const CHAR *getCSName () const = 0 ;
   } ;

}

#endif //DMS_EVENTHANDLER_HPP_

