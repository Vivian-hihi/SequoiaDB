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

   Source File Name = omCommandInterface.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/12/2014  LYB Initial Draft

   Last Changed =

*******************************************************************************/

#include "omCommandInterface.hpp"

using namespace bson;

namespace engine
{
   omCommandInterface::omCommandInterface()
   {
      _pKRCB  = pmdGetKRCB() ;
      _pDMDCB = _pKRCB->getDMSCB() ;
      _pRTNCB = _pKRCB->getRTNCB() ;
      _pDMSCB = _pKRCB->getDMSCB() ;
      _cb     = NULL ;
   }

   omCommandInterface::~omCommandInterface()
   {
   }

   INT32 omCommandInterface::init( pmdEDUCB * cb )
   {
      _cb = cb ;

      return SDB_OK ;
   }

   INT32 omCommandInterface::undoCommand()
   {
      return SDB_OK ;
   }

   bool omCommandInterface::isFetchAgentResponse( UINT64 requestID )
   {
      return false ;
   }

   INT32 omCommandInterface::doAgentResponse ( MsgHeader* pAgentResponse )
   {
      return SDB_OK ;
   }
}

