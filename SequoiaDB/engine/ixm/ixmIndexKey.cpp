/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = ixmIndexKey.cpp

   Descriptive Name = Index Manager Index Key Generator

   When/how to use: this program may be used on binary and text-formatted
   versions of Index Manager component. This file contains functions for index
   key generator, which is used to create key pairs from data record and index
   definition.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#include "ixm.hpp"
#include "ixmIndexKey.hpp"
#include "ixmGeo.hpp"
#include "ossMem.hpp"
#include "pdTrace.hpp"
#include "ixmTrace.hpp"

using namespace bson ;

namespace engine
{

   #define IXM_MAX_PREALLOCATED_UNDEFKEY        ( 10 )
   /*
      IXM Tool functions
   */
   BSONObj ixmGetUndefineKeyObj( INT32 fieldNum )
   {
      static BSONObj s_undefineKeys[ IXM_MAX_PREALLOCATED_UNDEFKEY ] ;
      static BOOLEAN s_init = FALSE ;
      static ossSpinXLatch s_latch ;

      // init undefine keys
      if ( FALSE == s_init )
      {
         s_latch.get() ;
         if ( FALSE == s_init )
         {
            for ( SINT32 i = 0; i < IXM_MAX_PREALLOCATED_UNDEFKEY ; ++i )
            {
               BSONObjBuilder b ;
               for ( SINT32 j = 0; j <= i; ++j )
               {
                  b.appendUndefined("") ;
               }
               s_undefineKeys[i] = b.obj() ;
            }
            s_init = TRUE ;
         }
         s_latch.release() ;
      }

      if ( fieldNum > 0 && fieldNum <= IXM_MAX_PREALLOCATED_UNDEFKEY )
      {
         return s_undefineKeys[ fieldNum - 1 ] ;
      }
      else
      {
         BSONObjBuilder b ;
         for ( INT32 i = 0; i < fieldNum; ++i )
         {
            b.appendUndefined("") ;
         }
         return b.obj() ;
      }
   }

   /*
      IXM Global opt var
   */
   const static BSONObj gUndefinedObj =
          BSONObjBuilder().appendUndefined("").obj() ;
   const static BSONElement gUndefinedElt = gUndefinedObj.firstElement() ;

   // provide a BSON object, generate keys based on index keygen
   // this object is only used by ixmIndexKeyGen class
   // this class only have 1 external function "getKeys" to extract a given
   // object to BSONObjSet. Note keys may contain one or more key, when there is
   // array included in the object
   class _ixmKeyGenerator
   {
   protected:
      const _ixmIndexKeyGen *_keygen ;
      mutable vector<BSONObj *> _objs ;
   public:
      _ixmKeyGenerator ( const _ixmIndexKeyGen *keygen )
      {
         _keygen = keygen ;
      }
      ~_ixmKeyGenerator()
      {
         vector<BSONObj *>::iterator itr = _objs.begin() ;
         for ( ; itr != _objs.end(); itr++ )
         {
            SDB_OSS_DEL *itr ;
         }
      }
      // input: BSONObj obj
      // output: BSONObjSet &keys
      // PD_TRACE_DECLARE_FUNCTION ( SDB__IXMKEYGEN_GETKEYS, "_ixmKeyGenerator::getKeys" )
      INT32 getKeys ( const BSONObj &obj, BSONObjSet &keys,
                      BSONElement *pArrEle ) const
      {
         INT32 rc = SDB_OK ;
         PD_TRACE_ENTRY ( SDB__IXMKEYGEN_GETKEYS );
         SDB_ASSERT ( _keygen, "spec can't be NULL" )
         // create vector for all fields we interest
         vector<const CHAR*> fieldNames ( _keygen->_fieldNames ) ;
         // create BSONelement vector for all elements
         // create from existing _fixedElements, faster
         vector<BSONElement> fixed ( _keygen->_fixedElements ) ;
         // note don't pass reference of fieldNames and fixed, because we might
         // need to modify the vectors, so we need to duplicate the object
         try
         {
            rc =_getKeys ( fieldNames, fixed, obj, keys, pArrEle ) ;
         }
         catch ( std::exception &e )
         {
            PD_LOG( PDERROR, "unexpected err:%s", e.what() ) ;
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to generate key from object: %s",
                     obj.toString().c_str() ) ;
            goto error ;
         }

         if ( keys.empty() )
         {
            keys.insert ( _keygen->_undefinedKey ) ;
         }

      done :
         PD_TRACE_EXITRC ( SDB__IXMKEYGEN_GETKEYS, rc );
         return rc ;
      error :
         goto done ;
      }
   protected:
      // PD_TRACE_DECLARE_FUNCTION ( SDB__IXMKEYGEN__EXTNXT2DELE, "_ixmKeyGenerator::_extractNext2dElement" )
      INT32 _extractNext2dElement( const BSONObj &obj, const BSONObj &arr,
                                   const CHAR *&field,
                                   BOOLEAN &arrayNestedArray,
                                   BSONElement &nextEle ) const
      {
         INT32 rc = SDB_OK ;
         PD_TRACE_ENTRY ( SDB__IXMKEYGEN__EXTNXT2DELE );
         CHAR *firstField = ossStrdup ( field ) ;
         CHAR *pCur = firstField ;
         PD_CHECK ( firstField, SDB_OOM, error, PDERROR,
                    "Failed to duplicate field name: %s",
                    field ) ;
         while ( *pCur )
         {
            if ( '.' == *pCur )
            {
               *pCur = '\0' ;
               break ;
            }
            pCur ++ ;
         }
         {
            BOOLEAN haveObjField = !obj.getField ( firstField ).eoo() ;
            BSONElement objField ;
            BSONElement arrField = arr.getField ( firstField ) ;
            BOOLEAN haveArrField = !arrField.eoo() ;
            PD_CHECK ( !haveObjField || !haveArrField, SDB_INVALIDARG, error,
                       PDERROR, "can't have both obj and arr field" ) ;
            arrayNestedArray = FALSE ;
            if ( haveObjField )
            {
               objField = obj.getFieldDottedOrArray ( field ) ;
               rc = ixmGeoHash( objField, nextEle, FALSE, _objs,
                                _keygen->_keyPattern ) ;
               if ( SDB_OK != rc )
               {
                  /// if it is a nested obj.
                  if ( objField.isABSONObj() &&
                       objField.embeddedObject().firstElement().isABSONObj() )
                  {
                     nextEle = objField ;
                     rc = SDB_OK ;
                     goto done ;
                  }
                  else
                  {
                     /// do not return err.
                     nextEle = BSONElement() ;
                     rc = SDB_OK ;
                     goto error ;
                  }
               }
            }
            else if ( haveArrField )
            {
               if ( arrField.type() == Array )
               {
                  arrayNestedArray = TRUE ;
               }
               nextEle = arr.getFieldDottedOrArray ( field ) ;
            }
            else
            {
               nextEle = BSONElement() ;
            }
         }
      done:
         if ( NULL != firstField )
         {
            SDB_OSS_FREE( firstField ) ;
         }
         PD_TRACE_EXITRC ( SDB__IXMKEYGEN__EXTNXT2DELE, rc );
         return rc ;
      error:
         goto done ;
      }
      
      // PD_TRACE_DECLARE_FUNCTION ( SDB__IXMKEYGEN__EXTNXTELE, "_ixmKeyGenerator::_extractNextElement" )
      INT32 _extractNextElement ( const BSONObj &obj, const BSONObj &arr,
                                  const CHAR *&field, BOOLEAN &arrayNestedArray,
                                  BSONElement &nextEle ) const
      {
         INT32 rc = SDB_OK ;
         PD_TRACE_ENTRY ( SDB__IXMKEYGEN__EXTNXTELE );
         // copy the field name
         CHAR *firstField = ossStrdup ( field ) ;
         CHAR *pCur = firstField ;
         PD_CHECK ( firstField, SDB_OOM, error, PDERROR,
                    "Failed to duplicate field name: %s",
                    field ) ;
         // find the first .
         while ( *pCur )
         {
            if ( '.' == *pCur )
            {
               *pCur = '\0' ;
               break ;
            }
            pCur ++ ;
         }
         {
            // if both array and object contains the element, let's return error
            BOOLEAN haveObjField = !obj.getField ( firstField ).eoo() ;
            BSONElement arrField = arr.getField ( firstField ) ;
            BOOLEAN haveArrField = !arrField.eoo() ;
            PD_CHECK ( !haveObjField || !haveArrField, SDB_INVALIDARG, error,
                       PDERROR, "can't have both obj and arr field" ) ;
            arrayNestedArray = FALSE ;
            // if it's object
            if ( haveObjField )
            {
               nextEle = obj.getFieldDottedOrArray ( field ) ;
            }
            else if ( haveArrField )
            {
               // if it's array
               if ( arrField.type() == Array )
               {
                  arrayNestedArray = TRUE ;
               }
               nextEle = arr.getFieldDottedOrArray ( field ) ;
            }
            else
            {
               // if it's value
               nextEle = BSONElement() ;
            }
         }
      done :
         if ( firstField )
            SDB_OSS_FREE ( firstField ) ;
         PD_TRACE_EXITRC ( SDB__IXMKEYGEN__EXTNXTELE, rc );
         return rc ;
      error :
         goto done ;
      }

      // arrIdxs contains the field indexes that contains array
      // this function will loop through those indxes and build fixed object
      // from them
      // PD_TRACE_DECLARE_FUNCTION ( SDB__IXMKEYGEN__GETKEYSARRELTFIXED, "_ixmKeyGenerator::_getKeysArrEltFixed" )
      INT32 _getKeysArrEltFixed ( vector<const CHAR*> &fieldNames,
                                  vector<BSONElement> &fixed,
                                  const BSONElement &arrEntry,
                                  BSONObjSet &keys,
                                  INT32 numNotFound,
                                  const BSONElement &arrObjElt,
                                  const set<UINT32>&arrIdxs,
                                  BOOLEAN mayExpandArrayUnembedded ) const
      {
         INT32 rc = SDB_OK ;
         PD_TRACE_ENTRY ( SDB__IXMKEYGEN__GETKEYSARRELTFIXED );
         for ( set<UINT32>::const_iterator j = arrIdxs.begin();
               j != arrIdxs.end();
               j++ )
         {
            if ( *fieldNames[*j] == '\0' )
            {
               if ( IXM_EXTENT_HAS_TYPE( IXM_EXTENT_TYPE_2D,
                                         _keygen->_type ) &&
                    0 == *j )
               {
                  BSONElement ele ;
                  rc = ixmGeoHash( arrEntry, ele, TRUE, _objs,
                                   _keygen->_keyPattern ) ;
                  if ( SDB_OK != rc )
                  {
                     goto error ;
                  }
                  fixed[*j] = ele ;
               }
               else
               {
                  fixed[*j] = mayExpandArrayUnembedded?arrEntry:arrObjElt ;
               }
            }
            rc = _getKeys ( fieldNames, fixed, (arrEntry.type() == Object)?
                                                arrEntry.embeddedObject():
                                                BSONObj(),
                            keys, NULL, numNotFound,
                            arrObjElt.embeddedObject() ) ;
            PD_RC_CHECK ( rc, PDERROR, "Failed to expand array" ) ;
         }
      done :
         PD_TRACE_EXITRC ( SDB__IXMKEYGEN__GETKEYSARRELTFIXED, rc );
         return rc ;
      error :
         goto done ;
      }

      BSONObj _produceObj( const vector <BSONElement> &fixed ) const
      {
         BSONObjBuilder builder ;
         CHAR numStr[ 10 ] = {0} ;
         UINT32 eleNum = fixed.size() ;
         for ( UINT32 index = 0 ; index < eleNum ; ++index )
         {
            if ( GEN_OBJ_KEEP_FIELD_NAME == _keygen->_keyGenType )
            {
               builder.appendAs ( fixed[index],
                                  _keygen->_fieldNames[index] ) ;
            }
            else if ( GEN_OBJ_ARRAY_FIELD_NAME == _keygen->_keyGenType )
            {
               ossSnprintf( numStr, sizeof(numStr)-1, "%d", index ) ;
               builder.appendAs ( fixed[index], numStr ) ;
            }
            else
            {
               builder.appendAs ( fixed[index], "" ) ;
            }
         }
         return builder.obj() ;
      }

      // PD_TRACE_DECLARE_FUNCTION ( SDB__IXMKEYGEN__GETKEYS, "_ixmKeyGenerator::_getKeys" )
      INT32 _getKeys ( vector <const CHAR *> fieldNames,
                       vector <BSONElement>  fixed,
                       const BSONObj &obj,
                       BSONObjSet &keys,
                       BSONElement *pArrEle = NULL,
                       UINT32 numNotFound = 0,
                       const BSONObj &array = BSONObj() ) const
      {
         INT32 rc = SDB_OK ;
         PD_TRACE_ENTRY ( SDB__IXMKEYGEN__GETKEYS );
         BSONElement arrElt ;
         set<UINT32> arrIdxs ;
         BOOLEAN mayExpandArrayUnembedded = TRUE ;
         // loop through each field
         for ( UINT32 i = 0 ; i < fieldNames.size(); i++ )
         {
            // skip empty fields
            if ( *fieldNames[i] == '\0' )
            {
               continue ;
            }
            BOOLEAN arrayNestedArray ;
            // extract element matching fieldName[i] from object or array
            BSONElement e ;
            if ( IXM_EXTENT_HAS_TYPE( IXM_EXTENT_TYPE_2D, _keygen->_type ) &&
                 0 == i )
            {
               rc = _extractNext2dElement( obj, array, fieldNames[i],
                                           arrayNestedArray, e) ;
            }
            else
            {
               rc = _extractNextElement ( obj, array, fieldNames[i],
                                          arrayNestedArray, e ) ;
            }
            PD_RC_CHECK ( rc, PDERROR,
                          "Failed to extract next element from obj: %s",
                          obj.toString().c_str() ) ;
            // if field not found
            if ( e.eoo() )
            {
               fixed[i] = gUndefinedElt ;
               fieldNames[i] = "" ;
               numNotFound ++ ;
            }
            else if ( e.type() == Array )
            {
               // if we found array, we need to expand the array and access each
               // element inside
               arrIdxs.insert ( i ) ;
               if ( arrElt.eoo() )
               {
                  // if this is the first time we hit this array, let's put it
                  // into arrElt
                  arrElt = e ;
               }
               // if there are two keys contains array, we don't do that so
               // let's dump error
               else if ( e.rawdata() != arrElt.rawdata() )
               {
                  PD_LOG ( PDERROR, "At most one array can be in the key: "
                           "%s, %s", e.fieldName(), arrElt.fieldName() ) ;
                  rc = SDB_IXM_MULTIPLE_ARRAY ;
                  goto error ;
               }
               if ( arrayNestedArray )
               {
                  mayExpandArrayUnembedded = FALSE ;
               }
               if ( pArrEle )
               {
                  *pArrEle = arrElt ;
               }
            }
            else
            {
               // if we are already looking for this array, let's put it into
               // our fixed list
               fixed[i] = e ;
            }
         }
         // if we are not deailing with array
         if ( arrElt.eoo() )
         {
            // if we can't find any fields
            if ( _keygen->_nFields == (INT32)numNotFound &&
                 GEN_OBJ_NO_FIELD_NAME == _keygen->_keyGenType )
            {
               goto done ;
            }
            keys.insert ( _produceObj( fixed ) ) ;
         }
         else if ( arrElt.embeddedObject().firstElement().eoo() )
         {
            // if it's empty array
            rc = _getKeysArrEltFixed ( fieldNames, fixed, gUndefinedElt,
                                       keys, numNotFound, arrElt, arrIdxs,
                                       TRUE ) ;
            PD_RC_CHECK ( rc, PDERROR,
                          "Failed to get keys array element fixed, rc=%d",
                          rc ) ;
         }
         else
         {
            // array that can be expanded, so generate a key for each member
            BSONObj arrObj = arrElt.embeddedObject() ;
            BSONObjIterator i(arrObj) ;
            while (i.more() )
            {
               rc = _getKeysArrEltFixed ( fieldNames, fixed, i.next(), keys,
                                          numNotFound, arrElt, arrIdxs,
                                          mayExpandArrayUnembedded ) ;
               PD_RC_CHECK ( rc, PDERROR,
                             "Failed to get keys array element fixed, rc=%d",
                             rc ) ;
            }
         }
      done :
         PD_TRACE_EXITRC ( SDB__IXMKEYGEN__GETKEYS, rc );
         return rc ;
      error :
         goto done ;
      }
   } ;
   typedef class _ixmKeyGenerator ixmKeyGenerator ;
   // create key generator from index control block
   _ixmIndexKeyGen::_ixmIndexKeyGen ( const _ixmIndexCB *indexCB,
                                      IXM_KEYGEN_TYPE genType )
   {
      SDB_ASSERT ( indexCB, "details can't be NULL" )
      _keyPattern = indexCB->keyPattern() ;
      // whole _infoObj
      _info = indexCB->_infoObj ;
      _type = indexCB->getIndexType() ;
      _keyGenType = genType ;
      //_indexCB = indexCB ;
      _init() ;
   }
   // create key generator from key
   _ixmIndexKeyGen::_ixmIndexKeyGen ( const BSONObj &keyDef,
                                      IXM_KEYGEN_TYPE genType )
   {
      _keyPattern = keyDef.copy () ;
      _type = IXM_EXTENT_TYPE_NONE ;
      _keyGenType = genType ;
      _init () ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__IXMINXKEYGEN__INIT, "_ixmIndexKeyGen::_init" )
   void _ixmIndexKeyGen::_init()
   {
      PD_TRACE_ENTRY ( SDB__IXMINXKEYGEN__INIT );
      _nFields = _keyPattern.nFields () ;
      INT32 fieldNum = 0 ;
      {
         BSONObjIterator i(_keyPattern) ;
         while ( i.more())
         {
            BSONElement e = i.next() ;
            _fieldNames.push_back(e.fieldName()) ;
            _fixedElements.push_back(BSONElement()) ;
            ++fieldNum ;
         }
         _undefinedKey = ixmGetUndefineKeyObj( fieldNum ) ;
      }
      PD_TRACE_EXIT ( SDB__IXMINXKEYGEN__INIT );
   }

   INT32 _ixmIndexKeyGen::getKeys ( const BSONObj &obj, BSONObjSet &keys,
                                    BSONElement *pArrEle ) const
   {
      ixmKeyGenerator g (this) ;
      if ( pArrEle )
      {
         *pArrEle = BSONElement() ;
      }
      return g.getKeys ( obj, keys, pArrEle ) ;
   }

   // return True if there are at least one element match the name
   static BOOLEAN anyElementNamesMatch( const BSONObj& a , const BSONObj& b )
   {
      BSONObjIterator x(a);
      while ( x.more() )
      {
         BSONElement e = x.next();
         BSONObjIterator y(b);
         while ( y.more() )
         {
            BSONElement f = y.next();
            FieldCompareResult res = compareDottedFieldNames( e.fieldName(),
                                                              f.fieldName()
                                                            ) ;
            if ( res == SAME || res == LEFT_SUBFIELD || res == RIGHT_SUBFIELD )
               return TRUE;
         }
      }
      return FALSE;
   }
   IndexSuitability ixmIndexKeyGen::suitability( const BSONObj &query ,
                                                 const BSONObj &order ) const
   {
      return _suitability( query , order );
   }

   IndexSuitability ixmIndexKeyGen::_suitability( const BSONObj& query ,
                                                  const BSONObj& order ) const
   {
       // TODO: optimize
       if ( anyElementNamesMatch( _keyPattern , query ) == 0 &&
            anyElementNamesMatch( _keyPattern , order ) == 0 )
          return USELESS;
       return HELPFUL;
   }
   
   // PD_TRACE_DECLARE_FUNCTION ( SDB_IXMINXKEYGEN, "ixmIndexKeyGen::reset" )
   INT32 ixmIndexKeyGen::reset ( const BSONObj & info )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_IXMINXKEYGEN );
      _info = info ;
      try
      {
         _keyPattern = _info[IXM_KEY_FIELD].embeddedObjectUserCheck() ;
      }
      catch ( std::exception &e )
      {
         PD_LOG ( PDERROR, "Unable to locate valid key in index: %s",
                  e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      if ( _keyPattern.objsize() == 0 )
      {
         PD_LOG ( PDERROR, "Empty key" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      _init() ;
   done :
      PD_TRACE_EXITRC ( SDB_IXMINXKEYGEN, rc );
      return rc ;
   error :
      goto done ;
   }
   INT32 ixmIndexKeyGen::reset ( const _ixmIndexCB *indexCB )
   {
      SDB_ASSERT ( indexCB, "details can't be NULL" )
      //_indexCB = indexCB ;
      return reset ( indexCB->_infoObj ) ;
   }
   BSONElement ixmIndexKeyGen::missingField() const
   {
      return gUndefinedElt ;
   }

   // note this validate is validating whether an key def has fields other than
   // 1/-1, this check should NOT be directly used against an index key def,
   // because it may contains inregular key def like 2d index
   BOOLEAN _ixmIndexKeyGen::validateKeyDef ( const BSONObj &keyDef )
   {
      BSONObjIterator i ( keyDef ) ;
      INT32 count = 0 ;
      while ( i.more () )
      {
         ++count ;
         BSONElement ie = i.next () ;
         if ( ie.type() != NumberInt ||
              ( ie.numberInt() != -1 &&
                ie.numberInt() != 1 ) )
         {
            return FALSE ;
         }
      }
      // at least we need 1 field
      return 0 != count ;
   }
}
