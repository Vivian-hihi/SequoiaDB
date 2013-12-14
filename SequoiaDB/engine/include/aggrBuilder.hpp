/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = aggrBuilder.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains functions for agent processing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/04/2013  JHL  Initial Draft

   Last Changed =

******************************************************************************/
#ifndef AGGRBUILDER_HPP__
#define AGGRBUILDER_HPP__

#include <map>
#include "aggrParser.hpp"
#include "pmdEDU.hpp"
#include "qgmPtrTable.hpp"
#include "qgmOptiTree.hpp"
#include "qgmParamTable.hpp"
#include "qgmPlanContainer.hpp"

namespace engine
{
   #define AGGR_PARSER_BEGIN  void aggrBuilder::addParser(){
   #define AGGR_PARSER_END    }
   #define AGGR_PARSER_ADD( parserName, parserClass ) {\
               aggrParser *pObj = SDB_OSS_NEW parserClass();\
               _parserMap.insert( AGGR_PARSER_MAP::value_type(parserName, pObj));}

   typedef std::map< std::string, aggrParser* > AGGR_PARSER_MAP;
   class aggrBuilder : public SDBObject
   {
   public:
      aggrBuilder();
      ~aggrBuilder();
      INT32 build( bson::BSONObj &objs, INT32 objNum,
                  const CHAR *pCLName, _pmdEDUCB *cb,
                  SINT64 &contextID  );

   private:
      INT32 buildTree( bson::BSONObj &objs,
                     INT32 objNum,
                     _qgmOptiTreeNode *&root,
                     _qgmPtrTable * pPtrTable,
                     _qgmParamTable *pParamTable,
                     const CHAR *pCollectionName );

      INT32 createContext( _qgmPlanContainer *container,
                          _pmdEDUCB *cb, SINT64 &contextID );

      void addParser();

   private:
      AGGR_PARSER_MAP         _parserMap;
   };

}

#endif