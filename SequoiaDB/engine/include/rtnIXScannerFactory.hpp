/*******************************************************************************


   Copyright (C) 2011-2018 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Source File Name = rtnIXScanner.hpp

   Descriptive Name = RunTime Index Scanner Factory Header

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains structure for index
   scanner, which is used to traverse index tree.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          11/09/2018  YXC Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef RTNIXSCANNERFAC_HPP__
#define RTNIXSCANNERFAC_HPP__

#include "rtnIXScanner.hpp"
#include "rtnDiskIXScanner.hpp"
#include "rtnMemIXTreeScanner.hpp"
#include "rtnMergeIXScanner.hpp"

namespace engine
{
   // class factory for initializing scanner purpose
   class scannerFactory
   {
      public:
      rtnIXScanner* getScanner( IXScannerType type, ixmIndexCB *indexCB,
                               rtnPredicateList *predList,
                               _dmsStorageUnit *su, _pmdEDUCB *cb,
			       scannerSharedInfo * sharedInfo = NULL )
      {
         rtnIXScanner *scanner = NULL;
         switch (type) 
         {
            case SCANNER_TYPE_DISK:
                 scanner = SDB_OSS_NEW rtnDiskIXScanner( indexCB, predList,
                                                      su, cb, sharedInfo ) ;
                 break;
            case SCANNER_TYPE_MEM_TREE:
		 scanner = SDB_OSS_NEW rtnMemIXTreeScanner( indexCB, predList,
                                                         su, cb, sharedInfo ) ;
                 break;
            case SCANNER_TYPE_MERGE:
                 scanner = SDB_OSS_NEW rtnMergeIXScanner( indexCB, predList,
                                                       su, cb, sharedInfo ) ;
                 break;
            default:
                 scanner = NULL;
                 break;
         }
         return scanner;
      } 
   };


}

#endif //RTNIXSCANNERFAC_HPP__

