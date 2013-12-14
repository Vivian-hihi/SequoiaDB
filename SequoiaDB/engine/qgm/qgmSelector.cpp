/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmSelector.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains functions for agent processing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft

   Last Changed =

******************************************************************************/

#include "qgmSelector.hpp"
#include "pd.hpp"
#include "pdTrace.hpp"
#include "qgmTrace.hpp"
#include <sstream>

namespace engine
{
   _qgmSelector::_qgmSelector()
   :_hasAlias( FALSE )
   {

   }

   _qgmSelector::~_qgmSelector()
   {
      _selector.clear() ;
   }

   string _qgmSelector::toString() const
   {
      stringstream ss ;
      if ( !_selector.empty() )
      {
         ss << "[" ;
         qgmOPFieldVec::const_iterator itr = _selector.begin() ;
         for ( ; itr != _selector.end(); itr++ )
         {
            if ( SQL_GRAMMAR::WILDCARD == itr->type )
            {
               ss << "{value:*}," ;
            }
            else
            {
               ss << "{value:" << itr->value.toString()
                  << ",alias:" << itr->alias.toString()
                 << "}," ;
            }
         }
         ss.seekp((INT32)ss.tellp()-1 ) ;
         ss << "]" ;
      }
      else
      {
         ss << "[*]" ;
      }
      return ss.str() ;
   }

   INT32 _qgmSelector::load( const qgmOPFieldVec &op )
   {
      INT32 rc = SDB_OK ;
      qgmOPFieldVec::const_iterator itr = op.begin() ;
      for ( ; itr != op.end(); itr++ )
      {
         _selector.push_back( *itr ) ;
         if ( !( itr->alias.empty() ) )
         {
            _hasAlias = TRUE ;
         }
      }
      return rc ;
   }

   PD_TRACE_DECLARE_FUNCTION( SDB__QGMSELECTOR_SELECT, "_qgmSelector::select" )
   INT32 _qgmSelector::select( const BSONObj &src, BSONObj &out ) const
   {
      PD_TRACE_ENTRY( SDB__QGMSELECTOR_SELECT ) ;
      INT32 rc = SDB_OK ;
      if ( _selector.empty() )
      {
         out = src ;
         goto done ;
      }
      try
      {
         BSONObjBuilder builder ;
         qgmOPFieldVec::const_iterator itr = _selector.begin() ;
         for ( ; itr != _selector.end(); itr++ )
         {
            if ( SQL_GRAMMAR::WILDCARD == itr->type )
            {
               out = src ;
               goto done ;
            }
            {
            BSONElement ele = src.getFieldDotted( itr->value.attr().toString() ) ;
            if ( ele.eoo() )
            {
               if ( itr->alias.empty() )
               {
                  builder.appendNull( itr->value.attr().toString() ) ;
               }
               else
               {
                  builder.appendNull( itr->alias.toString() ) ;
               }
            }
            else
            {
               if ( itr->alias.empty() )
               {
                  builder.append( ele ) ;
               }
               else
               {
                  builder.appendAs( ele, itr->alias.toString() ) ;
               }
            }
            }
         }

         out = builder.obj() ;
      }
      catch (std::exception &e)
      {
         PD_LOG( PDERROR, "unexcepted err happened:%s",
                 e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
   done:
      PD_TRACE_EXITRC( SDB__QGMSELECTOR_SELECT, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION( SDB__QGMSELECTOR_SELECT2, "_qgmSelector::select" )
   INT32 _qgmSelector::select( const qgmFetchOut &src,
                               BSONObj &out ) const
   {
      PD_TRACE_ENTRY( SDB__QGMSELECTOR_SELECT2 ) ;
      INT32 rc = SDB_OK ;
      if ( _selector.empty() )
      {
         out = src.mergedObj() ;
         goto done ;
      }
      else
      {
         BSONObjBuilder builder ;
         BSONElement ele ;
         try
         {
            qgmOPFieldVec::const_iterator itr = _selector.begin() ;
            for ( ; itr != _selector.end(); itr++ )
            {
               if ( SQL_GRAMMAR::WILDCARD == itr->type )
               {
                  out = src.mergedObj() ;
                  goto done ;
               }

               rc = src.element( itr->value, ele ) ;
               if ( SDB_INVALIDARG == rc )
               {
                  if ( itr->alias.empty() )
                  {
                     builder.appendNull( itr->value.attr().toString() ) ;
                  }
                  else
                  {
                     builder.appendNull( itr->alias.toString() ) ;
                  }
                  rc = SDB_OK ;
               }
               else if ( SDB_OK != rc )
               {
                  goto error ;
               }
               else if ( ele.eoo() )
               {
                  PD_LOG( PDERROR, "ele.eoo()" ) ;
                  rc = SDB_SYS ;
                  goto error ;
               }
               else if ( itr->alias.empty() )
               {
                  builder.append( ele ) ;
               }
               else
               {
                  builder.appendAs( ele, itr->alias.toString() ) ;
               }
            }

            out = builder.obj() ;
         }
         catch ( std::exception & e )
         {
            PD_LOG( PDERROR, "unexcepted err happened:%s",
                    e.what() ) ;
            rc = SDB_SYS ;
            goto error ;
         }
      }
   done:
      PD_TRACE_EXITRC( SDB__QGMSELECTOR_SELECT2, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   BSONObj _qgmSelector::selector() const
   {
      BSONObjBuilder builder ;

      qgmOPFieldVec::const_iterator itr = _selector.begin() ;
      for ( ; itr != _selector.end(); itr++ )
      {
         if ( !itr->value.attr().empty() )
         {
            builder.appendNull( itr->value.attr().toString()) ;
         }
      }

      return builder.obj() ;
   }
}
