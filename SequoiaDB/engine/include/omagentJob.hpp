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
//   class _omaCreateCatalogJob : public _rtnBaseJob
   class _omaCreateCatalogJob : public SDBObject
   {
      public:
         _omaCreateCatalogJob ( std::vector<BSONObj> &catalog,
                                InstallJobResult &catalogResult ) ;
         virtual ~_omaCreateCatalogJob () ;

      public:
         virtual RTN_JOB_TYPE type () const ;
         virtual const CHAR*  name () const ;
//         virtual BOOLEAN      muteXOn ( const _rtnBaseJob *pOther ) ;
         virtual BOOLEAN      muteXOn () ;
         virtual INT32        doit () ;

      private:
         std::string                 _name ;
         std::vector<BSONObj>        &_catalog ;
         InstallJobResult            &_catalogResult ;
         _omaJobRunInstallCatalogCmd _runCmd ;
   } ;

   /*
      create coord job
   */
//   class _omaCreateCoordJob : public _rtnBaseJob
   class _omaCreateCoordJob : public SDBObject
   {
      public:
         _omaCreateCoordJob ( std::vector<BSONObj> &coord,
                              InstallJobResult &coordResult ) ;
         virtual ~_omaCreateCoordJob () ;

      public:
         virtual RTN_JOB_TYPE type () const ;
         virtual const CHAR*  name () const ;
//         virtual BOOLEAN      muteXOn ( const _rtnBaseJob *pOther ) ;
         virtual BOOLEAN      muteXOn () ;
         virtual INT32        doit () ;

     private:
         std::string               _name ;
         std::vector<BSONObj>      &_coord ;
         InstallJobResult          &_coordResult ;
         _omaJobRunInstallCoordCmd _runCmd ;

   } ;

   /*
      create data job
   */
//   class _omaCreateDataJob : public _rtnBaseJob
   class _omaCreateDataJob : public SDBObject
   {
      public:
         _omaCreateDataJob ( const CHAR *groupname, std::vector<BSONObj> &coord,
                             InstallJobResult &coordResult ) ;
         virtual ~_omaCreateDataJob () ;

      public:
         virtual RTN_JOB_TYPE type () const ;
         virtual const CHAR*  name () const ;
//         virtual BOOLEAN      muteXOn ( const _rtnBaseJob *pOther ) ;
         virtual BOOLEAN      muteXOn () ;
         virtual INT32        doit () ;

     private:
         std::string               _groupname ;
         std::string               _name ;
         std::vector<BSONObj>      &_data ;
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
