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

   Source File Name = utilParam.cpp

   Descriptive Name =

   When/how to use: str util

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          26/04/2014  XJH Initial Draft

   Last Changed =

******************************************************************************/


#include "utilParam.hpp"
#include <iostream>


namespace engine
{

   INT32 utilReadConfigureFile( const CHAR *file,
                                po::options_description &desc,
                                po::variables_map &vm )
   {
      INT32 rc = SDB_OK;

      try
      {
         po::store ( po::parse_config_file<char> ( file, desc, TRUE ), vm ) ;
         po::notify ( vm ) ;
      }
      catch( po::reading_file )
      {
         std::cerr << "Failed to open config file: "
                   <<( std::string ) file << std::endl ;
         rc = SDB_IO ;
         goto error ;
      }
      catch ( po::unknown_option &e )
      {
         std::cerr << "Unknown config element: "
                   << e.get_option_name () << std::endl ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      catch ( po::invalid_option_value &e )
      {
         std::cerr << ( std::string ) "Invalid config element: "
                   << e.get_option_name () << std::endl ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      catch( po::error &e )
      {
         std::cerr << e.what () << std::endl ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done:
      return rc;
   error:
      goto done;
   }

   INT32 utilReadCommandLine( INT32 argc, CHAR **argv,
                              po::options_description &desc,
                              po::variables_map &vm )
   {
      INT32 rc = SDB_OK;

      try
      {
         po::store ( po::command_line_parser( argc, argv).options(
                     desc ).allow_unregistered().run(), vm ) ;
         po::notify ( vm ) ;
      }
      catch ( po::unknown_option &e )
      {
         std::cerr <<  "Unknown argument: "
                   << e.get_option_name () << std::endl ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      catch ( po::invalid_option_value &e )
      {
         std::cerr << "Invalid argument: "
                   << e.get_option_name () << std::endl ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      catch( po::error &e )
      {
         std::cerr << e.what () << std::endl ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

}


