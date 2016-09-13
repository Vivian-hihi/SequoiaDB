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

   Source File Name = mthMatchOpNode.cpp

   Descriptive Name = Method Match Operation Node

   When/how to use: this program may be used on binary and text-formatted
   versions of Method component. This file contains functions for matcher, which
   indicates whether a record matches a given matching rule.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/15/2016  LinYouBin  Initial Draft

   Last Changed =

*******************************************************************************/
#include "mthMatchOpNode.hpp"
#include "mthMatchTree.hpp"
#include "pd.hpp"
#include "mthDef.hpp"
#include "mthCommon.hpp"

using namespace bson ;

namespace engine
{
   //************************_mthMatchFunc********************************
   _mthMatchFunc::_mthMatchFunc( _mthNodeAllocator *allocator )
   {
      _funcEle   = BSONObj().firstElement() ;
      _allocator = allocator ;
   }
   
   _mthMatchFunc::~_mthMatchFunc()
   {
      clear() ;
   }

   INT32 _mthMatchFunc::init( const CHAR *fieldName, const BSONElement &ele )
   {
      INT32 rc = SDB_OK ;
      rc = _fieldName.setFieldName( fieldName ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "set fieldName failed:fieldName=%s,rc=%d",
                 fieldName, rc ) ;
         goto error ;
      }

      _funcEle   = ele ;

      rc = _init( fieldName, ele ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "_init failed:rc=%d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   void _mthMatchFunc::clear()
   {
      _fieldName.clear() ;
      _funcEle = BSONObj().firstElement() ;

      _clear() ;
   }

   void* _mthMatchFunc::operator new ( size_t size, 
                                       _mthNodeAllocator *allocator ) 
                                       throw ( const char * )
   {
      void *p = NULL ;
      if ( size > 0 )
      {
         if ( NULL != allocator )
         {
            p = allocator->allocate( size ) ;
         }

         if ( NULL == p )
         {
            p = SDB_OSS_MALLOC( size ) ;
         }
      }

      if ( NULL == p )
      {
         throw "allocation failure" ;
      }

      return p ;
   }

   void _mthMatchFunc::operator delete( void *p )
   {
      SDB_OSS_FREE(p) ;
   }

   string _mthMatchFunc::toString()
   {
      BSONObj obj = toBson() ;
      return obj.toString() ;
   }

   BSONObj _mthMatchFunc::toBson()
   {
      BSONObjBuilder builder ;

      BSONObjBuilder b( builder.subobjStart( _fieldName.getFieldName() ) ) ;
      b.append( _funcEle ) ;
      b.doneFast() ;

      return builder.obj() ;
   }

   //************************_mthMatchFuncABS********************************
   _mthMatchFuncABS::_mthMatchFuncABS( _mthNodeAllocator *allocator )
                    :_mthMatchFunc( allocator )
   {
   }

   _mthMatchFuncABS::~_mthMatchFuncABS()
   {
      clear() ;
   }

   void _mthMatchFuncABS::release()
   {
      if ( NULL != _allocator && _allocator->isAllocatedByme( this ) )
      {
         this->~_mthMatchFuncABS() ;
      }
      else
      {
         delete this ;
      }
   }

   INT32 _mthMatchFuncABS::call( const BSONElement &in, BSONElement &out )
   {
      INT32 rc = SDB_OK ;
      BSONObjBuilder builder ;
      BSONObj obj ;

      rc = mthAbs( _fieldName.getFieldName(), in, builder ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "mthAbs failed:rc=%d", rc ) ;
      }

      obj = builder.obj() ;
      out = obj.firstElement() ;

      return rc ;
   }

   INT32 _mthMatchFuncABS::_init( const CHAR *fieldName, 
                                  const BSONElement &ele )
   {
      if ( ele.numberInt() != 1 )
      {
         return SDB_INVALIDARG ;
      }

      return SDB_OK ;
   }

   void _mthMatchFuncABS::_clear()
   {
      return ;
   }

   //************************_mthMatchOpNode********************************
   _mthMatchOpNode::_mthMatchOpNode(  _mthNodeAllocator *allocator )
                   :_mthMatchNode( allocator )
   {
      _isCompareField     = FALSE ;
      _hasDollarFieldName = FALSE ;
      _cmpFieldName       = NULL ;
   }

   _mthMatchOpNode::~_mthMatchOpNode()
   {
      clear() ;
   }

   INT32 _mthMatchOpNode::init( const CHAR *fieldName, 
                                const BSONElement &element )
   {
      INT32 rc = SDB_OK ;
      const CHAR *name = NULL ;
      rc = _mthMatchNode::init( fieldName, element ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "set fieldName failed:fieldName=%s,rc=%d", 
                 fieldName, rc ) ;
         goto error ;
      }

      if ( NULL != ossStrstr( fieldName, ".$" ) )
      {
         _hasDollarFieldName = TRUE ;
      }

      _toMatch = element ;
      name     = element.fieldName() ;
      if ( ossStrcmp( name, MTH_OPERATOR_STR_FIELD ) == 0 )
      {
         if ( element.type() != String )
         {
            rc = SDB_INVALIDARG ;
            PD_LOG( PDERROR, "$field must be String type:ele=%s,type=%d,rc=%d", 
                    element.toString().c_str(), element.type(), rc ) ;
            goto error ;
         }

         _isCompareField = TRUE ;
         _cmpFieldName   = element.valuestr() ;
      }

      rc = _init( fieldName, element ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "_init failed:rc=%d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      clear() ;
      goto done ;
   }

   INT32 _mthMatchOpNode::_init( const CHAR *fieldName, 
                                 const BSONElement &element )
   {
      return SDB_OK ;
   }

   void _mthMatchOpNode::_clear()
   {
      return ;
   }

   void _mthMatchOpNode::clear()
   {
      _clear() ;

      MTH_FUNC_LIST::iterator iter = _funcList.begin() ;
      while ( iter != _funcList.end() )
      {
         _mthMatchFunc *func = *iter ;
         mthGetMatchNodeFactory()->releaseFunc( func ) ;
         iter++ ;
      }
      _funcList.clear() ;
      _hasDollarFieldName = FALSE ;
      _isCompareField     = FALSE ;
      _cmpFieldName       = NULL ;  

      _mthMatchNode::clear() ;
   }

   void _mthMatchOpNode::setWeight( UINT32 weight )
   {
      SDB_ASSERT( FALSE, "no need to setWeight in _mthMatchOpNode" ) ;
   }

   INT32 _mthMatchOpNode::calcPredicate( _rtnPredicateSet &predicateSet )
   {
      INT32 rc = SDB_OK ;
      const UINT32 bufLen        = 31 ;
      CHAR staticBuf[ bufLen+1 ] = { 0 } ;
      CHAR *buf                  = staticBuf ;
      BOOLEAN rebuildName        = FALSE ;
      const CHAR *fieldName      = NULL ;

      if ( _isCompareField )
      {
         // $field do not have predicate
         goto done ;
      }

      fieldName = ( CHAR * ) _fieldName.getFieldName() ;

      if ( NULL != ossStrstr( fieldName, ".$" ) )
      {
         UINT32 pos      = 0 ;
         const CHAR *p   = fieldName ;
         BOOLEAN ignored = FALSE ;
         rebuildName     = TRUE ;

         while ( '\0' != *p )
         {
            if ( !ignored )
            {
               if ( '$' == *p &&
                    0 < ( p - fieldName ) &&
                    '.' == *( p - 1 ) )
               {
                  ignored = TRUE ;
                  --pos ;
               }
               else
               {
                  buf[pos++] = *p ;
               }
            }
            else if ( '.' == *p )
            {
               ignored = FALSE ;
               buf[pos++] = *p ;
            }
            else
            {
               /// do nothing
            }

            ++p ;
            if ( bufLen == pos && buf == staticBuf )
            {
               UINT32 allocLen = ossStrlen( fieldName ) + 1 ;
               buf = ( CHAR * )SDB_OSS_MALLOC( allocLen ) ;
               if ( NULL == buf )
               {
                  PD_LOG( PDERROR, "failed to allocate mem." ) ;
                  rc = SDB_OOM ;
                  goto error ;
               }
               ossMemcpy( buf, staticBuf, pos ) ;
            }
         }

         buf[pos] = '\0' ;
      }

      PD_LOG( PDDEBUG, "add preicate[%s] to predicates set",
              rebuildName ? buf : fieldName ) ;
      predicateSet.addPredicate ( rebuildName ? buf : fieldName,
                                  _toMatch, _isUnderLogicNot ) ;

   done:
      if ( buf != staticBuf && NULL != buf )
      {
         SDB_OSS_FREE( buf ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _mthMatchOpNode::extraEqualityMatches( BSONObjBuilder &builder )
   {
      //only $all and $et have EqualityMatches
      return SDB_OK ;
   }

   BOOLEAN _mthMatchOpNode::_isNot()
   {
      if ( _isUnderLogicNot )
      {
         if ( getType() == EN_MATCH_OPERATOR_NE || 
              getType() == EN_MATCH_OPERATOR_NIN )
         {
            return FALSE ;
         }
         else
         {
            return TRUE ;
         }
      }
      else
      {
         if ( getType() == EN_MATCH_OPERATOR_NE || 
              getType() == EN_MATCH_OPERATOR_NIN )
         {
            return TRUE ;
         }
         else
         {
            return FALSE ;
         }
      }
   }

   INT32 _mthMatchOpNode::_dollarMatches( const CHAR *pFieldName, 
                                          const BSONElement &element,
                                          _mthMatchTreeContext &context,
                                          BOOLEAN &result )
   {
      INT32 rc              = SDB_OK ;
      const CHAR *p         = pFieldName ;
      const CHAR *childName = NULL ;
      INT32 dollarValue     = 0 ;

      SDB_ASSERT( NULL != pFieldName &&
                  MTH_OPERATOR_EYECATCHER == *p, "impossible" ) ;

      rc = ossStrToInt ( p + 1, &dollarValue ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Failed to parse number:p=%s,rc=%d", p, rc ) ;
         goto error ;
      }

      if ( Array != element.type() || ossStrlen( pFieldName ) <= 1 )
      {
         result = FALSE ;
         goto done ;
      }

      childName = ossStrchr( p, MTH_FIELDNAME_SEP ) ;
      {
         BSONObjIterator i( element.embeddedObject() ) ;
         while ( i.more() )
         {
            BSONElement e = i.next() ;
            if ( NULL != childName )
            {
               //a.$0.xxx
               if ( MTH_OPERATOR_EYECATCHER == *( childName + 1 ) && 
                    Array == e.type() )
               {
                  // a.$0.$1, now childName is .$1
                  rc = _dollarMatches( childName + 1, e, context, result ) ;
                  if ( SDB_OK != rc )
                  {
                     PD_LOG( PDERROR, "failed to child field name:%s, rc:%d",
                             childName, rc ) ;
                     goto error ;
                  }
               }
               else if ( Object == e.type() )
               {
                  // a.$0.b, now childName is .b
                  rc = _execute( childName + 1, e.embeddedObject(), FALSE, 
                                 context, result ) ;
                  if ( SDB_OK != rc )
                  {
                     PD_LOG( PDERROR, "_execute failed:childName=%s,rc:%d",
                             childName, rc ) ;
                     goto error ;
                  }
               }
               else
               {
                  result = FALSE ;
               }
            }
            else
            {
               BSONElement right = _toMatch ;
               if ( _isCompareField )
               {
                  right = context._originalObj.getFieldDotted( _cmpFieldName ) ;
               }

               result = _valueMatch( e, right, context ) ;
            }

            if ( context.isDollarListEnabled() )
            {
               if ( ( !_isNot() && result ) || ( _isNot() && !result ) )
               {
                  INT64 temp       = 0 ;
                  INT32 dollarNum2 = ossAtoi( e.fieldName() ) ;
                  temp = ( ( (INT64) dollarValue ) << 32 ) |
                         ( ( (INT64) dollarNum2 ) & 0xFFFFFFFF ) ;
                  context._dollarList.push_back( temp ) ;
               }               
            }

            if ( result )
            {
               goto done ;
            }
         }
      }

      result = FALSE ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _mthMatchOpNode::_execute( const CHAR *pFieldName, 
                                    const BSONObj &obj, BOOLEAN isArrayObj,
                                    _mthMatchTreeContext &context,
                                    BOOLEAN &result )
   {
      INT32 rc = SDB_OK ;
      CHAR *pTmpFieldName = NULL ;
      _mthMatchFieldName<> mthFieldName ;
      BSONElement recordEle ;
      BSONElement toMatchEle ;
      CHAR *p  = NULL ;

      rc = mthFieldName.setFieldName( pFieldName ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "set fieldName failed:fieldName=%s,rc=%d",
                 pFieldName, rc ) ;
         goto error ;
      }

      pTmpFieldName = ( CHAR * ) mthFieldName.getFieldName() ;
      p = ossStrchr ( pTmpFieldName, MTH_FIELDNAME_SEP ) ;
      if ( p )
      {
         //xxx.xxx.xxx
         *p = '\0' ;
         BSONElement ele = obj.getField( pTmpFieldName ) ;
         if ( ele.type() == Object || ele.type() == Array )
         {
            //xxx.$1.xxx
            if ( MTH_OPERATOR_EYECATCHER == *(p + 1) )
            {
               rc = _dollarMatches( p + 1, ele, context, result ) ;
               if ( SDB_OK != rc )
               {
                  PD_LOG( PDERROR, "_dollarMatches failed:rc=%d", rc ) ;
                  goto error ;
               }
            }
            else
            {
               // xxx.xxx.xxx
               BSONObj subObj = ele.embeddedObject () ;
               // obj : { "a" : [ { "b" : 1 }, { "c" : 2 } ] }
               // ele.type() == Array:
               //   subObj: { 0 : { "b" : 1 }, 1 : { "c" : 2 } }
               rc = _execute( p + 1, subObj, ( ele.type() == Array ), context, 
                              result ) ;
               if ( SDB_OK != rc )
               {
                  PD_LOG( PDERROR, "failed to match child field:%d", rc ) ;
                  goto error ;
               }
            }

            goto done ;
         }
      }

      if ( isArrayObj )
      {
         // obj: { 0 : { "b" : 1 }, 1 : { "c" : 2 } }
         BSONObjIterator it ( obj ) ;
         result = FALSE ;
         while ( it.more() )
         {
            BSONElement z = it.next() ;
            if ( ossStrcmp( z.fieldName(), pTmpFieldName ) == 0 )
            {
               result = _valueMatch( z, _toMatch, context ) ;
               if ( result )
               {
                  goto done ;
               }
            }

            if ( z.type() == Object )
            {
               BSONObj subObj = z.embeddedObject() ;
               //pass the input pFieldName, not pTmpFieldName
               rc = _execute( pFieldName, subObj, FALSE, context, result ) ;
               if ( SDB_OK != rc )
               {
                  PD_LOG ( PDERROR, "_execute failed:rc=%d", rc ) ;
                  goto error ;
               }

               if ( result )
               {
                  goto done ;
               }
            }
         }

         goto done ;
      }

      if ( p )
      {
         if ( EN_MATCH_OPERATOR_EXISTS != getType() && 
              EN_MATCH_OPERATOR_ISNULL != getType() )
         {
            result = FALSE ;
            goto done ;
         }
      }

      if ( _isCompareField )
      {
         toMatchEle = context._originalObj.getFieldDotted( _cmpFieldName ) ;
         if ( toMatchEle.eoo() )
         {
            result = FALSE ;
            goto done ;
         }
      }
      else
      {
         toMatchEle = _toMatch ;
      }

      recordEle = obj.getField( pTmpFieldName ) ;
      result = _valueMatch( recordEle, toMatchEle, context ) ;
      if ( EN_MATCH_OPERATOR_EXISTS == getType() || 
           EN_MATCH_OPERATOR_ISNULL == getType() ||
           EN_MATCH_OPERATOR_SIZE == getType() ||
           EN_MATCH_OPERATOR_IN == getType() ||
           EN_MATCH_OPERATOR_ALL == getType())
      {
         // no need to check if left is array
         goto done ;
      }

      if ( !result )
      {
         //if no match. try iterator and check the array items
         if ( Array == recordEle.type() )
         {
            BSONObj eEmbObj = recordEle.embeddedObject() ;
            BSONObjIterator iter( eEmbObj ) ;
            while ( iter.more() )
            {
               BSONElement innerEle = iter.next() ;
               if ( _valueMatch( innerEle, toMatchEle, context ) )
               {
                  result = TRUE ;
                  goto done ;
               }
            }
         }
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _mthMatchOpNode::execute( const BSONObj &obj, 
                                   _mthMatchTreeContext &context,
                                   BOOLEAN &result )
   {
      INT32 rc = SDB_OK ;

      rc = _execute( _fieldName.getFieldName(), obj, FALSE, context, result ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "_execute failed:rc=%d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   BOOLEAN _mthMatchOpNode::hasDollarFieldName()
   {
      return _hasDollarFieldName ;
   }

   BOOLEAN _mthMatchOpNode::isTotalConverted()
   {
      if ( _isCompareField )
      {
         return FALSE ;
      }

      return TRUE ;
   }

   INT32 _mthMatchOpNode::addChild( _mthMatchNode *child )
   {
      SDB_ASSERT( FALSE, "_mthMatchOpNode can't have child" ) ;
      return SDB_INVALIDARG ;
   }

   void _mthMatchOpNode::delChild( _mthMatchNode *child )
   {
      SDB_ASSERT( FALSE, "_mthMatchOpNode can't have child" ) ;
   }

   INT32 _mthMatchOpNode::addFunc( _mthMatchFunc *func )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( NULL != func, "func can't be null!" ) ;
      rc = _funcList.push_back( func ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "add funciton failed:rc=%d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _mthMatchOpNode::addFuncList( MTH_FUNC_LIST &funcList )
   {
      INT32 rc = SDB_OK ;
      MTH_FUNC_LIST::iterator iter = funcList.begin() ;
      while ( iter != funcList.end() )
      {
         rc = addFunc( *iter ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "add function failed:rc=%d", rc ) ;
            goto error ;
         }

         funcList.erase( iter++ ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   BSONObj _mthMatchOpNode::toBson()
   {
      BSONObjBuilder builder ;

      BSONObjBuilder b( builder.subobjStart( _fieldName.getFieldName() ) ) ;
      if ( !_isCompareField )
      {
         b.appendAs( _toMatch, getOperatorStr() ) ;
      }
      else
      {
         BSONObj fieldObj = BSON( MTH_OPERATOR_STR_FIELD << 
                                  _toMatch.valuestrsafe() ) ;
         b.append( getOperatorStr(), fieldObj ) ;
      }

      b.doneFast() ;

      return builder.obj() ;
   }

   //*******************_mthMatchOpNodeET***********************
   _mthMatchOpNodeET::_mthMatchOpNodeET(  _mthNodeAllocator *allocator )
                     :_mthMatchOpNode( allocator )
   {
   }

   _mthMatchOpNodeET::~_mthMatchOpNodeET()
   {
      clear() ;
   }

   INT32 _mthMatchOpNodeET::getType()
   {
      return ( INT32 )EN_MATCH_OPERATOR_ET ;
   }

   const CHAR* _mthMatchOpNodeET::getOperatorStr()
   {
      return MTH_OPERATOR_STR_ET ;
   }

   UINT32 _mthMatchOpNodeET::getWeight()
   {
      return MTH_WEIGHT_EQUAL ;
   }

   BOOLEAN _mthMatchOpNodeET::isTotalConverted()
   {
      if ( !_mthMatchOpNode::isTotalConverted() )
      {
         return FALSE ;
      }

      if ( _toMatch.type() == Array )
      {
         return FALSE ;
      }

      return TRUE ;
   }

   INT32 _mthMatchOpNodeET::extraEqualityMatches( BSONObjBuilder &builder )
   {
      BSONElement ele   = _toMatch ;
      if ( !ele.eoo() )
      {
         string fieldName = _fieldName.getFieldName() ;
         if ( string::npos == fieldName.find( '$', 0 ) )
         {
            builder.appendAs( ele, fieldName ) ;
         }
      }

      return SDB_OK ;
   }

   BOOLEAN _mthMatchOpNodeET::_valueMatch( const BSONElement &left, 
                                           const BSONElement &right,
                                           _mthMatchTreeContext &context )
   {
      if ( left.canonicalType() == right.canonicalType() )
      {
         if ( 0 == compareElementValues ( left, right ) )
         {
            return TRUE ;
         }
      }

      return FALSE ;
   }

   void _mthMatchOpNodeET::release()
   {
      if ( NULL != _allocator && _allocator->isAllocatedByme( this ) )
      {
         this->~_mthMatchOpNodeET() ;
      }
      else
      {
         delete this ;
      }
   }

   //**************_mthMatchOpNodeNE********************************
   _mthMatchOpNodeNE::_mthMatchOpNodeNE( _mthNodeAllocator *allocator )
                     :_mthMatchOpNodeET( allocator )
   {
   }

   _mthMatchOpNodeNE::~_mthMatchOpNodeNE()
   {
      clear() ;
   }

   INT32 _mthMatchOpNodeNE::getType()
   {
      return EN_MATCH_OPERATOR_NE ;
   }

   const CHAR* _mthMatchOpNodeNE::getOperatorStr()
   {
      return MTH_OPERATOR_STR_NE ;
   }

   UINT32 _mthMatchOpNodeNE::getWeight()
   {
      return MTH_WEIGHT_NE ;
   }

   BOOLEAN _mthMatchOpNodeNE::isTotalConverted()
   {
      return FALSE ;
   }

   INT32 _mthMatchOpNodeNE::execute( const BSONObj &obj, 
                                     _mthMatchTreeContext &context,
                                     BOOLEAN &result )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN tmpResult = FALSE ;
      rc = _mthMatchOpNodeET::execute( obj, context, tmpResult ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to execute _mthMatchOpNodeNE:rc=%d", rc ) ;
         goto error ;
      }

      result = !tmpResult ;

   done:
      return rc ;
   error:
      goto done ;
   }

   void _mthMatchOpNodeNE::release()
   {
      if ( NULL != _allocator && _allocator->isAllocatedByme( this ) )
      {
         this->~_mthMatchOpNodeNE() ;
      }
      else
      {
         delete this ;
      }
   }

   //**************_mthMatchOpNodeLT********************************
   _mthMatchOpNodeLT::_mthMatchOpNodeLT( _mthNodeAllocator *allocator )
                     :_mthMatchOpNode( allocator )
   {
   }
   
   _mthMatchOpNodeLT::~_mthMatchOpNodeLT()
   {
      clear() ;
   }

   INT32 _mthMatchOpNodeLT::getType()
   {
      return ( INT32 ) EN_MATCH_OPERATOR_LT ;
   }

   const CHAR* _mthMatchOpNodeLT::getOperatorStr()
   {
      return MTH_OPERATOR_STR_LT ;
   }

   BOOLEAN _mthMatchOpNodeLT::isTotalConverted()
   {
      return _mthMatchOpNode::isTotalConverted() ;
   }

   UINT32 _mthMatchOpNodeLT::getWeight()
   {
      return MTH_WEIGHT_LT ;
   }

   BOOLEAN _mthMatchOpNodeLT::_valueMatch( const BSONElement &left, 
                                           const BSONElement &right,
                                           _mthMatchTreeContext &context )
   {
      if ( left.canonicalType() == right.canonicalType() )
      {
         if ( compareElementValues ( left, right ) < 0 )
         {
            return TRUE ;
         }
      }

      return FALSE ;
   }

   void _mthMatchOpNodeLT::release()
   {
      if ( NULL != _allocator && _allocator->isAllocatedByme( this ) )
      {
         this->~_mthMatchOpNodeLT() ;
      }
      else
      {
         delete this ;
      }
   }

   //******************************************************
   _mthMatchOpNodeLTE::_mthMatchOpNodeLTE( _mthNodeAllocator *allocator )
                      :_mthMatchOpNode( allocator )
   {
   }
   
   _mthMatchOpNodeLTE::~_mthMatchOpNodeLTE()
   {
      clear() ;
   }

   INT32 _mthMatchOpNodeLTE::getType()
   {
      return ( INT32 ) EN_MATCH_OPERATOR_LTE ;
   }

   const CHAR* _mthMatchOpNodeLTE::getOperatorStr()
   {
      return MTH_OPERATOR_STR_LTE ;
   }

   UINT32 _mthMatchOpNodeLTE::getWeight()
   {
      return MTH_WEIGHT_LTE ;
   }

   BOOLEAN _mthMatchOpNodeLTE::isTotalConverted()
   {
      return _mthMatchOpNode::isTotalConverted() ;
   }

   BOOLEAN _mthMatchOpNodeLTE::_valueMatch( const BSONElement &left, 
                                            const BSONElement &right,
                                            _mthMatchTreeContext &context )
   {
      if ( left.canonicalType() == right.canonicalType() )
      {
         if ( compareElementValues ( left, right ) <= 0 )
         {
            return TRUE ;
         }
      }

      return FALSE ;
   }

   void _mthMatchOpNodeLTE::release()
   {
      if ( NULL != _allocator && _allocator->isAllocatedByme( this ) )
      {
         this->~_mthMatchOpNodeLTE() ;
      }
      else
      {
         delete this ;
      }
   }

   //**************_mthMatchOpNodeGT*****************************
   _mthMatchOpNodeGT::_mthMatchOpNodeGT( _mthNodeAllocator *allocator )
                     :_mthMatchOpNode( allocator )
   {
   }

   _mthMatchOpNodeGT::~_mthMatchOpNodeGT()
   {
      clear() ;
   }

   INT32 _mthMatchOpNodeGT::getType()
   {
      return EN_MATCH_OPERATOR_GT ;
   }

   const CHAR* _mthMatchOpNodeGT::getOperatorStr()
   {
      return MTH_OPERATOR_STR_GT ;
   }

   UINT32 _mthMatchOpNodeGT::getWeight()
   {
      return MTH_WEIGHT_GT ;
   }

   BOOLEAN _mthMatchOpNodeGT::isTotalConverted()
   {
      return _mthMatchOpNode::isTotalConverted() ;
   }

   BOOLEAN _mthMatchOpNodeGT::_valueMatch( const BSONElement &left, 
                                           const BSONElement &right,
                                           _mthMatchTreeContext &context )
   {
      if ( left.canonicalType() == right.canonicalType() )
      {
         if ( compareElementValues ( left, right ) > 0 )
         {
            return TRUE ;
         }
      }

      return FALSE ;
   }

   void _mthMatchOpNodeGT::release()
   {
      if ( NULL != _allocator && _allocator->isAllocatedByme( this ) )
      {
         this->~_mthMatchOpNodeGT() ;
      }
      else
      {
         delete this ;
      }
   }

   //**************_mthMatchOpNodeGTE*****************************
   _mthMatchOpNodeGTE::_mthMatchOpNodeGTE( _mthNodeAllocator *allocator )
                      :_mthMatchOpNode( allocator )
   {
   }

   _mthMatchOpNodeGTE::~_mthMatchOpNodeGTE()
   {
      clear() ;
   }

   INT32 _mthMatchOpNodeGTE::getType()
   {
      return EN_MATCH_OPERATOR_GTE ;
   }

   const CHAR* _mthMatchOpNodeGTE::getOperatorStr()
   {
      return MTH_OPERATOR_STR_GTE ;
   }

   UINT32 _mthMatchOpNodeGTE::getWeight()
   {
      return MTH_WEIGHT_GTE ;
   }

   BOOLEAN _mthMatchOpNodeGTE::isTotalConverted()
   {
      return _mthMatchOpNode::isTotalConverted() ;
   }

   BOOLEAN _mthMatchOpNodeGTE::_valueMatch( const BSONElement &left, 
                                            const BSONElement &right,
                                            _mthMatchTreeContext &context )
   {
      if ( left.canonicalType() == right.canonicalType() )
      {
         if ( compareElementValues ( left, right ) >= 0 )
         {
            return TRUE ;
         }
      }

      return FALSE ;
   }

   void _mthMatchOpNodeGTE::release()
   {
      if ( NULL != _allocator && _allocator->isAllocatedByme( this ) )
      {
         this->~_mthMatchOpNodeGTE() ;
      }
      else
      {
         delete this ;
      }
   }

   //**************_mthMatchOpNodeIN*****************************
   _mthMatchOpNodeIN::_mthMatchOpNodeIN( _mthNodeAllocator *allocator )
                     :_mthMatchOpNode( allocator )
   {
   }
   
   _mthMatchOpNodeIN::~_mthMatchOpNodeIN()
   {
      clear() ;
   }

   INT32 _mthMatchOpNodeIN::_init( const CHAR *fieldName, 
                                   const BSONElement &element )
   {
      INT32 rc = SDB_OK ;
      if ( element.type() != Array )
      {
         //element's type must be array
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "element is not Array:element=%s,rc=%d", 
                 element.toString().c_str(), rc ) ;
         goto error ;
      }

      {
         BSONObjIterator iter ( element.embeddedObject() ) ;
         while ( iter.more() )
         {
            BSONElement subEle = iter.next() ;
            if ( subEle.type() == RegEx )
            {
               _mthMatchNode *node             = NULL ;
               _mthMatchOpNodeRegex *regexNode = NULL ;
               node = mthGetMatchNodeFactory()->createOpNode( _allocator,
                                                     EN_MATCH_OPERATOR_REGEX ) ;
               if ( NULL == node )
               {
                  rc = SDB_INVALIDARG ;
                  PD_LOG( PDERROR, "createOpNodeByOp failed:type=%d,rc=%d", 
                          EN_MATCH_OPERATOR_REGEX, rc ) ;
                  goto error ;
               }

               regexNode = dynamic_cast< _mthMatchOpNodeRegex * > ( node ) ;
               if ( NULL == regexNode )
               {
                  rc = SDB_INVALIDARG ;
                  PD_LOG( PDERROR, "dynamic_cast(OpNode->OpNodeRegex) failed:"
                          "node=%s,rc=%d", node->toString().c_str(), rc ) ;
                  mthGetMatchNodeFactory()->releaseNode( node ) ;
                  goto error ;
               }

               rc = regexNode->init( fieldName, subEle.regex(), 
                                     subEle.regexFlags() ) ;
               if ( SDB_OK != rc )
               {
                  PD_LOG( PDERROR, "init regexNode failed:regex=%s,rc=%d",
                          subEle.toString().c_str(), rc ) ;
                  mthGetMatchNodeFactory()->releaseNode( node ) ;
                  goto error ;
               }

               _regexVector.push_back( regexNode ) ;
            }
            else
            {
               _valueSet.insert( subEle ) ;
            }
         }
      }

      _isCompareField = FALSE ;
      _cmpFieldName   = NULL ;

   done:
      return rc ;
   error:
      goto done ;
   }

   void _mthMatchOpNodeIN::_clear()
   {
      UINT32 i = 0 ;
      for ( i = 0 ; i < _regexVector.size() ; i++ )
      {
         _regexVector[i]->clear() ;
         mthGetMatchNodeFactory()->releaseNode( _regexVector[i] ) ;
      }

      _regexVector.clear() ;
      _valueSet.clear() ;
   }

   INT32 _mthMatchOpNodeIN::getType()
   {
      return EN_MATCH_OPERATOR_IN ;
   }

   const CHAR* _mthMatchOpNodeIN::getOperatorStr()
   {
      return MTH_OPERATOR_STR_IN ;
   }

   UINT32 _mthMatchOpNodeIN::getWeight()
   {
      return MTH_WEIGHT_IN ;
   }

   BOOLEAN _mthMatchOpNodeIN::isTotalConverted()
   {
      if ( _mthMatchOpNode::isTotalConverted() )
      {
         if ( _regexVector.size() == 0 )
         {
            return TRUE ;
         }
      }

      return FALSE ;
   }

   BOOLEAN _mthMatchOpNodeIN::_valueMatch( const BSONElement &left, 
                                           const BSONElement &right,
                                           _mthMatchTreeContext &context )
   {
      UINT32 i    = 0 ;
      VALUE_SET::iterator iterSet ;
      VALUE_SET leftValueSet ;
      if ( _valueSet.size() == 0 && _regexVector.size() == 0 
           && left.type() == Array )
      {
         BSONObj obj = left.embeddedObject() ;
         if ( obj.nFields() == 0 )
         {
            return TRUE ;
         }
      }

      if ( Array != left.type() )
      {
         leftValueSet.insert( left ) ;
      }
      else
      {
         BSONObjIterator iter( left.embeddedObject() ) ;
         while ( iter.more() )
         {
            BSONElement ele = iter.next() ;
            leftValueSet.insert( ele ) ;
         }
      }

      iterSet = leftValueSet.begin() ;
      while ( iterSet != leftValueSet.end() )
      {
         // at least find one matched
         INT32 count = _valueSet.count( *iterSet ) ;
         if ( count > 0 )
         {
            return TRUE ;
         }

         iterSet++ ;
      }

      for ( i = 0 ; i < _regexVector.size() ; i++ )
      {
         iterSet = leftValueSet.begin() ;
         while ( iterSet != leftValueSet.end() )
         {
            if ( _regexVector[i]->matches( *iterSet ) )
            {
               return TRUE ;
            }

            iterSet++ ;
         }
      }

      return FALSE ;
   }

   void _mthMatchOpNodeIN::release()
   {
      if ( NULL != _allocator && _allocator->isAllocatedByme( this ) )
      {
         this->~_mthMatchOpNodeIN() ;
      }
      else
      {
         delete this ;
      }
   }

   //**************_mthMatchOpNodeNIN*****************************
   _mthMatchOpNodeNIN::_mthMatchOpNodeNIN( _mthNodeAllocator *allocator )
                      :_mthMatchOpNodeIN( allocator )
   {
   }

   _mthMatchOpNodeNIN::~_mthMatchOpNodeNIN()
   {
      clear() ;
   }

   INT32 _mthMatchOpNodeNIN::getType()
   {
      return EN_MATCH_OPERATOR_NIN ;
   }

   const CHAR* _mthMatchOpNodeNIN::getOperatorStr()
   {
      return MTH_OPERATOR_STR_NIN ;
   }

   UINT32 _mthMatchOpNodeNIN::getWeight()
   {
      return MTH_WEIGHT_NIN ;
   }

   BOOLEAN _mthMatchOpNodeNIN::isTotalConverted()
   {
      return FALSE ;
   }

   INT32 _mthMatchOpNodeNIN::execute( const BSONObj &obj, 
                                      _mthMatchTreeContext &context,
                                      BOOLEAN &result )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN tmpResult = FALSE ;
      rc = _mthMatchOpNodeIN::execute( obj, context, tmpResult ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to execute _mthMatchOpNodeNIN:rc=%d", rc ) ;
         goto error ;
      }

      result = !tmpResult ;

   done:
      return rc ;
   error:
      goto done ;
   }

   void _mthMatchOpNodeNIN::release()
   {
      if ( NULL != _allocator && _allocator->isAllocatedByme( this ) )
      {
         this->~_mthMatchOpNodeNIN() ;
      }
      else
      {
         delete this ;
      }
   }

   //**************_mthMatchOpNodeALL*****************************
   _mthMatchOpNodeALL::_mthMatchOpNodeALL( _mthNodeAllocator *allocator )
                      :_mthMatchOpNodeIN( allocator )
   {
   }
   
   _mthMatchOpNodeALL::~_mthMatchOpNodeALL()
   {
      clear() ;
   }
   
   INT32 _mthMatchOpNodeALL::getType()
   {
      return EN_MATCH_OPERATOR_ALL ;
   }

   const CHAR* _mthMatchOpNodeALL::getOperatorStr()
   {
      return MTH_OPERATOR_STR_ALL ;
   }

   UINT32 _mthMatchOpNodeALL::getWeight()
   {
      return MTH_WEIGHT_ALL ;
   }
   
   BOOLEAN _mthMatchOpNodeALL::isTotalConverted()
   {
      return FALSE ;
   }

   INT32 _mthMatchOpNodeALL::extraEqualityMatches( BSONObjBuilder &builder )
   {
      BSONElement ele   = _toMatch ;
      if ( !ele.eoo() )
      {
         string fieldName = _fieldName.getFieldName() ;
         if ( string::npos == fieldName.find( '$', 0 ) )
         {
            builder.appendAs( ele, fieldName ) ;
         }
      }

      return SDB_OK ;
   }
   
   BOOLEAN _mthMatchOpNodeALL::_valueMatch( const BSONElement &left, 
                                            const BSONElement &right,
                                            _mthMatchTreeContext &context )
   {
      // all _toMatch elements must exist in left ;
      UINT32 i = 0 ;
      VALUE_SET::iterator iterSet ;
      VALUE_SET leftValueSet ;
      //TODO: check Object
      if ( Array != left.type() && Object != left.type() )
      {
         if ( _valueSet.size() == 0 && _regexVector.size() == 0 )
         {
            // {a:1} do not match {a:{$all:[]}}, while {a:[1]} do
            return FALSE ;
         }
      }

      if ( Array != left.type() )
      {
         leftValueSet.insert( left ) ;
      }
      else
      {
         BSONObjIterator iter( left.embeddedObject() ) ;
         while ( iter.more() )
         {
            BSONElement ele = iter.next() ;
            leftValueSet.insert( ele ) ;
         }
      }

      iterSet = _valueSet.begin() ;
      while ( iterSet != _valueSet.end() )
      {
         // all values in _valueSet must exist in lefValueSet
         INT32 count = leftValueSet.count( *iterSet ) ;
         if ( count == 0 )
         {
            return FALSE ;
         }

         iterSet++ ;
      }

      for ( i = 0 ; i < _regexVector.size() ; i++ )
      {
         // all regexs in _regexVector must exist in leftValueSet
         BOOLEAN isMatch = FALSE ;
         iterSet = leftValueSet.begin() ;
         while ( iterSet != leftValueSet.end() )
         {
            if ( _regexVector[i]->matches( *iterSet ) )
            {
               isMatch = TRUE ;
               break ;
            }

            iterSet++ ;
         }

         if ( !isMatch )
         {
            return FALSE ;
         }
      }

      return TRUE ;
   }

   void _mthMatchOpNodeALL::release()
   {
      if ( NULL != _allocator && _allocator->isAllocatedByme( this ) )
      {
         this->~_mthMatchOpNodeALL() ;
      }
      else
      {
         delete this ;
      }
   }

   //**************_mthMatchOpNodeSIZE*****************************
   _mthMatchOpNodeSIZE::_mthMatchOpNodeSIZE( _mthNodeAllocator *allocator )
                       :_mthMatchOpNode( allocator )
   {
   }

   _mthMatchOpNodeSIZE::~_mthMatchOpNodeSIZE()
   {
      clear() ;
   }

   INT32 _mthMatchOpNodeSIZE::getType()
   {
      return EN_MATCH_OPERATOR_SIZE ;
   }

   const CHAR* _mthMatchOpNodeSIZE::getOperatorStr()
   {
      return MTH_OPERATOR_STR_SIZE ;
   }

   UINT32 _mthMatchOpNodeSIZE::getWeight()
   {
      return MTH_WEIGHT_SIZE ;
   }

   BOOLEAN _mthMatchOpNodeSIZE::isTotalConverted()
   {
      return FALSE ;
   }

   INT32 _mthMatchOpNodeSIZE::_init( const CHAR *fieldName, 
                                     const BSONElement &element )
   {
      if ( !element.isNumber() )
      {     
         PD_LOG( PDERROR, "element is not number:element=%s",
                 element.toString().c_str() ) ;
         return SDB_INVALIDARG ;
      }

      return SDB_OK ;
   }

   BOOLEAN _mthMatchOpNodeSIZE::_valueMatch( const BSONElement &left, 
                                             const BSONElement &right,
                                             _mthMatchTreeContext &context )
   {
      if ( left.type() != Array )
      {
         return FALSE ;
      }

      BSONObj obj = left.embeddedObject() ;

      return obj.nFields() == right.numberInt() ;
   }

   void _mthMatchOpNodeSIZE::release()
   {
      if ( NULL != _allocator && _allocator->isAllocatedByme( this ) )
      {
         this->~_mthMatchOpNodeSIZE() ;
      }
      else
      {
         delete this ;
      }
   }

   //**************_mthMatchOpNodeEXISTS*****************************
   _mthMatchOpNodeEXISTS::_mthMatchOpNodeEXISTS( _mthNodeAllocator *allocator )
                         :_mthMatchOpNode( allocator )
   {
   }

   _mthMatchOpNodeEXISTS::~_mthMatchOpNodeEXISTS()
   {
      clear() ;
   }

   INT32 _mthMatchOpNodeEXISTS::getType()
   {
      return EN_MATCH_OPERATOR_EXISTS ;
   }

   const CHAR* _mthMatchOpNodeEXISTS::getOperatorStr()
   {
      return MTH_OPERATOR_STR_EXISTS ;
   }

   UINT32 _mthMatchOpNodeEXISTS::getWeight()
   {
      return MTH_WEIGHT_EXISTS ;
   }

   BOOLEAN _mthMatchOpNodeEXISTS::isTotalConverted()
   {
      return FALSE ;
   }

   BOOLEAN _mthMatchOpNodeEXISTS::_valueMatch( const BSONElement &left, 
                                               const BSONElement &right,
                                               _mthMatchTreeContext &context )
   {
      if ( left.eoo() )
      {
         if ( _toMatch.trueValue() )
         {
            //expect exists
            return FALSE ;
         }

         return TRUE ;
      }
      else
      {
         if ( _toMatch.trueValue() )
         {
            return TRUE ;
         }

         return FALSE ;
      }
   }

   void _mthMatchOpNodeEXISTS::release()
   {
      if ( NULL != _allocator && _allocator->isAllocatedByme( this ) )
      {
         this->~_mthMatchOpNodeEXISTS() ;
      }
      else
      {
         delete this ;
      }
   }

   //**************_mthMatchOpNodeMOD*****************************
   _mthMatchOpNodeMOD::_mthMatchOpNodeMOD( _mthNodeAllocator *allocator )
                      :_mthMatchOpNode( allocator )
   {
   }

   _mthMatchOpNodeMOD::~_mthMatchOpNodeMOD()
   {
      clear() ;
   }

   BOOLEAN _mthMatchOpNodeMOD::_isModValid( const BSONElement &modmEle )
   {
      return mthIsModValid( modmEle ) ;
   }

   INT32 _mthMatchOpNodeMOD::_init( const CHAR *fieldName, 
                                    const BSONElement &element )
   {
      INT32 rc = SDB_OK ;
      BSONObj o ;

      if ( element.type() != Array )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "$mod's element must be Array:element=%s,rc=%d",
                  element.toString().c_str(), rc ) ;
         goto error ;
      }

      o = element.embeddedObject() ;
      if ( o.nFields() != 2 )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "$mod's element must have two fields:element=%s,"
                  "rc=%d", element.toString().c_str(), rc ) ;
         goto error ;
      }

      if ( !o["0"].isNumber() || !o["1"].isNumber() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "$mod's fields must be number type:element=%s,"
                  "rc=%d", element.toString().c_str(), rc ) ;
         goto error ;
      }

      if ( !_isModValid( o["0"] ) )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "Modulo is invalid:Modulo=%s,rc=%d", 
                  o["0"].toString().c_str(), rc ) ;
         goto error ;
      }

      _mod       = o["0"] ;
      _modResult = o["1"] ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _mthMatchOpNodeMOD::getType()
   {
      return EN_MATCH_OPERATOR_MOD ;
   }

   const CHAR* _mthMatchOpNodeMOD::getOperatorStr()
   {
      return MTH_OPERATOR_STR_MOD ;
   }

   UINT32 _mthMatchOpNodeMOD::getWeight()
   {
      return MTH_WEIGHT_MOD ;
   }

   BOOLEAN _mthMatchOpNodeMOD::isTotalConverted()
   {
      return FALSE ;
   }

   BOOLEAN _mthMatchOpNodeMOD::_valueMatch( const BSONElement &left, 
                                            const BSONElement &right,
                                            _mthMatchTreeContext &context )
   {
      if ( !left.isNumber() )
      {
         return FALSE ;
      }
      else
      {
         if ( NumberDecimal == left.type() || NumberDecimal == _mod.type() )
         {
            INT32 rcTmp = SDB_OK ;
            bsonDecimal decimal ;
            bsonDecimal decimalMod ;
            bsonDecimal decimalModm ;
            bsonDecimal result ;
            result.init() ;
            decimal    = left.numberDecimal() ;
            decimalMod = _mod.numberDecimal() ;
            rcTmp      = decimal.mod( decimalMod, result ) ;
            if ( SDB_OK != rcTmp )
            {
               PD_LOG( PDERROR, "failed to mod decimal:%s mod %s,rc=%d", 
                       decimal.toString().c_str(), 
                       decimalMod.toString().c_str(), rcTmp ) ;
               return FALSE ;
            }

            decimalModm = _modResult.numberDecimal() ;
            return ( 0 == result.compare( decimalModm ) ) ;
         }
         else if ( NumberDouble == left.type() 
                   && NumberDouble == _mod.type() )
         {
            FLOAT64 v = MTH_MOD( left.numberDouble(),
                                 _mod.numberDouble() ) ;
            return fabs( v - _modResult.numberDouble() ) <= OSS_EPSILON ;
         }
         else if ( NumberDouble != left.type() 
                   && NumberDouble == _mod.type() )
         {
            FLOAT64 v = MTH_MOD( left.numberLong(),
                                 _mod.numberDouble() ) ;
            return fabs( v - _modResult.numberDouble() ) <= OSS_EPSILON ;
         }
         else if ( NumberDouble == left.type() 
                   && NumberDouble != _mod.type())
         {
            FLOAT64 v = MTH_MOD( left.numberDouble(),
                                 _mod.numberLong() ) ;
            return fabs( v - _modResult.numberDouble() ) <= OSS_EPSILON ;
         }
         else
         {
            return ( left.numberLong() % _mod.numberLong() ) 
                                                 == _modResult.numberLong() ;
         }
      }
   }

   void _mthMatchOpNodeMOD::release()
   {
      if ( NULL != _allocator && _allocator->isAllocatedByme( this ) )
      {
         this->~_mthMatchOpNodeMOD() ;
      }
      else
      {
         delete this ;
      }
   }

   //**************_mthMatchOpNodeTYPE*****************************
   _mthMatchOpNodeTYPE::_mthMatchOpNodeTYPE( _mthNodeAllocator *allocator )
                       :_mthMatchOpNode( allocator )
   {
   }

   _mthMatchOpNodeTYPE::~_mthMatchOpNodeTYPE()
   {
      clear() ;
   }

   INT32 _mthMatchOpNodeTYPE::_init( const CHAR *fieldName, 
                                     const BSONElement &element )
   {
      _type = element.numberInt() ;
      return SDB_OK ;
   }

   INT32 _mthMatchOpNodeTYPE::getType()
   {
      return EN_MATCH_OPERATOR_TYPE ;
   }

   const CHAR* _mthMatchOpNodeTYPE::getOperatorStr()
   {
      return MTH_OPERATOR_STR_TYPE ;
   }

   UINT32 _mthMatchOpNodeTYPE::getWeight()
   {
      return MTH_WEIGHT_TYPE ;
   }

   BOOLEAN _mthMatchOpNodeTYPE::isTotalConverted()
   {
      return FALSE ;
   }

   BOOLEAN _mthMatchOpNodeTYPE::_valueMatch( const BSONElement &left, 
                                             const BSONElement &right,
                                             _mthMatchTreeContext &context )
   {
      return left.type() == _type ;
   }

   void _mthMatchOpNodeTYPE::release()
   {
      if ( NULL != _allocator && _allocator->isAllocatedByme( this ) )
      {
         this->~_mthMatchOpNodeTYPE() ;
      }
      else
      {
         delete this ;
      }
   }

   //**************_mthMatchOpNodeISNULL*****************************
   _mthMatchOpNodeISNULL::_mthMatchOpNodeISNULL( _mthNodeAllocator *allocator )
                         :_mthMatchOpNode( allocator )
   {
   }

   _mthMatchOpNodeISNULL::~_mthMatchOpNodeISNULL()
   {
      clear() ;
   }

   INT32 _mthMatchOpNodeISNULL::getType()
   {
      return EN_MATCH_OPERATOR_ISNULL ;
   }

   const CHAR* _mthMatchOpNodeISNULL::getOperatorStr()
   {
      return MTH_OPERATOR_STR_ISNULL ;
   }

   UINT32 _mthMatchOpNodeISNULL::getWeight()
   {
      return MTH_WEIGHT_ISNULL ;
   }

   BOOLEAN _mthMatchOpNodeISNULL::isTotalConverted()
   {
      return FALSE ;
   }

   BOOLEAN _mthMatchOpNodeISNULL::_valueMatch( const BSONElement &left, 
                                               const BSONElement &right,
                                               _mthMatchTreeContext &context )
   {
      if ( _toMatch.trueValue() )
      {
         if ( left.eoo() || left.isNull() )
         {
            return TRUE ;
         }
         else
         {
            return FALSE ;
         }
      }
      else
      {
         if ( left.eoo() || left.isNull() )
         {
            return FALSE ;
         }
         else
         {
            return TRUE ;
         }
      }
   }

   void _mthMatchOpNodeISNULL::release()
   {
      if ( NULL != _allocator && _allocator->isAllocatedByme( this ) )
      {
         this->~_mthMatchOpNodeISNULL() ;
      }
      else
      {
         delete this ;
      }
   }

   //**************_mthMatchOpNodeELEMMATCH*****************************
   _mthMatchOpNodeELEMMATCH::_mthMatchOpNodeELEMMATCH( 
                                              _mthNodeAllocator *allocator )
                            :_mthMatchOpNode( allocator )
   {
   }
   
   _mthMatchOpNodeELEMMATCH::~_mthMatchOpNodeELEMMATCH()
   {
      clear() ;
   }

   INT32 _mthMatchOpNodeELEMMATCH::_init( const CHAR *fieldName, 
                                          const BSONElement &element )
   {
      INT32 rc = SDB_OK ;

      //BSONElement m = e ;
      if ( element.type() != Object )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "$elemMatch's element must be Object:element=%s,"
                  "rc=%d", element.toString().c_str(), rc ) ;
         goto error ;
      }

      _subTree = mthGetMatchNodeFactory()->createTree() ;
      if ( NULL == _subTree )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "create subTree failed:rc=%d", rc) ;
         goto error ;
      }

      rc = _subTree->loadPattern( element.embeddedObject(), FALSE ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to loadPattern:obj=%s,rc=%d",
                 element.embeddedObject().toString().c_str(), rc ) ;
         goto error ;
      }

      if ( _subTree->hasDollarFieldName() )
      {
         _hasDollarFieldName = TRUE ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   void _mthMatchOpNodeELEMMATCH::_clear()
   {
      if ( NULL != _subTree )
      {
         _subTree->clear() ;
         mthGetMatchNodeFactory()->releaseTree( _subTree ) ;
      }

      _subTree = NULL ;
   }

   INT32 _mthMatchOpNodeELEMMATCH::getType()
   {
      return EN_MATCH_OPERATOR_ELEMMATCH ;
   }

   const CHAR* _mthMatchOpNodeELEMMATCH::getOperatorStr()
   {
      return MTH_OPERATOR_STR_ELEMMATCH ;
   }

   UINT32 _mthMatchOpNodeELEMMATCH::getWeight()
   {
      return MTH_WEIGHT_ELEMMATCH ;
   }

   BOOLEAN _mthMatchOpNodeELEMMATCH::isTotalConverted()
   {
      return FALSE ;
   }

   BOOLEAN _mthMatchOpNodeELEMMATCH::_valueMatch( const BSONElement &left, 
                                                  const BSONElement &right,
                                                  _mthMatchTreeContext &context )
   {
      // for eleMatch, such like {a:{$eleMatch:{b:1}}}, this will
      // match {a:{b:1}}
      // or {a:{$eleMatch:{$and:[{b:1},{c:1}]}}}
      // this will match {a:{b:1,c:1}}
      // we do not support {a:{$eleMatch:{$lt:1}}} at the moment. The
      // object in eleMatch must be a full matching condition
      INT32 rc = SDB_OK ;
      _mthMatchTreeContext subContext ;
      BOOLEAN result = FALSE ;
      if ( left.type() != Object && left.type() != Array )
      {
         return FALSE ;
      }

      if ( context.isDollarListEnabled() )
      {
         subContext.enableDollarList() ;
      }

      subContext.setIsSubTree( TRUE ) ;

      rc = _subTree->matches( left.embeddedObject(), result, subContext ) ;
      context.appendDollarList( subContext._dollarList ) ;
      if ( SDB_OK != rc )
      {
         return FALSE ;
      }

      return result ;
   }

   void _mthMatchOpNodeELEMMATCH::release()
   {
      if ( NULL != _allocator && _allocator->isAllocatedByme( this ) )
      {
         this->~_mthMatchOpNodeELEMMATCH() ;
      }
      else
      {
         delete this ;
      }
   }

   //**************_mthMatchOpNodeRegex*****************************
   _mthMatchOpNodeRegex::_mthMatchOpNodeRegex( _mthNodeAllocator *allocator )
                        :_mthMatchOpNode( allocator )
   {
      _regex   = NULL ;
      _options = NULL ;
   }
   
   _mthMatchOpNodeRegex::~_mthMatchOpNodeRegex()
   {
      clear() ;  
   }

   INT32 _mthMatchOpNodeRegex::init( const CHAR *fieldName, 
                                     const BSONElement &element )
   {
      SDB_ASSERT( FALSE, "do not called this init function" ) ;
      return SDB_INVALIDARG ;
   }

   BOOLEAN _mthMatchOpNodeRegex::_isPureWords( const char* regex, 
                                               const char* options )
   {
      BOOLEAN extended = FALSE ;
      if( options )
      {
         while ( *options )
         {
            switch ( *( options++ ) )
            {
            case 'm': // multiline
            case 's':
               continue ;
            case 'x': // extended
               extended = TRUE ;
               continue ;
            default:
               return FALSE ;
            }
         }
      }

      if( regex )
      {
         //check if the regex contains metacharacters
         while( *regex )
         {
            CHAR c = *( regex++ ) ;
            if( ossStrchr( "|?*\\^$.[()+{", c ) ||
                ( ossStrchr( "# ", c ) && extended ) )
            {
               return FALSE ;
            }
         } 
      }
      else
      {
         return FALSE ;
      }

      return TRUE ;
   }

   pcrecpp::RE_Options _mthMatchOpNodeRegex::_flags2options( 
                                                          const char* options )
   {
      pcrecpp::RE_Options reOptions ;
      reOptions.set_utf8( true ) ;
      while ( options && *options )
      {
         if ( *options == 'i' )
         {
            reOptions.set_caseless( true ) ;
         }
         else if ( *options == 'm' )
         {
            reOptions.set_multiline( true ) ;
         }
         else if ( *options == 'x' )
         {
            reOptions.set_extended( true ) ;
         }
         else if ( *options == 's' )
         {
            reOptions.set_dotall( true ) ;
         }

         options++ ;
      }
      return reOptions ;
   }

   //use this to init _mthMatchOpNodeRegex.(not graceful here)
   INT32 _mthMatchOpNodeRegex::init( const CHAR *fieldName, const CHAR *regex, 
                                     const CHAR *options )
   {
      INT32 rc = SDB_OK ;
      BSONObjBuilder builder ;
      rc = _fieldName.setFieldName( fieldName ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "setFieldName failed:fieldName=%s,rc=%d", 
                 fieldName, rc ) ;
         goto error ;
      }

      if ( NULL != ossStrstr( fieldName, ".$" ) )
      {
         _hasDollarFieldName = TRUE ;
      }

      _regex = regex ;
      if ( NULL == options )
      {
         _options = "" ;
      }
      else
      {
         _options = options ;
      }

      builder.appendRegex( fieldName, _regex, _options ) ;
      _matchObj = builder.obj() ;
      _toMatch  = _matchObj.firstElement() ;

      _isSimpleMatch = _isPureWords( _regex, _options ) ;
      _re.reset ( new RE(_regex, _flags2options( _options ) ) ) ;

   done:
      return rc ;
   error:
      clear() ;
      goto done ;
   }

   INT32 _mthMatchOpNodeRegex::getType()
   {
      return ( INT32 ) EN_MATCH_OPERATOR_REGEX ;
   }

   const CHAR* _mthMatchOpNodeRegex::getOperatorStr()
   {
      return MTH_OPERATOR_STR_REGEX ;
   }

   void _mthMatchOpNodeRegex::_clear()
   {
      _regex   = NULL ;
      _options = NULL ;
      _isSimpleMatch = FALSE ;
      _re.reset() ;
   }

   BSONObj _mthMatchOpNodeRegex::toBson()
   {
      BSONObjBuilder builder ;
      builder.appendRegex( _fieldName.getFieldName(), _regex, _options ) ;
      return builder.obj() ;
   }

   BOOLEAN _mthMatchOpNodeRegex::isTotalConverted()
   {
      return FALSE ;
   }

   UINT32 _mthMatchOpNodeRegex::getWeight()
   {
      return MTH_WEIGHT_REGEX ;
   }

   BOOLEAN _mthMatchOpNodeRegex::_valueMatch( const BSONElement &left, 
                                              const BSONElement &right,
                                              _mthMatchTreeContext &context )
   {
      return matches( left ) ;
   }

   BOOLEAN _mthMatchOpNodeRegex::matches( const BSONElement &ele )
   {
      switch ( ele.type() )
      {
      case String:
      case Symbol:
         if ( _isSimpleMatch )
         {
            return ossStrstr( ele.valuestr(), _regex ) != NULL ;
         }
         else
         {
            return _re->PartialMatch( ele.valuestr() ) ;
         }
      case RegEx:
         return ( 0 == ossStrcmp( _regex, ele.regex() ) &&
                  0 == ossStrcmp( _options, ele.regexFlags() ) ) ;
      default:
         return FALSE ;
      }
   }

   void _mthMatchOpNodeRegex::release()
   {
      if ( NULL != _allocator && _allocator->isAllocatedByme( this ) )
      {
         this->~_mthMatchOpNodeRegex() ;
      }
      else
      {
         delete this ;
      }
   }
}



