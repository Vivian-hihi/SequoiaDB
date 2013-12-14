/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmConditionNodeHelper.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains functions for agent processing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef QGMCONDITIONNODEHELPER_HPP_
#define QGMCONDITIONNODEHELPER_HPP_

#include "qgmConditionNode.hpp"
#include <sstream>

using namespace std ;

namespace engine
{
   class _qgmConditionNodeHelper : public SDBObject
   {
   public:
      _qgmConditionNodeHelper( _qgmConditionNode *root ) ;
      virtual ~_qgmConditionNodeHelper() ;

      _qgmConditionNode* getRoot() { return _root ; }
      void setRoot( _qgmConditionNode *pRoot ) { _root = pRoot ; }

   public:
      static void releaseNodes( qgmConditionNodePtrVec &nodes ) ;


      static BSONObj toBson( const _qgmConditionNode *node,
                             const CHAR *begin,
                             UINT32 size ) ;
/*
      static BSONObj toBson( const _qgmConditionNode *node,
                             const CHAR *begin ) ;
*/

   public:
      string toJson() const ;

      BSONObj toBson( BOOLEAN keepAlias = TRUE ) const ;

      /// get all fields in condition tree.
      /// eg: a > 1 and b < d and c = "abc"
      /// --> a, b, d
      INT32 getAllAttr( qgmDbAttrPtrVec &fields ) ;

      /// _qgmConditionNode in nodes should be freed by caller.
      INT32 separate( qgmConditionNodePtrVec &nodes ) ;

      INT32 merge( qgmConditionNodePtrVec &nodes ) ;

      INT32 merge( _qgmConditionNode *node ) ;

//      BOOLEAN validate() ;

   private:
      INT32 _getAllAttr( _qgmConditionNode *node,
                         qgmDbAttrPtrVec &fields ) ;

      INT32 _toString( const _qgmConditionNode *,
                       stringstream &ss,
                       BOOLEAN keepAlias ) const ;

      INT32 _separate( _qgmConditionNode *predicate,
                       qgmConditionNodePtrVec &nodes ) ;

      INT32 _crtBson( const _qgmConditionNode *node,
                     BSONObj &obj,
                     BOOLEAN keepAlias ) const ;

      BSONObj _toBson( const _qgmConditionNode *node,
                       const CHAR *begin,
                       UINT32 size ) const ;

      BSONObj _toBson( const _qgmConditionNode *node,
                       const CHAR *begin ) const ;


   private:
      _qgmConditionNode *_root ;
   } ;
   typedef class _qgmConditionNodeHelper qgmConditionNodeHelper ;
}

#endif

