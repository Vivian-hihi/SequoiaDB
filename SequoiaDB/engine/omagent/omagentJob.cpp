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

   Source File Name = omagentJob.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/06/2014  TZB Initial Draft

   Last Changed =

*******************************************************************************/

#include "omagentUtil.hpp"
#include "omagentJob.hpp"

namespace engine
{
   /*
       omagent create catalog job
   */
   _omaCreateCatalogJob::_omaCreateCatalogJob ( std::vector<BSONObj> &catalog,
                                                InstallJobResult &catalogResult )
   :_catalog(catalog), _catalogResult(catalogResult)
   {
      _name = "create catalog job" ;
   }

   _omaCreateCatalogJob::~_omaCreateCatalogJob()
   {
   }

   RTN_JOB_TYPE _omaCreateCatalogJob::type () const
   {
      return RTN_JOB_CREATECATALOG ;
   }

   const CHAR* _omaCreateCatalogJob::name () const
   {
      return _name.c_str() ;
   }

   BOOLEAN _omaCreateCatalogJob::muteXOn ( const _rtnBaseJob *pOther )
   {
      return FALSE ;
   }

   INT32 _omaCreateCatalogJob::doit()
   {
      INT32 rc = SDB_OK ;
      rc = _runCmd.init( _catalog ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Job failed to init for creating catalog, rc = %d", rc ) ;
         goto error ;
      }
      rc = _runCmd.doit( _catalogResult ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Job failed to create catalog, rc = %d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 startCreateCatalogJob ( std::vector<BSONObj> &catalog,
                                 InstallJobResult &catalogResult,
                                 EDUID *pEDUID )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN returnResult = FALSE ;
      _omaCreateCatalogJob *pJob = NULL ;
      pJob = SDB_OSS_NEW _omaCreateCatalogJob( catalog, catalogResult ) ;
      if ( !pJob )
      {
         PD_LOG ( PDERROR, "Failed to alloc memory for creating catalog job" ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      rc = rtnGetJobMgr()->startJob( pJob, RTN_JOB_MUTEX_NONE, pEDUID,
                                     returnResult ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      omagent create coord job
   */
   _omaCreateCoordJob::_omaCreateCoordJob ( std::vector<BSONObj> &coord,
                                            InstallJobResult &coordResult )
   :_coord(coord), _coordResult(coordResult)
   {
      _name = "create coord job" ;
   }

   _omaCreateCoordJob::~_omaCreateCoordJob()
   {
   }

   RTN_JOB_TYPE _omaCreateCoordJob::type () const
   {
      return RTN_JOB_CREATECOORD ;
   }

   const CHAR* _omaCreateCoordJob::name () const
   {
      return _name.c_str() ;
   }

   BOOLEAN _omaCreateCoordJob::muteXOn ( const _rtnBaseJob *pOther )
   {
      return FALSE ;
   }

   INT32 _omaCreateCoordJob::doit()
   {
      INT32 rc = SDB_OK ;
      rc = _runCmd.init( _coord ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Job failed to init for creating coord, rc = %d", rc ) ;
         goto error ;
      }
      rc = _runCmd.doit( _coordResult ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Job failed to create coord, rc = %d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 startCreateCoordJob ( std::vector<BSONObj> &coord,
                               InstallJobResult &coordResult,
                               EDUID *pEDUID )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN returnResult = FALSE ;
      _omaCreateCoordJob *pJob = NULL ;
      pJob = SDB_OSS_NEW _omaCreateCoordJob( coord, coordResult ) ;
      if ( !pJob )
      {
         PD_LOG ( PDERROR, "Failed to alloc memory for creating coord job" ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      rc = rtnGetJobMgr()->startJob( pJob, RTN_JOB_MUTEX_NONE, pEDUID,
                                     returnResult ) ;
   done:
      return rc ;
   error:
      goto done ;
   }


   /*
      omagent create data job
   */
   _omaCreateDataJob::_omaCreateDataJob ( const CHAR *groupname,
                                          std::vector<BSONObj> &data,
                                          InstallJobResult &dataResult )
   :_groupname(groupname), _data(data), _dataResult(dataResult)
   {
      _name = "create data job" ;
   }

   _omaCreateDataJob::~_omaCreateDataJob()
   {
   }

   RTN_JOB_TYPE _omaCreateDataJob::type () const
   {
      return RTN_JOB_CREATEDATA ;
   }

   const CHAR* _omaCreateDataJob::name () const
   {
      return _name.c_str() ;
   }

   BOOLEAN _omaCreateDataJob::muteXOn ( const _rtnBaseJob *pOther )
   {
      return FALSE ;
   }

   INT32 _omaCreateDataJob::doit()
   {
      INT32 rc = SDB_OK ;
      rc = _runCmd.init( _data ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Job failed to init for creating data node, rc = %d", rc ) ;
         goto error ;
      }
      rc = _runCmd.doit( _dataResult ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Job failed to create data node, rc = %d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 startCreateDataJob ( const CHAR *groupname,
                              std::vector<BSONObj> &data,
                              InstallJobResult &dataResult,
                              EDUID *pEDUID )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN returnResult = FALSE ;
      _omaCreateDataJob *pJob = NULL ;
      pJob = SDB_OSS_NEW _omaCreateDataJob( groupname, data, dataResult ) ;
      if ( !pJob )
      {
         PD_LOG ( PDERROR, "Failed to alloc memory for creating data node job" ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      rc = rtnGetJobMgr()->startJob( pJob, RTN_JOB_MUTEX_NONE, pEDUID,
                                     returnResult ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

}
