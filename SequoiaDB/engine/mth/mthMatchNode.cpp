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

   Source File Name = mthMatchNode.cpp

   Descriptive Name = Method MatchNode

   When/how to use: this program may be used on binary and text-formatted
   versions of Method component. This file contains functions for matcher, which
   indicates whether a record matches a given matching rule.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/14/2016  LinYouBin  Initial Draft

   Last Changed =

*******************************************************************************/
#include "mthMatchNode.hpp"
#include "pd.hpp"

using namespace bson ;

namespace engine
{
   BOOLEAN mthCompareNode( _mthMatchNode * left, _mthMatchNode * right )
   {
      return left->getWeight() < right->getWeight() ;
   }

   _mthMatchTreeContext::_mthMatchTreeContext()
   {
      _dollarList.clear() ;
      _elements.clear() ;
      _fieldName.clear() ;

      _needExpand          = FALSE ;
      _isDollarListEnabled = TRUE ;
      _isSubTree           = FALSE ;
   }

   _mthMatchTreeContext::~_mthMatchTreeContext()
   {
      clear() ;
   }

   void _mthMatchTreeContext::clear()
   {
      _dollarList.clear() ;
      _elements.clear() ;
      _fieldName.clear() ;

      _needExpand          = FALSE ;
      _isDollarListEnabled = TRUE ;
      _isSubTree           = FALSE ;
   }

   INT32 _mthMatchTreeContext::setName( const CHAR *name )
   {
      return _fieldName.setFieldName( name ) ;
   }

   INT32 _mthMatchTreeContext::addElement( const BSONElement &ele )
   {
      return _elements.append( ele ) ;
   }

   void _mthMatchTreeContext::setExpand( BOOLEAN needExpand )
   {
      _needExpand = needExpand ;
   }

   void _mthMatchTreeContext::setObj( const BSONObj &obj )
   {
      _originalObj = obj ;
   }

   void _mthMatchTreeContext::enableDollarList()
   {
      _isDollarListEnabled = TRUE ;
      _dollarList.clear() ;
   }

   void _mthMatchTreeContext::disableDollarList()
   {
      _isDollarListEnabled = FALSE ;
      _dollarList.clear() ;
   }

   BOOLEAN _mthMatchTreeContext::isDollarListEnabled()
   {
      return _isDollarListEnabled ;
   }

   void _mthMatchTreeContext::appendDollarList( vector<INT64> dollarList )
   {
      UINT32 i = 0 ;
      for ( i = 0 ; i < dollarList.size() ; i++ )
      {
         _dollarList.push_back( dollarList[i] ) ;
      }
   }

   void _mthMatchTreeContext::getDollarList( vector<INT64> *dollarList )
   {
      UINT32 i = 0 ;

      if ( NULL == dollarList )
      {
         return ;
      }

      for ( i = 0 ; i < _dollarList.size() ; i++ )
      {
         dollarList->push_back( _dollarList[i] ) ;
      }
   }

   void _mthMatchTreeContext::setIsSubTree( BOOLEAN isSubTree )
   {
      _isSubTree = isSubTree ;
   }
   
   BOOLEAN _mthMatchTreeContext::isSubTree()
   {
      return _isSubTree ;
   }

   string _mthMatchTreeContext::toString()
   {
      UINT32 i = 0 ;
      string temp = "dollarList:" ;
      for ( ; i < _dollarList.size() ; i++ )
      {
         CHAR tmpDollar[ 20 ] ;
         CHAR tmpIndex[ 20 ] ;
         //(temp>>32)&0xFFFFFFFF) // num = (temp&0xFFFFFFFF) ;
         INT32 dollarNum = ( _dollarList[i] >> 32 ) & 0xFFFFFFFF ;
         INT32 objIndex  = _dollarList[i] & 0xFFFFFFFF ;

         ossLltoa( dollarNum, tmpDollar, 20 ) ;
         ossLltoa( objIndex, tmpIndex, 20 ) ;
         temp += string("$") + tmpDollar + "=" + tmpIndex + "; ";
      }

      return temp ;
   }

   //********************** _mthNodeAllocator ***************************
   _mthNodeAllocator::_mthNodeAllocator()
   {
      _offset = 0 ;
   }
   
   _mthNodeAllocator::~_mthNodeAllocator()
   {
      _offset = 0 ;
   }

   void* _mthNodeAllocator::allocate( size_t size )
   {
      void *p = NULL ;
      if ( _offset + size <= MTH_ALLOCATOR_SIZE )
      {
         p = _mem + _offset ;
         _offset += size ;
      }

      return p ;
   }

   BOOLEAN _mthNodeAllocator::isAllocatedByme( void *p )
   {
      if ( p >= _mem && p < _mem + MTH_ALLOCATOR_SIZE )
      {
         return TRUE ;
      }

      return FALSE ;
   }

   //********************** _mthMatchNodeIterator ***************************
   _mthMatchNodeIterator::_mthMatchNodeIterator( _mthMatchNode *node )
   {
      _node  = node ;
      _index = 0 ;
   }

   _mthMatchNodeIterator::~_mthMatchNodeIterator()
   {
      _node  = NULL ;
      _index = 0 ;
   }

   _mthMatchNodeIterator::_mthMatchNodeIterator()
   {
   }
   
   _mthMatchNodeIterator::_mthMatchNodeIterator( 
                                          const _mthMatchNodeIterator &right )
   {
   }

   BOOLEAN _mthMatchNodeIterator::more()
   {
      return _index < _node->_children.size() ;
   }

   _mthMatchNode* _mthMatchNodeIterator::next()
   {
      UINT32 i = _index++ ;
      if ( i < _node->_children.size() )
      {
         return _node->_children[ i ] ;
      }

      return NULL ;
   }

   //********************** _mthMatchNode ***************************
   _mthMatchNode::_mthMatchNode( _mthNodeAllocator *allocator )
                 :_allocator( allocator ), _parent( NULL ), 
                 _idx_in_parent( -1 ), _isUnderLogicNot( FALSE )
   {
   }

   _mthMatchNode::~_mthMatchNode()
   {
      _allocator = NULL ;
      clear() ;
   }

   void* _mthMatchNode::operator new ( size_t size, 
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

   void _mthMatchNode::operator delete( void *p )
   {
      SDB_OSS_FREE(p) ;
   }

   string _mthMatchNode::toString()
   {
      BSONObj obj = toBson() ;
      return obj.toString() ;
   }

   const CHAR* _mthMatchNode::getFieldName()
   {
      return _fieldName.getFieldName() ;
   }

   INT32 _mthMatchNode::init( const CHAR *fieldName, 
                              const BSONElement &element )
   {
      INT32 rc = SDB_OK ;
      rc = _fieldName.setFieldName( fieldName ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "set fieldName failed:fieldName=%s,rc=%d",
                 fieldName, rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      clear() ;
      goto done ;
   }

   //just clear parameter itself
   void _mthMatchNode::clear()
   {
      _parent          = NULL ;
      _idx_in_parent   = -1 ;
      _isUnderLogicNot = FALSE ;
      _children.clear() ;
      _fieldName.clear() ;
   }

   INT32 _mthMatchNode::addChild( _mthMatchNode *child )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( NULL != child, "child must not be null" ) ;

      _children.push_back( child ) ;
      if ( EN_MATCH_OPERATOR_LOGIC_NOT == getType() || isUnderLogicNot() )
      {
         child->rollIsUnderLogicNot() ;
      }

      child->_idx_in_parent = _children.size() - 1 ;
      child->_parent        = this ;

      return rc ;
   }

   void _mthMatchNode::delChild( _mthMatchNode *child )
   {
      SDB_ASSERT( NULL != child, "child must not be null" ) ;
      SDB_ASSERT( child->_idx_in_parent < _children.size(), 
                  "index must smaller than size" ) ;
      SDB_ASSERT( _children[ child->_idx_in_parent ] == child, 
                  "child must be true" ) ;

      UINT32 index = child->_idx_in_parent ;
      _children.erase( _children.begin() + index ) ;

      for ( ; index < _children.size() ; index++ )
      {
         _children[ index ]->_idx_in_parent-- ;
      }

      if ( EN_MATCH_OPERATOR_LOGIC_NOT == getType() || isUnderLogicNot() )
      {
         child->rollIsUnderLogicNot() ;
      }
   }

   void _mthMatchNode::rollIsUnderLogicNot()
   {
      UINT32 i = 0 ;
      _isUnderLogicNot = !_isUnderLogicNot ;

      for ( ; i < _children.size() ; i++ )
      {
         _children[ i ]->rollIsUnderLogicNot() ;
      }
   }

   BOOLEAN _mthMatchNode::isUnderLogicNot()
   {
      return _isUnderLogicNot ;
   }

   UINT32 _mthMatchNode::getChildrenCount()
   {
      return _children.size() ;
   }

   _mthMatchNode* _mthMatchNode::getParent()
   {
      return _parent ;
   }

   void _mthMatchNode::sortByWeight()
   {
      UINT32 i = 0 ;
      for ( ; i < _children.size() ; i++ )
      {
         _mthMatchNode *child = _children[ i ] ;
         child->sortByWeight() ;
      }

      if ( _children.size() > 1 )
      {
         std::sort( _children.begin(), _children.end(), mthCompareNode ) ;

         // rewrite _idx_in_parent
         for ( ; i < _children.size() ; i++ )
         {
            _mthMatchNode *child = _children[ i ] ;
            child->_idx_in_parent = i ;
         }
      }
   }

   BOOLEAN _mthMatchNode::hasDollarFieldName()
   {
      //default is false
      return FALSE ;
   }

   INT32 _mthMatchNode::calcPredicate( _rtnPredicateSet &predicateSet )
   {
      UINT32 i = 0 ;
      for ( ; i < _children.size() ; i++ )
      {
         _mthMatchNode *child = _children[ i ] ;
         child->calcPredicate( predicateSet ) ;
      }

      return SDB_OK ;
   }

   INT32 _mthMatchNode::extraEqualityMatches( BSONObjBuilder &builder )
   {
      UINT32 i = 0 ;
      for ( ; i < _children.size() ; i++ )
      {
         _mthMatchNode *child = _children[ i ] ;
         child->extraEqualityMatches( builder ) ;
      }

      return SDB_OK ;
   }

}


