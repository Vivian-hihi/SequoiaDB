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
         _mthMatchFunc() {};
         virtual ~_mthMatchFunc() {} ;

      public:
         virtual INT32 init( const BSONElement &ele ) = 0 ;
         virtual INT32 call( const BSONElement &in, BSONElement &out ) = 0 ;
         virtual void clear() = 0 ;
   } ;


   class _mthMatchOpNode : public _mthMatchNode
   {
      typedef _utilList< _mthMatchFunc* > MTH_FUNC_LIST ;
      public:
         _mthMatchOpNode() ;
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
         _mthMatchOpNodeET() ;
         virtual ~_mthMatchOpNodeET() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         virtual UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;
         virtual INT32 extraEqualityMatches( BSONObjBuilder &builder ) ;

      protected:
         virtual BOOLEAN _valueMatch( const BSONElement &left, 
                                      const BSONElement &right,
                                      _mthMatchTreeContext &context ) ;
   } ;

   class _mthMatchOpNodeNE : public _mthMatchOpNodeET
   {
      public:
         _mthMatchOpNodeNE() ;
         virtual ~_mthMatchOpNodeNE() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         virtual UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;
         virtual INT32 execute( const BSONObj &obj, 
                                _mthMatchTreeContext &context,
                                BOOLEAN &result ) ;
   } ;

   class _mthMatchOpNodeLT : public _mthMatchOpNode
   {
      public:
         _mthMatchOpNodeLT() ;
         virtual ~_mthMatchOpNodeLT() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         virtual UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;

      protected:
         virtual BOOLEAN _valueMatch( const BSONElement &left, 
                                      const BSONElement &right,
                                      _mthMatchTreeContext &context ) ;
   } ;

   class _mthMatchOpNodeLTE : public _mthMatchOpNode
   {
      public:
         _mthMatchOpNodeLTE() ;
         virtual ~_mthMatchOpNodeLTE() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         virtual UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;

      protected:
         virtual BOOLEAN _valueMatch( const BSONElement &left, 
                                      const BSONElement &right,
                                      _mthMatchTreeContext &context ) ;
   } ;

   class _mthMatchOpNodeGT : public _mthMatchOpNode
   {
      public:
         _mthMatchOpNodeGT() ;
         virtual ~_mthMatchOpNodeGT() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         virtual UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;

      protected:
         virtual BOOLEAN _valueMatch( const BSONElement &left, 
                                      const BSONElement &right,
                                      _mthMatchTreeContext &context ) ;
   } ;

   class _mthMatchOpNodeGTE : public _mthMatchOpNode
   {
      public:
         _mthMatchOpNodeGTE() ;
         virtual ~_mthMatchOpNodeGTE() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         virtual UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;

      protected:
         virtual BOOLEAN _valueMatch( const BSONElement &left, 
                                      const BSONElement &right,
                                      _mthMatchTreeContext &context ) ;
   } ;

   class _mthMatchOpNodeRegex ;

   class _mthMatchOpNodeIN : public _mthMatchOpNode
   {
      public:
         _mthMatchOpNodeIN() ;
         virtual ~_mthMatchOpNodeIN() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         virtual UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;
         

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
         _mthMatchOpNodeNIN() ;
         virtual ~_mthMatchOpNodeNIN() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         virtual UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;
         virtual INT32 execute( const BSONObj &obj, 
                                _mthMatchTreeContext &context,
                                BOOLEAN &result ) ;
   } ;

   class _mthMatchOpNodeALL : public _mthMatchOpNodeIN
   {
      public:
         _mthMatchOpNodeALL() ;
         virtual ~_mthMatchOpNodeALL() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         virtual UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;
         virtual INT32 extraEqualityMatches( BSONObjBuilder &builder ) ;

      protected:
         virtual BOOLEAN _valueMatch( const BSONElement &left, 
                                      const BSONElement &right,
                                      _mthMatchTreeContext &context ) ;
   } ;

   class _mthMatchOpNodeSIZE : public _mthMatchOpNode
   {
      public:
         _mthMatchOpNodeSIZE() ;
         virtual ~_mthMatchOpNodeSIZE() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         virtual UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;

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
         _mthMatchOpNodeEXISTS() ;
         virtual ~_mthMatchOpNodeEXISTS() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;

      protected:
         virtual BOOLEAN _valueMatch( const BSONElement &left, 
                                      const BSONElement &right,
                                      _mthMatchTreeContext &context ) ;
   } ;

   class _mthMatchOpNodeMOD : public _mthMatchOpNode
   {
      public:
         _mthMatchOpNodeMOD() ;
         virtual ~_mthMatchOpNodeMOD() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;

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
         _mthMatchOpNodeTYPE() ;
         virtual ~_mthMatchOpNodeTYPE() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         virtual UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;

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
         _mthMatchOpNodeISNULL() ;
         virtual ~_mthMatchOpNodeISNULL() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         virtual UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;

      protected:
         virtual BOOLEAN _valueMatch( const BSONElement &left, 
                                      const BSONElement &right,
                                      _mthMatchTreeContext &context) ;
   } ;

   class _mthMatchTree ;
   class _mthMatchOpNodeELEMMATCH : public _mthMatchOpNode
   {
      public:
         _mthMatchOpNodeELEMMATCH() ;
         virtual ~_mthMatchOpNodeELEMMATCH() ;

      public:
         virtual INT32 getType() ;
         virtual const CHAR *getOperatorStr() ;
         virtual UINT32 getWeight() ;
         virtual BOOLEAN isTotalConverted() ;

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
         _mthMatchOpNodeRegex() ;
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

