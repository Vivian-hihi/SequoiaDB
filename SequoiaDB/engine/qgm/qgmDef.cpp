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

   Source File Name = qgmDef.cpp

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

*******************************************************************************/

#include "qgmDef.hpp"
#include "qgmUtil.hpp"
#include "qgmPtrTable.hpp"
#include "pdTrace.hpp"
#include "qgmTrace.hpp"

using namespace bson ;

namespace engine
{

   const UINT32 _qgmField::npos = 0xFFFFFFFF ;

   /*
      _qgmField implement
   */
   _qgmField::_qgmField()
   :_ptrTable( NULL ), _begin( "" ), _size( 0 )
   {
   }

   _qgmField::_qgmField( const _qgmField &field )
   :_ptrTable( field._ptrTable ), _begin( field._begin ), _size( field._size)
   {
   }

   _qgmField& _qgmField::operator=(const _qgmField &field )
   {
      _ptrTable = field._ptrTable ;
      _begin = field._begin ;
      _size = field._size ;
      return *this ;
   }

   _qgmField::~_qgmField()
   {
      _begin = "" ;
      _size = 0 ;
   }

   BOOLEAN _qgmField::operator==( const _qgmField &field )const
   {
      if ( _size != field._size )
      {
         return FALSE ;
      }
      const CHAR *l = _begin ;
      const CHAR *r = field._begin ;
      while( *l && *r )
      {
         if ( *l != *r )
         {
            return FALSE ;
         }
         ++l ;
         ++r ;
      }
      return TRUE ;
   }

   BOOLEAN _qgmField::operator!=( const _qgmField &field )const
   {
      if ( _size != field._size )
      {
         return TRUE ;
      }
      const CHAR *l = _begin ;
      const CHAR *r = field._begin ;
      while( *l && *r )
      {
         if ( *l != *r )
         {
            return TRUE ;
         }
         ++l ;
         ++r ;
      }
      return FALSE ;
   }

   BOOLEAN _qgmField::operator<( const _qgmField &field )const
   {
      UINT32 i = 0 ;
      while ( i < this->_size && i < field._size )
      {
         if ( _begin[i] < field._begin[i] )
         {
            return TRUE ;
         }
         else if ( _begin[i] > field._begin[i] )
         {
            return FALSE ;
         }
         else
         {
            ++i ;
         }
      }

      return this->_size < field._size ? TRUE : FALSE ;
   }

   BOOLEAN _qgmField::isSubfix( const _qgmField &field,
                                BOOLEAN includeSame,
                                UINT32 *pPos ) const
   {
      if ( pPos )
      {
         *pPos = _qgmField::npos ;
      }

      UINT32 i = 0 ;
      while( i < _size && i < field._size )
      {
         if ( _begin[i] != field._begin[i] )
         {
            break ;
         }
         ++i ;
      }
      if ( i == field._size )
      {
         if ( i + 2 <= _size && '.' == _begin[i] )
         {
            if ( pPos )
            {
               *pPos = i ;
            }
            return TRUE ;
         }
         else if ( includeSame && i == _size )
         {
            return TRUE ;
         }
      }
      return FALSE ;
   }

   _qgmField _qgmField::subField( UINT32 pos, UINT32 size )
   {
      _qgmField sub ;
      sub._ptrTable = _ptrTable ;

      if ( pos < _size )
      {
         sub._begin = _begin + pos ;
         sub._size = size < ( _size - pos ) ? size : ( _size - pos ) ;
      }
      return sub ;
   }

   _qgmField _qgmField::rootField()
   {
      UINT32 pos = 0 ;
      _qgmField root( *this ) ;

      if( qgmUtilFirstDot( _begin, _size, pos ) )
      {
         root._size = pos ;
      }
      return root ;
   }

   void _qgmField::replace( UINT32 pos, UINT32 size, const _qgmField &field )
   {
      if ( _ptrTable )
      {
         if ( 0 == pos ) // header
         {
            *this = _ptrTable->getField( field, subField( size ) ) ;
         }
         else if ( pos + size >= _size ) // tail
         {
            *this = _ptrTable->getField( subField( pos ), field ) ;
         }
         else // middle
         {
            qgmField tmp = _ptrTable->getField( subField( 0, pos ),
                                                field ) ;
            *this = _ptrTable->getField( tmp, subField( size ) ) ;
         }
      }
   }

   string _qgmField::toFieldName() const
   {
      stringstream ss ;

      if ( _size > 0 )
      {
         INT32 num = 0 ;
         utilSplitIterator i( (CHAR*)_begin, '.', _size ) ;
         while ( i.more() )
         {
            const CHAR *left = i.next() ;
            if ( '$' == *left && '[' == *(left + 1) &&
                 SDB_OK == mthConvertSubElemToNumeric( left, num ) )
            {
               ss << num << '.' ;
            }
            else
            {
               ss << left << '.' ;
            }
         }
         ss.seekp( (INT32)ss.tellp() - 1 ) ;
         ss << '\0' ;
      }
      return ss.str() ;
   }

   // PD_TRACE_DECLARE_FUNCTION( SDB__QGMFETCHOUT_ELEMENT, "_qgmFetchOut::element" )
   INT32 _qgmFetchOut::element( const _qgmDbAttr &attr,
                                BSONElement &ele )const
   {
      PD_TRACE_ENTRY( SDB__QGMFETCHOUT_ELEMENT ) ;
      INT32 rc = SDB_OK ;
      BSONElement next, local ;

      SDB_ASSERT( !attr.empty(), "impossible" ) ;

      if ( NULL == this->next )
      {
         try
         {
            local = obj.getFieldDotted( attr.attr().toFieldName() );
         }
         catch ( std::exception &e )
         {
            PD_LOG( PDERROR, "unexpected err happend:%s", e.what() ) ;
            rc = SDB_SYS ;
            goto error ;
         }

         ele = local ;
      }
      else
      {
         if ( attr.relegation().empty() )
         {
            try
            {
               string fieldName = attr.attr().toFieldName() ;
               local = obj.getFieldDotted( fieldName ) ;
               next = this->next->obj.getFieldDotted( fieldName ) ;
            }
            catch ( std::exception &e )
            {
               PD_LOG( PDERROR, "unexpected err happend:%s", e.what() ) ;
               rc = SDB_SYS ;
               goto error ;
            }

            if ( local.eoo() && !next.eoo() )
            {
               ele = next ;
            }
            else if ( !local.eoo() && next.eoo() )
            {
               ele = local ;
            }
            else if ( !local.eoo() && !next.eoo() )
            {
               rc = SDB_QGM_AMBIGUOUS_FIELD ;
               PD_LOG( PDERROR, "ambiguous filed name:%s, obj: %s, "
                       "next obj: %s", attr.attr().toString().c_str(),
                       obj.toString().c_str(),
                       this->next->obj.toString().c_str() ) ;
               goto error ;
            }
            else
            {
               PD_LOG( PDERROR, "field [%s] not found from fetchout, obj: %s, "
                       "next obj: %s", attr.attr().toString().c_str(),
                       obj.toString().c_str(),
                       this->next->obj.toString().c_str() ) ;
               rc = SDB_INVALIDARG ;
               goto error ;
            }
         }
         else
         {
            BSONObj srcObj ;
            if ( attr.relegation() == this->next->alias )
            {
               srcObj = this->next->obj ;
            }
            else if ( attr.relegation() == this->alias )
            {
               srcObj = this->obj ;
            }
            else
            {
               rc = SDB_INVALIDARG ;
               PD_LOG( PDERROR, "relegaion [%s] not found, alias: %s, "
                       "next alias: %s", attr.relegation().toString().c_str(),
                       alias.toString().c_str(),
                       this->next->alias.toString().c_str() ) ;
               goto error ;
            }

            try
            {
               local = srcObj.getFieldDotted( attr.attr().toFieldName() ) ;
            }
            catch ( std::exception &e )
            {
               PD_LOG( PDERROR, "unexpected err happend:%s", e.what() ) ;
               rc = SDB_SYS ;
               goto error ;
            }

            ele = local ;
         }
      }

   done:
      PD_TRACE_EXITRC( SDB__QGMFETCHOUT_ELEMENT, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION( SDB__QGMFETCHOUT_ELEMENTS, "_qgmFetchOut::elements" )
   void _qgmFetchOut::elements( std::vector<BSONElement> &eles ) const
   {
      PD_TRACE_ENTRY( SDB__QGMFETCHOUT_ELEMENTS ) ;
      BSONObjIterator itr( obj ) ;
      while ( itr.more() )
      {
         eles.push_back( itr.next() ) ;
      }

      if ( NULL != next )
      {
         next->elements( eles ) ;
      }

      PD_TRACE_EXIT( SDB__QGMFETCHOUT_ELEMENTS ) ;
      return ;
   }

   BSONObj _qgmFetchOut::mergedObj()const
   {
      return NULL == next ?
             this->obj.getOwned() : qgmMerge( obj, next->mergedObj() ) ;
   }
}
