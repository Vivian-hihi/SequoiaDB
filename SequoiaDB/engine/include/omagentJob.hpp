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

   Source File Name = omagentJob.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/30/2014  TZB Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef OMAGENT_JOB_HPP_
#define OMAGENT_JOB_HPP_
#include "core.hpp"
#include "oss.hpp"
#include "ossUtil.hpp"
#include "pmd.hpp"
#include "omagent.hpp"
#include "omagentJobRunCmd.hpp"
#include "rtnBackgroundJob.hpp"
#include <string>
#include <vector>

using namespace bson ;

namespace engine
{

   /*
      create catalog job
   */
   class _omaCreateCatalogJob : public _rtnBaseJob
   {
      public:
         _omaCreateCatalogJob ( std::vector<BSONObj> &catalog,
                                InstallJobResult &catalogResult ) ;
         virtual ~_omaCreateCatalogJob () ;

      public:
         virtual RTN_JOB_TYPE type () const ;
         virtual const CHAR*  name () const ;
         virtual BOOLEAN      muteXOn ( const _rtnBaseJob *pOther ) ;
         virtual INT32        doit () ;

      private:
         std::string                 _name ;
         std::vector<BSONObj>        _catalog ;
         InstallJobResult            &_catalogResult ;
         _omaJobRunInstallCatalogCmd _runCmd ;
   } ;

   /*
      create coord job
   */
   class _omaCreateCoordJob : public _rtnBaseJob
   {
      public:
         _omaCreateCoordJob ( std::vector<BSONObj> &coord,
                              InstallJobResult &coordResult ) ;
         virtual ~_omaCreateCoordJob () ;

      public:
         virtual RTN_JOB_TYPE type () const ;
         virtual const CHAR*  name () const ;
         virtual BOOLEAN      muteXOn ( const _rtnBaseJob *pOther ) ;
         virtual INT32        doit () ;

     private:
         std::string               _name ;
         std::vector<BSONObj>      _coord ;
         InstallJobResult          &_coordResult ;
         _omaJobRunInstallCoordCmd _runCmd ;

   } ;

   /*
      create data job
   */
   class _omaCreateDataJob : public _rtnBaseJob
   {
      public:
         _omaCreateDataJob ( const CHAR *groupname, std::vector<BSONObj> &coord,
                             InstallJobResult &coordResult ) ;
         virtual ~_omaCreateDataJob () ;

      public:
         virtual RTN_JOB_TYPE type () const ;
         virtual const CHAR*  name () const ;
         virtual BOOLEAN      muteXOn ( const _rtnBaseJob *pOther ) ;
         virtual INT32        doit () ;

     private:
         std::string               _groupname ;
         std::string               _name ;
         std::vector<BSONObj>      _data ;
         InstallJobResult          &_dataResult ;
         _omaJobRunInstallDataCmd  _runCmd ;

   } ;

   // start create catalog job
   INT32 startCreateCatalogJob ( std::vector<BSONObj> &catalog,
                                 InstallJobResult &catalogResult,
                                 EDUID *pEDUID ) ;
   // start create coord job
   INT32 startCreateCoordJob ( std::vector<BSONObj> &coord,
                               InstallJobResult &coordResult,
                               EDUID *pEDUID ) ;

   // start create data job
   INT32 startCreateDataJob ( const CHAR *groupname,
                              std::vector<BSONObj> &data,
                              InstallJobResult &dataResult,
                              EDUID *pEDUID ) ;

}



#endif
