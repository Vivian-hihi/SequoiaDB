/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = mthModifier.hpp

   Descriptive Name = Method Modifier Header

   When/how to use: this program may be used on binary and text-formatted
   versions of Method component. This file contains structure for modify
   operation, which is changing a data record based on modification rule.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef MTHMODIFIER_HPP_
#define MTHMODIFIER_HPP_
#include "core.hpp"
#include "oss.hpp"
#include <vector>
#include "ossUtil.hpp"
#include "../bson/bson.h"
#include "../bson/bsonobj.h"
using namespace bson ;

#define MTH_OID_FIELD_NAME          "OID"
#define MTH_MODIFIER_FIELD_NAME     "modifier"

namespace engine
{
   enum ModType
   {
      INC = 0,
      SET,
      PUSH,
      PUSH_ALL,
      PULL,
      PULL_ALL,
      POP,
      UNSET,
      BITNOT,
      BITXOR,
      BITAND,
      BITOR,
      BIT,
      ADDTOSET,
      RENAME,

      UNKNOW
   } ;
   class _ModifierElement : public SDBObject
   {
   public :
      BSONElement _toModify ; // the element to modify
      ModType     _modType ;
      CHAR       *_shortName ;
      _ModifierElement ( const BSONElement &e, ModType type )
      {
         const CHAR *fn = e.fieldName() ;
         _toModify = e ;
         _modType = type ;
         _shortName = (CHAR*)ossStrrchr( fn , '.' );
         if ( _shortName )
             _shortName++ ;
         else
             _shortName = (CHAR*)_toModify.fieldName() ;
      }
   } ;
   typedef _ModifierElement ModifierElement ;

   class _compareFieldNames1
   {
   private:
      vector<INT64> *_dollarList ;
   private:
      /*CHAR *_dollarNum2Field ( const CHAR *fieldName ) ;*/
      inline BOOLEAN _isNumber( CHAR c ) ;
      inline INT32 _lexNumCmp ( const CHAR *s1,
                                const CHAR *s2 ) ;
   public:
      _compareFieldNames1( vector<INT64> *dollarList = NULL ): _dollarList(NULL)
      {
         _dollarList = dollarList ;
      }
      CHAR *getDollarValue ( CHAR *s, CHAR *in ) ;
      INT32 checkDollarValue( const char* pField ) ;
      FieldCompareResult compField(const char* l, const char* r) ;
   } ;

   class _compareFieldNames2
   {
   private:
      vector<INT64> *_dollarList ;
   public:
      _compareFieldNames2( vector<INT64> *dollarList = NULL ): _dollarList(NULL)
      {
         _dollarList = dollarList ;
      }
      BOOLEAN operator () ( const ModifierElement &l,
                            const ModifierElement &r ) const
      {
         _compareFieldNames1 compare ( _dollarList ) ;
         FieldCompareResult result = compare.compField( l._toModify.fieldName(),
                                                     r._toModify.fieldName() ) ;
         return ((result == RIGHT_SUBFIELD) || (result == LEFT_BEFORE)) ;
      }
   } ;

   class _mthModifier : public SDBObject
   {
   private :
      BSONObjBuilder *_srcChgBuilder ;
      BSONObjBuilder *_dstChgBuilder ;

      BSONObj _modifierPattern ;
      BOOLEAN _initialized ;
      vector<ModifierElement> _modifierElements ;
      vector<INT64> *_dollarList ;
      INT32 _addModifier ( const BSONElement &ele, ModType type ) ;
      INT32 _parseElement ( const BSONElement &ele ) ;
      ModType _parseModType ( const CHAR *field ) ;

      template<class VType>
      INT32 _bitCalc ( ModType type, VType l, VType r, VType &out );

      BOOLEAN _dupFieldName ( const BSONElement &l,
                              const BSONElement &r ) ;
      BOOLEAN _pullElementMatch( BSONElement& org,
                                 BSONElement& toMatch ) ;
      template<class Builder>
      void _applyUnsetModifier(Builder &b) ;

      void _applyUnsetModifier(BSONArrayBuilder &b) ;

      template<class Builder>
      INT32 _applyIncModifier ( Builder &bb, const BSONElement &in,
                                ModifierElement &me ) ;
      template<class Builder>
      INT32 _applySetModifier ( Builder &bb, const BSONElement &in,
                                ModifierElement &me ) ;
      template<class Builder>
      INT32 _applyPushModifier ( Builder &bb, const BSONElement &in,
                                 ModifierElement &me ) ;
      template<class Builder>
      INT32 _applyPushAllModifier ( Builder &bb, const BSONElement &in,
                                    ModifierElement &me ) ;
      template<class Builder>
      INT32 _applyPullModifier ( Builder &bb, const BSONElement &in,
                                 ModifierElement &me ) ;
      template<class Builder>
      INT32 _applyPopModifier ( Builder &bb, const BSONElement &in,
                                ModifierElement &me ) ;
      template<class Builder>
      INT32 _applyBitModifier ( Builder &bb, const BSONElement &in,
                                ModifierElement &me ) ;
      template<class Builder>
      INT32 _applyBitModifier2 ( Builder &bb,
                                 const BSONElement &in,
                                 ModifierElement &me ) ;
      template<class Builder>
      INT32 _applyAddtoSetModifier ( Builder &bb,
                                    const BSONElement &in,
                                    ModifierElement &me ) ;
      template<class Builder>
      INT32 _appendBitModifier ( Builder &bb, INT32 in,
                                 ModifierElement &me ) ;
      template<class Builder>
      INT32 _appendBitModifier2 ( Builder &bb, INT32 in,
                                  ModifierElement &me ) ;
      // if the original object has the element we asked to modify, then e is
      // the
      // original element, b is the builder, me is the info that we want to
      // modify
      // basically we need to take the original data from e, and use modifier
      // element me to make some change, and add into builder b
      template<class Builder>
      INT32 _applyChange ( CHAR **ppRoot,
                           INT32 &rootBufLen,
                           BSONElement &e,
                           Builder &b,
                           SINT32 *modifierIndex ) ;

      // when requested update want to change something that not exist in
      // original
      // object, we need to append the original object in those cases
      template<class Builder>
      INT32 _appendNew ( Builder& b, SINT32 *modifierIndex ) ;

      // Builder could be BSONObjBuilder or BSONArrayBuilder
      // _appendNewFromMods appends the current builder with the new field
      // root represent the current fieldName, me is the current modifier element
      // b is the builder, onedownseen represent the all subobjects have been
      // processed in the current object, and modifierIndex is the pointer for
      // current modifier
      template<class Builder>
      INT32 _appendNewFromMods ( CHAR **ppRoot,
                                 INT32 &rootBufLen,
                                 Builder &b,
                                 set<string>& onedownseen,
                                 SINT32 *modifierIndex ) ;
      // Builder could be BSONObjBuilder or BSONArrayBuilder
      // This function is recursively called to build new object
      // The prerequisit is that _modifierElement is sorted, which supposed to
      // happen at end of loadPattern
      template<class Builder>
      INT32 _buildNewObj ( CHAR **ppRoot,
                           INT32 &rootBufLen,
                           Builder &b,
                           BSONObjIteratorSorted &es,
                           SINT32 *modifierIndex ) ;
   public :
      _mthModifier ()
      {
         _initialized   = FALSE ;
         _srcChgBuilder = NULL ;
         _dstChgBuilder = NULL ;
         _dollarList    = NULL ;
      }
      ~_mthModifier()
      {
         _modifierElements.clear() ;
      }
      INT32 loadPattern ( const BSONObj &modifierPattern,
                          vector<INT64> *dollarList = NULL ) ;
      void modifierSort() ;
      INT32 modify ( const BSONObj &source, BSONObj &target,
                     BSONObj *srcID = NULL,
                     BSONObj *srcChange = NULL,
                     BSONObj *dstID = NULL,
                     BSONObj *dstChange = NULL ) ;
      inline BOOLEAN isInitialized () { return _initialized ; }
   } ;
   typedef _mthModifier mthModifier ;
}

#endif
