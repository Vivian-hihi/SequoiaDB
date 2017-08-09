/*******************************************************************************


   Copyright (C) 2011-2017 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = seAdptContext.cpp

   Descriptive Name = Context on search engine adapter.

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbcm,
   which is used to do cluster managing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/14/2017  YSD  Initial Draft

   Last Changed =

*******************************************************************************/
#include "msgDef.hpp"
#include "seAdptContext.hpp"

using namespace bson ;

#define DMS_ID_KEY_NAME        "_id"
namespace engine
{
   _seAdptQueryRebuilder::_seAdptQueryRebuilder()
   {
   }

   _seAdptQueryRebuilder::~_seAdptQueryRebuilder()
   {
   }

   // Initialize the rebuilder with the original message. It will store separate
   // parts of the query.
   INT32 _seAdptQueryRebuilder::init( const BSONObj &matcher,
                                      const BSONObj &selector,
                                      const BSONObj &orderBy,
                                      const BSONObj &hint )
   {
      _query = matcher.copy() ;
      _selector = selector.copy() ;
      _orderBy = orderBy.copy() ;
      _hint = hint.copy() ;

      return SDB_OK ;
   }

   // Rebuild the query message. The vectors of the second and third arguments
   // contain the items to be modified to the original.
   INT32 _seAdptQueryRebuilder::rebuild( REBUILD_ITEM_MAP &rebuildItems,
                                         utilCommObjBuff &objBuff )
   {
      INT32 rc = SDB_OK ;
      const BSONObj *object ;
      REBUILD_ITEM_MAP_ITR itr ;

      if ( rebuildItems.find( SE_QUERY_REBLD_QUERY ) != rebuildItems.end() )
      {
         object = rebuildItems[SE_QUERY_REBLD_QUERY] ;
      }
      else
      {
         object = &_query ;
      }
      rc = objBuff.appendObj( object ) ;
      PD_RC_CHECK( rc, PDERROR, "Append query to object buffer failed[ %d ]",
                   rc ) ;

      if ( rebuildItems.find( SE_QUERY_REBLD_SEL ) != rebuildItems.end() )
      {
         object = rebuildItems[SE_QUERY_REBLD_SEL] ;
      }
      else
      {
         object = &_selector ;
      }
      rc = objBuff.appendObj( object ) ;
      PD_RC_CHECK( rc, PDERROR, "Append selector to object buffer failed[ %d ]",
                   rc ) ;

      if ( rebuildItems.find( SE_QUERY_REBLD_ORD ) != rebuildItems.end() )
      {
         object = rebuildItems[SE_QUERY_REBLD_ORD] ;
      }
      else
      {
         object = &_orderBy ;
      }
      rc = objBuff.appendObj( object ) ;
      PD_RC_CHECK( rc, PDERROR, "Append orderby to object buffer failed[ %d ]",
                   rc ) ;

      {
         // The original hint is used to pass the text index name. In the
         // replay, we give an empty hint.
         // Any problem ?
         BSONObj object ;
         rc = objBuff.appendObj( object ) ;
         PD_RC_CHECK( rc, PDERROR, "Append hint to object buffer failed[ %d ]",
                      rc ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   _seAdptContextBase::_seAdptContextBase( const string &indexName,
                                           const string &typeName,
                                           utilESClt *seClt )
   {
      _indexName = indexName ;
      _type = typeName ;
      _esClt = seClt ;
   }

   _seAdptContextBase::~_seAdptContextBase()
   {
   }

   INT32 _seAdptContextBase::_getQueryCond( const BSONObj &matcher,
                                            std::string &queryStr )
   {
      INT32 rc = SDB_OK ;

      BSONElement ele = matcher.firstElement() ;
      if ( Object == ele.type() )
      {
         BSONElement subEle = ele.Obj().firstElement() ;
         if ( 0 == ossStrcmp( FIELD_NAME_TEXT, subEle.fieldName() ) )
         {
            if ( 1 != matcher.nFields() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG( PDERROR, "Only one query condition should be specified "
                       "for text search, actually: %d", matcher.nFields() ) ;
               goto error ;
            }

            if ( String == subEle.type() )
            {
               queryStr = subEle.valuestr() ;
            }
            else if ( Object == subEle.type() )
            {
               queryStr = subEle.Obj().jsonString() ;
            }
            else
            {
               rc = SDB_INVALIDARG ;
               PD_LOG( PDERROR, "Query conditioin type[%d] for text "
                       "search is wrong", subEle.type() ) ;
               goto error ;
            }
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   _seAdptContextData::_seAdptContextData( const string &indexName,
                                           const string &typeName,
                                           utilESClt *seClt )
   : _seAdptContextBase( indexName, typeName, seClt )
   {
   }

   _seAdptContextData::~_seAdptContextData()
   {
   }

   INT32 _seAdptContextData::open( const BSONObj &matcher,
                                   const BSONObj &selector,
                                   const BSONObj &orderBy,
                                   const BSONObj &hint,
                                   utilCommObjBuff &objBuff,
                                   pmdEDUCB *eduCB )
   {
      return SDB_OK ;
   }

   INT32 _seAdptContextData::getMore( INT32 returnNum, utilCommObjBuff &objBuff )
   {
      return SDB_OK ;
   }

   _seAdptContextQuery::_seAdptContextQuery( const string &indexName,
                                             const string &typeName,
                                             utilESClt *seClt )
   : _seAdptContextBase( indexName, typeName, seClt )
   {
   }

   _seAdptContextQuery::~_seAdptContextQuery()
   {
   }

   INT32 _seAdptContextQuery::open( const BSONObj &matcher,
                                    const BSONObj &selector,
                                    const BSONObj &orderBy,
                                    const BSONObj &hint,
                                    utilCommObjBuff &objBuff,
                                    pmdEDUCB *eduCB )
   {
      INT32 rc = SDB_OK ;
      string queryCond ;
      utilCommObjBuff searchResult ;
      BSONObj inCond ;
      std::map< _seadptQueryRebldType, const BSONObj* > rebuildItems ;

      vector<_seadptQueryRebldType> rebuildType ;
      vector<BSONObj*> rebuildObjs ;

      objBuff.reset() ;

      rc = _getQueryCond( matcher, queryCond ) ;
      PD_RC_CHECK( rc, PDERROR, "Get query string from matcher failed[ %d ], "
                   "matcher: %s", rc, matcher.toString().c_str() ) ;
      rc = _queryRebuilder.init(  matcher, selector, orderBy, hint ) ;
      PD_RC_CHECK( rc, PDERROR, "Initialize query rebuilder failed[ %d ]",
                   rc ) ;

      // The search result should only contain _id objects.
      if ( !_esClt->isActive() )
      {
         rc = _esClt->active() ;
         PD_RC_CHECK( rc, PDERROR, "Reactive ES client failed[ %d ]", rc ) ;
      }
      rc = _esClt->initScroll( _scrollID, _indexName.c_str(),
                               _type.c_str(),
                               queryCond, searchResult, 1000 ) ;
      PD_RC_CHECK( rc, PDERROR, "Initialize scroll for index[ %s ] and "
                   "type[ %s ] failed[ %d ], query string: %s ",
                   _indexName.c_str(), _type.c_str(), rc, queryCond.c_str() ) ;

      rc = _buildInCond( searchResult, inCond ) ;
      PD_RC_CHECK( rc, PDERROR, "Build the $in condition failed[ %d ]", rc ) ;
      PD_LOG( PDDEBUG, "The new in condition is: %s",
              inCond.toString().c_str() ) ;

      rebuildItems[ SE_QUERY_REBLD_QUERY ] = &inCond ;
      rc = _queryRebuilder.rebuild( rebuildItems, objBuff ) ;
      PD_RC_CHECK( rc, PDERROR, "Rebuild the query message failed[ %d ]", rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _seAdptContextQuery::getMore( INT32 returnNum, utilCommObjBuff &objBuff )
   {
      INT32 rc = SDB_OK ;
      utilCommObjBuff searchResult ;
      BSONObj inCond ;
      REBUILD_ITEM_MAP rebuildItems ;

      objBuff.reset() ;
      if ( !_esClt->isActive() )
      {
         rc = _esClt->active() ;
         PD_RC_CHECK( rc, PDERROR, "Reactive ES client failed[ %d ]", rc ) ;
      }
      rc = _esClt->scrollNext( _scrollID, searchResult ) ;
      PD_RC_CHECK( rc, PDERROR, "Scroll with id[ %s ] for index[ %s ] and "
                   "type[ %s ] failed[ %d ]", _scrollID.c_str(),
                   _indexName.c_str(), _type.c_str(), rc ) ;
      rc = _buildInCond( searchResult, inCond ) ;
      PD_RC_CHECK( rc, PDERROR, "Build the $in condition failed[ %d ]", rc ) ;

      rebuildItems[ SE_QUERY_REBLD_QUERY ] = &inCond ;
      rc = _queryRebuilder.rebuild( rebuildItems, objBuff ) ;
      PD_RC_CHECK( rc, PDERROR, "Rebuild the query message failed[ %d ]", rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   /*
    * Build an $in condition with all the provided _id objects.
    * The _id objects in objBuff are like:
    * { "_id":"1234567890"} { "_id":"0987654321" } { "_id":"567890987654" }
    * Get them one by one and format to an $in clause.
    * The condition should be in the following format:
    * { "_id" : { $in : [ {"$oid" : "1234567890"}, {"$oid" : "0987654321" } ] } }
    */
   INT32 _seAdptContextQuery::_buildInCond( utilCommObjBuff &objBuff,
                                            BSONObj &condition )
   {
      INT32 rc = SDB_OK ;
      const CHAR *idValue = NULL ;
      BSONArrayBuilder arrayBuilder ;
      BSONArray idArray ;

      BSONObj obj ;

      try
      {
         while ( !objBuff.eof() )
         {
            objBuff.nextObj( obj ) ;
            idValue = obj.getStringField( DMS_ID_KEY_NAME ) ;
            SDB_ASSERT( idValue, "id value should not be NULL" ) ;

            bson::OID oid( idValue ) ;
            arrayBuilder.append( oid ) ;
         }

         idArray = arrayBuilder.arr() ;

         condition = BSON( "_id" << BSON( "$in" << idArray ) ) ;
         condition.getOwned() ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "Exception occurred: %s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      PD_LOG( PDDEBUG, "New in condition: %s", condition.toString().c_str() ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

}

