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

   Source File Name = mthMatchOpNode.hpp

   Descriptive Name = Method Match Operation Node Header

   When/how to use: this program may be used on binary and text-formatted
   versions of Method component. This file contains structure for matching
   operation, which is indicating whether a record matches a given matching
   rule.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/15/2016  LinYouBin    Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef MTH_MATCH_OPNODE_HPP_
#define MTH_MATCH_OPNODE_HPP_
#include "core.hpp"
#include "oss.hpp"
#include "ossUtil.hpp"
#include "utilList.hpp"
#include <set>
#include <boost/shared_ptr.hpp>
#include "../bson/bson.hpp"
#include "mthMatchNode.hpp"
#include "mthCommon.hpp"
#include <vector>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "pcrecpp.h"

using namespace bson ;
using namespace pcrecpp ;
using namespace std ;

namespace engine
{
   class _mthMatchFunc : public SDBObject
   {
      public:
         _mthMatchFunc( _mthNodeAllocator *allocator ) {} ;
         virtual ~_mthMatchFunc() {} ;

      public:
         virtual INT32 init( const CHAR *fieldName,
                             const BSONElement &ele ) = 0 ;
         virtual INT32 call( const BSONElement &in, BSONElement &out ) = 0 ;
         virtual void clear() = 0 ;
         virtual string toString() = 0 ;
   } ;

   typedef _utilList< _mthMatchFunc* > MTH_FUNC_LIST ;

   class _mthMatchOpNode : public _mthMatchNode
   {
      public:
         _mthMatchOpNode( _mthNodeAllocator *allocator ) ;
         virtual ~_mthMatchOpNode() ;

      public: /* from parent */
         virtual INT32 init( const CHAR *fieldName, 
                             const BSONElement &element ) ;

         virtual INT32 addChild( _mthMatchNode *child ) ;
         virtual void delChild( _mthMatchNode *child ) ;

         virtual void clear() ;

         virtual void setWeight( UINT32 weight ) ;

         virtual INT32 calcPredicate( _rtnPredicateSet &predicateSet ) ;

         virtual INT32 extraEqualityMatches( BSONObjBuilder &builder ) ;

         virtual BOOLEAN isTotalConverted() ;

         virtual BOOLEAN hasDollarFieldName() ;

         virtual INT32 execute( const BSONObj &obj, 
                                _mthMatchTreeContext &context,
                                BOOLEAN &result ) ;

         virtual BSONObj toBson() ;

      public:
         INT32 addFunc( _mthMatchFunc *func ) ;
         INT32 addFuncList( MTH_FUNC_LIST &funcList ) ;

      protected: /* from itself */
         virtual BOOLEAN _valueMatch( const BSONElement &left, 
                                      const BSONElement &right,
                                      _mthMatchTreeContext &context ) = 0 ;

         virtual INT32 _init( const CHAR *fieldName, 
                              const BSONElement &element) ;
         virtual void _clear() ;

      protected:
         INT32 _execute( const CHAR *pFieldName, const BSONObj &obj, 
                         BOOLEAN isArrayObj, 
                         _mthMatchTreeContext &context, 
                         BOOLEAN &result ) ;

         INT32 _dollarMatches( const CHAR *pFieldName, 
                               const BSONElement &element,
                               _mthMatchTreeContext &context,
                               BOOLEAN &result ) ;

         BOOLEAN _isNot() ;

      protected:
         MTH_FUNC_LIST _funcList ;
         BSONElement _toMatch ;
         BOOLEAN _isCompareField ;
         const CHAR *_cmpFieldName ;
         BOOLEAN _hasDollarFieldName ;
   } ;

   class _mthMatchOpNodeET : public _mthMatchOpNode
   {
      public:
         _mthMatchOpNodeET(  _mthNodeAllocator *allocator ) ;
         virtual ~_mthMatchOpNodeET() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         virtual UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;
         virtual INT32 extraEqualityMatches( BSONObjBuilder &builder ) ;
         virtual void release() ;

      protected:
         virtual BOOLEAN _valueMatch( const BSONElement &left, 
                                      const BSONElement &right,
                                      _mthMatchTreeContext &context ) ;
   } ;

   class _mthMatchOpNodeNE : public _mthMatchOpNodeET
   {
      public:
         _mthMatchOpNodeNE( _mthNodeAllocator *allocator ) ;
         virtual ~_mthMatchOpNodeNE() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         virtual UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;
         virtual INT32 execute( const BSONObj &obj, 
                                _mthMatchTreeContext &context,
                                BOOLEAN &result ) ;
         virtual void release() ;
   } ;

   class _mthMatchOpNodeLT : public _mthMatchOpNode
   {
      public:
         _mthMatchOpNodeLT( _mthNodeAllocator *allocator ) ;
         virtual ~_mthMatchOpNodeLT() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         virtual UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;
         virtual void release() ;

      protected:
         virtual BOOLEAN _valueMatch( const BSONElement &left, 
                                      const BSONElement &right,
                                      _mthMatchTreeContext &context ) ;
   } ;

   class _mthMatchOpNodeLTE : public _mthMatchOpNode
   {
      public:
         _mthMatchOpNodeLTE( _mthNodeAllocator *allocator ) ;
         virtual ~_mthMatchOpNodeLTE() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         virtual UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;
         virtual void release() ;

      protected:
         virtual BOOLEAN _valueMatch( const BSONElement &left, 
                                      const BSONElement &right,
                                      _mthMatchTreeContext &context ) ;
   } ;

   class _mthMatchOpNodeGT : public _mthMatchOpNode
   {
      public:
         _mthMatchOpNodeGT( _mthNodeAllocator *allocator ) ;
         virtual ~_mthMatchOpNodeGT() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         virtual UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;
         virtual void release() ;

      protected:
         virtual BOOLEAN _valueMatch( const BSONElement &left, 
                                      const BSONElement &right,
                                      _mthMatchTreeContext &context ) ;
   } ;

   class _mthMatchOpNodeGTE : public _mthMatchOpNode
   {
      public:
         _mthMatchOpNodeGTE( _mthNodeAllocator *allocator ) ;
         virtual ~_mthMatchOpNodeGTE() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         virtual UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;
         virtual void release() ;

      protected:
         virtual BOOLEAN _valueMatch( const BSONElement &left, 
                                      const BSONElement &right,
                                      _mthMatchTreeContext &context ) ;
   } ;

   class _mthMatchOpNodeRegex ;

   class _mthMatchOpNodeIN : public _mthMatchOpNode
   {
      public:
         _mthMatchOpNodeIN( _mthNodeAllocator *allocator ) ;
         virtual ~_mthMatchOpNodeIN() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         virtual UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;
         virtual void release() ;

      protected:
         virtual INT32 _init( const CHAR *fieldName, 
                              const BSONElement &element ) ;
         virtual void _clear() ;
         virtual BOOLEAN _valueMatch( const BSONElement &left, 
                                      const BSONElement &right,
                                      _mthMatchTreeContext &context ) ;

      protected:
         typedef set<BSONElement, element_cmp_lt> VALUE_SET ;
         set<BSONElement, element_cmp_lt> _valueSet ;

         typedef vector<_mthMatchOpNodeRegex *> REGEX_VECTOR ;
         REGEX_VECTOR _regexVector ;
   } ;

   class _mthMatchOpNodeNIN : public _mthMatchOpNodeIN
   {
      public:
         _mthMatchOpNodeNIN( _mthNodeAllocator *allocator ) ;
         virtual ~_mthMatchOpNodeNIN() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         virtual UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;
         virtual INT32 execute( const BSONObj &obj, 
                                _mthMatchTreeContext &context,
                                BOOLEAN &result ) ;
         virtual void release() ;
   } ;

   class _mthMatchOpNodeALL : public _mthMatchOpNodeIN
   {
      public:
         _mthMatchOpNodeALL( _mthNodeAllocator *allocator ) ;
         virtual ~_mthMatchOpNodeALL() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         virtual UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;
         virtual INT32 extraEqualityMatches( BSONObjBuilder &builder ) ;
         virtual void release() ;

      protected:
         virtual BOOLEAN _valueMatch( const BSONElement &left, 
                                      const BSONElement &right,
                                      _mthMatchTreeContext &context ) ;
   } ;

   class _mthMatchOpNodeSIZE : public _mthMatchOpNode
   {
      public:
         _mthMatchOpNodeSIZE( _mthNodeAllocator *allocator ) ;
         virtual ~_mthMatchOpNodeSIZE() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         virtual UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;
         virtual void release() ;

      protected:
         virtual INT32 _init( const CHAR *fieldName, 
                              const BSONElement &element ) ;
         virtual BOOLEAN _valueMatch( const BSONElement &left, 
                                      const BSONElement &right,
                                      _mthMatchTreeContext &context ) ;
   } ;

   class _mthMatchOpNodeEXISTS : public _mthMatchOpNode
   {
      public:
         _mthMatchOpNodeEXISTS( _mthNodeAllocator *allocator ) ;
         virtual ~_mthMatchOpNodeEXISTS() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;
         virtual void release() ;

      protected:
         virtual BOOLEAN _valueMatch( const BSONElement &left, 
                                      const BSONElement &right,
                                      _mthMatchTreeContext &context ) ;
   } ;

   class _mthMatchOpNodeMOD : public _mthMatchOpNode
   {
      public:
         _mthMatchOpNodeMOD( _mthNodeAllocator *allocator ) ;
         virtual ~_mthMatchOpNodeMOD() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;
         virtual void release() ;

      protected:
         virtual INT32 _init( const CHAR *fieldName, 
                              const BSONElement &element ) ;
         virtual BOOLEAN _valueMatch( const BSONElement &left, 
                                      const BSONElement &right,
                                      _mthMatchTreeContext &context ) ;

      protected:
         BOOLEAN _isModValid( const BSONElement &modmEle ) ;

      protected:
         BSONElement _mod ;
         BSONElement _modResult ;
   } ;

   class _mthMatchOpNodeTYPE : public _mthMatchOpNode
   {
      public:
         _mthMatchOpNodeTYPE( _mthNodeAllocator *allocator ) ;
         virtual ~_mthMatchOpNodeTYPE() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         virtual UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;
         virtual void release() ;

      protected:
         virtual INT32 _init( const CHAR *fieldName, 
                              const BSONElement &element ) ;
         virtual BOOLEAN _valueMatch( const BSONElement &left, 
                                      const BSONElement &right,
                                      _mthMatchTreeContext &context ) ;

      protected:
         INT32 _type ;
   } ;

   class _mthMatchOpNodeISNULL : public _mthMatchOpNode
   {
      public:
         _mthMatchOpNodeISNULL( _mthNodeAllocator *allocator ) ;
         virtual ~_mthMatchOpNodeISNULL() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         virtual UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;
         virtual void release() ;

      protected:
         virtual BOOLEAN _valueMatch( const BSONElement &left, 
                                      const BSONElement &right,
                                      _mthMatchTreeContext &context) ;
   } ;

   class _mthMatchTree ;
   class _mthMatchOpNodeELEMMATCH : public _mthMatchOpNode
   {
      public:
         _mthMatchOpNodeELEMMATCH( _mthNodeAllocator *allocator ) ;
         virtual ~_mthMatchOpNodeELEMMATCH() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         virtual UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;
         virtual void release() ;

      protected:
         virtual INT32 _init( const CHAR *fieldName, 
                              const BSONElement &element ) ;
         virtual void _clear() ;
         virtual BOOLEAN _valueMatch( const BSONElement &left, 
                                      const BSONElement &right,
                                      _mthMatchTreeContext &context ) ;

      protected:
         _mthMatchTree *_subTree ;
   } ;

   class _mthMatchOpNodeRegex : public _mthMatchOpNode
   {
      public:
         _mthMatchOpNodeRegex( _mthNodeAllocator *allocator ) ;
         virtual ~_mthMatchOpNodeRegex() ;

      public:
         //forbiddened
         virtual INT32 init( const CHAR *fieldName, 
                             const BSONElement &element ) ;

         //use this to init _mthMatchOpNodeRegex.(not graceful here)
         INT32 init( const CHAR *fieldName, const CHAR *regex, 
                     const CHAR *options ) ;

         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         virtual BSONObj toBson() ;
         virtual BOOLEAN isTotalConverted() ;
         virtual UINT32 getWeight() ;
         virtual void release() ;

      public:
         BOOLEAN matches( const BSONElement &ele ) ;

      protected:
         virtual void _clear() ;
         virtual BOOLEAN _valueMatch( const BSONElement &left, 
                                      const BSONElement &right,
                                      _mthMatchTreeContext &context ) ;

      private:
         pcrecpp::RE_Options _flags2options( const char* options ) ;
         BOOLEAN _isPureWords( const char* regex, const char* options ) ;

      private:
         const CHAR *_regex ;
         const CHAR *_options ;
         boost::shared_ptr<RE> _re ;
         BOOLEAN _isSimpleMatch ;
         BSONObj _matchObj ;
   } ;
}

#endif

