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
#include "pd.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"
#include "pmdDef.hpp"
#include "pmdOptions.h"
#include "utilNodeOpr.hpp"
#include "utilCommon.hpp"
#include "ossVer.h"
#include "utilParam.hpp"
#include "utilStr.hpp"
#include "pmdDaemon.hpp"
#include <string>
#include <iostream>
#include <vector>
#include <dirent.h>

using namespace std;

namespace engine
{

   #define COMMANDS_OPTIONS \
       ( PMD_COMMANDS_STRING( PMD_OPTION_HELP, ",h"), "help" ) \
       ( PMD_COMMANDS_STRING( PMD_OPTION_TYPE, ",t"), boost::program_options::value<string>(), "node type: db/om/cm/all, default: db" ) \
       ( PMD_COMMANDS_STRING( PMD_OPTION_SVCNAME, ",p"), boost::program_options::value<string>(), "service name, use ',' to seperator" )  \
       ( PMD_COMMANDS_STRING( PMD_OPTION_MODE, ",m"), boost::program_options::value<string>(),"mode type: run/local,default: local" ) \
       ( PMD_COMMANDS_STRING( PMD_OPTION_ROLE, ",r"), boost::program_options::value<string>(), "role type: coord/data/cata" ) \
       ( PMD_OPTION_VERSION, "show version" ) \
       ( PMD_OPTION_DETAIL,"show detail configuration" ) \
       ( PMD_OPTION_EXPAND,"show expand configuration" )

   //list all nodes by confpath "installpath/conf/local/"
   static INT32 utilListAllNodes(const CHAR *confpath, vector<string> &nodes )
   {
      INT32 rc               = SDB_OK ;
      DIR *pDir              = NULL ;
      struct dirent *pDirent = NULL ;
      BOOLEAN  isOpen        = FALSE ;
      CHAR confLocalPath[OSS_MAX_PATHSIZE + 1] = { 0 } ;
      utilBuildFullPath( confpath, SDB_CONF_LOCAL_PATH,
                         OSS_MAX_PATHSIZE, confLocalPath ) ;
      pDir = opendir( confLocalPath ) ;
      isOpen = TRUE ;
      PD_CHECK(pDir != NULL, SDB_IO,error,PDERROR,"Failed to open the \
            directory:%s,error=%d",confLocalPath,ossGetLastError() ) ;
      while( (pDirent = readdir(pDir)) != NULL )
      {
            if( ossAtoi(pDirent->d_name))
            {
               nodes.push_back( pDirent->d_name ) ;
            }
      }

      done:
         if( isOpen )
         {
            closedir( pDir ) ;
         }
         return rc ;
      error:
         goto done ;
   }

   // get sdb conf detail
   static INT32 utilGetSdbConfDetail( const CHAR *confpath, string &svcname,
                string &dbpath, string &role, string &catalogaddr )
   {
      INT32 rc = SDB_OK ;
      po::options_description desc ;
      po::variables_map vm ;
      desc.add_options()
         ( PMD_OPTION_SVCNAME, po::value<string>(), "" ) \
         ( PMD_OPTION_ROLE, po::value<string>(), "" ) \
         ( PMD_OPTION_DBPATH, po::value<string>(), "" ) \
         ( PMD_OPTION_CATALOGADDR, po::value<string>(), "" ) ;


      rc = utilReadConfigureFile( confpath, desc, vm ) ;
      if ( SDB_IO == rc )
      {
         rc = SDB_OK ;
         goto done ;
      }
      if ( rc )
      {
         goto error ;
      }

      if ( vm.count ( PMD_OPTION_SVCNAME ) )
      {
         svcname = vm [ PMD_OPTION_SVCNAME ].as<string>() ;
      }

      if( vm.count ( PMD_OPTION_ROLE ) )
      {
         role = vm [ PMD_OPTION_ROLE ].as<string>() ;
      }

      if ( vm.count ( PMD_OPTION_DBPATH ) )
      {
         dbpath = vm [ PMD_OPTION_DBPATH].as<string>() ;
      }

      if( vm.count ( PMD_OPTION_CATALOGADDR ))
      {
         catalogaddr = vm [ PMD_OPTION_CATALOGADDR ].as<string>() ;
      }

   done :
      return rc ;
   error :
      goto done ;
   }

   // get node's role
   static INT32 utilGetNodeRole( const CHAR *confpath, const CHAR *svcname,
                                 string &role )
   {
      CHAR sdbConfTemp[OSS_MAX_PATHSIZE + 1] = { 0 } ;
      CHAR sdbConfPath[OSS_MAX_PATHSIZE + 1] = { 0 } ;
      INT32 rc = SDB_OK ;
      po::options_description desc ;
      po::variables_map vm ;
      desc.add_options()
         ( PMD_OPTION_ROLE, po::value<string>(), "" );


      ossSnprintf(sdbConfTemp,OSS_MAX_PATHSIZE,SDB_CONF_FILE_PATH_FORMAT,
                 svcname ) ;
      utilBuildFullPath( confpath, sdbConfTemp,
                         OSS_MAX_PATHSIZE, sdbConfPath ) ;

      rc = utilReadConfigureFile( sdbConfPath, desc, vm ) ;
      if ( SDB_IO == rc )
      {
         rc = SDB_OK ;
         goto done ;
      }
      if ( rc )
      {
         goto error ;
      }
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
   static INT32 utilGetCMPort( const CHAR *confpath, string &port )
   {
      CHAR sdbConfPath[OSS_MAX_PATHSIZE + 1] = { 0 } ;
      INT32 rc = SDB_OK ;
      po::options_description desc ;
      po::variables_map vm ;
      desc.add_options()
         ( PMD_OPTION_CM_PORT, po::value<string>(), "" );

      utilBuildFullPath( confpath, SDB_CM_FILE_PATH,
                         OSS_MAX_PATHSIZE, sdbConfPath ) ;

      rc = utilReadConfigureFile( sdbConfPath, desc, vm ) ;
      if ( SDB_IO == rc )
      {
         rc = SDB_OK ;
         goto done ;
      }
      if ( rc )
      {
         goto error ;
      }
      if( vm.count ( PMD_OPTION_CM_PORT ) )
      {
         port = vm [ PMD_OPTION_CM_PORT ].as<string>() ;
      }

   done :
      return rc ;
   error :
      goto done ;

   }

   //print node's detail configuration by sdb conf file and svcname
   static void printfDetail( const CHAR *confpath, const CHAR *svcname, INT32 type )
   {
      string nodename ;
      string dbpath ;
      string role ;
      string catalogaddr ;
      FILE *fp = NULL ;
      CHAR temp[ 500 ] = { 0 } ;

      CHAR sdbConfTemp[OSS_MAX_PATHSIZE + 1] = { 0 } ;
      CHAR sdbConfPath[OSS_MAX_PATHSIZE + 1] = { 0 } ;

      if( (SDB_TYPE)type == SDB_TYPE_DB )
      {
         ossSnprintf(sdbConfTemp,OSS_MAX_PATHSIZE,SDB_CONF_FILE_PATH_FORMAT,
                     svcname ) ;
         utilBuildFullPath( confpath, sdbConfTemp,
                            OSS_MAX_PATHSIZE, sdbConfPath ) ;
         utilGetSdbConfDetail( sdbConfPath, nodename, dbpath, role, catalogaddr ) ;

         ossPrintf( "\t%s=%s"OSS_NEWLINE, PMD_OPTION_SVCNAME, nodename.c_str() ) ;
         ossPrintf( "\t%s=%s"OSS_NEWLINE, PMD_OPTION_DBPATH, dbpath.c_str() ) ;
         ossPrintf( "\t%s=%s"OSS_NEWLINE, PMD_OPTION_ROLE, role.c_str() ) ;
         ossPrintf( "\t%s=%s"OSS_NEWLINE, PMD_OPTION_CATALOGADDR, catalogaddr.c_str() ) ;
      }
      else
      {
         utilBuildFullPath( confpath, SDB_CM_FILE_PATH,
                            OSS_MAX_PATHSIZE, sdbConfPath ) ;
         fp = fopen( sdbConfPath, "r" ) ;
         while( TRUE )
         {
           fgets( temp, 500, fp ) ;
           if( feof(fp) )
           {
               fclose(fp);
               break ;
           }
           if('#' == temp[0] || '\n' == temp[0]|| '\r' == temp [0]) continue ;

           ossPrintf("\t%s", temp ) ;
           ossMemset(temp, 500, 0 ) ;
         }
      }
   }

   //print node's expand configuration by sdb conf file and svcname
   static void printfExpand( const CHAR *confpath, const CHAR *svcname,
                              INT32 type )
   {
      FILE *fp = NULL ;
      CHAR temp[ 500 ] = { 0 } ;
      CHAR sdbConfTemp[OSS_MAX_PATHSIZE + 1] = { 0 } ;
      CHAR sdbConfPath[OSS_MAX_PATHSIZE + 1] = { 0 } ;
      ossSnprintf(sdbConfTemp,OSS_MAX_PATHSIZE,SDB_CONF_FILE_PATH_FORMAT,
                  svcname ) ;
      if( (SDB_TYPE)type == SDB_TYPE_DB )
      {
         utilBuildFullPath( confpath, sdbConfTemp,
                            OSS_MAX_PATHSIZE, sdbConfPath ) ;
      }
      else
      {
        utilBuildFullPath( confpath,SDB_CM_FILE_PATH,
                           OSS_MAX_PATHSIZE, sdbConfPath ) ;
      }

      fp = fopen( sdbConfPath, "r" ) ;
      while( TRUE )
      {
         fgets( temp, 500, fp ) ;
         if( feof(fp) )
         {
            fclose(fp) ;
            break ;
         }
         if('#' == temp[0] || '\n' == temp[0]|| '\r' == temp [0]) continue ;

         ossPrintf("\t%s", temp ) ;
         ossMemset(temp, 500, 0 ) ;
      }

   }

   //
  static void printfAll(const CHAR *confpath, string &svcname,INT32 type,
                        BOOLEAN detail, BOOLEAN expand, INT32 pid, BOOLEAN online )
   {
      if( online )
      {
        ossPrintf( "%s(%s) (%d)"OSS_NEWLINE, utilDBTypeStr( (SDB_TYPE)type ),
                   svcname.c_str(), pid ) ;
        if(detail)
        {
            printfDetail(confpath, svcname.c_str(), (SDB_TYPE)type ) ;
        }
        if(expand)
        {
            printfExpand(confpath, svcname.c_str(), (SDB_TYPE)type ) ;
        }
      }
      else
      {
         ossPrintf( "%s(%s) (--)"OSS_NEWLINE, utilDBTypeStr( (SDB_TYPE)type),
                    svcname.c_str()) ;
         if(detail)
         {
             printfDetail(confpath, svcname.c_str(), SDB_TYPE_DB ) ;
         }
         if(expand)
         {
             printfExpand(confpath, svcname.c_str(), SDB_TYPE_DB ) ;
         }
      }
   }

   static const CHAR* utilDBRunModeTypeStr( SDB_RUN_MODE_TYPE type )
   {
      switch ( type )
      {
         case LOCAL:
            return SDB_RUN_MODE_TYPE_LOCAL_STR ;
         case RUN:
            return SDB_RUN_MODE_TYPE_RUN_STR ;
         default:
            break ;
      }
      return "Unknown" ;
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
         if ( 0 == ossStrcasecmp( listType.c_str(), "db" ) )
         {
            typeFilter = SDB_TYPE_DB ;
         }
         else if ( 0 == ossStrcasecmp( listType.c_str(), "om" ) )
         {
            typeFilter = SDB_TYPE_OM ;
         }
         else if ( 0 == ossStrcasecmp( listType.c_str(), "cm" ) )
         {
            typeFilter = SDB_TYPE_OMA ;
         }
         else if ( 0 == ossStrcasecmp( listType.c_str(), "all" ) )
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
         string mode_temp = vm[PMD_OPTION_MODE].as<string>() ;
         if( 0 == ossStrcasecmp( mode_temp.c_str(), "local" ))
         {
            mode = LOCAL ;
         }
         else if( 0 == ossStrcasecmp( mode_temp.c_str(), "run" ))
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
         if( 0 == ossStrcasecmp( roleTemp.c_str(), "coord" ))
         {
            role = SDB_ROLE_COORD ;
         }
         else if( 0 == ossStrcasecmp( roleTemp.c_str(),"catalog" ))
         {
            role = SDB_ROLE_CATALOG ;
         }
         else if( 0 == ossStrcasecmp( roleTemp.c_str(),"data"))
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
   INT32 mainEtnry ( INT32 argc, CHAR **argv )
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
      string roleTemp ;
      string portTemp ;
      INT32 mode = LOCAL ;
      CHAR rootPath[OSS_MAX_PATHSIZE] = { 0 } ;


      getcwd(rootPath,OSS_MAX_PATHSIZE) ;
      po::options_description desc ( "Command options" ) ;
      init ( desc ) ;

      // validate arguments
      rc = resolveArgument ( desc, argc, argv, listServices, typeFilter,
                             mode, role, detail, expand ) ;
      if ( rc )
      {
         if ( SDB_PMD_HELP_ONLY != rc && SDB_PMD_VERSION_ONLY != rc )
         {
            std::cout << "Invalid argument" << endl ;
            displayArg ( desc ) ;
         }
         goto done ;
      }

      if ( listServices.size() > 0 )
      {
         // if used -p, so list all nodes and cm,except cmd
         typeFilter = -1 ;
         utilListNodes( listNodes, typeFilter ) ;

         for ( UINT32 i = 0 ; i < listNodes.size() ; ++i )
         {
            utilNodeInfo &info = listNodes[ i ] ;

            if ( listServices.size() > 0 )
            {
               bFind = FALSE ;
               for ( UINT32 j = 0 ; j < listServices.size() ; ++j )
               {
                  if ( 0 == ossStrcmp( info._svcname.c_str(),
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

            if ( bFind )
            {
               ++total ;
               printfAll( rootPath, info._svcname, info._type, detail,
                          expand, info._pid, TRUE ) ;
            }
         }
      }

      // not use -p
      else
      {
         utilGetCMPort( rootPath, portTemp ) ;
         if ( typeFilter == -1 || (SDB_TYPE)typeFilter == SDB_TYPE_DB )
         {
            utilListAllNodes(rootPath, listAllNodes) ;
         }
         if( typeFilter == -1 || (SDB_TYPE)typeFilter == SDB_TYPE_OMA )
         {
            listAllNodes.push_back( portTemp ) ;
         }
         utilListNodes(listNodes,typeFilter) ;
         for( UINT32 i = 0; i < listAllNodes.size(); i++ )
         {
            string &info = listAllNodes[i] ;
            bFind = FALSE ;
            for ( UINT32 j = 0; j < listNodes.size(); j++ )
            {
               // find and run
               if ( 0 == ossStrcmp(info.c_str(),listNodes[j]._svcname.c_str()))
               {
                  bFind = TRUE ;
                  if( role == -1 )
                  {
                     ++total ;
                     printfAll( rootPath, listNodes[j]._svcname,listNodes[j]._type,
                                detail, expand, listNodes[j]._pid, TRUE ) ;

                  }
                  // check role
                  else
                  {
                     // find cm,but not sdb
                     if( (SDB_TYPE)listNodes[j]._type != SDB_TYPE_DB )
                     {
                        bFind = TRUE;
                        break ;
                     }
                     utilGetNodeRole(rootPath, listNodes[j]._svcname.c_str(),
                                     roleTemp ) ;
                     if( 0 == ossStrcmp(roleTemp.c_str(),
                                        utilDBRoleStr( (SDB_ROLE)role )))
                     {
                        ++total ;
                        printfAll( rootPath, listNodes[j]._svcname,listNodes[j]._type,
                                detail, expand, listNodes[j]._pid, TRUE ) ;
                     }
                   }
                }

                if( bFind ) break;
            }
            if( bFind ) continue ;
            //not run,check run mode type
            if ( 0 == ossStrcmp("local", utilDBRunModeTypeStr(
                                (SDB_RUN_MODE_TYPE)mode )))
            {
               if( role == -1 )
               {
                  ++total ;
                  printfAll( rootPath, info, SDB_TYPE_DB, detail,
                             expand, 0, FALSE ) ;

               }
               // not run ,check role
               else
               {
                  utilGetNodeRole(rootPath, info.c_str(),
                                  roleTemp ) ;
                  if( 0 == ossStrcmp(roleTemp.c_str(),
                                     utilDBRoleStr((SDB_ROLE) role )))
                  {
                        ++total ;
                        printfAll( rootPath, info, SDB_TYPE_DB, detail,
                                   expand, 0, FALSE ) ;
                  }
               }
            }
         }
      }
      // if no -p, and list all/list cm, need to show sdbcmd
      if ( listServices.size() == 0 && ( SDB_TYPE_OMA == typeFilter ||
           -1 == typeFilter ) && role == -1)
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
   }
}

INT32 main ( INT32 argc, CHAR **argv )
{
   return engine::mainEtnry( argc, argv ) ;
}


