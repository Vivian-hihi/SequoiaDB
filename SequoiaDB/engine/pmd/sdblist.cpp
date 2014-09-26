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

   Source File Name = sdblist.cpp

   Descriptive Name = sdbstop Main

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbstop,
   which is used to stop SequoiaDB engine.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#include "core.hpp"
#include "ossUtil.hpp"
#include "ossProc.hpp"
#include "ossMem.hpp"
#include "ossPath.hpp"
#include "msgDef.hpp"
#include "pd.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"
#include "pmdDef.hpp"
#include "pmd.hpp"
#include "pmdOptionsMgr.hpp"
#include "utilNodeOpr.hpp"
#include "utilCommon.hpp"
#include "ossVer.h"
#include "utilParam.hpp"
#include "utilStr.hpp"
#include "pmdDaemon.hpp"
#include "../bson/bson.h"
#include <string>
#include <iostream>
#include <vector>

using namespace std;

/*
   SDB_RUN_MODE_TYPE_STR DEFINE
*/
#define SDB_RUN_MODE_TYPE_LOCAL_STR  "local"
#define SDB_RUN_MODE_TYPE_RUN_STR    "run"

/*
  SDBLIST_TYPE_STR
*/
#define SDBLIST_TYPE_OMA_STR    "cm"
#define SDBLIST_TYPE_OM_STR     "om"
#define SDBLIST_TYPE_DB_STR     "db"
#define SDBLIST_TYPE_ALL_STR    "all"

/*
   SDB_RUN_MODE_TYPE define
*/
enum SDB_RUN_MODE_TYPE
{
   LOCAL = 1,
   RUN,
   SDB_RUN_MODE_TYPE_MAX
} ;

/*
   OPTION DEFINE
*/
#define PMD_OPTION_TYPE             "type"
#define PMD_OPTION_MODE             "mode"
#define PMD_OPTION_DETAIL           "detail"
#define PMD_OPTION_EXPAND           "expand"
#define SDB_CONF_FILE_PATH_FORMAT   SDBCM_LOCAL_PATH OSS_FILE_SEP "%s" OSS_FILE_SEP PMD_DFT_CONF

#define COMMANDS_OPTIONS \
       ( PMD_COMMANDS_STRING( PMD_OPTION_HELP, ",h" ), "help" ) \
       ( PMD_COMMANDS_STRING( PMD_OPTION_TYPE, ",t" ), boost::program_options::value<string>(), "node type: db/om/cm/all, default: db" ) \
       ( PMD_COMMANDS_STRING( PMD_OPTION_SVCNAME, ",p" ), boost::program_options::value<string>(), "service name, use ',' to seperator" )  \
       ( PMD_COMMANDS_STRING( PMD_OPTION_MODE, ",m" ), boost::program_options::value<string>(),"mode type: run/local,default: run" ) \
       ( PMD_COMMANDS_STRING( PMD_OPTION_ROLE, ",r" ), boost::program_options::value<string>(), "role type: coord/data/cata" ) \
       ( PMD_OPTION_VERSION, "show version" ) \
       ( PMD_OPTION_DETAIL,"show detail configuration" ) \
       ( PMD_OPTION_EXPAND,"show expand configuration" )


namespace engine
{
   // get node's role
   INT32 _getNodeRole( const CHAR *confpath, const CHAR *svcname, string &role )
   {
      CHAR sdbConfTemp[OSS_MAX_PATHSIZE + 1] = { 0 } ;
      CHAR sdbConfPath[OSS_MAX_PATHSIZE + 1] = { 0 } ;
      INT32 rc = SDB_OK ;
      po::options_description desc ;
      po::variables_map vm ;
      desc.add_options()
         ( PMD_OPTION_ROLE, po::value<string>(), "" );


      ossSnprintf( sdbConfTemp,OSS_MAX_PATHSIZE,SDB_CONF_FILE_PATH_FORMAT,
                  svcname ) ;
      utilBuildFullPath( confpath, sdbConfTemp,
                         OSS_MAX_PATHSIZE, sdbConfPath ) ;

      rc = ossAccess( sdbConfPath, 0 ) ;
      if ( rc )
      {
         goto error ;
      }

      utilReadConfigureFile( sdbConfPath, desc, vm ) ;

      if( vm.count ( PMD_OPTION_ROLE ) )
      {
         role = vm [ PMD_OPTION_ROLE ].as<string>() ;
      }

   done :
      return rc ;
   error :
      goto done ;


   }

   // get sdbcm's port
   INT32 _getCMPort( const CHAR *confpath, string &port )
   {
      CHAR sdbConfPath[OSS_MAX_PATHSIZE + 1] = { 0 } ;
      INT32 rc = SDB_OK ;
      po::options_description desc ;
      po::variables_map vm ;
      desc.add_options()
         ( SDBCM_CONF_DFTPORT, po::value<string>(), "" );

      utilBuildFullPath( confpath, SDBCM_CONF_PATH_FILE,
                         OSS_MAX_PATHSIZE, sdbConfPath ) ;

      rc = ossAccess( sdbConfPath, 0 ) ;
      if ( rc )
      {
         goto error ;
      }
      utilReadConfigureFile( sdbConfPath, desc, vm ) ;

      if( vm.count ( SDBCM_CONF_DFTPORT ) )
      {
         port = vm [ SDBCM_CONF_DFTPORT ].as<string>() ;
      }

   done :
      return rc ;
   error :
      goto done ;

   }

   //print node's detail configuration by sdb conf file and svcname
   void _printfDetail( const CHAR *confpath, const CHAR *svcname, INT32 type )
   {
      INT32 rc = SDB_OK ;
      po::options_description desc ;
      po::variables_map vm ;
      desc.add_options()
         ("*",po::value<string>(),"") ;

      CHAR sdbConfTemp[OSS_MAX_PATHSIZE + 1] = { 0 } ;
      CHAR sdbConfPath[OSS_MAX_PATHSIZE + 1] = { 0 } ;
      ossSnprintf( sdbConfTemp,OSS_MAX_PATHSIZE,SDB_CONF_FILE_PATH_FORMAT,
                  svcname ) ;
      if( (SDB_TYPE)type == SDB_TYPE_DB )
      {
         utilBuildFullPath( confpath, sdbConfTemp,
                            OSS_MAX_PATHSIZE, sdbConfPath ) ;
      }
      else
      {
         utilBuildFullPath( confpath,SDBCM_CONF_PATH_FILE,
                           OSS_MAX_PATHSIZE, sdbConfPath ) ;
      }
      rc = ossAccess( sdbConfPath, 0 ) ;
      if ( rc )
      {
         return ;
      }
      utilReadConfigureFile( sdbConfPath, desc, vm ) ;

      po::variables_map::iterator it = vm.begin() ;
      while( it != vm.end() )
      {
         ossPrintf("   %-18.18s: %s\n", it->first.data(),
                                        it->second.as<string>().c_str()) ;
         ++it ;
      }

   }


   //print node's expand configuration by sdb conf file and svcname
   void _printfExpand( const CHAR *confpath, const CHAR *svcname, INT32 type )
   {
      INT32 rc = SDB_OK ;
      BSONObj objData ;
      CHAR sdbConfTemp[OSS_MAX_PATHSIZE + 1] = { 0 } ;
      CHAR sdbConfPath[OSS_MAX_PATHSIZE + 1] = { 0 } ;
      ossSnprintf( sdbConfTemp,OSS_MAX_PATHSIZE,SDB_CONF_FILE_PATH_FORMAT,
                  svcname ) ;
      if( (SDB_TYPE)type == SDB_TYPE_DB )
      {
         utilBuildFullPath( confpath, sdbConfTemp,
                            OSS_MAX_PATHSIZE, sdbConfPath ) ;
         engine::pmdOptionsCB option ;

         rc = ossAccess( sdbConfPath, 0 ) ;
         if ( rc )
         {
            return  ;
         }
         option.initFromFile( sdbConfPath, FALSE ) ;
         option.toBSON( objData ) ;
         BSONObjIterator it = objData.begin() ;
         while( it.more() )
         {
            BSONElement e = it.next() ;
            if( e.type() == String )
            {
               ossPrintf("   %-18.18s: %s\n", e.fieldName(), e.valuestr() ) ;
            }
            else if( e.type() == NumberInt )
            {
               ossPrintf("   %-18.18s: %d\n", e.fieldName(), e.numberInt() ) ;
            }
            else if( e.type() == NumberLong )
            {
               #if defined(_WINDOWS)
                  ossPrintf("   %-18.18s: %I64d\n", e.fieldName(), e.numberLong() ) ;
               #else
                  ossPrintf("   %-18.18s: %lld\n", e.fieldName(), e.numberLong() ) ;
               #endif
            }
            else if( e.type() == NumberDouble )
            {
               ossPrintf("   %-18.18s: %f\n", e.fieldName(), e.numberDouble() ) ;
            }
            else if( e.type() == Bool )
            {
               ossPrintf("   %-18.18s: %s\n", e.fieldName(),(e.boolean()?"TRUE":"FALSE" ));
            }
            else
            {
               ossPrintf("   %-18.18s: %s\n", e.fieldName(),"-" ) ;
            }
         }
      }
      else
      {
         _printfDetail( confpath, NULL, SDB_TYPE_OMA );

      }
   }

   //printf detail or expand
   void _printfAll( const CHAR *confpath, string &svcname,INT32 type,
                    BOOLEAN detail, BOOLEAN expand, INT32 pid, BOOLEAN online )
   {
      if( online )
      {
         ossPrintf( "%s(%s) (%d)"OSS_NEWLINE, utilDBTypeStr( (SDB_TYPE)type ),
                   svcname.c_str(), pid ) ;
         if( detail )
         {
            _printfDetail( confpath, svcname.c_str(), (SDB_TYPE)type ) ;
         }
         if( expand )
         {
            _printfExpand( confpath, svcname.c_str(), (SDB_TYPE)type ) ;
         }
      }
      else
      {
         ossPrintf( "%s(%s) (-)"OSS_NEWLINE, utilDBTypeStr( (SDB_TYPE)type),
                    svcname.c_str()) ;
         if( detail )
         {
            _printfDetail( confpath, svcname.c_str(), SDB_TYPE_DB ) ;
         }
         if( expand )
         {
            _printfExpand( confpath, svcname.c_str(), SDB_TYPE_DB ) ;
         }
      }
   }

   // initialize options
   void init ( po::options_description &desc )
   {
      PMD_ADD_PARAM_OPTIONS_BEGIN ( desc )
         COMMANDS_OPTIONS
      PMD_ADD_PARAM_OPTIONS_END
   }

   void displayArg ( po::options_description &desc )
   {
      std::cout << desc << std::endl ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_SDBLIST_RESVARG, "resolveArgument" )
   INT32 resolveArgument ( po::options_description &desc,
                           INT32 argc, CHAR **argv,
                           vector<string> &listServices,
                           INT32 &typeFilter, INT32 &mode,
                           INT32 &role, BOOLEAN &detail,
                           BOOLEAN &expand )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_SDBLIST_RESVARG ) ;
      po::variables_map vm ;

      rc = utilReadCommandLine( argc, argv,  desc, vm, FALSE ) ;
      if ( rc )
      {
         std::cout << "Read command line failed: " << rc << endl ;
         goto error ;
      }

      if ( vm.count ( PMD_OPTION_HELP ) )
      {
         displayArg ( desc ) ;
         rc = SDB_PMD_HELP_ONLY ;
         goto error ;
      }
      else if ( vm.count( PMD_OPTION_VERSION ) )
      {
         ossPrintVersion( "Sdb list" ) ;
         rc = SDB_PMD_VERSION_ONLY ;
         goto error ;
      }

      if ( vm.count ( PMD_OPTION_SVCNAME ) )
      {
         string svcname = vm[PMD_OPTION_SVCNAME].as<string>() ;
         // break service names using ';'
         rc = utilSplitStr( svcname, listServices, ", \t" ) ;
         if ( rc )
         {
            std::cout << "Parse svcname failed: " << rc << endl ;
            goto error ;
         }
      }
      if ( vm.count( PMD_OPTION_TYPE ) )
      {
         string listType = vm[ PMD_OPTION_TYPE ].as<string>() ;
         if ( 0 == ossStrcasecmp( listType.c_str(), SDBLIST_TYPE_DB_STR ) )
         {
            typeFilter = SDB_TYPE_DB ;
         }
         else if ( 0 == ossStrcasecmp( listType.c_str(), SDBLIST_TYPE_OM_STR ) )
         {
            typeFilter = SDB_TYPE_OM ;
         }
         else if ( 0 == ossStrcasecmp( listType.c_str(), SDBLIST_TYPE_OMA_STR ) )
         {
            typeFilter = SDB_TYPE_OMA ;
         }
         else if ( 0 == ossStrcasecmp( listType.c_str(), SDBLIST_TYPE_ALL_STR ) )
         {
            typeFilter = -1 ;
         }
         else
         {
            std::cout << "type invalid" << endl ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }
      if ( vm.count( PMD_OPTION_MODE ))
      {
         string modeTemp = vm[PMD_OPTION_MODE].as<string>() ;
         if( 0 == ossStrcasecmp( modeTemp.c_str(), SDB_RUN_MODE_TYPE_LOCAL_STR  ))
         {
            mode = LOCAL ;
         }
         else if( 0 == ossStrcasecmp( modeTemp.c_str(), SDB_RUN_MODE_TYPE_RUN_STR  ))
         {
            mode = RUN ;
         }
         else
         {
            std::cout << "mode invalid" << endl ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }

      if ( vm.count( PMD_OPTION_ROLE ))
      {
         string roleTemp = vm[PMD_OPTION_ROLE].as<string>() ;
         if( 0 == ossStrcasecmp( roleTemp.c_str(), SDB_ROLE_COORD_STR ))
         {
            role = SDB_ROLE_COORD ;
         }
         else if( 0 == ossStrcasecmp( roleTemp.c_str(), SDB_ROLE_CATALOG_STR ))
         {
            role = SDB_ROLE_CATALOG ;
         }
         else if( 0 == ossStrcasecmp( roleTemp.c_str(), SDB_ROLE_DATA_STR ))
         {
            role = SDB_ROLE_DATA ;
         }
         else
         {
            std::cout << "role invalid" << endl ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }

      if( vm.count(PMD_OPTION_DETAIL ))
      {
         detail = TRUE ;
      }

      if ( vm.count(PMD_OPTION_EXPAND ))
      {
         expand = TRUE ;
         detail = FALSE ;
      }
   done :
      PD_TRACE_EXITRC ( SDB_SDBLIST_RESVARG, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_SDBLIST_MAIN, "mainEtnry" )
   INT32 mainEntry ( INT32 argc, CHAR **argv )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_SDBLIST_MAIN );
      INT32 total = 0 ;
      vector<string> listServices ;
      vector<string> listAllNodes ;
      UTIL_VEC_NODES listNodes ;
      BOOLEAN bFind = TRUE ;
      INT32 typeFilter = SDB_TYPE_DB ;
      BOOLEAN detail = FALSE ;
      BOOLEAN expand = FALSE ;
      INT32 role =  -1 ;
      string roleTemp = "Unknown" ;
      string portTemp = "Unknown" ;
      INT32 mode = RUN ;
      CHAR rootPath[OSS_MAX_PATHSIZE + 1] = { 0 } ;
      CHAR localPath[OSS_MAX_PATHSIZE + 1] = { 0 } ;


      po::options_description desc ( "Command options" ) ;
      init ( desc ) ;

      // validate arguments
      rc = resolveArgument ( desc, argc, argv, listServices, typeFilter,
                             mode, role, detail, expand ) ;
      if( rc )
      {
         if( SDB_PMD_HELP_ONLY != rc && SDB_PMD_VERSION_ONLY != rc )
         {
            std::cout << "Invalid argument" << endl ;
            displayArg ( desc ) ;
         }
         goto done ;
      }

      // get program's running  path
      rc = ossGetEWD( rootPath, OSS_MAX_PATHSIZE ) ;
      if( rc )
      {
        ossPrintf("Error:Get module self path failed: %d"OSS_NEWLINE, rc ) ;
        goto error ;
      }

      if( listServices.size() > 0 )
      {
         // if used -p, so list all nodes and cm,except cmd
         typeFilter = -1 ;
         utilListNodes( listNodes, typeFilter ) ;

         for( UINT32 i = 0 ; i < listNodes.size() ; ++i )
         {
            utilNodeInfo &info = listNodes[ i ] ;

            if( listServices.size() > 0 )
            {
               bFind = FALSE ;
               for( UINT32 j = 0 ; j < listServices.size() ; ++j )
               {
                  if( 0 == ossStrcmp( info._svcname.c_str(),
                                    listServices[ j ].c_str() ) )
                  {
                     bFind = TRUE ;
                     break ;
                  }
               }
            }
            else
            {
               bFind = TRUE ;
            }

            if( bFind )
            {
               ++total ;
               _printfAll( rootPath, info._svcname, info._type, detail,
                          expand, info._pid, TRUE ) ;
            }
         }
      }

      // not use -p
      else
      {
         if( typeFilter == -1 || (SDB_TYPE)typeFilter == SDB_TYPE_DB )
         {
            utilBuildFullPath( rootPath, SDBCM_LOCAL_PATH,
                               OSS_MAX_PATHSIZE, localPath ) ;

            ossEnumSubDirs( localPath, listAllNodes ) ;
         }
         if( typeFilter == -1 || (SDB_TYPE)typeFilter == SDB_TYPE_OMA )
         {
            rc = _getCMPort( rootPath, portTemp ) ;
            if( !rc )
            {
               listAllNodes.push_back( portTemp ) ;
            }
         }

         utilListNodes( listNodes,typeFilter ) ;


         for( UINT32 i = 0; i < listNodes.size(); i++ )
         {
            utilNodeInfo &info = listNodes[i] ;
            // not use role
            if( role == -1 )
            {
               ++total ;
               _printfAll( rootPath,info._svcname,info._type,
                           detail, expand, info._pid, TRUE ) ;
            }
            else
            {
               // type = db
               if( (SDB_TYPE)info._type == SDB_TYPE_DB )
               {
                  rc = _getNodeRole( rootPath, info._svcname.c_str(),
                                     roleTemp ) ;
                  if( rc )
                  {
                     roleTemp = string( utilDBRoleStr((SDB_ROLE)role) ) ;
                  }
                  if( 0 == ossStrcmp( roleTemp.c_str(),
                                      utilDBRoleStr( (SDB_ROLE)role )) )
                  {
                     ++total ;
                     _printfAll( rootPath, info._svcname,info._type,
                                detail, expand, info._pid, TRUE ) ;
                  }
               }
            }
         }

         //mode = LOCAL
         if( mode == LOCAL )
         {
            for( UINT32 i = 0; i < listAllNodes.size(); i++ )
            {
               string &info = listAllNodes[i] ;
               bFind = FALSE ;
               for( UINT32 j = 0; j < listNodes.size(); j++ )
               {
                  if( 0 == ossStrcmp(info.c_str(),listNodes[j]._svcname.c_str()) )
                  {
                     bFind = TRUE ;
                     break ;
                  }
               }
               if( bFind )
               {
                  continue ;
               }
               // node is offline and not use role
               if( role == -1 )
               {
                  ++total ;
                  _printfAll( rootPath,info, SDB_TYPE_DB,
                              detail, expand, 0, FALSE ) ;
               }
               else
               {
                  rc = _getNodeRole( rootPath, info.c_str(),
                                     roleTemp ) ;
                  if ( rc )
                  {
                     roleTemp = string( utilDBRoleStr((SDB_ROLE)role) ) ;
                  }

                  if( 0 == ossStrcmp( roleTemp.c_str(),
                                      utilDBRoleStr( (SDB_ROLE)role )) )
                  {
                     ++total ;
                     _printfAll( rootPath, info,SDB_TYPE_DB,
                                detail, expand, 0, FALSE ) ;
                  }
               }
            }
         }
      }

      // if no -p, and list all/list cm, need to show sdbcmd
      if ( listServices.size() == 0 && ( SDB_TYPE_OMA == typeFilter ||
           -1 == typeFilter ) && role == -1 )
      {
         vector < ossProcInfo > procs ;
         ossEnumProcesses( procs, PMDDMN_EXE_NAME, TRUE, FALSE ) ;

         for ( UINT32 i = 0 ; i < procs.size() ; ++i )
         {
            ++total ;
            ossPrintf( "%s (%d)"OSS_NEWLINE, PMDDMN_SVCNAME_DEFAULT,
                       procs[ i ]._pid ) ;
         }
      }
      ossPrintf ( "Total: %d"OSS_NEWLINE, total ) ;
   done :
      PD_TRACE_EXITRC ( SDB_SDBLIST_MAIN, rc );
      // return SDB_OK == rc ? 0 : 1 ;
      return total > 0 ? 0 : ( rc ? 127 : 1 ) ;
   error :
      goto done ;
   }
}

INT32 main ( INT32 argc, CHAR **argv )
{
   return engine::mainEntry( argc, argv ) ;
}


