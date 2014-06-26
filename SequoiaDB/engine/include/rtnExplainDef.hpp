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

   Source File Name = rtnExplainDef.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of Optimizer component. This file contains structure for access
   plan, which is indicating how to run a given query.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/16/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef RTN_EXPLAINDEF_HPP_
#define RTN_EXPLAINDEF_HPP_

namespace engine
{
   #define RTN_EXPLAIN_FULLNAME "FullName"
   #define RTN_EXPLAIN_SCANTYPE "ScanType"
   #define RTN_EXPLAIN_IXMSCAN "IXSCAN"
   #define RTN_EXPLAIN_TBLSCAN "TBSCAN"
   #define RTN_EXPLAIN_USR_EX_SORT "UseExtSort"
   #define RTN_EXPLAIN_IDXNAME "IdxName"
   #define RTN_EXPLAIN_NODE "Node"
   #define RTN_EXPLAIN_RETURNNUM "ReturnNum"
   #define RTN_EXPLAIN_MILLIS "Millis"
   #define RTN_EXPLAIN_IDXREAD "IdxRead"
   #define RTN_EXPLAIN_DATAREAD "DataRead"
   #define RTN_EXPLAIN_USRCPU "UsrCpu"   
   #define RTN_EXPLAIN_SYSCPU "SysCpu"
   #define RTN_EXPLAIN_SUBCL "SubCollections"
}

#endif
