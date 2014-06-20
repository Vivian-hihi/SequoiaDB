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

   Source File Name = sptUsrCmd.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "sptUsrCmd.hpp"
#include "sptCmdRunner.hpp"
#include "ossMem.hpp"
#include "ossUtil.hpp"

using namespace bson ;

static const UINT32 SPT_STACK_OUTPUT = 1024 * 2 ;

namespace engine
{
JS_STATIC_FUNC_DEFINE( _sptUsrCmd, exec )
//JS_CONSTRUCT_FUNC_DEFINE( _sptUsrCmd, construct )
JS_BEGIN_MAPPING( _sptUsrCmd, "Cmd" )
   JS_ADD_STATIC_FUNC( "run", exec )
//   JS_ADD_CONSTRUCT_FUNC( construct)
JS_MAPPING_END()
/*
   INT32 _sptUsrCmd::construct( const _sptArguments &arg,
                                _sptReturnVal &rval,
                                bson::BSONObj &detail )
   {
      return SDB_OK ;
   }
*/
   INT32 _sptUsrCmd::exec( const _sptArguments &arg,
                           _sptReturnVal &rval,
                           bson::BSONObj &detail )
   {
      INT32 rc = SDB_OK ;
      string cmd ;
      string ev ;
      UINT32 exit = SDB_OK ;
      sptCmdRunner runner ;

      rc = arg.getString( 0, cmd ) ;
      if ( SDB_OK != rc )
      {
         rc = SDB_INVALIDARG ;
         detail = BSON( SPT_ERR << "need at least one argument" ) ;
         goto error ;
      }

      rc = arg.getString( 1, ev ) ;
      if ( SDB_OK != rc && SDB_OUT_OF_BOUND != rc )
      {
         rc = SDB_INVALIDARG ;
         detail = BSON( SPT_ERR << "environment should be a string" ) ;
         goto error ;
      }

      rc = runner.exec( cmd.c_str(), exit ) ; 
      if ( SDB_OK != rc )
      {
         detail = BSON( SPT_ERR << BSON( "errno" << rc ) ) ;
         rc = SDB_SPT_EVAL_FAIL ;
         goto error ;
      }
      else
      {
         rc = _setRVal( &runner, rval, exit == SDB_OK, detail ) ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }

         if ( SDB_OK != exit )
         {
            rc = SDB_SPT_EVAL_FAIL ;
         }
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptUsrCmd::_setRVal( _sptCmdRunner *runner,
                               _sptReturnVal &rval,
                               BOOLEAN setToRVal,
                               BSONObj &detail )
   {
      INT32 rc = SDB_OK ;
      CHAR stackBuf[SPT_STACK_OUTPUT + 1] ;
      CHAR *allocateBuf = NULL ;
      CHAR *mybuf = ( CHAR * )stackBuf ;
      SINT64 total = 0 ;
      SINT64 expected = SPT_STACK_OUTPUT ;
      SINT64 maxLen = SPT_STACK_OUTPUT * 1024 ;
      while ( TRUE )
      {
         SINT64 once = 0 ;
         rc = runner->read( mybuf + total, expected, once ) ;
         if ( SDB_EOF == rc )
         {
            rc = SDB_OK ;
            break ;
         }
         else if ( SDB_OK == rc )
         {
            total += once ;
            if ( NULL != allocateBuf )
            {
               SDB_ASSERT( total <= maxLen, "impossible" ) ;
               if ( maxLen == total )
               {
                  break ;
               }
               else
               {
                  expected = maxLen - total ;
               }
            }
            else
            {
               SDB_ASSERT( total <= SPT_STACK_OUTPUT, "impossible" ) ;
               if ( SPT_STACK_OUTPUT == total )
               {
                  allocateBuf = ( CHAR *)SDB_OSS_MALLOC( maxLen + 1 ) ;
                  if ( NULL == mybuf )
                  {
                     PD_LOG( PDERROR, "failed to allocate mem." ) ;
                     rc = SDB_OOM ;
                     goto error ;
                  }

                  ossMemcpy( allocateBuf, stackBuf, SPT_STACK_OUTPUT ) ;
                  expected = maxLen - total ;
                  mybuf = allocateBuf ;
               }
               else
               {
                  expected = SPT_STACK_OUTPUT - total ;
               }
            }
         }
      }

      mybuf[total] = '\0' ;

      if ( setToRVal )
      {
         rval.setStringVal( "", mybuf ) ;
      }
      else
      {
         detail = BSON( SPT_ERR << mybuf ) ;
      }
   done:
      if ( NULL != allocateBuf )
      {
         SDB_OSS_FREE( allocateBuf ) ;
      }
      return rc  ;
   error:
      goto done ;
   }
}

