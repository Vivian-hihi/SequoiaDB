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

   Source File Name = omagentNodeCmd.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          20/08/2014  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#include "omagentNodeCmd.hpp"
#include "rtnCommandDef.hpp"
#include "omagentMgr.hpp"

using namespace bson ;

namespace engine
{

   /*
      _omaCreateNodeCmd implement
   */
   IMPLEMENT_OACMD_AUTO_REGISTER( _omaCreateNodeCmd )

   _omaCreateNodeCmd::_omaCreateNodeCmd()
   {
   }

   _omaCreateNodeCmd::~_omaCreateNodeCmd()
   {
   }

   const CHAR* _omaCreateNodeCmd::name()
   {
      return NAME_CREATE_NODE ;
   }

   INT32 _omaCreateNodeCmd::init( const CHAR * pInfomation )
   {
      INT32 rc = SDB_OK ;

      try
      {
         BSONObj obj( pInfomation ) ;
         _config = obj ;

         BSONElement e = obj.getField( FIELD_NAME_GROUPNAME ) ;
         if ( String != e.type() )
         {
            PD_LOG( PDERROR, "Field[%s] type[%d] error in command[%s]",
                    e.fieldName(), e.type(), name() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         else if ( 0 != ossStrcmp( e.valuestr(), COORD_GROUPNAME ) )
         {
            PD_LOG( PDERROR, "Group[%s] is not %s in command[%s]",
                    e.valuestr(), COORD_GROUPNAME, name() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Ocurr exception: %s", e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _omaCreateNodeCmd::doit( BSONObj & retObj )
   {
      BSONObj dummy ;
      return sdbGetOMAgentMgr()->getNodeMgr()->addANode( _config.objdata(),
                                                         dummy.objdata() ) ;
   }

   /*
      _omaRemoveNodeCmd implement
   */
   IMPLEMENT_OACMD_AUTO_REGISTER( _omaRemoveNodeCmd )

   _omaRemoveNodeCmd::_omaRemoveNodeCmd()
   {
   }

   _omaRemoveNodeCmd::~_omaRemoveNodeCmd()
   {
   }

   const CHAR* _omaRemoveNodeCmd::name()
   {
      return NAME_REMOVE_NODE ;
   }

   INT32 _omaRemoveNodeCmd::doit( BSONObj & retObj )
   {
      BSONObj dummy ;
      return sdbGetOMAgentMgr()->getNodeMgr()->rmANode( _config.objdata(),
                                                        dummy.objdata() ) ;
   }

   /*
      _omaStartNodeCmd implement
   */
   IMPLEMENT_OACMD_AUTO_REGISTER( _omaStartNodeCmd )

   _omaStartNodeCmd::_omaStartNodeCmd()
   {
      _pData = NULL ;
   }

   _omaStartNodeCmd::~_omaStartNodeCmd()
   {
   }

   const CHAR* _omaStartNodeCmd::name()
   {
      return NAME_START_NODE ;
  }

   INT32 _omaStartNodeCmd::init( const CHAR * pInfomation )
   {
      _pData = pInfomation ;
      return SDB_OK ;
   }

   INT32 _omaStartNodeCmd::doit( BSONObj & retObj )
   {
      return sdbGetOMAgentMgr()->getNodeMgr()->startANode( _pData ) ;
   }

   /*
      _omaStopNodeCmd implement
   */
   IMPLEMENT_OACMD_AUTO_REGISTER( _omaStopNodeCmd )

   _omaStopNodeCmd::_omaStopNodeCmd()
   {
   }

   _omaStopNodeCmd::~_omaStopNodeCmd()
   {
   }

   const CHAR* _omaStopNodeCmd::name()
   {
      return NAME_SHUTDOWN_NODE ;
   }

   INT32 _omaStopNodeCmd::doit( BSONObj & retObj )
   {
      return sdbGetOMAgentMgr()->getNodeMgr()->stopANode( _pData ) ;
   }

}

