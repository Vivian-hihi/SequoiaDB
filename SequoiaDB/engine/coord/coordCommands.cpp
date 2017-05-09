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

   Source File Name = coordCommands.cpp

   Descriptive Name = Coord Commands

   When/how to use: this program may be used on binary and text-formatted
   versions of runtime component. This file contains code logic for
   user command processing on coordinator node.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =

*******************************************************************************/

#include "coordCommands.hpp"
#include "msgMessage.hpp"
#include "pdTrace.hpp"
#include "coordTrace.hpp"

using namespace bson ;

namespace engine
{

   /*
      _coordCMDSetSessionAttr implement
   */
   COORD_IMPLEMENT_CMD_AUTO_REGISTER( _coordCMDSetSessionAttr,
                                      CMD_NAME_SETSESS_ATTR,
                                      TRUE ) ;
   _coordCMDSetSessionAttr::_coordCMDSetSessionAttr()
   {
   }

   _coordCMDSetSessionAttr::~_coordCMDSetSessionAttr()
   {
   }

   //PD_TRACE_DECLARE_FUNCTION( COORD_SETSESSIONATTR_EXE, "_coordCMDSetSessionAttr::execute" )
   INT32 _coordCMDSetSessionAttr::execute( MsgHeader *pMsg,
                                           pmdEDUCB *cb,
                                           INT64 &contextID,
                                           rtnContextBuf *buf )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( COORD_SETSESSIONATTR_EXE ) ;
      coordSessionPropSite *pPropSite = NULL ;
      pmdRemoteSessionSite *pSite = NULL ;
      // fill default-reply(delete success)
      contextID = -1 ;

      CHAR *pQuery                     = NULL ;
      rc = msgExtractQuery( (CHAR*)pMsg, NULL, NULL, NULL, NULL,
                            &pQuery, NULL, NULL, NULL );
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to parse unlink collection request(rc=%d)",
                   rc ) ;

      pSite = ( pmdRemoteSessionSite* )cb->getRemoteSite() ;
      if ( pSite )
      {
         pPropSite = ( coordSessionPropSite* )pSite->getUserData() ;
      }
      if ( !pPropSite )
      {
         PD_LOG( PDERROR, "Session's prop site is NULL" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      try
      {
         BSONElement ele ;
         BSONObj boQuery( pQuery ) ;
         BSONObjIterator itr( boQuery ) ;
         while( itr.more() )
         {
            ele = itr.next() ;

            /// preferedInstance
            if ( 0 == ossStrcmp( ele.fieldName(),
                                 FIELD_NAME_PREFERED_INSTANCE ) )
            {
               INT32 sessReplType = PREFER_REPL_TYPE_MIN ;
               PD_CHECK( ele.type() == NumberInt, SDB_INVALIDARG, error,
                         PDERROR, "Field[%s] is not numberInt",
                         FIELD_NAME_PREFERED_INSTANCE );
               sessReplType = ele.Int();
               PD_CHECK( sessReplType > PREFER_REPL_TYPE_MIN &&
                         sessReplType < PREFER_REPL_TYPE_MAX,
                         SDB_INVALIDARG, error, PDERROR,
                         "Failed to set preferedInstanace, invalid value[%d], "
                         "Value range:(%d~%d)", sessReplType,
                         PREFER_REPL_TYPE_MIN, PREFER_REPL_TYPE_MAX ) ;

               /// set and clear last nodes
               pPropSite->setPreferInsType( sessReplType ) ;
               pPropSite->clear() ;
            }
            /// timeout
            else if ( 0 == ossStrcmp( ele.fieldName(), FIELD_NAME_TIMEOUT ) )
            {
               INT64 timeout = -1 ;
               PD_CHECK( ele.isNumber(), SDB_INVALIDARG, error,
                         PDERROR, "Feild[%s] is not number",
                         FIELD_NAME_TIMEOUT ) ;
               timeout = (INT64)ele.numberLong() ;

               pPropSite->setOprTimeout( timeout ) ;
            }
            else
            {
               PD_LOG( PDERROR, "Options[%s] is not support in operator[%s]",
                       ele.toString().c_str(), getName() ) ;
               rc = SDB_INVALIDARG ;
               goto error ;
            }
         }
      }
      catch ( std::exception &e )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Failed to set sessionAttr, received unexpected "
                 "error:%s", e.what() ) ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC ( COORD_SETSESSIONATTR_EXE, rc ) ;
      return rc;
   error:
      goto done;
   }

}

