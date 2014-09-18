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

   Source File Name = omCommandInterface.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/12/2014  LYB Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef OM_COMMANDINTERFACE_HPP_
#define OM_COMMANDINTERFACE_HPP_

#include "rtnCB.hpp"
#include "pmd.hpp"
#include "dmsCB.hpp"
#include <map>
#include <string>

using namespace bson ;

namespace engine
{
   class omCommandInterface : public SDBObject
   {
      public:
         omCommandInterface() ;
         virtual ~omCommandInterface() ;

      public:
         virtual INT32     init( pmdEDUCB * cb ) ;
         virtual INT32     doCommand() = 0 ;
         virtual INT32     undoCommand() ;
         virtual bool      isFetchAgentResponse( UINT64 requestID ) ;
         virtual INT32     doAgentResponse ( MsgHeader* pAgentResponse ) ;

      protected:
         INT32             _getBusinessInfo( string business, 
                                             BSONObj &businessInfo ) ;
         INT32             _deleteHost( const string &hostName ) ;
         INT32             _getClusterInfo( const string &clusterName, 
                                            BSONObj &clusterInfo ) ;
      protected:
         SDB_RTNCB         *_pRTNCB ;
         SDB_DMSCB         *_pDMDCB ;
         pmdKRCB           *_pKRCB ;
         SDB_DMSCB         *_pDMSCB ;
         
         pmdEDUCB          *_cb ;

         string            _errorDetail ;
   };
}

#endif /* OM_COMMANDINTERFACE_HPP_ */


