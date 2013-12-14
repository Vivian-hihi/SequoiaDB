/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgm

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains declare for QGM operators

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/09/2013  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef QGM_HPP__
#define QGM_HPP__
#include "core.hpp"
#include "oss.hpp"
#include "pmdEDU.hpp"
#include "pd.hpp"
#include "../bson/bson.h"
#include <vector>
#include "ossMem.hpp"
#include "dms.hpp"
#include "mthMergeSelector.hpp"
#include "rtnContext.hpp"
#include "monCB.hpp"
#include "dmsStorageUnit.hpp"
#include "rtn.hpp"
#include "dmsTempCB.hpp"
#include "rtnCoordQuery.hpp"
#include "msg.hpp"
#include "ixmKey.hpp"

using namespace bson ;
namespace engine
{
   INT32 qgmInit () ;
   enum _qgmJoinPredicateType
   {
      QGM_PDTYPE_EQ = 0,
      QGM_PDTYPE_NE,
      QGM_PDTYPE_GT,
      QGM_PDTYPE_GTE,
      QGM_PDTYPE_LT,
      QGM_PDTYPE_LTE
   } ;
   class _qgmJoinPredicate : public SDBObject
   {
   public :
      _qgmJoinPredicateType _type ;
      CHAR *_pFieldOuter ;
      CHAR *_pFieldInner ;
      void setType ( _qgmJoinPredicateType type )
      {
         _type = type ;
      }
      INT32 setOuter ( const CHAR *pOuterField )
      {
         _pFieldOuter = ossStrdup ( pOuterField ) ;
         if ( !_pFieldOuter )
            return SDB_OOM ;
         return SDB_OK ;
      }
      INT32 setInner ( const CHAR *pInnerField )
      {
         _pFieldInner = ossStrdup ( pInnerField ) ;
         if ( !_pFieldInner )
            return SDB_OOM ;
         return SDB_OK ;
      }
      _qgmJoinPredicate ()
      {
         _type = QGM_PDTYPE_EQ ;
         _pFieldOuter = NULL ;
         _pFieldInner = NULL ;
      }
      ~_qgmJoinPredicate ()
      {
         if ( _pFieldOuter )
            SDB_OSS_FREE ( _pFieldOuter ) ;
         if ( _pFieldInner )
            SDB_OSS_FREE ( _pFieldInner ) ;
      }
      INT32 fromBson ( const BSONObj &obj )
      {
         INT32 rc = SDB_OK ;
         BSONElement eleType  = obj.getField ( FIELD_NAME_TYPE ) ;
         BSONElement eleOuter = obj.getField ( FIELD_NAME_OUTER ) ;
         BSONElement eleInner = obj.getField ( FIELD_NAME_INNER ) ;
         PD_CHECK ( eleType.type() == NumberInt &&
                    eleOuter.type() == String &&
                    eleInner.type() == String,
                    SDB_INVALIDARG, error, PDERROR,
                    "Invalid type, outer or inner type: %s",
                    obj.toString().c_str() ) ;
         setType ( (_qgmJoinPredicateType)eleType.numberInt() ) ;
         rc = setOuter ( eleOuter.valuestr() ) ;
         PD_RC_CHECK ( rc, PDERROR, "Failed to set outer, rc = %d", rc ) ;
         rc = setInner ( eleInner.valuestr() ) ;
         PD_RC_CHECK ( rc, PDERROR, "Failed to set inner, rc = %d", rc ) ;
      done :
         return rc ;
      error :
         goto done ;
      }
      BSONObj toBson ()
      {
         return BSON ( FIELD_NAME_TYPE << (INT32)_type <<
                       FIELD_NAME_OUTER << _pFieldOuter <<
                       FIELD_NAME_INNER << _pFieldInner ) ;
      }
   } ;
   typedef class _qgmJoinPredicate qgmJoinPredicate ;
   typedef std::vector<qgmJoinPredicate*> qgmJoinPredicateList ;

   enum _qgmJoinType
   {
      QGM_JNTYPE_INNER = 0,
      QGM_JNTYPE_LEFT,
      QGM_JNTYPE_RIGHT,
      QGM_JNTYPE_FULL
   } ;

   enum _qgmOperatorType
   {
      // insert type, one subops for input
      // collection
      QGM_OPTYPE_INSERT = 0 ,
      // update type
      // predicate
      // rule
      // collection
      QGM_OPTYPE_UPDATE, // 1
      // delete type
      // predicate
      // collection
      QGM_OPTYPE_DELETE, // 2
      // return to user, one subops, top op for query
      QGM_OPTYPE_RETURN, // 3
      // nested loop join op, two subops,
      // predicate
      // selector
      // hint
      QGM_OPTYPE_NLJOIN, // 4
      // merge join op, two subops,
      // predicate
      // selector
      // hint
      QGM_OPTYPE_MSJOIN, // 5
      // hash join op, two subops,
      // predicate
      // selector
      // hint
      QGM_OPTYPE_HSJOIN, // 6
      // sort op, single subop, blocked op
      // sort field
      QGM_OPTYPE_SORT, // 7
      // aggr op, single subop, blocked op
      // aggr condition
      // selector
      QGM_OPTYPE_AGGR, // 8
      // tbscan or ixscan, depends on data node, single subop
      // predicate
      // selector
      // hint
      QGM_OPTYPE_SCAN, // 9
      // filter records from input, single subop
      // predicate
      // selector
      QGM_OPTYPE_FILTER, // 10
      // operator for user-provided records
      // records
      QGM_OPTYPE_RECORD // 11
   } ;

   // base operator
   // fetchNext
   class _qgmOperatorBase : public SDBObject
   {
   public :
      virtual INT32 fetchNext ( BSONObj &obj ) = 0 ;
      virtual INT32 execute(_pmdEDUCB *eduCB) = 0 ;
      virtual void close()
      {
         _hasExecuted = FALSE ;
         std::vector<_qgmOperatorBase*>::iterator it ;
         for ( it = _inputLegs.begin() ;
               it != _inputLegs.end() ;
               ++it )
         {
            (*it)->close() ;
         }
      }
      virtual INT32 pushDownPredicates ( const BSONObj &pred ) = 0 ;
      _qgmOperatorBase()
      {
         _eduCB = NULL ;
         _isConstructed = FALSE ;
         _hasExecuted = FALSE ;
         _numSkip = 0 ;
         _numReturn = -1 ;
         _alias = NULL ;
      }
      virtual ~_qgmOperatorBase()
      {
         std::vector<_qgmOperatorBase*>::iterator it ;
         for ( it = _inputLegs.begin() ;
               it != _inputLegs.end() ;
               ++it )
         {
            (*it)->close() ;
            SDB_OSS_DEL ( (*it) ) ;
         }
         _inputLegs.clear() ;
         if ( _alias )
         {
            SDB_OSS_FREE ( _alias ) ;
            _alias = NULL ;
         }
      }
      // generate from access plan
      static INT32 fromBson ( const BSONObj &plan,
                              _qgmOperatorBase **output ) ;
      // generate access plan for the current node and all child nodes
      virtual BSONObj toBson () = 0 ;
      // generate detailed info for the current node and all child nodes
      virtual std::string toString ( INT32 &id ) = 0 ;
      _qgmOperatorType getType()
      {
         return _opType ;
      }
      INT64 getNumSkip() { return _numSkip ; }
      INT64 getNumReturn() { return _numReturn ; }
      INT32 addInput ( _qgmOperatorBase *input )
      {
         INT32 rc = SDB_OK ;
         try
         {
            _inputLegs.push_back ( input ) ;
         }
         catch ( std::exception &e )
         {
            PD_LOG ( PDERROR, "Failed to add input: %s",
                     e.what() ) ;
            rc = SDB_SYS ;
         }
         return rc ;
      }
      INT32 getNumInput () { return _inputLegs.size() ; }
      _qgmOperatorBase *getInput ( INT32 id )
      {
         if ( (UINT32)id >= _inputLegs.size() )
            return NULL ;
         return _inputLegs[id] ;
      }
      CHAR *getAlias ()
      {
         return _alias ;
      }
   protected :
      _qgmOperatorType _opType ;
      _pmdEDUCB *_eduCB ;
      std::vector<_qgmOperatorBase*> _inputLegs ;
      BOOLEAN _isConstructed ;
      BOOLEAN _hasExecuted ;
      INT64   _numSkip ;    // <=0 for skip none
      INT64   _numReturn ;  // <0 for return all
      CHAR   *_alias ;
   } ;
   typedef class _qgmOperatorBase qgmOperatorBase ;

   // insert operator, 1 input leg
   class _qgmOperatorInsert : public qgmOperatorBase
   {
   public :
      _qgmOperatorInsert ( const CHAR *pCollectionName ) ;
      virtual ~_qgmOperatorInsert() {}
      virtual INT32 fetchNext ( BSONObj &obj ) { return SDB_SYS ; }
      virtual INT32 pushDownPredicates ( const BSONObj &pred )
      { return SDB_OK ; }
      virtual INT32 execute ( _pmdEDUCB *eduCB ) ;
      virtual BSONObj toBson () ;
      virtual std::string toString ( INT32 &id ) ;
      static INT32 fromBson ( const BSONObj &plan,
                              qgmOperatorBase **output ) ;
   private :
      CHAR _collectionName [ DMS_COLLECTION_SPACE_NAME_SZ +
                             DMS_COLLECTION_NAME_SZ + 2 ] ;
   } ;
   typedef class _qgmOperatorInsert qgmOperatorInsert ;

   // update operator, 0 input leg
   class _qgmOperatorUpdate : public qgmOperatorBase
   {
   public :
      _qgmOperatorUpdate ( const CHAR *pCollectionName,
                           const BSONObj &condition,
                           const BSONObj &updateRule,
                           const BSONObj &hint ) ;
      virtual ~_qgmOperatorUpdate() {}
      virtual INT32 fetchNext ( BSONObj &obj ) { return SDB_SYS ; }
      virtual INT32 pushDownPredicates ( const BSONObj &pred )
      { return SDB_OK ; }
      virtual INT32 execute ( _pmdEDUCB *eduCB ) ;
      virtual BSONObj toBson () ;
      virtual std::string toString ( INT32 &id ) ;
      static INT32 fromBson ( const BSONObj &plan,
                              qgmOperatorBase **output ) ;
   private :
      BSONObj _condition ;
      BSONObj _updateRule ;
      BSONObj _hint ;
      CHAR _collectionName [ DMS_COLLECTION_SPACE_NAME_SZ +
                             DMS_COLLECTION_NAME_SZ + 2 ] ;
   } ;
   typedef class _qgmOperatorUpdate qgmOperatorUpdate ;

   // delete operator, 0 input leg
   class _qgmOperatorDelete : public qgmOperatorBase
   {
   public :
      _qgmOperatorDelete ( const CHAR *pCollectionName,
                           const BSONObj &condition,
                           const BSONObj &hint ) ;
      virtual ~_qgmOperatorDelete() {}
      virtual INT32 fetchNext ( BSONObj &obj ) { return SDB_SYS ; }
      virtual INT32 pushDownPredicates ( const BSONObj &pred )
      { return SDB_OK ; }
      virtual INT32 execute ( _pmdEDUCB *eduCB ) ;
      virtual BSONObj toBson () ;
      virtual std::string toString ( INT32 &id ) ;
      static INT32 fromBson ( const BSONObj &plan,
                              qgmOperatorBase **output ) ;
   private :
      BSONObj _condition ;
      BSONObj _hint ;
      CHAR _collectionName [ DMS_COLLECTION_SPACE_NAME_SZ +
                             DMS_COLLECTION_NAME_SZ + 2 ] ;
   } ;
   typedef class _qgmOperatorDelete qgmOperatorDelete ;

   // return operator, 1 input leg
   class _qgmOperatorReturn : public qgmOperatorBase
   {
   public :
      _qgmOperatorReturn ( INT64 numSkip,
                           INT64 numReturn ) ;
      virtual ~_qgmOperatorReturn() {}
      virtual INT32 fetchNext ( BSONObj &obj ) ;
      virtual INT32 pushDownPredicates ( const BSONObj &pred )
      { return SDB_OK ; }
      virtual INT32 execute ( _pmdEDUCB *eduCB ) ;
      virtual BSONObj toBson () ;
      virtual std::string toString ( INT32 &id ) ;
      static INT32 fromBson ( const BSONObj &plan,
                              qgmOperatorBase **output ) ;
   private :
      qgmOperatorBase *_input ;
      INT64 _currentSkip ;
      INT64 _currentReturn ;
   } ;
   typedef class _qgmOperatorReturn qgmOperatorReturn ;

   class _qgmOperatorJoin : public qgmOperatorBase
   {
   public :
      virtual INT32 addPredicate ( qgmJoinPredicate *pred )
      {
         INT32 rc = SDB_OK ;
         try
         {
            _joinPredicates.push_back ( pred ) ;
         }
         catch ( std::exception &e )
         {
            PD_RC_CHECK ( SDB_SYS, PDERROR,
                          "Failed to push pred into list: %s",
                          e.what() ) ;
         }
      done :
         return rc ;
      error :
         goto done ;
      }

      virtual ~_qgmOperatorJoin () ;
   protected :
      struct cmp_str
      {
         bool operator() (const char *a, const char *b)
         {
            return std::strcmp(a,b)<0 ;
         }
      } ;
      INT32 _mergeResult ( const BSONObj &outer,
                           const BSONObj &inner,
                           BSONObj &output ) ;
      qgmJoinPredicateList _joinPredicates ;
      BSONObj _hint ;
      qgmOperatorBase *_outerSide ;
      qgmOperatorBase *_innerSide ;
      BOOLEAN _earlyOut ;
      _qgmJoinType _joinType ;
      BSONObj      _selectorObj ;
      mthMergeSelector _selector ;
      BOOLEAN _existSelector ;
      CHAR *_outerAlias ;
      CHAR *_innerAlias ;
      INT64 _currentSkip ;
      INT64 _currentReturn ;
   } ;
   typedef class _qgmOperatorJoin qgmOperatorJoin ;

   // nested loop join operator, 2 input legs
   class _qgmOperatorNLJoin : public qgmOperatorJoin
   {
   public :
      _qgmOperatorNLJoin ( const CHAR *pAlias,
                           // each selector must be in the form of
                           // <string1>:<string2>
                           // where string1 represents the field of the input,
                           // for example if the outerAlias is T1 and innerAlias
                           // is T2, each field in selector must be started with
                           // either T1 or T2, like "T1.field1":"", or
                           // "T2.field2":"newField2"
                           const BSONObj &selector,
                           const BSONObj &hint,
                           _qgmJoinType joinType,
                           BOOLEAN earlyOut,
                           INT64 numSkip,
                           INT64 numReturn ) ;
      virtual ~_qgmOperatorNLJoin () ;
      virtual INT32 fetchNext ( BSONObj &obj ) ;
      virtual INT32 execute ( _pmdEDUCB *eduCB ) ;
      virtual INT32 pushDownPredicates ( const BSONObj &pred ) ;
      virtual BSONObj toBson () ;
      virtual std::string toString ( INT32 &id ) ;
      static INT32 fromBson ( const BSONObj &plan,
                              qgmOperatorBase **output ) ;
   private :
      INT32 buildInnerSearchCondition ( BSONObj &outerRecord,
                                        BSONObjBuilder &searchCondition ) ;
      BOOLEAN _innerEnd ;
      INT32   _innerJoinNum ;
      BSONObj _outerObj ;
   } ;
   typedef class _qgmOperatorNLJoin qgmOperatorNLJoin ;

   enum _qgmOperatorCompareResult
   {
      QGM_OPERATOR_COMPARE_RESULT_BEGIN = 0,
      QGM_OPERATOR_COMPARE_RESULT_OUTERSMALL,
      QGM_OPERATOR_COMPARE_RESULT_INNERSMALL,
      QGM_OPERATOR_COMPARE_RESULT_EQUAL
   } ;
#define QGM_OPERATOR_MSJOIN_MAXTEMPSTORE 10
   // merge sort join operator, 2 input legs
   /*class _qgmOperatorMSJoin : public qgmOperatorJoin
   {
   public :
      _qgmOperatorMSJoin ( // each selector must be in the form of
                           // <string1>:<string2>
                           // where string1 represents the field of the input,
                           // for example if the outerAlias is T1 and innerAlias
                           // is T2, each field in selector must be started with
                           // either T1 or T2, like "T1.field1":"", or
                           // "T2.field2":"newField2"
                           const BSONObj &selector,
                           const BSONObj &hint,
                           const CHAR *pOuterAlias,
                           const CHAR *pInnerAlias,
                           _qgmJoinType joinType,
                           BOOLEAN earlyOut,
                           INT64 numSkip,
                           INT64 numReturn ) ;
      virtual ~_qgmOperatorMSJoin() {}
      virtual INT32 fetchNext ( BSONObj &obj ) ;
      virtual INT32 execute ( _pmdEDUCB *eduCB ) ;
      virtual INT32 pushDownPredicates ( const BSONObj &pred ) ;
      virtual BSONObj toBson () ;
      virtual void close () ;
      static INT32 fromBson ( const BSONObj &plan,
                              qgmOperatorBase **output ) ;
   private :
      BOOLEAN _innerEnd ;
      INT32   _innerJoinNum ;
      BSONObj _outerObj ;

      // overflow to temp
      INT32 _fillupTemp () ;
      INT32 _extractOuter ( BSONObj &outer ) ;
      INT32 _extractInner ( BSONObj &inner ) ;
      vector<BSONObj> _tempStore ;
      rtnContext *_pContext ;
      SINT64 _contextID ;
      monAppCB *_monAppCB ;
      UINT16 _tempID ;
      UINT32 _clLID ;
      BOOLEAN _tempCreated ;
      dmsStorageUnit *_tempSU ;
      SDB_RTNCB *_rtnCB ;
      dmsTempCB *_tempCB ;
      CHAR _tempTableName[12] ;

      // merge key
      ixmKeyOwned _innerKey ;
      ixmKeyOwned _outerKey ;
      BSONObj _innerRecord ;
      BSONObj _outerRecord ;
      BOOLEAN _innerUsed ;
      BOOLEAN _outerUsed ;
      BSONObj _outerOrderBy ;
      BSONObj _innerOrderBy ;
      _qgmOperatorCompareResult _compareResult ;
      BOOLEAN _isDoingCrossProduct ;
      ixmIndexKeyGen *_outerKeyGen ;
      ixmIndexKeyGen *_innerKeyGen ;
   } ;
   typedef class _qgmOperatorMSJoin qgmOperatorMSJoin ; */


   // sort operator, 1 input leg, block operation
   class _qgmOperatorSort : public qgmOperatorBase
   {
   public :
      _qgmOperatorSort ( const CHAR *pAlias,
                         const BSONObj &orderBy ) ;
      virtual ~_qgmOperatorSort() ;
      virtual INT32 fetchNext ( BSONObj &obj ) ;
      virtual INT32 execute ( _pmdEDUCB *eduCB ) ;
      virtual INT32 pushDownPredicates ( const BSONObj &pred ) ;
      virtual BSONObj toBson() ;
      virtual std::string toString ( INT32 &id ) ;
      virtual void close () ;
      static INT32 fromBson ( const BSONObj &plan,
                              qgmOperatorBase **output ) ;
   private :
      INT32 _fillupTemp () ;
      rtnContext *_pContext ;
      SINT64 _contextID ;
      BSONObj _orderBy ;
      monAppCB *_monAppCB ;
      UINT16 _tempID ;
      UINT32 _clLID ;
      BOOLEAN _tempCreated ;
      dmsStorageUnit *_tempSU ;
      SDB_RTNCB *_rtnCB ;
      dmsTempCB *_tempCB ;
      CHAR _tempTableName[12] ;
      BSONObj _sortHint ;
   } ;
   typedef class _qgmOperatorSort qgmOperatorSort ;

   // scan operator, 0 input leg, non-block operation
   class _qgmOperatorScan : public qgmOperatorBase
   {
   public :
      _qgmOperatorScan ( const CHAR *pAlias,
                         const CHAR *pCollectionFullName,
                         // each element in condition does NOT include
                         // collection name, only field name
                         const BSONObj &condition,
                         // each element in selector must be
                         // <string1>:<defaultvalue>
                         // where string1 represents the field name from source
                         // collection, and string 1 does NOT include the
                         // collection name
                         // default value represent the default value when the
                         // field does not exist
                         const BSONObj &selector,
                         const BSONObj &orderBy,
                         const BSONObj &hint,
                         INT64 numSkip,
                         INT64 numReturn ) ;
      virtual ~_qgmOperatorScan() ;
      virtual INT32 fetchNext ( BSONObj &obj ) ;
      virtual INT32 execute ( _pmdEDUCB *eduCB ) ;
      virtual void close () ;
      virtual INT32 pushDownPredicates ( const BSONObj &pred ) ;
      virtual BSONObj toBson () ;
      virtual std::string toString ( INT32 &id ) ;
      static INT32 fromBson ( const BSONObj &plan,
                              qgmOperatorBase **output ) ;
      const CHAR *getCollectionName ()
      {
         return _collectionName ;
      }
   private :
      void _killContext () ;
      BSONObj _finalCondition ;
      INT64 _currentSkip ;
      INT64 _currentReturn ;
      BSONObj _condition ;
      BSONObj _selector ;
      BSONObj _orderBy ;
      BSONObj _hint ;
      const CHAR *_collectionName ;
      // following parameters are for collection reading ( RTN )
      // for coord mode
      rtnCoordQuery _coordQuery ;
      MsgOpGetMore *_getMoreRequest ;
      rtnCoordGetmore _coordGetMore ;
      rtnCoordKillContext _coordKillContext ;
      // for data mode
      SINT64 _contextID ;
      SDB_DMSCB *_dmsCB ;
      SDB_RTNCB *_rtnCB ;
      netMultiRouteAgent *_pRouteAgent ;
      SDB_ROLE _dbRole ;
   } ;
   typedef class _qgmOperatorScan qgmOperatorScan ;

   // filter operator, 1 leg, non-block operator
   // similar behavior as qgmScan, but not directly reading from collection
   class _qgmOperatorFilter : public qgmOperatorBase
   {
   public :
      _qgmOperatorFilter ( const CHAR *pAlias,
                           // each element in condition does NOT include the
                           // alias
                           const BSONObj &condition,
                           // each element in selector must be in format
                           // <string1>:<string2>
                           // string1 represents the field name of source, and
                           // it's ALWAYS include the pInputAlias as leading
                           // field name. For example if the input alias is T,
                           // each element must be like T.ele1, T.field2, etc...
                           const BSONObj &selector,
                           INT64 numSkip,
                           INT64 numReturn ) ;
      virtual ~_qgmOperatorFilter() ;
      virtual INT32 fetchNext ( BSONObj &obj ) ;
      virtual INT32 execute ( _pmdEDUCB *eduCB ) ;
      virtual INT32 pushDownPredicates ( const BSONObj &pred ) ;
      virtual BSONObj toBson () ;
      virtual std::string toString ( INT32 &id ) ;
      static INT32 fromBson ( const BSONObj &plan,
                              qgmOperatorBase **output ) ;
   private :
      void _killContext () ;
      BSONObj _finalCondition ;
      INT64 _currentSkip ;
      INT64 _currentReturn ;
      BSONObj _condition ;
      BSONObj _sourceSelector ;
      mthMergeSelector _selector ;
      mthMatcher _matcher ;
      BOOLEAN _existSelector ;
   } ;
   typedef class _qgmOperatorFilter qgmOperatorFilter ;

#define QGM_OPERATOR_RECORD_MAX 2147483647
   // record operator, no leg, non-block operator
   class _qgmOperatorRecord : public qgmOperatorBase
   {
   public :
      _qgmOperatorRecord ( const CHAR *pAlias ) ;
      virtual ~_qgmOperatorRecord () ;
      virtual INT32 fetchNext ( BSONObj &obj ) ;
      virtual INT32 execute ( _pmdEDUCB *eduCB ) ;
      virtual INT32 pushDownPredicates ( const BSONObj &pred )
      { return SDB_OK ; }
      virtual BSONObj toBson () ;
      virtual std::string toString ( INT32 &id ) ;
      static INT32 fromBson ( const BSONObj &plan,
                              qgmOperatorBase **output ) ;
      INT32 addRecord ( const BSONObj &record ) ;
   private :
      vector<BSONObj> _recordList ;
      UINT32 _currentPos ;
   } ;
   typedef class _qgmOperatorRecord qgmOperatorRecord ;

   //INT32 qgmDump ( _qgmOperatorBase *op, CHAR *pBuffer, INT32 bufferSize ) ;
#if defined (_DEBUG)
   INT32 qgmDebugQuery ( BSONObj &queryPlan ) ;
#endif
}


#endif
