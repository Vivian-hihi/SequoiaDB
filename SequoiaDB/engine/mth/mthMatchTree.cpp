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

   Source File Name = mthMatchTree.cpp

   Descriptive Name = Method Matcher Tree

   When/how to use: this program may be used on binary and text-formatted
   versions of Method component. This file contains functions for matcher, which
   indicates whether a record matches a given matching rule.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/25/2016  LYB  Initial Draft

   Last Changed =

*******************************************************************************/
#include "mthMatchTree.hpp"
#include "pd.hpp"
#include <string>

using namespace bson ;

namespace engine
{
   static mthMatchOpMapping g_opstr_to_type_array[] =
   {
      //opstr,                        nodeType
      //logic
      { MTH_OPERATOR_STR_AND,         EN_MATCH_OPERATOR_LOGIC_AND },
      { MTH_OPERATOR_STR_OR,          EN_MATCH_OPERATOR_LOGIC_OR },
      { MTH_OPERATOR_STR_NOT,         EN_MATCH_OPERATOR_LOGIC_NOT },

      //operator
      { MTH_OPERATOR_STR_ET,          EN_MATCH_OPERATOR_ET },
      { MTH_OPERATOR_STR_LT,          EN_MATCH_OPERATOR_LT },
      { MTH_OPERATOR_STR_LTE,         EN_MATCH_OPERATOR_LTE },
      { MTH_OPERATOR_STR_GTE,         EN_MATCH_OPERATOR_GTE },
      { MTH_OPERATOR_STR_GT,          EN_MATCH_OPERATOR_GT },
      { MTH_OPERATOR_STR_IN,          EN_MATCH_OPERATOR_IN },
      { MTH_OPERATOR_STR_NE,          EN_MATCH_OPERATOR_NE },
      { MTH_OPERATOR_STR_SIZE,        EN_MATCH_OPERATOR_SIZE },
      { MTH_OPERATOR_STR_ALL,         EN_MATCH_OPERATOR_ALL },
      { MTH_OPERATOR_STR_NIN,         EN_MATCH_OPERATOR_NIN },
      { MTH_OPERATOR_STR_EXISTS,      EN_MATCH_OPERATOR_EXISTS },
      { MTH_OPERATOR_STR_MOD,         EN_MATCH_OPERATOR_MOD },
      { MTH_OPERATOR_STR_TYPE,        EN_MATCH_OPERATOR_TYPE },
      { MTH_OPERATOR_STR_ELEMMATCH,   EN_MATCH_OPERATOR_ELEMMATCH },
      { MTH_OPERATOR_STR_ISNULL,      EN_MATCH_OPERATOR_ISNULL },

      //special operator
      { MTH_OPERATOR_STR_FIELD,       EN_MATCH_OPERATOR_FIELD },
      { MTH_OPERATOR_STR_REGEX,       EN_MATCH_OPERATOR_REGEX },
      { MTH_OPERATOR_STR_OPTIONS,     EN_MATCH_OPERATOR_OPTIONS },

      //functions
      { MTH_FUNCTION_STR_ABS,         EN_MATCH_FUNC_ABS },
      { MTH_FUNCTION_STR_CEILING,     EN_MATCH_FUNC_CEILING },
      { MTH_FUNCTION_STR_FLOOR,       EN_MATCH_FUNC_FLOOR },
      //{ MTH_FUNCTION_STR_MOD,         EN_MATCH_FUNC_MOD },
      { MTH_FUNCTION_STR_ADD,         EN_MATCH_FUNC_ADD },
      { MTH_FUNCTION_STR_SUBTRACT,    EN_MATCH_FUNC_SUBTRACT },
      { MTH_FUNCTION_STR_MULTIPLY,    EN_MATCH_FUNC_MULTIPLY },
      { MTH_FUNCTION_STR_DIVIDE,      EN_MATCH_FUNC_DIVIDE },
      { MTH_FUNCTION_STR_SUBSTR,      EN_MATCH_FUNC_SUBSTR },
      { MTH_FUNCTION_STR_STRLEN,      EN_MATCH_FUNC_STRLEN },
      { MTH_FUNCTION_STR_LOWER,       EN_MATCH_FUNC_LOWER },
      { MTH_FUNCTION_STR_UPPER,       EN_MATCH_FUNC_UPPER },
      { MTH_FUNCTION_STR_LTRIM,       EN_MATCH_FUNC_LTRIM },
      { MTH_FUNCTION_STR_RTRIM,       EN_MATCH_FUNC_RTRIM },
      { MTH_FUNCTION_STR_TRIM,        EN_MATCH_FUNC_TRIM },
      { MTH_FUNCTION_STR_CAST,        EN_MATCH_FUNC_CAST },
   } ;

   //******************_mthMatchNodeFactory*************************
   _mthMatchNodeFactory::_mthMatchNodeFactory()
   {
      INT32 i   = 0 ;
      INT32 len = 0 ;

      len = sizeof( g_opstr_to_type_array ) / sizeof( mthMatchOpMapping ) ;
      for ( ; i < len ; i++ )
      {
         mthMatchOpMapping *ptype = &g_opstr_to_type_array[i] ;
         SDB_ASSERT( _opstrMap.find( ptype->opStr ) == _opstrMap.end(), 
                     "duplicate key is not allowed" ) ;

         _opstrMap[ ptype->opStr ] = ptype ;
      }
   }

   _mthMatchNodeFactory::~_mthMatchNodeFactory()
   {
      _opstrMap.clear() ;
   }

   EN_MATCH_OP_FUNC_TYPE _mthMatchNodeFactory::getMatchNodeType( 
                                                             const CHAR *opStr )
   {
      MTH_OPSTRMAP::iterator iter ;
      iter = _opstrMap.find( opStr ) ;
      if ( iter != _opstrMap.end() )
      {
         mthMatchOpMapping* mapping = iter->second ;
         return ( EN_MATCH_OP_FUNC_TYPE ) mapping->nodeType ;
      }
      else
      {
         return EN_MATCH_OP_FUNC_END ;
      }
   }

   _mthMatchOpNode* _mthMatchNodeFactory::createOpNode( 
                                                   _mthNodeAllocator *allocator,
                                                   EN_MATCH_OP_FUNC_TYPE type )
   {
      _mthMatchOpNode *opNode = NULL ;

      switch( type )
      {
      case EN_MATCH_OPERATOR_ET:
         opNode = new ( allocator ) _mthMatchOpNodeET( allocator ) ;
         break ;
      case EN_MATCH_OPERATOR_LT:
         opNode = new ( allocator ) _mthMatchOpNodeLT( allocator ) ;
         break ;
      case EN_MATCH_OPERATOR_LTE:
         opNode = new ( allocator ) _mthMatchOpNodeLTE( allocator ) ;
         break ;
      case EN_MATCH_OPERATOR_GTE:
         opNode = new ( allocator ) _mthMatchOpNodeGTE( allocator ) ;
         break ;
      case EN_MATCH_OPERATOR_GT:
         opNode = new ( allocator ) _mthMatchOpNodeGT( allocator ) ;
         break ;
      case EN_MATCH_OPERATOR_IN:
         opNode = new ( allocator ) _mthMatchOpNodeIN( allocator ) ;
         break ;
      case EN_MATCH_OPERATOR_NE:
         opNode = new ( allocator ) _mthMatchOpNodeNE( allocator ) ;
         break ;
      case EN_MATCH_OPERATOR_SIZE:
         opNode = new ( allocator ) _mthMatchOpNodeSIZE( allocator ) ;
         break ;
      case EN_MATCH_OPERATOR_ALL:
         opNode = new ( allocator ) _mthMatchOpNodeALL( allocator ) ;
         break ;
      case EN_MATCH_OPERATOR_NIN:
         opNode = new ( allocator ) _mthMatchOpNodeNIN( allocator ) ;
         break ;
      case EN_MATCH_OPERATOR_EXISTS:
         opNode = new ( allocator ) _mthMatchOpNodeEXISTS( allocator ) ;
         break ;
      case EN_MATCH_OPERATOR_MOD:
         opNode = new ( allocator ) _mthMatchOpNodeMOD( allocator ) ;
         break ;
      case EN_MATCH_OPERATOR_TYPE:
         opNode = new ( allocator ) _mthMatchOpNodeTYPE( allocator ) ;
         break ;
      case EN_MATCH_OPERATOR_ELEMMATCH:
         opNode = new( allocator ) _mthMatchOpNodeELEMMATCH( allocator ) ;
         break ;
      case EN_MATCH_OPERATOR_ISNULL:
         opNode = new ( allocator ) _mthMatchOpNodeISNULL( allocator ) ;
         break ;
      case EN_MATCH_OPERATOR_REGEX:
         opNode = new ( allocator ) _mthMatchOpNodeRegex( allocator ) ;
         break ;
      default :
         break ;
      }

      return opNode ;
   }

   _mthMatchLogicNode* _mthMatchNodeFactory::createLogicNode( 
                                                   _mthNodeAllocator *allocator,
                                                   EN_MATCH_OP_FUNC_TYPE type )
   {
      _mthMatchLogicNode *logicNode = NULL ;

      switch( type )
      {
      case EN_MATCH_OPERATOR_LOGIC_AND :
         logicNode = new ( allocator ) _mthMatchLogicAndNode( allocator ) ;
         break ;

      case EN_MATCH_OPERATOR_LOGIC_OR :
         logicNode = new ( allocator ) _mthMatchLogicOrNode( allocator ) ;
         break ;

      case EN_MATCH_OPERATOR_LOGIC_NOT :
         logicNode = new ( allocator ) _mthMatchLogicNotNode( allocator ) ;
         break ;

      default :
         //do nothing
         break ;
      }

      return logicNode ;
   }

   void _mthMatchNodeFactory::releaseNode( _mthMatchNode *node )
   {
      node->release() ;
   }

   _mthMatchTree* _mthMatchNodeFactory::createTree()
   {
      return SDB_OSS_NEW _mthMatchTree() ;
   }

   void _mthMatchNodeFactory::releaseTree( _mthMatchTree *tree )
   {
      if ( NULL != tree )
      {
         tree->clear() ;
         SAFE_OSS_DELETE( tree ) ;
      }   
   }

   _mthMatchFunc* _mthMatchNodeFactory::createFunc( 
                                                _mthNodeAllocator *allocator,
                                                EN_MATCH_OP_FUNC_TYPE type )
   {
      _mthMatchFunc *func = NULL ;

      switch( type )
      {
//      case EN_MATCH_FUNC_ABS:
//         func  = new ( allocator ) _mthMatchFuncABS( allocator ) ;
//         break ;
      default :
         break ;
      }

      return func ;
   }

   void _mthMatchNodeFactory::releaseFunc( _mthMatchFunc *func )
   {
      //TODO:
   }

   _mthMatchNodeFactory *mthGetMatchNodeFactory()
   {
      static _mthMatchNodeFactory factory ;

      return &factory ;
   }

   //******************_mthMatchTree*************************
   _mthMatchTree::_mthMatchTree()
   {
      _root = NULL ;

      _predicateSet.clear() ;
      _isInitialized       = FALSE ;
      _isMatchesAll        = TRUE ;
      _isTotallyConverted  = TRUE ;
      _hasDollarFieldName  = FALSE ;
   }

   _mthMatchTree::~_mthMatchTree()
   {
      clear() ;
   }

   INT32 _mthMatchTree::_addFunction( const CHAR *fieldName, 
                                      const BSONElement &ele, 
                                      EN_MATCH_OP_FUNC_TYPE nodeType,
                                      MTH_FUNC_LIST &funcList )
   {
      INT32 rc = SDB_OK ;
      _mthMatchFunc* func = NULL ;
      func = mthGetMatchNodeFactory()->createFunc( &_allocator, nodeType ) ;
      if ( NULL == func )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "createFunc failed:nodeType=%d,rc=%d", 
                 nodeType, rc ) ;
         goto error ;
      }

      rc = func->init( fieldName, ele ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "init function failed:ele=%s,rc=%d",
                 ele.toString().c_str(), rc ) ;
         goto error ;
      }

      funcList.push_back( func ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _mthMatchTree::_addOperator( const CHAR *fieldName, 
                                      const BSONElement &ele, 
                                      EN_MATCH_OP_FUNC_TYPE nodeType, 
                                      MTH_FUNC_LIST &funcList,
                                      _mthMatchLogicNode *parent )
   {
      INT32 rc = SDB_OK ;
      _mthMatchOpNode *node = NULL ;
      BOOLEAN hasAddToTree  = FALSE ;

      SDB_ASSERT( EN_MATCH_OPERATOR_REGEX != nodeType, 
                  "regex is processed in another branch" ) ;

      if ( EN_MATCH_OPERATOR_FIELD == nodeType )
      {
         //{ a: {$field:"b" } }
         nodeType = EN_MATCH_OPERATOR_ET ;
      }

      node = mthGetMatchNodeFactory()->createOpNode( &_allocator, nodeType ) ;
      if ( NULL == node )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "createOpNodeByOp failed:nodeType=%d,rc=%d", 
                 nodeType, rc ) ;
         goto error ;
      }

      rc = node->init( fieldName, ele ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "init node failed:name=%s,ele=%s,rc=%d",
                 fieldName, ele.toString().c_str(), rc ) ;
         goto error ;
      }

      rc = node->addFuncList( funcList ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "add function list failed:rc=%d", rc ) ;
         goto error ;
      }

      if ( node->hasDollarFieldName() )
      {
         _hasDollarFieldName = TRUE ;
      }

      rc = parent->addChild( node ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "add child failed:parent=%s,child=%s,rc=%d",
                 parent->toString().c_str(), node->toString().c_str(), rc ) ;
         goto error ;
      }
      hasAddToTree = TRUE ;

   done:
      return rc ;
   error:
      if ( !hasAddToTree )
      {
         _releaseTree( node ) ;
      }
      goto done ;
   }

   INT32 _mthMatchTree::_addRegExOp( const CHAR *fieldName, const CHAR *regex, 
                                     const CHAR *options, 
                                     MTH_FUNC_LIST &funcList,
                                     _mthMatchLogicNode *parent )
   {
      INT32 rc = SDB_OK ;
      _mthMatchOpNode *node           = NULL ;
      _mthMatchOpNodeRegex *regexNode = NULL ;
      BOOLEAN hasAddToTree            = FALSE ;
      
      node = mthGetMatchNodeFactory()->createOpNode( &_allocator,
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
         PD_LOG( PDERROR, "dynamic_cast(OpNode -> OpNodeRegex)"
                 " failed:node=%s,rc=%d", node->toString().c_str(), rc ) ;
         goto error ;
      }

      rc = regexNode->init( fieldName, regex, options );
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "init regexNode failed:regex=%s,options=%s,rc=%d", 
                 ( NULL == regex ) ? "" : regex,
                 ( NULL == options ) ? "" : options, rc ) ;
         goto error ;
      }

      rc = regexNode->addFuncList( funcList ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "add funclist failed:rc=%d", rc ) ;
         goto error ;
      }

      rc = parent->addChild( regexNode ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "add child failed:parent=%s,child=%s,rc=%d",
                 parent->toString().c_str(), node->toString().c_str(), rc ) ;
         goto error ;
      }
      hasAddToTree = TRUE ;

   done:
      return rc ;
   error:
      if ( !hasAddToTree )
      {
         _releaseTree( node ) ;
      }
      goto done ;
   }

   //explicit regex {a:{$regex:""}}
   INT32 _mthMatchTree::_parseRegExElement( const BSONElement &ele, 
                                            _mthMatchLogicNode *parent )
   {
      INT32 rc = SDB_OK ;
      MTH_FUNC_LIST empty ;
      rc = _addRegExOp( ele.fieldName(), ele.regex(), ele.regexFlags(), empty,
                        parent ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to _addRegExOp:ele=%s,rc=%d", 
                 ele.toString().c_str(), rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   //{a:1}
   INT32 _mthMatchTree::_parseNormalElement( const BSONElement &ele, 
                                             _mthMatchLogicNode *parent )
   {
      INT32 rc = SDB_OK ;
      MTH_FUNC_LIST empty ;
      const CHAR *eFieldName = ele.fieldName() ;
      //fieldName can't start with '$'
      if ( MTH_OPERATOR_EYECATCHER == eFieldName[0] )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "operator can not in the head:ele=%s",
                  ele.toString().c_str() ) ;
         goto error ;
      }

      rc = _addOperator( ele.fieldName(), ele, EN_MATCH_OPERATOR_ET, empty,
                         parent ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed inject element, rc: %d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   // ele: {$and:[{a:{$lt:1}}]}
   INT32 _mthMatchTree::_pareseLogicElemnts( const BSONElement ele, 
                                             _mthMatchLogicNode *parent )
   {
      INT32 rc = SDB_OK ;

      // iterator array
      BSONObjIterator iterArray( ele.embeddedObject() ) ;
      while ( iterArray.more() )
      {
         BSONElement eleArrayItem = iterArray.next() ;
         if ( Object != eleArrayItem.type() )
         {
            rc = SDB_INVALIDARG ;
            PD_LOG ( PDERROR, "Array item's element type must be Object:"
                     "item=%s,rc=%d", eleArrayItem.toString().c_str(), rc ) ;
            goto error ;
         }

         {
            _mthMatchLogicNode *tmpParent = parent ;
            if ( parent->getType() == EN_MATCH_OPERATOR_LOGIC_OR || 
                 parent->getType() == EN_MATCH_OPERATOR_LOGIC_NOT )
            {
               // {a:{$or:[ {a:1, b:1}, {c:1} ]}}
               // eleTemp = { a:1, b:1 }  condition a=1 and b=1 should be 
               // under LOGIC_AND
               _mthMatchLogicNode *child = NULL ;
               child = mthGetMatchNodeFactory()->createLogicNode( &_allocator,
                                                 EN_MATCH_OPERATOR_LOGIC_AND ) ;
               if ( NULL == child )
               {
                  rc = SDB_INVALIDARG ;
                  PD_LOG( PDERROR, "create logicAnd failed:rc=%d", rc ) ;
                  goto error ;
               }

               rc = child->init( "", ele ) ;
               if ( SDB_OK != rc )
               {
                  PD_LOG( PDERROR, "init child failed:rc=%d", rc ) ;
                  mthGetMatchNodeFactory()->releaseNode( child ) ;
                  goto error ;
               }

               rc = parent->addChild( child ) ;
               if ( SDB_OK != rc )
               {
                  PD_LOG( PDERROR, "add child failed:parent=%s,child=%s,"
                          "rc=%d", parent->toString().c_str(), 
                          child->toString().c_str(), rc ) ;
                  mthGetMatchNodeFactory()->releaseNode( child ) ;
                  goto error ;
               }

               tmpParent = child ;
            }

            // iterator object
            BSONObjIterator iterObject( eleArrayItem.embeddedObject() ) ;
            while ( iterObject.more() )
            {
               BSONElement eleTemp = iterObject.next() ;
               rc = _parseElement ( eleTemp, tmpParent ) ;
               if ( rc )
               {
                  PD_LOG ( PDERROR, "Failed to _parseElement:eleTemp=%s,"
                           "rc=%d", eleTemp.toString().c_str(), rc ) ;
                  goto error ;
               }
            }
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   // ele: {$and:[{a:{$lt:1}}]}
   INT32 _mthMatchTree::_pareseLogicAnd( const BSONElement ele, 
                                         _mthMatchLogicNode *parent )
   {
      INT32 rc = SDB_OK ;
      _mthMatchLogicNode *logicAnd = NULL ;
      BOOLEAN hasAddToTree = FALSE ;

      // logic's element type must be Array
      if ( ele.type() != Array )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "LogicAnd's element type must be Array:ele=%s,rc=%d",
                  ele.toString().c_str(), rc );
         goto error ;
      }

      logicAnd = mthGetMatchNodeFactory()->createLogicNode( &_allocator,
                                                EN_MATCH_OPERATOR_LOGIC_AND ) ;
      if ( !logicAnd )
      {
         rc = SDB_OOM ;
         PD_LOG ( PDERROR, "Failed to allocate memory for "
                  "EN_MATCH_OPERATOR_LOGIC_AND:rc=%d", rc ) ;
         goto error ;
      }

      rc = logicAnd->init( "", ele ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "init logicAnd failed:rc=%d", rc ) ;
         goto error ;
      }

      rc = parent->addChild( logicAnd ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "add child failed:parent=%s,child=%s,rc=%d",
                 parent->toString().c_str(), logicAnd->toString().c_str(), 
                 rc ) ;
         goto error ;
      }

      hasAddToTree = TRUE ;

      rc = _pareseLogicElemnts( ele, logicAnd ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "_pareseLogicElemnts failed:ele=%s,rc=%d",
                 ele.toString().c_str(), rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      if ( !hasAddToTree )
      {
         _releaseTree( logicAnd ) ;
         logicAnd = NULL ;
      }
      //otherwise, logicAnd will be released when release the _root 
      //in _mthMatchTree::clear()
      /* else */
      goto done ;
   }

   // ele: {$or:[{a:{$lt:1}}]}
   INT32 _mthMatchTree::_pareseLogicOr( const BSONElement ele, 
                                        _mthMatchLogicNode *parent )
   {
      INT32 rc = SDB_OK ;
      _mthMatchLogicNode *logicOr  = NULL ;
      BOOLEAN hasAddToTree = FALSE ;

      // logic's element type must be Array
      if ( ele.type() != Array )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "LogicAnd's element type must be Array:ele=%s,rc=%d",
                  ele.toString().c_str(), rc );
         goto error ;
      }

      logicOr = mthGetMatchNodeFactory()->createLogicNode( &_allocator,
                                                EN_MATCH_OPERATOR_LOGIC_OR ) ;
      if ( !logicOr )
      {
         rc = SDB_OOM ;
         PD_LOG ( PDERROR, "Failed to allocate memory for "
                  "EN_MATCH_OPERATOR_LOGIC_OR:rc=%d", rc ) ;
         goto error ;
      }

      rc = logicOr->init( "", ele ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "init logicOr failed:rc=%d", rc ) ;
         goto error ;
      }

      rc = parent->addChild( logicOr ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "add child failed:parent=%s,child=%s,rc=%d",
                 parent->toString().c_str(), logicOr->toString().c_str(), 
                 rc ) ;
         goto error ;
      }

      hasAddToTree = TRUE ;

      rc = _pareseLogicElemnts( ele, logicOr ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "_pareseLogicElemnts failed:ele=%s,rc=%d",
                 ele.toString().c_str(), rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      if ( !hasAddToTree )
      {
         _releaseTree( logicOr ) ;
         logicOr = NULL ;
      }
      //otherwise, logicAnd will be released when release the _root 
      //in _mthMatchTree::clear()
      /* else */
      goto done ;
   }

   // ele: {$not:[{a:{$lt:1}}]}
   INT32 _mthMatchTree::_pareseLogicNot( const BSONElement ele, 
                                         _mthMatchLogicNode *parent )
   {
      INT32 rc = SDB_OK ;
      _mthMatchLogicNode *logicNot = NULL ;
      BOOLEAN hasAddToTree = FALSE ;

      // logic's element type must be Array
      if ( ele.type() != Array )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "LogicAnd's element type must be Array:ele=%s,rc=%d",
                  ele.toString().c_str(), rc );
         goto error ;
      }

      logicNot = mthGetMatchNodeFactory()->createLogicNode( &_allocator,
                                                EN_MATCH_OPERATOR_LOGIC_NOT ) ;
      if ( !logicNot )
      {
         rc = SDB_OOM ;
         PD_LOG ( PDERROR, "Failed to allocate memory for "
                  "EN_MATCH_OPERATOR_LOGIC_OR:rc=%d", rc ) ;
         goto error ;
      }

      rc = logicNot->init( "", ele ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "init logicNot failed:rc=%d", rc ) ;
         goto error ;
      }

      rc = parent->addChild( logicNot ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "add child failed:parent=%s,child=%s,rc=%d",
                 parent->toString().c_str(), logicNot->toString().c_str(), 
                 rc ) ;
         goto error ;
      }

      hasAddToTree = TRUE ;

      rc = _pareseLogicElemnts( ele, logicNot ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "_pareseLogicElemnts failed:ele=%s,rc=%d",
                 ele.toString().c_str(), rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      if ( !hasAddToTree )
      {
         _releaseTree( logicNot ) ;
         logicNot = NULL ;
      }
      //otherwise, logicAnd will be released when release the _root 
      //in _mthMatchTree::clear()
      /* else */
      goto done ;
   }

   INT32 _mthMatchTree::_parseArrayElement( const BSONElement &ele, 
                                            _mthMatchLogicNode *parent )
   {
      INT32 rc = SDB_OK ;
      const CHAR *fieldName = ele.fieldName() ;
      if ( MTH_OPERATOR_EYECATCHER == fieldName[0] )
      {
         EN_MATCH_OP_FUNC_TYPE nodeType ;
         nodeType = mthGetMatchNodeFactory()->getMatchNodeType( fieldName ) ;
         if ( EN_MATCH_OPERATOR_LOGIC_AND == nodeType )
         {
            // logic and
            rc = _pareseLogicAnd( ele, parent ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "_pareseLogicAnd failed:ele=%s,rc=%d",
                       ele.toString().c_str(), rc ) ;
               goto error ;
            }
         }
         else if ( EN_MATCH_OPERATOR_LOGIC_OR == nodeType )
         {
            // logic or
            rc = _pareseLogicOr( ele, parent ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "_pareseLogicOr failed:ele=%s,rc=%d",
                       ele.toString().c_str(), rc ) ;
               goto error ;
            }
         }
         else if ( EN_MATCH_OPERATOR_LOGIC_NOT == nodeType )
         {
            // logic not
            rc = _pareseLogicNot( ele, parent ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "_pareseLogicNot failed:ele=%s,rc=%d",
                       ele.toString().c_str(), rc ) ;
               goto error ;
            }
         }
         else
         {
            rc = SDB_INVALIDARG ;
            PD_LOG ( PDERROR, "unsupported logic operation:ele=%s,rc=%d",
                     ele.toString().c_str(), rc ) ;
            goto error ;
         }
      }
      else
      {
         //just normal array {a:[1,3]}
         MTH_FUNC_LIST empty ;
         rc = _addOperator( ele.fieldName(), ele, EN_MATCH_OPERATOR_ET, 
                            empty, parent ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "_addOperator failed:ele=%s,rc=%d",
                    ele.toString().c_str(), rc ) ;
            goto error ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   /* ignoreCurrentField: default FALSE */
   BOOLEAN _mthMatchTree::_isExistOpFieldRecursive( const BSONElement &ele,
                                                    BOOLEAN ignoreCurrentField )
   {
      const CHAR *fieldName = ele.fieldName() ;
      if ( Object == ele.type() || Array == ele.type() )
      {
         EN_MATCH_OP_FUNC_TYPE nodeType ;
         nodeType = mthGetMatchNodeFactory()->getMatchNodeType( 
                                                             ele.fieldName() ) ;
         if ( EN_MATCH_OPERATOR_ELEMMATCH == nodeType )
         {
            //we treat $eleMatch as not OpField
            return FALSE ;
         }

         if ( ignoreCurrentField || MTH_OPERATOR_EYECATCHER != fieldName[0] )
         {
            BSONObjIterator j( ele.embeddedObject() ) ;
            while ( j.more () )
            {
               if ( _isExistOpFieldRecursive( j.next() ) )
               {
                  return TRUE ;
               }
            }

            return FALSE ;
         }

         return TRUE ;
      }
      else
      {
         if ( MTH_OPERATOR_EYECATCHER == fieldName[0] )
         {
            return TRUE ;
         }
         else
         {
            return FALSE ;
         }
      }
   }

   BOOLEAN _mthMatchTree::_isExistOpEyeCatcher( const BSONElement &ele )
   {
      const CHAR *eFieldName = NULL ;
      BSONElement temEle ;
      if ( Object == ele.type() || Array == ele.type() )
      {
            BSONObjIterator j( ele.embeddedObject() ) ;
            while ( j.more () )
            {
               temEle     = j.next() ;
               eFieldName = temEle.fieldName() ;
               if ( MTH_OPERATOR_EYECATCHER == eFieldName[0] )
               {
                  return TRUE ;
               }
            }
            return FALSE ;
      }
      else
      {
         // only object and array have SubObject!
         return FALSE ;
      }
   }

   INT32 _mthMatchTree::_pareseObjectInnerOp( const BSONElement &ele, 
                                              const BSONElement &innerEle,
                                              MTH_FUNC_LIST &funcList,
                                              _mthMatchLogicNode *parent,
                                              const char *&regex,
                                              const char *&options )
   {
      INT32 rc = SDB_OK ;
      const CHAR *innerFieldName = NULL ;
      EN_MATCH_OP_FUNC_TYPE nodeType ;

      innerFieldName = innerEle.fieldName() ;
      nodeType       = mthGetMatchNodeFactory()->getMatchNodeType( 
                                                              innerFieldName ) ;
      if ( Object != innerEle.type() && Array != innerEle.type() )
      {
         // { a : { $xx : xxxxxxx } }
         if ( EN_MATCH_OPERATOR_REGEX == nodeType )
         {
            regex = innerEle.valuestrsafe() ;
         }
         else if ( EN_MATCH_OPERATOR_OPTIONS == nodeType )
         {
            options = innerEle.valuestrsafe() ;
         }
         else if ( EN_MATCH_OPERATOR_MOD == nodeType )
         {
            //mod is operator or function
            if ( innerEle.type() == Array )
            {
               //operator mod
               rc = _addOperator( ele.fieldName(), innerEle, nodeType, funcList, 
                                  parent ) ;
               if ( SDB_OK != rc )
               {
                  PD_LOG( PDERROR, "_addOperator failed:innerEle=%s,rc=%d",
                          innerEle.toString().c_str(), rc ) ;
                  goto error ;
               }
            }
            else
            {
               //func mod
               rc = _addFunction( ele.fieldName(), innerEle, nodeType, 
                                  funcList ) ;
               if ( SDB_OK != rc )
               {
                  PD_LOG( PDERROR, "add function failed:fieldName=%s,"
                          "innerEle=%s,rc=%d", ele.fieldName(), 
                          innerEle.toString().c_str(), rc ) ;
                  goto error ;
               }
            }
         }
         else if ( ( nodeType < EN_MATCH_OPERATOR_END && 
                     nodeType >= EN_MATCH_OPERATOR_ET ) )
         {
            rc = _addOperator( ele.fieldName(), innerEle, nodeType, funcList, 
                               parent ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "_addOperator failed:innerEle=%s,rc=%d",
                       innerEle.toString().c_str(), rc ) ;
               goto error ;
            }
         }
         else if ( nodeType < EN_MATCH_FUNC_END && 
                   nodeType >= EN_MATCH_FUNC_ABS )
         {
            rc = _addFunction( ele.fieldName(), innerEle, nodeType, funcList ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "add function failed:fieldName=%s,innerEle=%s,"
                       "rc=%d", ele.fieldName(), innerEle.toString().c_str(), 
                       rc ) ;
               goto error ;
            }
         }
         else
         {
            // EN_MATCH_OPERATOR_END
            rc = SDB_INVALIDARG ;
            PD_LOG( PDERROR, "unreconigzed operator:embEle=%s,rc=%d",
                    innerEle.toString().c_str(), rc ) ;
            goto error ;
         }
      }
      else
      {
         // { a : { $xx : {...} } }
         if ( _isExistOpFieldRecursive( innerEle, TRUE ) )
         {
            // { a : { $xx : { $xx : ... } } }
            BSONObjIterator k( innerEle.embeddedObject() ) ;
            while ( k.more() )
            {
               BSONElement tElem       = k.next () ;
               const CHAR *tEleFieName = tElem.fieldName () ;
               EN_MATCH_OP_FUNC_TYPE tmpType ;
               tmpType = mthGetMatchNodeFactory()->getMatchNodeType( 
                                                                tEleFieName ) ;
               if ( EN_MATCH_OPERATOR_FIELD != tmpType 
                    && _isExistOpFieldRecursive( innerEle ) )
               {
                  //only support $field
                  rc = SDB_INVALIDARG ;
                  PD_LOG ( PDERROR, "Matching syntax can not "
                           "have more than one operator") ;
                  goto error ;
               }

               rc = _addOperator( ele.fieldName(), tElem, nodeType, funcList,
                                  parent ) ;
               if ( SDB_OK != rc )
               {
                  PD_LOG( PDERROR, "_addOperator failed:tElem=%s,rc=%d",
                          tElem.toString().c_str(), rc ) ;
                  goto error ;
               }
            }
         }
         else
         {
            // { a : { $xx : { xx : ... } } }
            rc = _addOperator( ele.fieldName(), innerEle, nodeType, funcList,
                               parent ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "_addOperator failed:innerEle=%s,rc=%d",
                       innerEle.toString().c_str(), rc ) ;
               goto error ;
            }
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _mthMatchTree::_paresePrevOptions( const CHAR *fieldName,
                                            const BSONElement &ele,
                                            const CHAR *options,
                                            MTH_FUNC_LIST &funcList,
                                            _mthMatchLogicNode *parent )
   {
      //previous is options. regex must come next
      INT32 rc = SDB_OK ;
      const CHAR *tmpRegex = NULL ;
      EN_MATCH_OP_FUNC_TYPE type ;
      type = mthGetMatchNodeFactory()->getMatchNodeType( ele.fieldName() ) ;
      if ( EN_MATCH_OPERATOR_REGEX != type )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "exist extra options operator:options=%s", 
                 options ) ;
         goto error ;
      }
      
      tmpRegex = ele.valuestrsafe() ;
      rc = _addRegExOp( fieldName, tmpRegex, options, funcList, parent ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "_addRegExOp failed:fieldName=%s,"
                 "regex=%s,options=%s,rc=%d", fieldName, tmpRegex,
                 options, rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   void _mthMatchTree::_clearFuncList( MTH_FUNC_LIST &funcList )
   {
      MTH_FUNC_LIST::iterator iter = funcList.begin() ;
      while ( iter != funcList.end() )
      {
         _mthMatchFunc *func = *iter ;
         mthGetMatchNodeFactory()->releaseFunc( func ) ;
         iter++ ;
      }

      funcList.clear() ;
   }

   INT32 _mthMatchTree::_parseObjectElement( const BSONElement &ele, 
                                             _mthMatchLogicNode *parent )
   {
      INT32 rc = SDB_OK ;
      MTH_FUNC_LIST funcList ;
      const CHAR *fieldName = ele.fieldName() ;
      //fieldName can't start with '$'
      if ( MTH_OPERATOR_EYECATCHER == fieldName[0] )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "operator can not in the head:ele=%s",
                  ele.toString().c_str() ) ;
         goto error ;
      }

      if ( !_isExistOpEyeCatcher( ele ) )
      {
         // { a : { xx : xxxxxx, yy : yyyyyy } }
         MTH_FUNC_LIST empty ;
         rc = _addOperator( ele.fieldName(), ele, EN_MATCH_OPERATOR_ET, empty,
                            parent ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "_addOperator failed:ele=%s,rc=%d",
                    ele.toString().c_str(), rc ) ;
            goto error ;
         }

         goto done ;
      }

      {
         // { a : { $xx : xxxxxx, yy : yyyyyy } }
         BSONObjIterator iter( ele.embeddedObject() ) ;
         const CHAR *regex   = NULL ;
         const CHAR *options = NULL ;
         while ( iter.more() )
         {
            BSONElement embEle       = iter.next () ;
            const CHAR *embFieldName = embEle.fieldName () ;

            SDB_ASSERT( NULL != options ? NULL == regex : TRUE, 
                        "if options is not null, regex must be null" ) ;
            SDB_ASSERT( NULL != regex ? NULL == options : TRUE, 
                        "if regex is not null, options must be null" ) ;
            if ( NULL != options )
            {
               //previous is options. regex must come next
               rc = _paresePrevOptions( fieldName, embEle, options, funcList,
                                        parent ) ;
               if ( SDB_OK != rc )
               {
                  PD_LOG( PDERROR, "_paresePrevOptions failed:fieldName=%s,"
                          "options=%s,rc=%d", fieldName, options, rc ) ;
                  goto error ;
               }
               options = NULL ;
               continue ;
            }

            if ( NULL != regex )
            {
               //previous is regex. check if options come next
               EN_MATCH_OP_FUNC_TYPE type ;
               type = mthGetMatchNodeFactory()->getMatchNodeType( 
                                                                embFieldName ) ;
               if ( EN_MATCH_OPERATOR_OPTIONS == type )
               {
                  const CHAR *tmpOptions = embEle.valuestrsafe() ;
                  rc = _addRegExOp( fieldName, regex, tmpOptions, funcList,
                                    parent ) ;
                  if ( SDB_OK != rc )
                  {
                     PD_LOG( PDERROR, "_addRegExOp failed:fieldName=%s,"
                             "regex=%s,options=%s,rc=%d", fieldName, regex,
                             tmpOptions, rc ) ;
                     goto error ;
                  }
                  regex = NULL ;
                  continue ;
               }

               rc = _addRegExOp( fieldName, regex, NULL, funcList, parent ) ;
               if ( SDB_OK != rc )
               {
                  PD_LOG( PDERROR, "_addRegExOp failed:fieldName=%s,"
                          "regex=%s,rc=%d", fieldName, regex, rc ) ;
                  goto error ;
               }
               regex = NULL ;
            }

            if ( MTH_OPERATOR_EYECATCHER == embFieldName[0] )
            {
               // { a : { $xx : xxxxxxx } }
               rc = _pareseObjectInnerOp( ele, embEle, funcList, parent, regex, 
                                          options ) ;
               if ( SDB_OK != rc )
               {
                  PD_LOG( PDERROR, "_pareseObjectInnerOp failed:rc=%d", rc ) ;
                  goto error ;
               }
            }
            else
            {
               // { a : { xxx : xxxxxx } }, quality condition
               rc = _addOperator( ele.fieldName(), embEle, EN_MATCH_OPERATOR_ET, 
                                  funcList, parent ) ;
            }
         }

         if ( NULL != regex )
         {
            rc = _addRegExOp( fieldName, regex, options, funcList, parent ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "_addRegExOp failed:fieldName=%s,"
                       "regex=%s,rc=%d", fieldName, regex, rc ) ;
               goto error ;
            }
            regex   = NULL ;
            options = NULL ;
         }

         if ( NULL != options )
         {
            rc = SDB_INVALIDARG ;
            PD_LOG( PDERROR, "exist extra options operator:options=%s", 
                    options ) ;
            goto error ;
         }

         if ( funcList.size() > 0 )
         {
            rc = SDB_INVALIDARG ;
            MTH_FUNC_LIST::iterator iter = funcList.begin() ;
            PD_LOG( PDERROR, "exist extra func:first func=%s", 
                    ( *iter )->toString().c_str() ) ;
            goto error ;
         }
      }

   done:
      return rc ;
   error:
      _clearFuncList( funcList ) ;
      goto done ;
   }

   INT32 _mthMatchTree::_parseElement( const BSONElement &ele, 
                                       _mthMatchLogicNode *parent )
   {
      INT32 rc = SDB_OK ;
      switch ( ele.type() )
      {
      case RegEx:
         rc = _parseRegExElement( ele, parent ) ;
         break ;

      case Object:
         rc = _parseObjectElement( ele, parent ) ;
         break ;

      case Array:
         rc = _parseArrayElement( ele, parent ) ;
         break ;

      default:
         rc = _parseNormalElement( ele, parent ) ;
      }

      return rc ;
   }

   INT32 _mthMatchTree::loadPattern( const BSONObj &matcher,
                                     BOOLEAN needPredicate /* = TRUE */)
   {
      SDB_ASSERT ( !_isInitialized, "mthMatcher can't be initialized "
                   "multiple times" ) ;
      INT32 rc      = SDB_OK ;
      _matchPattern = matcher.copy() ;
      _isMatchesAll = TRUE ;
      INT32 eleNum  = 0 ;

      BSONObjIterator i( _matchPattern ) ;
      try
      {
         //create root node( AND )
         _root = mthGetMatchNodeFactory()->createLogicNode( &_allocator,
                                                 EN_MATCH_OPERATOR_LOGIC_AND ) ;
         if ( NULL == _root )
         {
            rc = SDB_OOM ;
            PD_LOG ( PDERROR, "Failed to allocate memory for "
                     "EN_MATCH_OPERATOR_LOGIC_AND:rc=%d", rc ) ;
            goto error ;
         }

         rc = _root->init( "", BSONObj().firstElement() ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "init logciNode failed:rc=%d", rc ) ;
            goto error ;
         }

         while ( i.more() )
         {
            BSONElement temp = i.next() ;
            rc = _parseElement( temp, ( _mthMatchLogicNode* )_root ) ;
            if ( rc )
            {
               PD_LOG ( PDERROR, "parse element failed:element=%s,rc=d",
                        temp.toString().c_str(), rc ) ;
               goto error ;
            }
            ++eleNum ;
            _isMatchesAll = FALSE ;
         }
      }
      catch ( std::exception &e )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "Failed to call loadPattern: %s", e.what() ) ;
         goto error ;
      }

      rc = _optimize( needPredicate ) ;

      _isInitialized = TRUE ;

   done :
      return rc ;
   error :
      clear() ;   /* _root is cleared in clear() */
      goto done ;
   }

   //delete node; then add node->children as parent's new children
   INT32 _mthMatchTree::_deleteNode( _mthMatchNode *parent,
                                     _mthMatchNode *node )
   {
      INT32 rc = SDB_OK ;

      while ( node->getChildrenCount() > 0 )
      {
         _mthMatchNodeIterator iter( node ) ;
         _mthMatchNode *child = iter.next() ;

         node->delChild( child ) ;

         rc = parent->addChild( child ) ;
         if ( SDB_OK != rc )
         {
            // if add child failed. child is not longer in the tree.
            mthGetMatchNodeFactory()->releaseNode( child ) ;
            PD_LOG( PDERROR, "add child failed:rc=%d" ) ;
            
            goto error ;
         }
      }

      parent->delChild( node ) ;
      mthGetMatchNodeFactory()->releaseNode( node ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _mthMatchTree::_deleteExtraLogicNode( _mthMatchNode *node )
   {
      INT32 rc = SDB_OK ;
      if ( node->getType() > EN_MATCH_OPERATOR_LOGIC_END )
      {
         goto done ;
      }

      {
         UINT32 idx = 0 ;
         MATCHNODE_VECTOR childrenVec ;
         _mthMatchNodeIterator iter( node ) ;
         while ( iter.more() )
         {
            _mthMatchNode *child = iter.next() ;
            childrenVec.push_back( child ) ;
         }

         for ( idx = 0 ; idx < childrenVec.size() ; idx++ )
         {
            rc = _deleteExtraLogicNode( childrenVec[idx] ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "delete extra logic node failed:rc=%d", rc ) ;
               goto error ;
            }
         }
      }

      //$logic -> $or/$and -> op => $logic -> op
      if ( ( node->getType() == EN_MATCH_OPERATOR_LOGIC_OR || 
             node->getType() == EN_MATCH_OPERATOR_LOGIC_AND ) && 
             node->getChildrenCount() == 1 )
      {
         _mthMatchNode *parent = node->getParent() ;
         if ( NULL != parent )
         {
            SDB_ASSERT( parent->getType() >= EN_MATCH_OPERATOR_LOGIC_AND &&
                        parent->getType() < EN_MATCH_OPERATOR_LOGIC_END, 
                        "parent must be logic node" ) ;

            rc = _deleteNode( parent, node ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "_deleteNode failed:rc=%d", rc ) ;
               goto error ;
            }
            goto done ;
         }
      }

      //$not/and -> $and -> op1|$logic|op3 => $not/and -> op1|$logic|op3
      if ( node->getType() == EN_MATCH_OPERATOR_LOGIC_AND )
      {
         _mthMatchNode *parent = node->getParent() ;
         if ( NULL != parent )
         {
            if ( parent->getType() == EN_MATCH_OPERATOR_LOGIC_AND || 
                 parent->getType() == EN_MATCH_OPERATOR_LOGIC_NOT )
            {
               rc = _deleteNode( parent, node ) ;
               if ( SDB_OK != rc )
               {
                  PD_LOG( PDERROR, "_deleteNode failed:rc=%d", rc ) ;
                  goto error ;
               }

               goto done ;
            }
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _mthMatchTree::_optimizeNodeLevel()
   {
      INT32 rc = SDB_OK ;
      UINT32 childrenCount = 0 ;

      childrenCount = _root->getChildrenCount() ;
      if ( 0 == childrenCount )
      {
         goto done ;
      }

      rc = _deleteExtraLogicNode( _root ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "_deleteExtraLogicNode failed:rc=%d", rc ) ;
         goto error ;
      }

      if ( _root->getChildrenCount() == 1 && 
           _root->getType() == EN_MATCH_OPERATOR_LOGIC_AND )
      {
         _mthMatchNodeIterator iter( _root ) ;
         _mthMatchNode *child = iter.next() ;
         if ( child->getType() < EN_MATCH_OPERATOR_LOGIC_END &&
              child->getType() >= EN_MATCH_OPERATOR_LOGIC_AND )
         {
            _root->delChild( child ) ;
            mthGetMatchNodeFactory()->releaseNode( _root ) ;
            _root = child ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   void _mthMatchTree::_setWeight( _mthMatchNode *node )
   {
      if ( node->getType() >= EN_MATCH_OPERATOR_LOGIC_AND &&
           node->getType() < EN_MATCH_OPERATOR_LOGIC_END )
      {
         _mthMatchNodeIterator iter( node ) ;
         UINT32 weight = 0 ;
         while ( iter.more() )
         {
            _mthMatchNode *child = iter.next() ;
            _setWeight( child ) ;
            weight += child->getWeight() ;
         }

         node->setWeight( weight ) ;
      }
   }

   void _mthMatchTree::_sortByWeight()
   {
      _root->sortByWeight() ;
   }

   INT32 _mthMatchTree::_setPredicate()
   {
      INT32 rc = SDB_OK ;
      rc = _root->calcPredicate( _predicateSet ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "calc predicate failed:rc=%d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   void _mthMatchTree::_checkTotallyConverted( _mthMatchNode *node, 
                                               BOOLEAN &isTotallyConverted )
   {
      if ( !isTotallyConverted )
      {
         return ;
      }

      if ( !node->isTotalConverted() )
      {
         isTotallyConverted = FALSE ;
         return ;
      }

      _mthMatchNodeIterator iter( node ) ;
      while ( iter.more() )
      {
         _mthMatchNode *child = iter.next() ;
         _checkTotallyConverted( child, isTotallyConverted ) ;
         if ( !isTotallyConverted )
         {
            return ;
         }
      }
   }

   void _mthMatchTree::_checkTotallyConverted()
   {
      if ( _hasDollarFieldName )
      {
         _isTotallyConverted = FALSE ;
         return ;
      }

      _checkTotallyConverted( _root, _isTotallyConverted ) ;
   }

   INT32 _mthMatchTree::_optimize( BOOLEAN needPredicate )
   {
      INT32 rc = SDB_OK ;
 
      //1. _optimize NodeLevel (impact the weight and order )
      rc = _optimizeNodeLevel() ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "_optimizeNodeLevel failed:rc=%d", rc ) ;
         goto error ;
      }

      //2. set all weight
      _setWeight( _root ) ;

      //3. sort by weight
      _sortByWeight() ;

      //4. set predicate key
      if ( needPredicate )
      {
         rc = _setPredicate() ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "_setPredicate failed:rc=%d", rc ) ;
            goto error ;
         }
      }

      //5. check total converted
      _checkTotallyConverted() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _mthMatchTree::matches( const BSONObj &matchTarget, BOOLEAN &result,
                                 vector<INT64> *dollarList /* = NULL */)
   {
      INT32 rc = SDB_OK ;
      _mthMatchTreeContext context ;
      if ( NULL == dollarList || !_hasDollarFieldName )
      {
         //1. user do not care dollarList.
         //2. this matchTree have not dollarFieldName
         context.disableDollarList() ;
      }

      rc = matches( matchTarget, result, context ) ;

      if ( NULL != dollarList )
      {
         context.getDollarList( dollarList ) ;
      }

      return rc ;
   }

   INT32 _mthMatchTree::matches( const BSONObj &matchTarget, BOOLEAN &result,
                                 _mthMatchTreeContext &context )
   {
      SDB_ASSERT( _isInitialized, "must be init first" ) ;
      INT32 rc = SDB_OK ;

      if ( _isMatchesAll )
      {
         result = TRUE ;
         goto done ;
      }

      try
      {
         context.setObj( matchTarget ) ;
         result = FALSE ;
         rc = _root->execute( matchTarget, context, result ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "execute failed:taget=%s,rc=%d", 
                     matchTarget.toString().c_str(), rc ) ;
            goto error ;
         }
      }
      catch (  std::exception &e )
      {
         PD_LOG ( PDERROR, "Failed to match: %s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   void _mthMatchTree::_releaseTree( _mthMatchNode *node )
   {
      if ( NULL == node )
      {
         return ;
      }

      _mthMatchNodeIterator iter( node ) ;
      while ( iter.more() )
      {
         _mthMatchNode *child = iter.next() ;
         _releaseTree( child ) ;
      }

      mthGetMatchNodeFactory()->releaseNode( node ) ;
   }

   void _mthMatchTree::clear()
   {
      _releaseTree( _root ) ;
      _root = NULL ;

      _predicateSet.clear() ;
      _isInitialized      = FALSE ;
      _isMatchesAll       = TRUE ;
      _isTotallyConverted = TRUE ;
      _hasDollarFieldName = FALSE ;
      _matchPattern       = BSONObj() ;
   }

   BSONObj _mthMatchTree::getEqualityQueryObject()
   {
      BSONObj obj ;
      if ( NULL == _root )
      {
         goto done ;
      }

      try
      {
         BSONObjBuilder builder ;
         _root->extraEqualityMatches( builder ) ;
         obj = dotted2nested( builder.obj() ) ;
      }
      catch (  std::exception &e )
      {
         PD_LOG ( PDWARNING, "Failed to extract equality matches: %s", 
                  e.what() ) ;
         goto done ;
      }

   done:
      return obj ;
   }

   BOOLEAN _mthMatchTree::isInitialized()
   {
      return _isInitialized ;
   }

   BOOLEAN _mthMatchTree::isMatchesAll()
   {
      return _isMatchesAll ;
   }

   const rtnPredicateSet& _mthMatchTree::getPredicateSet()
   {
      return _predicateSet ;
   }

   BSONObj& _mthMatchTree::getMatchPattern()
   {
      return _matchPattern ;
   }

   BOOLEAN _mthMatchTree::hasDollarFieldName()
   {
      return _hasDollarFieldName ;
   }

   BOOLEAN _mthMatchTree::totallyConverted() const
   {
      return _isTotallyConverted ;
   }

   void _mthMatchTree::setMatchesAll( BOOLEAN matchesAll )
   {
      _isMatchesAll = matchesAll ;
   }

   BSONObj _mthMatchTree::getParsedQuery() const
   {
      return NULL != _root ? _root->toBson() : BSONObj() ;
   }

   BSONObj _mthMatchTree::toBson()
   {
      if ( NULL != _root )
      {
         return _root->toBson() ;
      }
      else
      {
         BSONObjBuilder builder ;
         return builder.obj() ;
      }
   }

   string _mthMatchTree::toString()
   {
      string output ;
      BSONObj obj ;

      obj = toBson() ;

      output = "Pattern:" + _matchPattern.toString() + "\n" ;
      output += "Root:" + obj.toString() + "\n" ;
      output += string( "isInitialized:" ) +
                ( _isInitialized ? "TRUE" : "FALSE" ) + "\n" ;
      output += string( "_isMatchesAll:" ) + 
                ( _isMatchesAll ? "TRUE" : "FALSE" ) + "\n" ;
      output += string( "_isTotallyConverted:" ) + 
                ( _isTotallyConverted ? "TRUE" : "FALSE" ) + "\n" ;
      output += string( "_hasDollarFieldName:" ) + 
                ( _hasDollarFieldName ? "TRUE" : "FALSE" ) + "\n" ;
      output += string( "_predicateSet:" ) + 
                _predicateSet.toString() ;

      return output ;
   }
}

