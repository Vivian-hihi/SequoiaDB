/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = mthModifier.cpp

   Descriptive Name = Method Modifier

   When/how to use: this program may be used on binary and text-formatted
   versions of Method component. This file contains functions for creating a
   new record based on old record and modify rule.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#include <algorithm>
#include "core.hpp"
#include "pd.hpp"
#include "mthMatcher.hpp"
#include "mthModifier.hpp"
#include "dms.hpp"
#include "rtn.hpp"
#include "pdTrace.hpp"
#include "mthTrace.hpp"
#include "mthCommon.hpp"
using namespace bson ;
using namespace std ;

#define MTH_DOLLAR_FIELD_SIZE 11

namespace engine
{

#define ADD_CHG_ELEMENT( builder, ele, strChg ) \
   do { \
      if ( builder ) \
      { \
         BSONObjBuilder subBuilder ( builder->subobjStart ( strChg ) ) ; \
         subBuilder.append ( ele ) ; \
         subBuilder.done () ; \
      } \
   } while ( 0)

#define ADD_CHG_ELEMENT_AS( builder, ele, eleFieldName, strChg ) \
   do { \
      if ( builder ) \
      { \
         BSONObjBuilder subBuilder ( builder->subobjStart ( strChg ) ) ; \
         subBuilder.appendAs ( ele, eleFieldName ) ; \
         subBuilder.done () ; \
      } \
   } while ( 0)

#define ADD_CHG_UNSET_FIELD( builder, fieldName ) \
   do { \
      if ( builder ) \
      { \
         BSONObjBuilder subBuilder ( builder->subobjStart ( "$unset" ) ) ; \
         subBuilder.append ( fieldName, "" ) ; \
         subBuilder.done () ; \
      } \
   } while ( 0)

#define ADD_CHG_NUMBER( builder, fieldName, value, strChg ) \
   do { \
      if ( builder ) \
      { \
         BSONObjBuilder subBuilder ( builder->subobjStart ( strChg ) ) ; \
         subBuilder.append ( fieldName, value ) ; \
         subBuilder.done () ; \
      } \
   } while ( 0 )

#define ADD_CHG_ARRAY_OBJ(builder, obj, objFiledName, strChg ) \
   do { \
      if ( builder ) \
      { \
         BSONObjBuilder subBuilder ( builder->subobjStart ( strChg ) ) ; \
         subBuilder.appendArray ( objFiledName, obj ) ; \
         subBuilder.done () ; \
      } \
   } while ( 0 )

   INT32 _checkFieldName( const char* pField )
   {
      INT32 rc = SDB_OK ;
      static INT32 maxLoops = 1024 * 1024;
      INT32 num = 0 ;
      UINT32 lstart = 0;
      UINT32 lsize = strlen ( pField ) ;
      CHAR *a = NULL ;
      CHAR *lend = NULL ;
      CHAR  lold = 0 ;
      const CHAR *temp = NULL ;

      for ( INT32 i = 0; i < maxLoops ; ++i )
      {
         if ( lstart >= lsize )
         {
            goto done ;
         }
         // find the earliest '.' from current position
         a = (CHAR *)ossStrchr ( &pField[lstart], '.' ) ;
         // locate the ., or end of the string
         lend = ( NULL == a ) ? ( (CHAR *)&pField[lsize] ) : a ;
         // get the original left and right
         lold = *lend ;
         // set as end of string
         *lend = '\0' ;
         temp = &pField[lstart] ;
         if ( temp && '$' == *temp )
         {
            rc = ossStrToInt ( temp + 1, &num ) ;
            if ( rc )
            {
               goto error ;
            }
         }
         // restore old value
         *lend  = lold ;
         lstart = UINT32(lend - pField) + 1;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__MTHMDF__ADDMDF, "_mthModifier::_addModifier" )
   INT32 _mthModifier::_addModifier ( const BSONElement &ele, ModType type )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__MTHMDF__ADDMDF );
      if ( RENAME == type && ( ele.type() != String ||
           ossStrchr ( ele.valuestr(), '.' ) != NULL ||
           ossStrncmp ( ele.valuestr(), "$", 1 ) == 0 ) )
      {
         PD_LOG_MSG ( PDERROR, "Rename field must be string,and can't start "
                      "with '$', and can't include '.', %s",
                      ele.toString().c_str() ) ;
         goto error ;
      }
      else if ( INC == type &&  !ele.isNumber() )
      {
         PD_LOG_MSG ( PDERROR, "$inc field must be number, %s",
                      ele.toString().c_str() ) ;
         goto error ;
      }
      else if ( ( PUSH_ALL == type || PULL_ALL == type )
         && Array != ele.type () )
      {
         PD_LOG_MSG ( PDERROR, "$push_all/pull_all field must be array, %s",
                      ele.toString().c_str() ) ;
         goto error ;
      }
      else if ( POP == type && !ele.isNumber() )
      {
         PD_LOG_MSG ( PDERROR, "$pop field must be number, %s",
                      ele.toString().c_str() ) ;
         goto error ;
      }
      else if ( ( BITOR == type || BITAND == type || BITXOR == type
         || BITNOT == type ) && !ele.isNumber() )
      {
         PD_LOG_MSG ( PDERROR, "bitor/bitand/bitxor/bitnot field must be array, %s",
                      ele.toString().c_str()) ;
         goto error ;
      }
      else if ( BIT == type && !ele.isABSONObj() )
      {
         PD_LOG_MSG ( PDERROR, "bit field must be object , %s",
                      ele.toString().c_str()) ;
         goto error ;
      }
      else if ( ADDTOSET == type && Array != ele.type() )
      {
         PD_LOG_MSG ( PDERROR, "addtoset field must be array, %s",
                      ele.toString().c_str() ) ;
         goto error ;
      }
      else if ( ( UNSET == type || RENAME == type ) &&
           ossStrcmp ( ele.fieldName(), DMS_ID_KEY_NAME ) == 0 )
      {
         PD_LOG_MSG ( PDERROR, "ID field can't be renamed or unset" ) ;
         goto error ;
      }

      {
         rc = _checkFieldName ( ele.fieldName() ) ;
         if ( rc )
         {
            PD_LOG_MSG ( PDERROR, "Faild field name : %s", ele.fieldName() ) ;
            goto error ;
         }
         ModifierElement me(ele, type) ;
         _modifierElements.push_back(me) ;
      }

   done :
      PD_TRACE_EXITRC ( SDB__MTHMDF__ADDMDF, rc );
      return rc ;
   error :
      rc = SDB_INVALIDARG ;
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__MTHMDF__APPINCMDF, "_mthModifier::_applyIncModifier" )
   template<class Builder>
   INT32 _mthModifier::_applyIncModifier ( Builder &bb, const BSONElement &in,
                                           ModifierElement &me, CHAR **ppRoot )
   {
      PD_TRACE_ENTRY ( SDB__MTHMDF__APPINCMDF );
      BSONElement elt = me._toModify ;
      BSONType a = in.type() ;
      BSONType b = elt.type() ;
      CHAR dollarValue[ MTH_DOLLAR_FIELD_SIZE ] ;
      _compareFieldNames1 comp( _dollarList ) ;
      if ( NumberLong == a || NumberInt == a || NumberDouble == a )
      {
         ADD_CHG_ELEMENT_AS ( _srcChgBuilder, in, (*ppRoot),
                              "$set" ) ;

         if ( NumberDouble == a || NumberDouble == b )
         {
            bb.append ( comp.getDollarValue( me._shortName, &dollarValue[0] ),
                        in.numberDouble()+elt.numberDouble()) ;
            ADD_CHG_NUMBER ( _dstChgBuilder, (*ppRoot),
                             in.numberDouble()+elt.numberDouble(), "$set" ) ;
         }
         else if ( NumberLong == a || NumberLong == b )
         {
            bb.append ( comp.getDollarValue( me._shortName, &dollarValue[0] ),
                        in.numberLong() + elt.numberLong()) ;
            ADD_CHG_NUMBER ( _dstChgBuilder, (*ppRoot),
                             in.numberLong() + elt.numberLong(), "$set" ) ;
         }
         else
         {
            INT32 result = in.numberInt() + elt.numberInt() ;
            INT64 result64 = (INT64)in.numberInt() + (INT64)elt.numberInt() ;
            if ( (result64<0 && result>0) || (result64>0 && result<0))
            {
               //32 bit overflow or underflow happened
               bb.append ( comp.getDollarValue( me._shortName,&dollarValue[0]),
                           in.numberLong() + elt.numberLong()) ;
               ADD_CHG_NUMBER ( _dstChgBuilder, (*ppRoot),
                                in.numberLong() + elt.numberLong(), "$set" ) ;
            }
            else
            {
               bb.append ( comp.getDollarValue( me._shortName,&dollarValue[0]),
                           result ) ;
               ADD_CHG_NUMBER ( _dstChgBuilder, (*ppRoot),
                                result, "$set" ) ;
            }
         }
      }
      else
      {
         //not change, add the old element
         bb.append ( in ) ;
      }
      PD_TRACE_EXIT ( SDB__MTHMDF__APPINCMDF );
      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__MTHMDF__APPSETMDF, "_mthModifier::_applySetModifier" )
   template<class Builder>
   INT32 _mthModifier::_applySetModifier( Builder &bb, const BSONElement &in,
                                          ModifierElement &me, CHAR **ppRoot )
   {
      // for SET, if the type is Object, we need to make sure we can store it
      // in BSON, if the object contains keyword $ref or $id, then it's not
      // storable
      CHAR dollarValue[ MTH_DOLLAR_FIELD_SIZE ] ;
      _compareFieldNames1 comp( _dollarList ) ;
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__MTHMDF__APPSETMDF );

      if ( in.type() == Object )
      {
         if ( !in.embeddedObject().okForStorage() )
         {
            PD_LOG_MSG ( PDERROR, "embbed object can't be stored for %s",
                         in.toString().c_str());
            rc = SDB_INVALIDARG ;
            goto done ;
         }
      }
      {
         /*ADD_CHG_ELEMENT_AS ( _srcChgBuilder, in, me._toModify.fieldName(),
                             "$set" ) ;
         ADD_CHG_ELEMENT ( _dstChgBuilder, me._toModify, "$set" ) ;*/
         ADD_CHG_ELEMENT_AS ( _srcChgBuilder, in, (*ppRoot),
                             "$set" ) ;
         ADD_CHG_ELEMENT_AS ( _dstChgBuilder, me._toModify, (*ppRoot),
                             "$set" ) ;
         // get the element and shortname
         bb.appendAs ( me._toModify,
                       comp.getDollarValue( me._shortName, &dollarValue[0] ) ) ;
      }

   done :
      PD_TRACE_EXITRC ( SDB__MTHMDF__APPSETMDF, rc );
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__MTHMDF__APPPSHMDF, "_mthModifier::_applyPushModifier" )
   template<class Builder>
   INT32 _mthModifier::_applyPushModifier ( Builder &bb, const BSONElement &in,
                                            ModifierElement &me, CHAR **ppRoot )
   {
      PD_TRACE_ENTRY ( SDB__MTHMDF__APPPSHMDF );
      INT32 rc= SDB_OK ;
      // make sure the original type is array
      if ( Array != in.type() )
      {
         PD_LOG_MSG ( PDERROR, "Original data type is not array: %s",
                      in.toString().c_str());
         rc = SDB_INVALIDARG ;
         goto done ;
      }
      {
         // create bson builder for the array
         BSONObjBuilder sub ( bb.subarrayStart ( me._shortName ) ) ;
         BSONObjIterator i ( in.embeddedObject() ) ;
         INT32 n = 0 ;
         while ( i.more() )
         {
            sub.append( i.next() ) ;
            n++ ;
         }
         sub.appendAs ( me._toModify, sub.numStr(n) ) ;
         BSONObj newObj = sub.done() ;

         ADD_CHG_ARRAY_OBJ ( _dstChgBuilder, newObj, (*ppRoot),
                             "$set" ) ;
         ADD_CHG_ELEMENT_AS ( _srcChgBuilder, in, (*ppRoot),
                              "$set" ) ;
      }

   done :
      PD_TRACE_EXITRC ( SDB__MTHMDF__APPPSHMDF, rc );
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__MTHMDF__APPPSHALLMDF, "_mthModifier::_applyPushAllModifier" )
   template<class Builder>
   INT32 _mthModifier::_applyPushAllModifier ( Builder & bb,
                                               const BSONElement & in,
                                               ModifierElement & me,
                                               CHAR **ppRoot )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__MTHMDF__APPPSHALLMDF );
      // make sure the original type is array
      if ( in.type() != Array )
      {
         PD_LOG_MSG ( PDERROR, "Original data type is not array: %s",
                      in.toString().c_str());
         rc = SDB_INVALIDARG ;
         goto done ;
      }
      // make sure the new type is array too
      if ( me._toModify.type() != Array )
      {
         PD_LOG_MSG ( PDERROR, "pushed data type is not array: %s",
                      me._toModify.toString().c_str());
         rc = SDB_INVALIDARG ;
         goto done ;
      }
      {
         // create bson builder for the array
         BSONObjBuilder sub ( bb.subarrayStart ( me._shortName ) ) ;
         BSONObjIterator i ( in.embeddedObject()) ;
         INT32 n = 0 ;
         while ( i.more() )
         {
            sub.append( i.next() ) ;
            n++ ;
         }

         i = BSONObjIterator ( me._toModify.embeddedObject()) ;
         while( i.more() )
         {
            sub.appendAs( i.next(), sub.numStr(n++) ) ;
         }
         BSONObj newObj = sub.done() ;

         ADD_CHG_ARRAY_OBJ ( _dstChgBuilder, newObj, (*ppRoot),
                             "$set" ) ;
         ADD_CHG_ELEMENT_AS ( _srcChgBuilder, in, (*ppRoot),
                              "$set" ) ;
      }

   done :
      PD_TRACE_EXITRC ( SDB__MTHMDF__APPPSHALLMDF, rc );
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__MTHMDF__APPPLLMDF, "_mthModifier::_applyPullModifier" )
   template<class Builder>
   INT32 _mthModifier::_applyPullModifier ( Builder &bb, const BSONElement &in,
                                            ModifierElement &me, CHAR **ppRoot )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__MTHMDF__APPPLLMDF );
      // make sure the original type is array
      if ( in.type() != Array )
      {
         PD_LOG_MSG ( PDERROR, "Original data type is not array: %s",
                      in.toString().c_str());
         rc = SDB_INVALIDARG ;
         goto done ;
      }
      {
         // need to create a builder regardless if pull success or not
         // even if all elements matches, we still need this empty array
         BSONObjBuilder sub ( bb.subarrayStart ( me._shortName ) ) ;
         INT32 n = 0 ;
         // for each element in the original data
         BSONObjIterator i ( in.embeddedObject() ) ;
         while ( i.more() )
         {
            BSONElement ele = i.next() ;
            BOOLEAN allowed = TRUE ;
            if ( PULL == me._modType )
            {
               allowed = ! _pullElementMatch ( ele, me._toModify ) ;
            }
            else
            {
               BSONObjIterator j ( me._toModify.embeddedObject() ) ;
               while ( j.more() )
               {
                  BSONElement arrJ = j.next() ;
                  if ( ele.woCompare(arrJ, FALSE) == 0 )
                  {
                     allowed = FALSE ;
                     break ;
                  }
               }
            }
            if ( allowed )
            {
               sub.appendAs ( ele, sub.numStr(n++) ) ;
            }
         }
         BSONObj newObj = sub.done() ;

         ADD_CHG_ARRAY_OBJ ( _dstChgBuilder, newObj, (*ppRoot),
                             "$set" ) ;
         ADD_CHG_ELEMENT_AS ( _srcChgBuilder, in, (*ppRoot),
                              "$set" ) ;
      }

   done :
      PD_TRACE_EXITRC ( SDB__MTHMDF__APPPLLMDF, rc );
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__MTHMDF__APPPOPMDF, "_mthModifier::_applyPopModifier" )
   template<class Builder>
   INT32 _mthModifier::_applyPopModifier ( Builder &bb, const BSONElement &in,
                                           ModifierElement &me, CHAR **ppRoot )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__MTHMDF__APPPOPMDF );
      // remove the n'th element from array's front or end
      // if input number is 0, then it doesn't do anything
      // input number < 0 means remove from front
      // input number > 0 means remove from end
      if ( Array != in.type() )
      {
         PD_LOG_MSG ( PDERROR, "Original data type is not array: %s",
                      in.toString().c_str());
         rc = SDB_INVALIDARG ;
         goto done ;
      }

      if ( !me._toModify.isNumber() )
      {
         PD_LOG_MSG ( PDERROR, "pop data type must be a number: %s",
                      me._toModify.toString().c_str());
         rc = SDB_INVALIDARG ;
         goto done ;
      }

      // if specify 0, which means don't pop anything
      if ( me._toModify.number() == 0 )
      {
         bb.append ( in ) ;
         goto done ;
      }
      {
         BSONObjBuilder sub ( bb.subarrayStart ( me._shortName ) ) ;
         INT32 n = 0 ;
         // if specify < 0, which means pop the n'th element from front
         if ( me._toModify.number() < 0 )
         {
            INT32 m = (INT32)me._toModify.number() ;
            BSONObjIterator i ( in.embeddedObject() ) ;
            while ( i.more() )
            {
               m++ ;
               if ( m > 0 )
               {
                  sub.appendAs( i.next(), sub.numStr(n++) ) ;
               }
               else
               {
                  i.next() ;
               }
            }
         }
         else
         {
            // if specify > 0, we need to pop the n'th element from end
            // first we need to know how many elements in total
            INT32 count = 0 ;
            INT32 m = (INT32)me._toModify.number() ;
            BSONObjIterator i ( in.embeddedObject() ) ;
            while ( i.more())
            {
               i.next() ;
               count++ ;
            }
            count = count - m ;
            i = BSONObjIterator ( in.embeddedObject() ) ;
            while ( i.more() )
            {
               if ( count > 0 )
               {
                  sub.appendAs( i.next(), sub.numStr(n++) ) ;
               }
               else
               {
                  break ;
               }

               count-- ;
            }
         }
         BSONObj newObj = sub.done() ;

         ADD_CHG_ARRAY_OBJ ( _dstChgBuilder, newObj, (*ppRoot),
                             "$set" ) ;
         ADD_CHG_ELEMENT_AS ( _srcChgBuilder, in, (*ppRoot),
                              "$set" ) ;
      }

   done :
      PD_TRACE_EXITRC ( SDB__MTHMDF__APPPOPMDF, rc );
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__MTHMDF__APPBITMDF, "_mthModifier::_applyBitModifier" )
   template<class Builder>
   INT32 _mthModifier::_applyBitModifier ( Builder &bb, const BSONElement &in,
                                           ModifierElement &me, CHAR **ppRoot )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__MTHMDF__APPBITMDF );
      BSONElement elt = me._toModify ;
      BSONType a = in.type() ;
      BSONType b = elt.type() ;
      BOOLEAN change = FALSE ;
      CHAR dollarValue[ MTH_DOLLAR_FIELD_SIZE ] ;
      _compareFieldNames1 comp( _dollarList ) ;

      if ( NumberLong == a || NumberInt == a )
      {
         if ( NumberLong == a || NumberLong == b )
         {
            INT64 result = 0 ;
            rc = _bitCalc ( me._modType, in.numberLong(),
                            elt.numberLong(), result ) ;
            if ( SDB_OK == rc )
            {
               change = TRUE ;
               bb.append ( comp.getDollarValue( me._shortName, &dollarValue[0]),
                           ( long long )result ) ;
               ADD_CHG_NUMBER ( _dstChgBuilder, (*ppRoot),
                               (long long)result, "$set" ) ;
            }
         }
         else if ( NumberInt == a || NumberInt == b )
         {
            INT32 result = 0 ;
            rc = _bitCalc ( me._modType, in.numberInt(),
                            elt.numberInt(), result ) ;
            if ( SDB_OK == rc )
            {
               change = TRUE ;
               bb.append ( comp.getDollarValue( me._shortName, &dollarValue[0]),
                           (int)result ) ;
               ADD_CHG_NUMBER ( _dstChgBuilder, (*ppRoot),
                               (int)result, "$set" ) ;
            }
         }
      }

      if ( change )
      {
         ADD_CHG_ELEMENT_AS ( _srcChgBuilder, in, (*ppRoot),
                              "$set" ) ;
      }
      else
      {
         //not change, should add the org element
         bb.append ( in ) ;
      }
      PD_TRACE_EXITRC ( SDB__MTHMDF__APPBITMDF, rc );
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__MTHMDF__APPBITMDF2, "_mthModifier::_applyBitModifier2" )
   template<class Builder>
   INT32 _mthModifier::_applyBitModifier2 ( Builder &bb,
                                            const BSONElement &in,
                                            ModifierElement &me,
                                            CHAR **ppRoot )
   {
      INT32 rc = SDB_OK ;
      CHAR dollarValue[ MTH_DOLLAR_FIELD_SIZE ] ;
      _compareFieldNames1 comp( _dollarList ) ;
      //if org is not int or long, not change
      if ( NumberInt != in.type() && NumberLong != in.type() )
      {
         bb.append ( in ) ;
         goto done ;
      }
      {
         ModType type = UNKNOW ;
         BSONObjIterator it ( me._toModify.embeddedObject () ) ;
         INT32 x = in.numberInt () ;
         INT64 y = in.numberLong () ;

         while ( it.more () )
         {
            BSONElement e = it.next () ;
            if ( NumberInt != e.type () && NumberLong != e.type () )
            {
               PD_LOG_MSG ( PDERROR, "bit object field elements must be int or long, %s",
                            e.toString( TRUE ).c_str() ) ;
               rc = SDB_INVALIDARG ;
               goto done ;
            }
            {
               string oprName = "$bit" ;
               oprName += e.fieldName () ;
               type = _parseModType ( oprName.c_str() ) ;
            }
            if ( UNKNOW == type )
            {
               PD_LOG_MSG ( PDERROR, "unknow bit operator, %s",
                            e.toString( TRUE ).c_str() ) ;
               rc = SDB_INVALIDARG ;
               goto done ;
            }

            rc = _bitCalc ( type, x, e.numberInt(), x ) ;
            if ( SDB_OK != rc )
            {
               goto done ;
            }
            rc = _bitCalc ( type, y, e.numberLong(), y ) ;
            if ( SDB_OK != rc )
            {
               goto done ;
            }
         }

         if ( ( INT64 ) x != y )
         {
            bb.append ( comp.getDollarValue( me._shortName, &dollarValue[0] ),
                        y ) ;
            ADD_CHG_NUMBER ( _dstChgBuilder, (*ppRoot),
                             y, "$set" ) ;
         }
         else
         {
            bb.append ( comp.getDollarValue( me._shortName, &dollarValue[0] ),
                        x ) ;
            ADD_CHG_NUMBER ( _dstChgBuilder, (*ppRoot),
                             x, "$set" ) ;
         }
         ADD_CHG_ELEMENT_AS ( _srcChgBuilder, in, (*ppRoot),
                              "$set" ) ;
      }
   done :
      PD_TRACE_EXITRC ( SDB__MTHMDF__APPBITMDF2, rc );
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__MTHMDF__APPADD2SETMDF, "_mthModifier::_applyAddtoSetModifier" )
   template<class Builder>
   INT32 _mthModifier::_applyAddtoSetModifier ( Builder &bb,
                                                const BSONElement &in,
                                                ModifierElement &me,
                                                CHAR **ppRoot )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__MTHMDF__APPADD2SETMDF );
      // add each element in array into existing array
      // don't want to add duplicates in addtoset
      // make sure original data is array
      if ( Array != in.type() )
      {
         PD_LOG_MSG ( PDERROR, "Original data type is not array: %s",
                      in.toString().c_str() );
         rc = SDB_INVALIDARG ;
         goto done ;
      }
      // make sure added value is array
      if ( Array != me._toModify.type() )
      {
        PD_LOG_MSG ( PDERROR, "added data type is not array: %s",
                     me._toModify.toString().c_str()) ;
        rc = SDB_INVALIDARG ;
        goto done ;
      }
      {
         BSONObjBuilder sub ( bb.subarrayStart ( me._shortName ) ) ;
         BSONObjIterator i ( in.embeddedObject() ) ;
         BSONObjIterator j ( me._toModify.embeddedObject() ) ;
         INT32 n = 0 ;
         // make bsonelementset for everything we want to add
         BSONElementSet eleset ;
         while ( j.more() )
         {
            eleset.insert( j.next() ) ;
         }
         while ( i.more() )
         {
            BSONElement cur = i.next() ;
            sub.append( cur ) ;
            n++ ;
            eleset.erase( cur ) ;
         }
         INT32 orgNum = n ;
         BSONElementSet::iterator it ;
         for ( it = eleset.begin(); it != eleset.end(); it++ )
         {
            sub.appendAs( (*it), sub.numStr(n++) ) ;
         }
         BSONObj newObj = sub.done() ;

         //add new element
         if ( orgNum != n )
         {
            ADD_CHG_ARRAY_OBJ ( _dstChgBuilder, newObj, (*ppRoot),
                                "$set" ) ;
            ADD_CHG_ELEMENT_AS ( _srcChgBuilder, in, (*ppRoot),
                                 "$set" ) ;
         }
      }
   done :
      PD_TRACE_EXITRC ( SDB__MTHMDF__APPADD2SETMDF, rc );
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__MTHMDF__BITCALC, "_mthModifier::_bitCalc" )
   template<class VType>
   INT32 _mthModifier::_bitCalc ( ModType type, VType l, VType r, VType & out )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__MTHMDF__BITCALC );
      switch ( type )
      {
         case BITAND :
            out = l & r ;
            break ;
         case BITOR :
            out = l | r ;
            break ;
         case BITNOT :
            out = ~l ;
            break ;
         case BITXOR :
            out = l ^ r ;
            break ;
         default :
            PD_LOG_MSG ( PDERROR, "unknow bit modifier type[%d]", type ) ;
            rc = SDB_INVALIDARG ;
            break ;
      }
      PD_TRACE_EXITRC ( SDB__MTHMDF__BITCALC, rc );
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__MTHMDF__APPBITMDF3, "_mthModifier::_appendBitModifier" )
   template<class Builder>
   INT32 _mthModifier::_appendBitModifier ( Builder &bb, INT32 in,
                                            ModifierElement &me,
                                            CHAR **ppRoot )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__MTHMDF__APPBITMDF3 );
      BSONElement elt = me._toModify ;
      BSONType b = elt.type () ;
      BOOLEAN change = FALSE ;
      CHAR dollarValue[ MTH_DOLLAR_FIELD_SIZE ] ;
      _compareFieldNames1 comp( _dollarList ) ;

      if ( NumberLong == b )
      {
         INT64 result = 0 ;
         rc = _bitCalc ( me._modType, (INT64)in, elt.numberLong(), result ) ;
         if ( SDB_OK == rc )
         {
            change = TRUE ;
            bb.append ( comp.getDollarValue( me._shortName, &dollarValue[0] ),
                        result ) ;
            ADD_CHG_NUMBER ( _dstChgBuilder, (*ppRoot),
                             result, "$set" ) ;
         }
      }
      else if ( NumberInt == b )
      {
         INT32 result = 0 ;
         rc = _bitCalc ( me._modType, in, elt.numberInt(), result ) ;
         if ( SDB_OK == rc )
         {
            change = TRUE ;
            bb.append ( comp.getDollarValue( me._shortName, &dollarValue[0] ),
                        result ) ;
            ADD_CHG_NUMBER ( _dstChgBuilder, (*ppRoot),
                             result, "$set" ) ;
         }
      }

      if ( change )
      {
         ADD_CHG_UNSET_FIELD ( _srcChgBuilder, (*ppRoot) ) ;
      }
      PD_TRACE_EXITRC ( SDB__MTHMDF__APPBITMDF3, rc );
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__MTHMDF__APPBITMDF22, "_mthModifier::_appendBitModifier2" )
   template<class Builder>
   INT32 _mthModifier::_appendBitModifier2 ( Builder &bb, INT32 in,
                                             ModifierElement &me,
                                             CHAR **ppRoot )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__MTHMDF__APPBITMDF22 );
      ModType type = UNKNOW ;
      BSONObjIterator it ( me._toModify.embeddedObject () ) ;
      INT32 x = in ;
      INT64 y = (INT64)in ;
      CHAR dollarValue[ MTH_DOLLAR_FIELD_SIZE ] ;
      _compareFieldNames1 comp( _dollarList ) ;

      while ( it.more () )
      {
         BSONElement e = it.next () ;

         if ( NumberInt != e.type() && NumberLong != e.type() )
         {
            PD_LOG_MSG ( PDERROR,
                         "bit object field elements must be int or long, %s",
                         e.toString( TRUE ).c_str() ) ;
            rc = SDB_INVALIDARG ;
            goto done ;
         }

         {
            string oprName = "$bit" ;
            oprName += e.fieldName () ;
            type = _parseModType ( oprName.c_str() ) ;
         }
         if ( UNKNOW == type )
         {
            PD_LOG_MSG ( PDERROR, "unknow bit operator, %s",
                         e.toString( TRUE ).c_str() ) ;
            rc = SDB_INVALIDARG ;
            goto done ;
         }

         rc = _bitCalc ( type, x, e.numberInt(), x ) ;
         if ( SDB_OK != rc )
         {
            goto done ;
         }
         rc = _bitCalc ( type, y, e.numberLong(), y ) ;
         if ( SDB_OK != rc )
         {
            goto done ;
         }
      }

      if ( (INT64)x != y )
      {
         bb.append ( comp.getDollarValue( me._shortName, &dollarValue[0] ),
                     y ) ;
         ADD_CHG_NUMBER ( _dstChgBuilder, (*ppRoot),
                          y, "$set" ) ;
      }
      else
      {
         bb.append ( comp.getDollarValue( me._shortName, &dollarValue[0] ),
                     x ) ;
         ADD_CHG_NUMBER ( _dstChgBuilder, (*ppRoot),
                          x, "$set" ) ;
      }
      ADD_CHG_UNSET_FIELD ( _srcChgBuilder, (*ppRoot) ) ;

   done :
      PD_TRACE_EXITRC ( SDB__MTHMDF__APPBITMDF22, rc );
      return rc ;
   }

   BOOLEAN _mthModifier::_pullElementMatch( BSONElement& org,
                                            BSONElement& toMatch )
   {
      // if the one we are trying to match is not object, then we call woCompare
      if ( toMatch.type() != Object )
      {
         return org.valuesEqual(toMatch) ;
      }
      // if we want to match an object but original data is not object, then
      // it's not possible to have a match
      if ( org.type() != Object )
      {
         return FALSE ;
      }
      // otherwise let's do full compare if both sides are object
      return org.woCompare(toMatch, FALSE) == 0 ;
   }


   template<class Builder>
   void _mthModifier::_applyUnsetModifier(Builder &b)
   {}

   void _mthModifier::_applyUnsetModifier(BSONArrayBuilder &b)
   {
      b.appendNull() ;
   }
   ModType _mthModifier::_parseModType ( const CHAR * field )
   {
      SDB_ASSERT ( field, "field can't be NULL " )

      if ( MTH_OPERATOR_EYECATCHER == field[0] )
      {
         if ( field[1] == 'a' )
         {
            if ( field[2] == 'd' && field[3] == 'd' &&
                 field[4] == 't' && field[5] == 'o' &&
                 field[6] == 's' && field[7] == 'e' &&
                 field[8] == 't' && field[9] == 0 )
            {
               return ADDTOSET ;
            }
         }
         else if ( field[1] == 'b' )
         {
            if ( field[2] == 'i' && field[3] == 't' )
            {
               if ( field[4] == 0 )
               {
                  return BIT ;
               }
               else if ( field[4]=='a'&&field[5]=='n'&&
                         field[6]=='d'&&field[7]==0 )
               {
                  return BITAND ;
               }
               else if ( field[4]=='o'&&field[5]=='r'&&
                         field[6]==0 )
               {
                  return BITOR ;
               }
               else if ( field[4]=='n'&&field[5]=='o'&&
                         field[6]=='t'&&field[7]==0 )
               {
                  return BITNOT ;
               }
               else if ( field[4]=='x'&&field[5]=='o'&&
                         field[6]=='r'&&field[7]==0 )
               {
                  return BITXOR ;
               }
            }
         }
         else if ( field[1] == 'i' )
         {
            if ( field[2] == 'n' && field[3] == 'c' &&
                 field[4] == 0 )
            {
               return INC ;
            }
         }
         else if ( field[1] == 'p' )
         {
            if ( field[2] == 'u' )
            {
               if ( field[3] == 'l' && field[4] == 'l' )
               {
                  if ( field[5] == 0 )
                  {
                     return PULL ;
                  }
                  else if ( field[5]=='_'&&field[6]=='a'&&
                            field[7]=='l'&&field[8]=='l'&&
                            field[9]==0 )
                  {
                     return PULL_ALL ;
                  }
               }
               else if ( field[3]=='s' && field[4] == 'h' )
               {
                  if ( field[5] == 0 )
                  {
                     return PUSH ;
                  }
                  else if ( field[5]=='_'&&field[6]=='a'&&
                            field[7]=='l'&&field[8]=='l'&&
                            field[9]==0 )
                  {
                     return PUSH_ALL ;
                  }
               }
            } //u
            else if (field[2] == 'o' )
            {
               if ( field[3] == 'p' && field[4] == 0 )
               {
                  return POP ;
               }
            }
         }// p
         else if ( field[1] == 'r' )
         {
            if ( field[2] == 'e' && field[3] == 'n' &&
                 field[4] == 'a' && field[5] == 'm' &&
                 field[6] == 'e' && field[7] == 0 )
            {
               return RENAME ;
            }
         } // r
         else if ( field[1] == 's' )
         {
            if ( field[2] == 'e' && field[3] == 't' &&
                 field[4] == 0 )
            {
               return SET ;
            }
         }
         else if ( field[1] == 'u' )
         {
            if ( field[2] == 'n' && field[3] == 's' &&
                 field[4] == 'e' && field[5] == 't' &&
                 field[6] == 0 )
            {
               return UNSET ;
            }
         }
      }

      return UNKNOW ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__MTHMDF_PASELE, "_mthModifier::_parseElement" )
   INT32 _mthModifier::_parseElement ( const BSONElement &ele )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__MTHMDF_PASELE );
      SDB_ASSERT ( ele.type() != Undefined, "Undefined element type" ) ;
      // get field name first
      ModType type = _parseModType( ele.fieldName () ) ;
      if ( UNKNOW == type )
      {
         PD_LOG_MSG ( PDERROR, "Updator operator[%s] error", ele.fieldName () ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      // then check element type
      switch ( ele.type() )
      {
      case Object:
      {
         // for {$inc, $pull, etc...} cases
         BSONObjIterator j(ele.embeddedObject()) ;
         // even thou this is a loop, we always exist after parsing the first
         // element
         while ( j.more () )
         {
            rc = _addModifier ( j.next(), type ) ;

            if ( SDB_OK != rc )
            {
               goto error ;
            }
         } // while
         break ;
      }
      default:
      {
         // each element must be an object, the field name is operator and
         // object contains field name and value
         // for example
         // $inc : { votes: 1 }    # for increment votes by 1
         PD_LOG ( PDERROR, "each element in modifier pattern must be object" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      }

   done :
      PD_TRACE_EXITRC ( SDB__MTHMDF_PASELE, rc );
      return rc ;
   error :
      goto done ;
   }

   CHAR *_compareFieldNames1::getDollarValue ( CHAR *s, CHAR *in )
   {
      INT64 temp = 0 ;
      INT32 dollarNum = 0 ;
      INT32 num = -1 ;
      CHAR *t = in ;

      if ( *s && '$' == *s )
      {
         ossMemset ( t, 0, MTH_DOLLAR_FIELD_SIZE ) ;
         ossStrToInt ( s + 1, &dollarNum ) ;
         if ( _dollarList )
         {
            for ( vector<INT64>::iterator it = _dollarList->begin();
                  it != _dollarList->end(); ++it )
            {
               temp = *it ;
               if ( dollarNum == ((temp>>32)&0xFFFFFFFF) )
               {
                  num = (temp&0xFFFFFFFF) ;
                  break ;
               }
            }
         }
         ossSnprintf ( t, MTH_DOLLAR_FIELD_SIZE, "%d", num ) ;
         return t ;
      }
      else
      {
         return s ;
      }
   }

   INT32 _compareFieldNames1::checkDollarValue( const char* pField )
   {
      INT32 rc = SDB_OK ;
      static INT32 maxLoops = 1024 * 1024;
      INT32 num = 0 ;
      UINT32 lstart = 0;
      UINT32 lsize = strlen ( pField ) ;
      CHAR *a = NULL ;
      CHAR *lend = NULL ;
      CHAR  lold = 0 ;
      const CHAR *temp = NULL ;
      INT64 tempValue = 0 ;
      for ( INT32 i = 0; i < maxLoops ; ++i )
      {
         if ( lstart >= lsize )
         {
            goto done ;
         }
         // find the earliest '.' from current position
         a = (CHAR *)ossStrchr ( &pField[lstart], '.' ) ;
         // locate the ., or end of the string
         lend = ( NULL == a ) ? ( (CHAR *)&pField[lsize] ) : a ;
         // get the original left and right
         lold = *lend ;
         // set as end of string
         *lend = '\0' ;
         temp = &pField[lstart] ;
         if ( temp && '$' == *temp )
         {
            rc = ossStrToInt ( temp + 1, &num ) ;
            if ( rc )
            {
               goto error ;
            }
            if ( _dollarList )
            {
               for ( vector<INT64>::iterator it = _dollarList->begin();
                  it != _dollarList->end(); ++it )
               {
                  tempValue = *it ;
                  if ( num == ((tempValue>>32)&0xFFFFFFFF) )
                  {
                     goto done ;
                  }
               }
               rc = SDB_INVALIDARG ;
               goto error ;
            }
            else
            {
               rc = SDB_INVALIDARG ;
               goto error ;
            }
         }
         // restore old value
         *lend  = lold ;
         lstart = UINT32(lend - pField) + 1;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   inline BOOLEAN _compareFieldNames1::_isNumber( CHAR c )
   {
      return c >= '0' && c <= '9';
   }

   inline INT32 _compareFieldNames1::_lexNumCmp ( const CHAR *s1,
                                                  const CHAR *s2 )
   {
      BOOLEAN p1 = FALSE ;
      BOOLEAN p2 = FALSE ;
      BOOLEAN n1 = FALSE ;
      BOOLEAN n2 = FALSE ;

      CHAR   *e1 = NULL ;
      CHAR   *e2 = NULL ;

      INT32   len1   = 0 ;
      INT32   len2   = 0 ;
      INT32   result = 0 ;

      CHAR t1[MTH_DOLLAR_FIELD_SIZE] ;
      CHAR t2[MTH_DOLLAR_FIELD_SIZE] ;

      if ( *s1 && '$' == *s1 )
      {
         INT64 temp = 0 ;
         INT32 dollarNum = 0 ;
         INT32 num = -1 ;
         ossMemset ( t1, 0, MTH_DOLLAR_FIELD_SIZE ) ;
         ossStrToInt ( s1 + 1, &dollarNum ) ;
         if ( _dollarList )
         {
            for ( vector<INT64>::iterator it = _dollarList->begin();
                  it != _dollarList->end(); ++it )
            {
               temp = *it ;
               if ( dollarNum == ((temp>>32)&0xFFFFFFFF) )
               {
                  num = (temp&0xFFFFFFFF) ;
                  break ;
               }
            }
         }
         ossSnprintf ( t1, MTH_DOLLAR_FIELD_SIZE, "%d", num ) ;
         s1 = t1 ;
      }

      if ( *s2 && '$' == *s2 )
      {
         INT64 temp = 0 ;
         INT32 dollarNum = 0 ;
         INT32 num = -1 ;
         ossMemset ( t2, 0, MTH_DOLLAR_FIELD_SIZE ) ;
         ossStrToInt ( s2 + 1, &dollarNum ) ;
         if ( _dollarList )
         {
            for ( vector<INT64>::iterator it = _dollarList->begin();
                  it != _dollarList->end(); ++it )
            {
               temp = *it ;
               if ( dollarNum == ((temp>>32)&0xFFFFFFFF) )
               {
                  num = (temp&0xFFFFFFFF) ;
                  break ;
               }
            }
         }
         ossSnprintf ( t2, MTH_DOLLAR_FIELD_SIZE, "%d", num ) ;
         s2 = t2 ;
      }

      while( *s1 && *s2 )
      {
         p1 = ( *s1 == (CHAR)255 ) ;
         p2 = ( *s2 == (CHAR)255 ) ;
         if ( p1 && !p2 )
         {
            return 1 ;
         }
         if ( !p1 && p2 )
         {
            return -1 ;
         }

         n1 = _isNumber( *s1 ) ;
         n2 = _isNumber( *s2 ) ;

         if ( n1 && n2 )
         {
            // get rid of leading 0s
            while ( *s1 == '0' )
            {
               ++s1 ;
            }
            while ( *s2 == '0' )
            {
               ++s2 ;
            }

            e1 = (CHAR *)s1 ;
            e2 = (CHAR *)s2 ;
            // find length
            // if end of string, will break immediately ('\0')
            while ( _isNumber ( *e1 ) )
            {
               ++e1 ;
            }
            while ( _isNumber ( *e2 ) )
            {
               ++e2 ;
            }

            len1 = (INT32)( e1 - s1 ) ;
            len2 = (INT32)( e2 - s2 ) ;

            // if one is longer than the other, return
            if ( len1 > len2 )
            {
               return 1 ;
            }
            else if ( len2 > len1 )
            {
               return -1 ;
            }
            // if the lengths are equal, just strcmp
            else if ( ( result = ossStrncmp ( s1, s2, len1 ) ) != 0 )
            {
               return result ;
            }
            // otherwise, the numbers are equal
            s1 = e1 ;
            s2 = e2 ;
            continue ;
         }

         if ( n1 )
         {
            return 1 ;
         }

         if ( n2 )
         {
            return -1 ;
         }

         if ( *s1 > *s2 )
         {
            return 1 ;
         }

         if ( *s2 > *s1 )
         {
            return -1 ;
         }

         ++s1 ;
         ++s2 ;
      }

      if ( *s1 )
      {
         return 1 ;
      }
      if ( *s2 )
      {
         return -1 ;
      }
      return 0 ;
   }

   FieldCompareResult _compareFieldNames1::compField ( const CHAR* l,
                                                       const CHAR* r )
   {
      static INT32 maxLoops = 1024 * 1024;
      UINT32 lstart = 0;
      UINT32 rstart = 0;

      UINT32 lsize = strlen ( l ) ;
      UINT32 rsize = strlen ( r ) ;
      for ( INT32 i = 0; i < maxLoops; i++ )
      {
         if ( lstart >= lsize )
         {
            if ( rstart >= rsize )
            {
               return SAME;
            }
            return RIGHT_SUBFIELD;
         }
         if ( rstart >= rsize )
         {
            return LEFT_SUBFIELD;
         }

         // find the earliest '.' from current position
         CHAR *a = (CHAR *)ossStrchr ( &l[lstart], '.' ) ;
         CHAR *b = (CHAR *)ossStrchr ( &r[rstart], '.' ) ;
         // locate the ., or end of the string
         CHAR *lend = ( NULL == a ) ? ( (CHAR *)&l[lsize] ) : a ;
         CHAR *rend = ( NULL == b ) ? ( (CHAR *)&r[rsize] ) : b ;

         // get the original left and right
         CHAR lold = *lend ;
         CHAR rold = *rend ;
         // set as end of string
         *lend     = '\0' ;
         *rend     = '\0' ;
         // do string compare
         INT32 x = _lexNumCmp ( &l[lstart], &r[rstart] ) ;
         // restore old value
         *lend     = lold ;
         *rend     = rold ;
         if ( x < 0 )
         {
             return LEFT_BEFORE;
         }
         if ( x > 0 )
         {
             return RIGHT_BEFORE;
         }

         lstart = UINT32(lend - l) + 1;
         rstart = UINT32(rend - r) + 1;
      }

      return SAME ;
   }

   void _mthModifier::modifierSort()
   {
      std::sort( _modifierElements.begin(),
                 _modifierElements.end(),
                 _compareFieldNames2( _dollarList ) ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__MTHMDF_LDPTN, "_mthModifier::loadPattern" )
   INT32 _mthModifier::loadPattern ( const BSONObj &modifierPattern,
                                     vector<INT64> *dollarList )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__MTHMDF_LDPTN );
      _modifierPattern = modifierPattern.copy() ;
      BSONObjIterator i(_modifierPattern) ;
      INT32 eleNum = 0 ;
      _dollarList = dollarList ;
      while ( i.more() )
      {
         rc = _parseElement(i.next() ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to parse match pattern %d, rc: %d",
                     eleNum, rc ) ;
            goto error ;
         }
         eleNum ++ ;
      }
      modifierSort() ;
      _initialized = TRUE ;
   done :
      PD_TRACE_EXITRC ( SDB__MTHMDF_LDPTN, rc );
      return rc ;
   error :
      goto done ;
   }

   BOOLEAN _mthModifier::_dupFieldName ( const BSONElement &l,
                                         const BSONElement &r )
   {
      return !l.eoo() && !r.eoo() && (r.rawdata() != r.rawdata()) &&
        ossStrncmp(l.fieldName(),r.fieldName(),ossStrlen(r.fieldName()))==0 ;
   }

   // when requested update want to change something that not exist in original
   // object, we need to append the original object in those cases
   // PD_TRACE_DECLARE_FUNCTION ( SDB__MTHMDF__APPNEW, "_mthModifier::_appendNew" )
   template<class Builder>
   INT32 _mthModifier::_appendNew ( Builder& b, SINT32 *modifierIndex,
                                    CHAR **ppRoot )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__MTHMDF__APPNEW );
      ModifierElement *me = &_modifierElements[(*modifierIndex)] ;
      CHAR dollarValue[ MTH_DOLLAR_FIELD_SIZE ] ;
      _compareFieldNames1 comp( _dollarList ) ;

      UINT32 rootLen = ossStrlen ( *ppRoot ) ;
      if ( rootLen > 0 )
      {
         (*ppRoot)[rootLen-1] = '\0' ;
      }

      switch ( me->_modType )
      {
      case INC:
      case SET:
      {
         ADD_CHG_UNSET_FIELD ( _srcChgBuilder, (*ppRoot) ) ;
         ADD_CHG_ELEMENT_AS ( _dstChgBuilder, me->_toModify,
                              (*ppRoot), "$set" ) ;
         b.appendAs ( me->_toModify,
                      comp.getDollarValue( me->_shortName, &dollarValue[0] ) ) ;
         break ;
      }
      // this codepath should never been hit
      case UNSET:
      case PULL:
      case PULL_ALL:
      case POP:
      case RENAME:
      {
         PD_LOG_MSG ( PDERROR, "Unexpected codepath" ) ;
         rc = SDB_SYS ;
         goto done ;
      }
      // need to do something, but not implemented yet
      case PUSH:
      {
         // create bson builder for the array
         BSONObjBuilder bb ( b.subarrayStart(me->_shortName)) ;
         bb.appendAs ( me->_toModify, bb.numStr(0) ) ;
         BSONObj newObj = bb.done() ;

         ADD_CHG_ARRAY_OBJ ( _dstChgBuilder, newObj, (*ppRoot),
                             "$set" ) ;
         ADD_CHG_UNSET_FIELD ( _srcChgBuilder, (*ppRoot) ) ;
         break ;
      }
      case PUSH_ALL:
      {
         // make sure the new type is array too
         if ( me->_toModify.type() != Array )
         {
            PD_LOG_MSG ( PDERROR, "pushed data type is not array: %s",
                         me->_toModify.toString().c_str());
            rc = SDB_INVALIDARG ;
            goto done ;
         }

         b.appendAs ( me->_toModify,
                      comp.getDollarValue( me->_shortName, &dollarValue[0] ) ) ;

         ADD_CHG_UNSET_FIELD ( _srcChgBuilder, (*ppRoot) ) ;
         ADD_CHG_ELEMENT_AS ( _dstChgBuilder, me->_toModify,
                              (*ppRoot), "$set" ) ;
         break ;
      }
      case ADDTOSET:
      {
         // make sure added value is array
         if ( Array != me->_toModify.type() )
         {
           PD_LOG_MSG ( PDERROR, "added data type is not array: %s",
                        me->_toModify.toString().c_str()) ;
           rc = SDB_INVALIDARG ;
           goto done ;
         }
         BSONObjBuilder bb (b.subarrayStart(me->_shortName) ) ;
         BSONObjIterator j ( me->_toModify.embeddedObject() ) ;
         INT32 n = 0 ;
         // make bsonelementset for everything we want to add
         BSONElementSet eleset ;
         // insert into set to deduplicate
         while ( j.more() )
         {
            eleset.insert( j.next() ) ;
         }
         BSONElementSet::iterator it ;
         for ( it = eleset.begin(); it != eleset.end(); it++ )
         {
            bb.appendAs((*it), bb.numStr(n++)) ;
         }
         BSONObj newObj = bb.done() ;

         //add new element
         if ( n != 0 )
         {
            ADD_CHG_ARRAY_OBJ ( _dstChgBuilder, newObj, (*ppRoot),
                                "$set" ) ;
            ADD_CHG_UNSET_FIELD ( _srcChgBuilder, (*ppRoot) ) ;
         }
         break ;
      }
      case BITXOR:
      case BITNOT:
      case BITAND:
      case BITOR:
         rc = _appendBitModifier ( b, 0, *me, ppRoot ) ;
         break ;
      case BIT:
         rc = _appendBitModifier2 ( b, 0, *me, ppRoot ) ;
         break ;
      default:
         PD_LOG_MSG ( PDERROR, "unknow modifier type[%d]", me->_modType ) ;
         rc = SDB_INVALIDARG ;
         goto done ;
      }

      // here we actually consume modifier, then we add index
      if ( SDB_OK == rc )
      {
         (*modifierIndex)++ ;
      }

   done :
      PD_TRACE_EXITRC ( SDB__MTHMDF__APPNEW, rc );
      return rc ;
   }

   // Builder could be BSONObjBuilder or BSONArrayBuilder
   // _appendNewFromMods appends the current builder with the new field
   // root represent the current fieldName, me is the current modifier element
   // b is the builder, onedownseen represent the all subobjects have been
   // processed in the current object, and modifierIndex is the pointer for
   // current modifier
   // PD_TRACE_DECLARE_FUNCTION ( SDB__MTHMDF__APPNEWFRMMODS, "_mthModifier::_appendNewFromMods" )
   template<class Builder>
   INT32 _mthModifier::_appendNewFromMods ( CHAR **ppRoot,
                                            INT32 &rootBufLen,
                                            Builder &b,
                                            set<string>& onedownseen,
                                            SINT32 *modifierIndex )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__MTHMDF__APPNEWFRMMODS );
      ModifierElement *me = &_modifierElements[(*modifierIndex)] ;
      INT32 rootLen = ossStrlen ( *ppRoot ) ;
      _compareFieldNames1 comp ( _dollarList ) ;


      // if the modified request does not exist in original one
      // first let's see if there's nested object in the request
      // ex. current root is user.name
      // however request is user.name.first.origin
      // in this case we'll have to create sub object 'first'

      // note fieldName is the FULL path "user.name.first.origin"
      // root is user.name.
      const CHAR *fieldName = me->_toModify.fieldName() ;

      // now temp is "first.origin"
      const CHAR *temp = fieldName+rootLen ;

      // find the "." starting from root length
      const CHAR *dot = ossStrchr ( temp,'.') ;

      rc = comp.checkDollarValue ( me->_toModify.fieldName() ) ;
      if ( rc )
      {
         rc = SDB_OK ;
         (*modifierIndex)++ ;
         goto done ;
      }

      if ( UNSET == me->_modType ||
           PULL == me->_modType ||
           PULL_ALL == me->_modType ||
           POP == me->_modType ||
           RENAME == me->_modType )
      {
         // we don't continue for those types since they are not going to append
         // new records
         (*modifierIndex)++ ;
         goto done ;
      }
      // given example
      // user.name.first.origin
      // |         ^    #
      // | represent fieldName
      // ^ represent temp
      // # represent dot
      // if there is sub object
      if ( dot )
      {
         // nr = "user.name.first."
         // const string nr ( fieldName, 0, 1+(dot-fieldName)) ;
         // nf = first
         const string nf ( temp, 0, dot-temp ) ;

         // we already added, let's return SDB_OK, this may happen when user
         // provided two fields with same name, it's a duplicate and we should
         // ignore
         if ( onedownseen.count(nf))
            goto done ;

         // make sure we mark this one has been processed
         onedownseen.insert(nf);
         {
            // create object builder for nf ("first" field)
            BSONObjBuilder bb ( b.subobjStart(nf)) ;
            // create a es for empty object
            const BSONObj obj ;
            BSONObjIteratorSorted es(obj);
            // preallocate 128 bytes
            CHAR tempBuffer [ MTH_TEMPSTRBUFLEN ] ;
            CHAR *pOldRoot = &tempBuffer[0] ;

            // if root len is greater than stack, we have to malloc, otherwise
            // use preallocated memory
            if ( rootLen >= MTH_TEMPSTRBUFLEN )
               pOldRoot = ossStrdup ( *ppRoot ) ;
            else
               ossStrcpy ( pOldRoot, *ppRoot ) ;
            if ( !pOldRoot )
            {
               PD_LOG ( PDERROR, "Failed to duplicate root string" ) ;
               rc = SDB_OOM ;
               goto error ;
            }

            // copy nr to root, first we set root to empty string, then "append"
            // nr to it, that means we replace root with nr.
            // note we already called ossStrdup to copy root to pOldRoot, so we
            // should be safe here
            (*ppRoot)[0] = '\0' ;
            rc = mthAppendString ( ppRoot, rootBufLen,
                                   0, fieldName,
                                   1+(dot-fieldName) ) ;
            // it may possible nr is larger than root so we need to realloc
            // memory
            if ( rc )
            {
               // revert root to original value
               // strcpy is safe here since the memory was for root
               ossStrcpy ( *ppRoot, pOldRoot ) ;
               if ( pOldRoot != &tempBuffer[0] )
                  SDB_OSS_FREE ( pOldRoot ) ;
               PD_LOG ( PDERROR, "Failed to append string, rc = %d", rc ) ;
               goto error ;
            }

            // create an object for path "user.name.first."
            // bb is the new builder, es is iterator
            // modifierIndex is the index
            rc = _buildNewObj ( ppRoot, rootBufLen,
                                bb, es, modifierIndex ) ;
            // strcpy is safe here because the memory should not be less than
            // original
            ossStrcpy ( *ppRoot, pOldRoot ) ;
            if ( pOldRoot != &tempBuffer[0] )
            {
               SDB_OSS_FREE ( pOldRoot ) ;
            }
            if ( rc )
            {
               PD_LOG ( PDERROR, "Failed to build new object for %s, rc: %d",
                        me->_toModify.toString().c_str(), rc ) ;
               goto error ;
            }
            bb.done() ;
         }
      }
      // if we can't find ".", then we are not embedded BSON, let's just
      // create whatever object we asked
      // for example current root is "user.name."
      // and we want {$set: {user.name.firstname, "tao wang"}}
      // here temp will be firstname, and dot will be NULL
      else
      {
         // call _appendNew to append modified element into the current builder
         try
         {
            rc = _appendNew ( b, modifierIndex, ppRoot ) ;
         }
         catch( std::exception &e )
         {
            PD_LOG ( PDERROR, "Failed to append for %s: %s",
                     me->_toModify.toString().c_str(), e.what() );
            rc = SDB_INVALIDARG ;
            goto error ;
         }

         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to append for %s, rc: %d",
                     me->_toModify.toString().c_str(), rc ) ;
            goto error ;
         }
      }

   done :
      PD_TRACE_EXITRC ( SDB__MTHMDF__APPNEWFRMMODS, rc );
      return rc ;
   error :
      goto done ;
   }
   // if the original object has the element we asked to modify, then e is the
   // original element, b is the builder, me is the info that we want to modify
   // PD_TRACE_DECLARE_FUNCTION ( SDB__MTHMDF__ALYCHG, "_mthModifier::_applyChange" )
   template<class Builder>
   INT32 _mthModifier::_applyChange ( CHAR **ppRoot,
                                      INT32 &rootBufLen,
                                      BSONElement &e,
                                      Builder &b,
                                      SINT32 *modifierIndex )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__MTHMDF__ALYCHG );
      ModifierElement *me = &_modifierElements[(*modifierIndex)] ;

      UINT32 rootLen = ossStrlen ( *ppRoot ) ;
      if ( rootLen > 0 )
      {
         (*ppRoot)[rootLen-1] = '\0' ;
      }

      // basically we need to take the original data from e, and use modifier
      // element me to make some change, and add into builder b
      switch ( me->_modType )
      {
      case INC:
         // for INC, we need to call _applyIncModifier
         rc = _applyIncModifier ( b, e, *me, ppRoot ) ;
         break ;
      case SET:
         rc = _applySetModifier ( b, e, *me, ppRoot ) ;
         break ;
      case UNSET:
      {
         ADD_CHG_ELEMENT_AS ( _srcChgBuilder, e, (*ppRoot),
                              "$set" ) ;
         ADD_CHG_ELEMENT_AS ( _dstChgBuilder, me->_toModify, (*ppRoot),
                              "$unset" ) ;

         _applyUnsetModifier(b) ;
         break ;
      }
      case PUSH:
         rc = _applyPushModifier ( b, e, *me, ppRoot ) ;
         break ;
      case PUSH_ALL:
         rc = _applyPushAllModifier ( b, e, *me, ppRoot ) ;
         break ;
      // given an input, remove all matching items when they match any of the
      // input
      case PULL:
      // given an input, remove all matching items when they match the whole
      // input
      case PULL_ALL:
         rc = _applyPullModifier ( b, e, *me, ppRoot ) ;
         break ;
      case POP:
         rc = _applyPopModifier ( b, e, *me, ppRoot ) ;
         break ;
      case BITNOT:
      case BITXOR:
      case BITAND:
      case BITOR:
         rc = _applyBitModifier ( b, e, *me, ppRoot ) ;
         break ;
      case BIT:
         rc = _applyBitModifier2 ( b, e, *me, ppRoot ) ;
         break ;
      case ADDTOSET:
         rc = _applyAddtoSetModifier ( b, e, *me, ppRoot ) ;
         break ;
      case RENAME:
      {
         INT32 rootLen = ossStrlen ( *ppRoot ) ;
         rc = mthAppendString ( ppRoot, rootBufLen,
                                rootLen, me->_toModify.valuestr(),
                                0 ) ;
         ADD_CHG_ELEMENT_AS ( _srcChgBuilder, e, (*ppRoot),
                              "$set" ) ;
         ADD_CHG_UNSET_FIELD ( _srcChgBuilder,
                               string( (*ppRoot )) ) ;
         //for the new obj,should unset the old, and set the new
         ADD_CHG_UNSET_FIELD ( _dstChgBuilder, (*ppRoot) ) ;
         ADD_CHG_ELEMENT_AS ( _dstChgBuilder, e,
                              string ((*ppRoot )),
                              "$set" ) ;
         (*ppRoot)[rootLen] = '\0' ;
         b.appendAs ( e, me->_toModify.valuestr() ) ;
         break ;
      }
      default :
         PD_LOG_MSG ( PDERROR, "unknow modifier type[%d]", me->_modType ) ;
         rc = SDB_INVALIDARG ;
         goto done ;
      }
      if ( SDB_OK == rc )
      {
         (*modifierIndex)++ ;
      }
   done :
      PD_TRACE_EXITRC ( SDB__MTHMDF__ALYCHG, rc );
      return rc ;
   }

   // Builder could be BSONObjBuilder or BSONArrayBuilder
   // This function is recursively called to build new object
   // The prerequisit is that _modifierElement is sorted, which supposed to
   // happen at end of loadPattern
   // PD_TRACE_DECLARE_FUNCTION ( SDB__MTHMDF__BLDNEWOBJ, "_mthModifier::_buildNewObj" )
   template<class Builder>
   INT32 _mthModifier::_buildNewObj ( CHAR **ppRoot,
                                      INT32 &rootBufLen,
                                      Builder &b,
                                      BSONObjIteratorSorted &es,
                                      SINT32 *modifierIndex )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__MTHMDF__BLDNEWOBJ );
      _compareFieldNames1 compare ( _dollarList ) ;
      // get the next element in the object
      BSONElement e = es.next() ;
      // previous element is set to empty
      BSONElement prevE ;
      // current root length
      UINT32 rootLen = ossStrlen ( *ppRoot ) ;

      // in the current scope, which subobject have we seen?
      set<string> onedownseen ;

      // loop until we hit end of original object, or end of modifier list
      while( !e.eoo() && (*modifierIndex)<(SINT32)_modifierElements.size() )
      {
         // if we get two elements with same field name, we don't need to
         // continue checking, simply append it to the builder
         /* TODO: should we really do that? */
         if ( _dupFieldName(prevE, e))
         {
            b.append(e) ;
            prevE = e ;
            e = es.next() ;
            continue ;
         }
         /*if ( *modifierIndex != 0 && ossStrcmp(
            _modifierElements[*modifierIndex-1]._toModify.fieldName(),
            _modifierElements[*modifierIndex]._toModify.fieldName() ) == 0 )*/
         if ( *modifierIndex != 0 && SAME == compare.compField (
              _modifierElements[*modifierIndex-1]._toModify.fieldName(),
              _modifierElements[*modifierIndex]._toModify.fieldName() ) )
         {
            (*modifierIndex)++ ;
            continue ;
         }

         prevE = e ;

         // every time we build the current field, let's set root to original
         (*ppRoot)[rootLen] = '\0' ;

         // construct the full path of the current field name
         // say current root is "user.employee.", and this object contains
         // "name, age" fields, then first loop we get user.employee.name
         // second round get user.employee.age
         rc = mthAppendString ( ppRoot, rootBufLen,
                                rootLen, e.fieldName(),
                                0 ) ;
         PD_RC_CHECK ( rc, PDERROR, "Failed to append string, rc = %d", rc ) ;

         // compare the full field name with requested update field
         /*FieldCompareResult cmp = compareDottedFieldNames (
               _modifierElements[(*modifierIndex)]._toModify.fieldName(),
               *ppRoot ) ;*/
         FieldCompareResult cmp = compare.compField (
               _modifierElements[(*modifierIndex)]._toModify.fieldName(),
               *ppRoot ) ;
         // add "." at end
         rc = mthAppendString ( ppRoot, rootBufLen,
                                0, ".",
                                1 ) ;
         PD_RC_CHECK ( rc, PDERROR, "Failed to append string, rc = %d", rc ) ;

         // compare the full path
         // we have few situations need to handle
         // 1) current field is a parent of requested field
         // for example, currentfield = user, requested field = user.name.test
         // this situation called LEFT_SUBFIELD

         // 2) current field is same as requested field
         // for example both current field and requests are user.name.test
         // this situation called SAME

         // 3) current field is not same as requested field, and alphabatically
         // current field is greater than requested field
         // for example current field is user.myname, requested fialed is
         // user.abc
         // this situation called LEFT_BEFORE

         // 4) current field is not same as requested field, and alphabatically
         // current field is smaller than requested field
         // for example current field is user.myname, requested field is
         // user.name
         // this situation called RIGHT_BEFORE

         // 5) requested field is a parent of current field
         // for example current field is user.name.test, requested field is user
         // howwever since we are doing merge, this situation should NEVER
         // HAPPEN!!
         switch ( cmp )
         {
         case LEFT_SUBFIELD:
            // ex, modify request $set:{user.name,"taoewang"}
            // field: user
            // make sure the BSONElement is object or array
            // if the requested field already exist but it's not object nor
            // array, we should report error since we can't create sub field in
            // other type of element
            if ( e.type() != Object && e.type() != Array )
            {
               PD_LOG ( PDERROR,
                        "Invalid field type: %s", e.toString().c_str());
               rc = SDB_INVALIDARG ;
               goto error ;
            }
            // have we processed this field before?
            // if we haven't processed it before, let's do it
            // otherwise we may have a dup somewhere so let's ignore and report
            // warning
            if ( onedownseen.count(e.fieldName())==0 )
            {
               // insert into list
               onedownseen.insert(e.fieldName()) ;
               // if we are dealing with object, then let's create a new object
               // builder starting from our current fieldName
               if ( e.type() == Object )
               {
                  BSONObjBuilder bb(b.subobjStart(e.fieldName()));
                  // get the object for the current element, and create sorted
                  // iterator on it
                  BSONObjIteratorSorted bis(e.Obj());

                  // add fieldname into path and recursively call _buildNewObj
                  // to create embedded object
                  // root is original root + current field + .
                  // bb is new object builder
                  // bis is the sorted iterator
                  // modifierIndex is the current modifier we are working on
                  rc = _buildNewObj ( ppRoot, rootBufLen, bb, bis,
                                      modifierIndex ) ;
                  if ( rc )
                  {
                     PD_LOG ( PDERROR, "Failed to build object: %s, rc: %d",
                              e.toString().c_str(), rc ) ;
                     rc = SDB_INVALIDARG ;
                     goto error ;
                  }
                  // call bb.done() to close the builder
                  bb.done() ;
               }
               else
               {
                  // if it's not object, then we must have array
                  // now let's create BSONArrayBuilder
                  BSONArrayBuilder ba(b.subarrayStart(e.fieldName()));
                  //BSONArrayIteratorSorted bis(BSONArray(e.embeddedObject()));
                  BSONObjIteratorSorted bis(e.embeddedObject());
                  // add fieldname into path and recursively call _buildNewObj
                  // to create embedded object
                  // root is original root + current field + .
                  // ba is new array builder
                  // bis is the sorted iterator
                  // modifierIndex is the current modifier we are working on
                  rc = _buildNewObj ( ppRoot, rootBufLen, ba, bis,
                                      modifierIndex ) ;
                  if ( rc )
                  {
                     PD_LOG ( PDERROR,
                              "Failed to build array: %s",
                              e.toString().c_str());
                     rc = SDB_INVALIDARG ;
                     goto error ;
                  }
                  ba.done();
               }
               // process to the next element
               e = es.next() ;
               // note we shouldn't touch modifierIndex here, we should only
               // change it at the place actually consuming it
            }
            else
            {
               PD_LOG ( PDWARNING, "dup detected for %s", e.fieldName() );
            }
            break ;
         case LEFT_BEFORE:
            // if the modified request does not exist in original one
            // first let's see if there's nested object in the request
            // ex. current root is user. and our first element is "name"
            // however request is user.address
            // in this case we'll have to create sub object 'address' first

            // _appendNewFromMods appends the current builder with the new field
            // _modifierElement[modifierIndex] represents the current
            // ModifyElement, b is the builder, root is the string of current
            // root field, onedownseen is the set for all subobjects

            // first let's revert root to original
            (*ppRoot)[rootLen] = '\0' ;
            rc = _appendNewFromMods ( ppRoot, rootBufLen,
                                      b, onedownseen, modifierIndex) ;
            PD_RC_CHECK ( rc, PDERROR,
                          "Failed to append for %s",
                          _modifierElements[(*modifierIndex)
                                           ]._toModify.toString().c_str());
            // note we don't change e here because we just add the field
            // requested by modifier into new object, the original e shoudln't
            // be changed.

            // we also don't change modifierIndex here since it should be
            // changed by the actual consumer function, not in this loop
            break ;
         case SAME:
            // in this situation, the requested field is the one we are
            // processing, so that we don't need to change object metadata,
            // let's just apply the change
            // e is the current element, b is the current builder, modifierIndex
            // is the current modifier
            try
            {
               rc = _applyChange ( ppRoot, rootBufLen, e, b, modifierIndex);
            }
            catch( std::exception &e )
            {
               PD_LOG ( PDERROR,
                        "Failed to apply changes for %s: %s",
                        _modifierElements[(*modifierIndex)
                                         ]._toModify.toString().c_str(),
                        e.what() );
               rc = SDB_INVALIDARG ;
               goto error ;
            }
            if ( rc )
            {
               PD_LOG ( PDERROR,
                        "Failed to apply change for %s",
                        _modifierElements[(*modifierIndex)
                                         ]._toModify.toString().c_str());
               goto error ;
            }
            // since we have processed the original data, we increase element
            e=es.next();
            // again, don't change modifierIndex in loop
            break ;
         case RIGHT_BEFORE:
            // in this situation, the original field is alphabetically ahead of
            // requested field.
            // for example current field is user.name but requested field is
            // user.plan, then we simply add the field into new object
            // original object doesn't need to change

            // In the situation we are processing different object, for example
            // requested update field is user.newfield.test
            // current processing e is mydata.test
            // in this case, we still keep appending mydata.test until hitting
            // end of the object and return, without touching user.newfield.test
            // so we should be safe here
            b.append(e) ;
            // and increase element
            e=es.next() ;
            break ;
         case RIGHT_SUBFIELD:
         default :
            //we should never reach this codepath
            PD_LOG ( PDERROR, "Reaching unexpected codepath, cmp( %s, %s, "
                     "res: %d )", _modifierElements[(*modifierIndex)
                     ]._toModify.toString().c_str(),
                     *ppRoot, cmp ) ;
            rc = SDB_SYS ;
            goto error ;
         }
      }
      // we break out the loop either hitting end of original object, or end of
      // the modifier list

      // if there's still any leftover in original object, let's append them
      while ( !e.eoo() )
      {
         b.append(e) ;
         e=es.next() ;
      }

      while ( (*modifierIndex)<(SINT32)_modifierElements.size() )
      {
         (*ppRoot)[rootLen] = '\0' ;
         /*if ( *modifierIndex != 0 && ossStrcmp(
            _modifierElements[*modifierIndex-1]._toModify.fieldName(),
            _modifierElements[*modifierIndex]._toModify.fieldName() ) == 0 )*/
         if ( *modifierIndex != 0 && SAME == compare.compField (
              _modifierElements[*modifierIndex-1]._toModify.fieldName(),
              _modifierElements[*modifierIndex]._toModify.fieldName() ) )
         {
            (*modifierIndex)++ ;
            continue ;
         }

         // compare the full field name with requested update field
         /*FieldCompareResult cmp = compareDottedFieldNames (
               _modifierElements[(*modifierIndex)]._toModify.fieldName(),
               *ppRoot ) ;*/
         FieldCompareResult cmp = compare.compField (
               _modifierElements[(*modifierIndex)]._toModify.fieldName(),
               *ppRoot ) ;
         if ( LEFT_SUBFIELD == cmp )
         {
            rc = _appendNewFromMods ( ppRoot, rootBufLen,
                                      b, onedownseen, modifierIndex ) ;
            if ( rc )
            {
               PD_LOG ( PDERROR,
                        "Failed to append for %s",
                        _modifierElements[(*modifierIndex)
                                         ]._toModify.toString().c_str());
               goto error ;
            }
         }
         else
            goto done ;
      }
   done :
      PD_TRACE_EXITRC ( SDB__MTHMDF__BLDNEWOBJ, rc );
      return rc ;
   error :
      goto done ;
   }
   // given a source BSON object and empty target, the returned target will
   // contains modified data

   // since we are dealing with tons of BSON object conversion, this part should
   // ALWAYS protected by try{} catch{}
   // PD_TRACE_DECLARE_FUNCTION ( SDB__MTHMDF_MODIFY, "_mthModifier::modify" )
   INT32 _mthModifier::modify ( const BSONObj &source, BSONObj &target,
                                BSONObj *srcID, BSONObj *srcChange,
                                BSONObj *dstID, BSONObj *dstChange )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__MTHMDF_MODIFY );
      SDB_ASSERT(_initialized, "The modifier has not been initialized, please "
                 "call 'loadPattern' before using it" )
      SDB_ASSERT(target.isEmpty(), "target should be empty")

      modifierSort() ;

      // create a builder with 10% extra space for buffer
      BSONObjBuilder builder ( (int)(source.objsize()*1.1));
      // create sorted iterator
      BSONObjIteratorSorted es(source) ;

      // index for modifier, should be less than _modifierElements.size()
      // say if we have
      // {$inc: {employee.salary, 100}, $set: {employee.status, "promoted"}},
      // then we have 2 modifier ($inc and $set), so modifierIndex start from 0
      // and should end at 1
      SINT32 modifierIndex = 0 ;

      CHAR *pBuffer = (CHAR*)SDB_OSS_MALLOC ( SDB_PAGE_SIZE ) ;
      INT32 bufferSize = SDB_PAGE_SIZE ;
      if ( !pBuffer )
      {
         PD_LOG ( PDERROR, "Failed to allocate buffer for select" ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      pBuffer[0] = '\0' ;

      if ( srcChange )
      {
         _srcChgBuilder = SDB_OSS_NEW BSONObjBuilder ( (int)(source.objsize()*0.3) ) ;
         if ( !_srcChgBuilder )
         {
            rc = SDB_OOM ;
            PD_LOG_MSG ( PDERROR, "Failed to alloc memory for src BSONObjBuilder" ) ;
            goto error ;
         }
      }
      if ( dstChange )
      {
         _dstChgBuilder = SDB_OSS_NEW BSONObjBuilder ( (int)(source.objsize()*0.3) ) ;
         if ( !_dstChgBuilder )
         {
            rc = SDB_OOM ;
            PD_LOG_MSG ( PDERROR, "Failed to alloc memory for dst BSONObjBuilder" ) ;
            goto error ;
         }
      }

      // create a new object based on the source
      // "" is empty root, builder is BSONObjBuilder
      // es is our iterator, and modifierIndex is the current modifier we are
      // going to apply
      // when this call returns SDB_OK, we should call builder.obj() to create
      // BSONObject from the builder.
      rc = _buildNewObj ( &pBuffer,
                          bufferSize,
                          builder,
                          es,
                          &modifierIndex ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to modify target, rc: %d", rc ) ;
         goto error ;
      }
      // now target owns the builder buffer, since obj() will decouple() the
      // buffer from builder, and assign holder to the new BSONObj
      target=builder.obj();

      if ( srcID )
      {
         BSONObjBuilder srcIDBuilder ;
         BSONElement id = source.getField ( DMS_ID_KEY_NAME ) ;
         if ( id.eoo() )
         {
            PD_LOG_MSG ( PDERROR, "Failed to get source oid, %s",
                         source.toString().c_str() ) ;
            rc = SDB_SYS ;
            goto error ;
         }
         srcIDBuilder.append ( id ) ;
         *srcID = srcIDBuilder.obj() ;
      }
      if ( srcChange )
      {
         *srcChange = _srcChgBuilder->obj () ;
      }
      if ( dstID )
      {
         BSONObjBuilder dstIDBuilder ;
         BSONElement id = target.getField ( DMS_ID_KEY_NAME ) ;
         if ( id.eoo() )
         {
            PD_LOG_MSG ( PDERROR, "Failed to get target oid, %s",
                         target.toString().c_str() ) ;
            rc = SDB_SYS ;
            goto error ;
         }
         dstIDBuilder.append ( id ) ;
         *dstID = dstIDBuilder.obj() ;
      }
      if ( dstChange )
      {
         *dstChange = _dstChgBuilder->obj () ;
      }
   done :
      if ( pBuffer )
      {
         SDB_OSS_FREE ( pBuffer ) ;
      }
      if ( _srcChgBuilder )
      {
         SDB_OSS_DEL _srcChgBuilder ;
         _srcChgBuilder = NULL ;
      }
      if ( _dstChgBuilder )
      {
         SDB_OSS_DEL _dstChgBuilder ;
         _dstChgBuilder = NULL ;
      }
      if ( srcChange && dstChange )
      {
         PD_LOG_MSG ( PDEVENT, "source bson, %s,\n dest bson, %s",
                      srcChange->toString(false,false).c_str(),
                      dstChange->toString(false,false).c_str() ) ;
      }
      PD_TRACE_EXITRC ( SDB__MTHMDF_MODIFY, rc );
      return rc ;
   error :
      goto done ;
   }
}
