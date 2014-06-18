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

   Source File Name = omGetFileCommand.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/12/2014  LYB Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef OM_GETFILECOMMAND_HPP_
#define OM_GETFILECOMMAND_HPP_

#include "omCommandInterface.hpp"
#include "restAdaptor.hpp"
#include "pmdRestSession.hpp"
#include "rtnCB.hpp"
#include "pmd.hpp"
#include "dmsCB.hpp"
#include <map>
#include <string>

namespace engine
{
   class omAuthCommand : public omCommandInterface
   {
      public:
         omAuthCommand( restAdaptor *pRestAdaptor, pmdRestSession *pRestSession, 
                       const CHAR *pRootPath) ;

         ~omAuthCommand() ;

      public:
         virtual INT32   doCommand() ;

      private:
         INT32 _verifyUser( const CHAR *pUserName, const CHAR *pPasswd, const CHAR *pTimestamp ) ;
         
      private:
         restAdaptor*    _restAdaptor ;
         pmdRestSession* _restSession ;
         string          _rootPath ;
   };

   class omCheckSessionCommand : public omCommandInterface
   {
      public:
         omCheckSessionCommand( restAdaptor *pRestAdaptor, pmdRestSession *pRestSession ) ;

         ~omCheckSessionCommand() ;

      public:
         virtual INT32   doCommand() ;
         
      private:
         restAdaptor*    _restAdaptor ;
         pmdRestSession* _restSession ;
   };

   class omCreateClusterCommand : public omCommandInterface
   {
      public:
         omCreateClusterCommand( restAdaptor *pRestAdaptor, pmdRestSession *pRestSession ) ;

         ~omCreateClusterCommand() ;

      public:
         virtual INT32   doCommand() ;
         
      private:
         restAdaptor*    _restAdaptor ;
         pmdRestSession* _restSession ;
   };
   
   class omGetFileCommand : public omCommandInterface
   {
      public:
         omGetFileCommand( restAdaptor *pRestAdaptor, pmdRestSession *pRestSession, 
                       const CHAR *pRootPath, const CHAR *pSubPath ) ;
         virtual ~omGetFileCommand() ;

      public:
         virtual INT32   doCommand() ;
         virtual INT32   undoCommand() ;

      private:
         INT32           _getFileContent( string filePath, CHAR **pFileContent, 
                                            INT32 &fileContentLen ) ;

      private:
         restAdaptor*    _restAdaptor ;
         pmdRestSession* _restSession ;
         string          _rootPath ;
         string          _subPath ;

   };

   class restFileController
   {
      public:
         static restFileController* getTransferInstance() ;

         INT32 getTransferedPath( const char *src_file, string &transfered ) ;

         bool isFileAuthorPublic( const char *file ) ;

      private:
         restFileController() ;
         restFileController(const restFileController &) ;
         restFileController& operator = ( const restFileController & ) ;

      private:
         typedef map < string, string >::iterator mapIteratorType ; 
         typedef map < string, string >::value_type mapValueType ;
         map < string, string > _transfer ;

         map < string, string > _publicAccessFiles ;
   };
}

#endif /* OM_GETFILECOMMAND_HPP_ */

