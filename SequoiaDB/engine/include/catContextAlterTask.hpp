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

   Source File Name = catContextAlterTask.hpp

   Descriptive Name = Alter-tasks for Catalog RunTime Context of Header

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains structure for Runtime
   Context of Catalog.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =

*******************************************************************************/

#ifndef CATCONTEXTALTERTASK_HPP_
#define CATCONTEXTALTERTASK_HPP_

#include "catContextTask.hpp"
#include "rtnAlterJob.hpp"

using namespace bson ;

namespace engine
{

   /*
      _catCtxAlterTask define
    */
   class _catCtxAlterTask : public _catCtxDataTask
   {
      public :
         _catCtxAlterTask ( const std::string & dataName,
                            const rtnAlterTask * task ) ;
         virtual ~_catCtxAlterTask () ;

         OSS_INLINE const rtnAlterTask * getTask () const
         {
            return _task ;
         }

      protected :
         const rtnAlterTask * _task ;
   } ;

   /*
      _catCtxAlterCLTask define
    */
   class _catCtxAlterCLTask : public _catCtxAlterTask
   {
      public :
         _catCtxAlterCLTask ( const std::string & collection,
                              const rtnAlterTask * task ) ;
         virtual ~_catCtxAlterCLTask () ;

         INT32 startPostTasks ( _pmdEDUCB * cb,
                                SDB_DMSCB * pDmsCB,
                                SDB_DPSCB * pDpsCB,
                                INT16 w ) ;

         OSS_INLINE const _utilList< UINT64 > & getPostTasks () const
         {
            return _postTasks ;
         }

      protected :
         virtual INT32 _checkInternal ( _pmdEDUCB * cb,
                                        catCtxLockMgr & lockMgr ) ;

         virtual INT32 _executeInternal ( _pmdEDUCB * cb,
                                          SDB_DMSCB * pDmsCB,
                                          SDB_DPSCB * pDpsCB,
                                          INT16 w ) ;

         virtual INT32 _rollbackInternal ( _pmdEDUCB * cb,
                                           SDB_DMSCB * pDmsCB,
                                           SDB_DPSCB * pDpsCB,
                                           INT16 w ) ;

         INT32 _buildFields ( clsCatalogSet & cataSet,
                              bson::BSONObj & setObject,
                              bson::BSONObj & unsetObject ) ;

         INT32 _buildRollbackFields ( clsCatalogSet & cataSet,
                                      bson::BSONObj & setObject,
                                      bson::BSONObj & unsetObject ) ;

         INT32 _checkCreateIDIndex ( const clsCatalogSet & cataSet,
                                     _pmdEDUCB * cb,
                                     catCtxLockMgr & lockMgr ) ;

         INT32 _checkDropIDIndex ( const clsCatalogSet & cataSet,
                                   _pmdEDUCB * cb,
                                   catCtxLockMgr & lockMgr ) ;

         INT32 _checkEnableShard ( const clsCatalogSet & cataSet,
                                   const rtnCLShardingArgument & argument,
                                   _pmdEDUCB * cb,
                                   catCtxLockMgr & lockMgr ) ;

         INT32 _checkDisableShard ( const clsCatalogSet & cataSet,
                                    _pmdEDUCB * cb,
                                    catCtxLockMgr & lockMgr ) ;

         INT32 _checkEnableCompress ( const clsCatalogSet & cataSet,
                                      _pmdEDUCB * cb,
                                      catCtxLockMgr & lockMgr ) ;

         INT32 _checkDisableCompress ( const clsCatalogSet & cataSet,
                                       _pmdEDUCB * cb,
                                       catCtxLockMgr & lockMgr ) ;

         INT32 _checkSetAttributes ( const clsCatalogSet & cataSet,
                                     _pmdEDUCB * cb,
                                     catCtxLockMgr & lockMgr ) ;

         INT32 _buildEnableShardFields ( clsCatalogSet & cataSet,
                                         const rtnCLShardingArgument & argument,
                                         UINT32 & attribute,
                                         bson::BSONObjBuilder & setBuilder,
                                         bson::BSONObjBuilder & unsetBuilder ) ;

         INT32 _buildDisableShardFields ( clsCatalogSet & cataSet,
                                          UINT32 & attribute,
                                          bson::BSONObjBuilder & setBuilder,
                                          bson::BSONObjBuilder & unsetBuilder ) ;

         INT32 _buildEnableCompressFields ( clsCatalogSet & cataSet,
                                            const rtnCLCompressArgument & argument,
                                            UINT32 & attribute,
                                            bson::BSONObjBuilder & setBuilder,
                                            bson::BSONObjBuilder & unsetBuilder ) ;

         INT32 _buildDisableCompressFields ( clsCatalogSet & cataSet,
                                             UINT32 & attribute,
                                             bson::BSONObjBuilder & setBuilder,
                                             bson::BSONObjBuilder & unsetBuilder ) ;

         INT32 _buildExtOptionFields ( clsCatalogSet & cataSet,
                                       const rtnCLExtOptionArgument & argument,
                                       UINT32 & attribute,
                                       bson::BSONObjBuilder & setBuilder,
                                       bson::BSONObjBuilder & unsetBuilder ) ;

         INT32 _buildSetAttributeFields ( clsCatalogSet & cataSet,
                                          UINT32 & attribute,
                                          bson::BSONObjBuilder & setBuilder,
                                          bson::BSONObjBuilder & unsetBuilder ) ;

         INT32 _fillShardingArgument ( clsCatalogSet & cataSet,
                                       rtnCLShardingArgument & argument ) ;

         INT32 _buildPostTasks ( clsCatalogSet & cataSet,
                                 _pmdEDUCB * cb,
                                 SDB_DMSCB * pDmsCB,
                                 SDB_DPSCB * pDpsCB,
                                 INT16 w ) ;

         INT32 _buildAutoHashSplit ( clsCatalogSet & cataSet,
                                     _pmdEDUCB * cb,
                                     SDB_DMSCB * pDmsCB,
                                     SDB_DPSCB * pDpsCB,
                                     INT16 w ) ;

      protected :
         _utilList< UINT64 > _postTasks ;
   } ;

   typedef class _catCtxAlterCLTask catCtxAlterCLTask ;

   /*
      _catCtxAlterCSTask define
    */
   class _catCtxAlterCSTask : public _catCtxAlterTask
   {
      public :
         _catCtxAlterCSTask ( const std::string & collectionSpace,
                              const rtnAlterTask * task ) ;
         virtual ~_catCtxAlterCSTask () ;

         OSS_INLINE const CAT_GROUP_LIST & getGroups () const
         {
            return _groups ;
         }

      protected :
         virtual INT32 _checkInternal ( _pmdEDUCB * cb,
                                        catCtxLockMgr & lockMgr ) ;

         virtual INT32 _executeInternal ( _pmdEDUCB * cb,
                                          SDB_DMSCB * pDmsCB,
                                          SDB_DPSCB * pDpsCB,
                                          INT16 w ) ;

         INT32 _checkSetDomain ( _pmdEDUCB * cb,
                                 catCtxLockMgr & lockMgr ) ;

         INT32 _checkRemoveDomain ( _pmdEDUCB * cb,
                                    catCtxLockMgr & lockMgr ) ;

         INT32 _checkEnableCapped ( _pmdEDUCB * cb,
                                    catCtxLockMgr & lockMgr ) ;

         INT32 _checkDisableCapped ( _pmdEDUCB * cb,
                                     catCtxLockMgr & lockMgr ) ;

         INT32 _checkSetAttributes ( _pmdEDUCB * cb,
                                     catCtxLockMgr & lockMgr ) ;

         INT32 _checkDomainGroups ( const CHAR * domain,
                                    _pmdEDUCB * cb,
                                    catCtxLockMgr & lockMgr ) ;

         INT32 _checkGroups ( _pmdEDUCB * cb,
                              catCtxLockMgr & lockMgr ) ;

         INT32 _checkEmptyCollectionSpace ( _pmdEDUCB * cb,
                                            catCtxLockMgr & lockMgr ) ;

         virtual INT32 _buildSetFields ( BSONObj & setObject ) ;

         virtual INT32 _buildUnsetFields ( BSONObj & unsetObject ) ;

      protected :
         CAT_GROUP_LIST _groups ;
   } ;

   typedef class _catCtxAlterCSTask catCtxAlterCSTask ;

   /*
      _catCtxAlterDomainTask define
    */
   typedef map< std::string, UINT32 >  CAT_DOMAIN_GROUP_MAP ;

   class _catCtxAlterDomainTask : public _catCtxAlterTask
   {
      public :
         _catCtxAlterDomainTask ( const std::string & domain,
                                  const rtnAlterTask * task ) ;
         virtual ~_catCtxAlterDomainTask () ;

      protected :
         virtual INT32 _checkInternal ( _pmdEDUCB * cb,
                                        catCtxLockMgr & lockMgr ) ;

         virtual INT32 _executeInternal ( _pmdEDUCB * cb,
                                          SDB_DMSCB * pDmsCB,
                                          SDB_DPSCB * pDpsCB,
                                          INT16 w ) ;

         virtual INT32 _checkAddGroupTask ( _pmdEDUCB * cb ) ;

         virtual INT32 _checkRemoveGroupTask ( _pmdEDUCB * cb ) ;

         virtual INT32 _checkSetGroupTask ( _pmdEDUCB * cb ) ;

         virtual INT32 _checkSetAttrTask ( _pmdEDUCB * cb ) ;

         virtual INT32 _checkAddingGroups ( const RTN_DOMAIN_GROUP_LIST & groups,
                                            _pmdEDUCB * cb ) ;

         virtual INT32 _checkRemovingGroups ( const RTN_DOMAIN_GROUP_LIST & groups,
                                              _pmdEDUCB * cb ) ;

         virtual void _extractGroups ( const RTN_DOMAIN_GROUP_LIST & groups,
                                       RTN_DOMAIN_GROUP_LIST & addingGroups,
                                       RTN_DOMAIN_GROUP_LIST & removingGroups ) ;

         virtual INT32 _toDomainGroups ( BSONObjBuilder & builder ) ;

      protected :
         CAT_DOMAIN_GROUP_MAP    _groupMap ;
   } ;

   typedef class _catCtxAlterDomainTask catCtxAlterDomainTask ;

}

#endif //CATCONTEXTBASEALTERTASK_HPP_
