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

#include "restAdaptor.hpp"
#include "pmdRestSession.hpp"
#include <map>
#include <string>

namespace engine
{
    class omGetFileCommand
    {
        public:
            omGetFileCommand( restAdaptor *pRestAdaptor, pmdRestSession *pRestSession, 
                              const CHAR *pRootPath, const CHAR *pSubPath ) ;
            virtual ~omGetFileCommand() ;

        public:
            virtual INT32   init() ;
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

    class restSubPathTransfer
    {
        public:
            static restSubPathTransfer* getTransferInstance() ;

            INT32 getTransferedString( const char *src, string &transfered ) ;

        private:
            restSubPathTransfer() ;
            restSubPathTransfer(const restSubPathTransfer &) ;
            restSubPathTransfer& operator = ( const restSubPathTransfer & ) ;

        private:
            typedef map < string, string >::iterator mapIteratorType ; 
            typedef map < string, string >::value_type mapValueType ;
            map < string, string > _transfer ;
    };
}

#endif /* OM_GETFILECOMMAND_HPP_ */

