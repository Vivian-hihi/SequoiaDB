/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmParamTable.hpp

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

#ifndef QGMPARAMTABLE_HPP_
#define QGMPARAMTABLE_HPP_

#include "qgmDef.hpp"
#include <map>
#include <vector>

namespace engine
{
   struct _qgmBsonPair
   {
      BSONObj obj ;
      BSONElement ele ;
   } ;

   typedef std::vector<_qgmBsonPair> QGM_CONST_TABLE ;
   typedef std::map<qgmDbAttr, BSONElement> QGM_VAR_TABLE ;

   class _qgmParamTable : public SDBObject
   {
   public:
      _qgmParamTable() ;
      virtual ~_qgmParamTable() ;

   public:
      INT32 addConst( const qgmOpField &value,
                      const BSONElement *&out ) ;

      INT32 addConst( const BSONObj &obj,
                      const BSONElement *&out ) ;

      INT32 addVar( const qgmDbAttr &key,
                    const BSONElement *&out,
                    BOOLEAN *pExisted = NULL ) ;

      /// ensure that obj will not be released until
      /// u do not use this var or set a new value.
      INT32 setVar( const varItem &item,
                    const BSONObj &obj ) ;

      inline void removeVar( const qgmDbAttr &key )
      {
         _var.erase( key ) ;
         return ;
      }

      inline void clearVar()
      {
         _var.clear() ;
         return ;
      }

      inline void clearConst()
      {
         _const.clear() ;
         return ;
      }

      inline void clear()
      {
         _var.clear() ;
         _const.clear() ;
         return ;
      }

   private:
      QGM_CONST_TABLE _const ;
      QGM_VAR_TABLE _var ;
   } ;
   typedef class _qgmParamTable qgmParamTable ;
}

#endif

