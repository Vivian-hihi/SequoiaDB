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
#include <string>
#include <iostream>
#include <vector>

using namespace std;

namespace engine
{

   #define COMMANDS_OPTIONS \
       ( PMD_COMMANDS_STRING( PMD_OPTION_HELP, ",h"), "help" ) \
       ( PMD_OPTION_VERSION, "show version" ) \
       ( PMD_COMMANDS_STRING( PMD_OPTION_SVCNAME, ",p"), boost::program_options::value<string>(), "service name, use ',' to seperator" )

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

   // PD_TRACE_DECLARE_FUNCTION ( SDB_SVCSPLIT2, "serviceSplit" )
   INT32 serviceSplit ( const string &input, vector<string> &listServices )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_SVCSPLIT2 );
      CHAR *cstr = NULL ;
      CHAR *p = NULL ;
      CHAR *pContext = NULL ;
      INT32 bufSize = input.size() ;

      cstr = (CHAR*)SDB_OSS_MALLOC ( bufSize + 1 ) ;
      if ( !cstr )
      {
         std::cout << "Alloc memory(" << bufSize + 1 << ") failed" << endl ;
         rc = SDB_OOM ;
         goto error ;
      }
      ossMemset ( cstr, 0, bufSize + 1 ) ;
      ossStrncpy ( cstr, input.c_str(), bufSize ) ;

      p = ossStrtok ( cstr, ", \t", &pContext ) ;
      while ( p )
      {
         string ts ( p ) ;
         listServices.push_back ( ts ) ;
         p = ossStrtok ( NULL, ", \t", &pContext ) ;
      }

   done :
      if ( cstr )
      {
         SDB_OSS_FREE ( cstr ) ;
      }
      PD_TRACE_EXITRC ( SDB_SVCSPLIT2, rc );
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_SDBLIST_RESVARG, "resolveArgument" )
   INT32 resolveArgument ( po::options_description &desc,
                           INT32 argc, CHAR **argv,
                           vector<string> &listServices )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_SDBLIST_RESVARG ) ;
      po::variables_map vm ;

      rc = utilReadCommandLine( argc, argv,  desc, vm ) ;
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
         rc = serviceSplit ( svcname, listServices ) ;
         if ( rc )
         {
            std::cout << "Parse svcname failed: " << rc << endl ;
            goto error ;
         }
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
      UTIL_VEC_NODES listNodes ;
      BOOLEAN bFind = TRUE ;

      po::options_description desc ( "Command options" ) ;
      init ( desc ) ;

      // validate arguments
      rc = resolveArgument ( desc, argc, argv, listServices ) ;
      if ( rc )
      {
         if ( SDB_PMD_HELP_ONLY != rc &&
              SDB_PMD_VERSION_ONLY != rc )
         {
            std::cout << "Invalid argument" << endl ;
            displayArg ( desc ) ;
         }
         goto done ;
      }

      utilListNodes( listNodes, SDB_TYPE_DB ) ;

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
            ossPrintf( "%s(%s) (%s)"OSS_NEWLINE,
                       utilDBTypeStr( (SDB_TYPE)info._type ),
                       info._svcname.c_str(), info._pid ) ;
         }
      }

      ossPrintf ( "Total: %d"OSS_NEWLINE, total ) ;

   done :
      PD_TRACE_EXITRC ( SDB_SDBLIST_MAIN, rc );
      return SDB_OK == rc ? 0 : 1 ;
   }

}

INT32 main ( INT32 argc, CHAR **argv )
{
   return engine::mainEtnry( argc, argv ) ;
}


