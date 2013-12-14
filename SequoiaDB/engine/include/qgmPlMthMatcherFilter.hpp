/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmPlMthMatcherFilter.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains declare for QGM operators

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/29/2013  JHL  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef QGMPLMTHMATCHERFILTER_HPP__
#define QGMPLMTHMATCHERFILTER_HPP__

#include "qgmPlFilter.hpp"
#include "mthMatcher.hpp"

namespace engine
{
   class qgmPlMthMatcherFilter : public _qgmPlFilter
   {
   public:
      qgmPlMthMatcherFilter( const qgmOPFieldVec &selector,
                           INT64 numSkip,
                           INT64 numReturn,
                           const qgmField &alias );

      INT32 loadPattern( bson::BSONObj matcher );

   private:
      virtual INT32 _fetchNext( qgmFetchOut & next );

   private:
      mthMatcher        _mthMatcher;
   };
}

#endif