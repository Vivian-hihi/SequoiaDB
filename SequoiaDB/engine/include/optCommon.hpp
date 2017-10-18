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

   Source File Name = optCommon.hpp

   Descriptive Name = Optimizer Plan Common Header

   When/how to use: this program may be used on binary and text-formatted
   versions of Optimizer component. This file contains common structure for
   plan, which is indicating how to run a given query.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          03/14/2017  HGM Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef OPTCOMMON_HPP__
#define OPTCOMMON_HPP__

#include "core.hpp"
#include "oss.hpp"

namespace engine
{

   // When index scan is best enough, its cost is smaller than 1/10 of table
   // scan
   #define OPT_IDX_PREFERRED_RATE            ( 10 )

   // Maximum number of candidate plans
   #define OPT_MAX_CANDIDATE_COUNT           ( 5 )

   // Rate to convert IO cost to final cost
   #define OPT_IO_CPU_RATE                   ( 2000 )

   // CPU cost to extract a record from data page
   #define OPT_RECORD_CPU_COST               ( 4 )

   // CPU cost to extract a index item from index page
   #define OPT_IDX_CPU_COST                  ( 2 )

   // Base CPU cost to process a operator
   #define OPT_OPTR_BASE_CPU_COST            ( 1 )

   // IO Cost to sequential scan a 4k-size page
   #define OPT_SEQ_SCAN_IO_COST              ( 1 )

   // IO Cost to randomly scan a 4k-size page
   #define OPT_RANDOM_SCAN_IO_COST           ( 10 )

   // Rate to convert cost to ms
   #define OPT_COST_TO_MS                    ( 0.0005 )

   // Default start cost of table scan
   #define OPT_TBLSCAN_DEFAULT_START_COST    ( 0 )

   // Default start cost of indes scan
   #define OPT_IDXSCAN_DEFAULT_START_COST    ( 0 )

   // Threshold selectivity of candidate index scan plans
   #define OPT_PRED_THRESHOLD_SELECTIVITY    ( 0.1 )

   // Default selectivity
   #define OPT_DEF_SELECTIVITY               ( 0.3333333333333333 )

   // Default selectivity of matcher
   #define OPT_MTH_DEFAULT_SELECTIVITY       ( 1.0 )

   // Default selectivity of operators in matcher
   #define OPT_MTH_OPTR_DEFAULT_SELECTIVITY  ( OPT_DEF_SELECTIVITY )

   // Default CPU cost of matcher
   #define OPT_MTH_DEFAULT_CPU_COST          ( 0 )

   // Base CPU cost to process a operator in matcher
   #define OPT_MTH_OPTR_BASE_CPU_COST        ( OPT_OPTR_BASE_CPU_COST )

   // CPU cost to process $regex in matcher
   #define OPT_MTH_REGEX_CPU_COST            ( OPT_MTH_OPTR_BASE_CPU_COST * 10 )

   // CPU cost to process a function in matcher
   #define OPT_MTH_FUNC_DEF_CPU_COST         ( OPT_MTH_OPTR_BASE_CPU_COST * 2 )

   // Default selectivity of a predicate
   #define OPT_PRED_DEFAULT_SELECTIVITY      ( 1.0 )

   // Default CPU cost of predicate
   #define OPT_PRED_DEFAULT_CPU_COST         ( 0 )

   // Default selectivity of a valid predicate
   #define OPT_PRED_DEF_SELECTIVITY          ( OPT_DEF_SELECTIVITY )

   // Default selectivity of a range predicate
   #define OPT_PRED_RANGE_DEF_SELECTIVITY    ( 0.05 )

   // Default selectivity of a $et predicate
   #define OPT_PRED_EQ_DEF_SELECTIVITY       ( 0.005 )

   #define OPT_ROUND( x, min, max )          ( OSS_MIN( OSS_MAX( ( x ), ( min ) ), ( max ) ) )

   // Selectivity should between 0.0 and 1.0
   #define OPT_ROUND_SELECTIVITY( x )        OPT_ROUND( x, 0.0, 1.0 )

   // Numbers ( number of records, pages ) should be larger than 1
   #define OPT_ROUND_NUM( x )              ( OSS_MAX( 1, ( x ) ) )
   #define OPT_ROUND_NUM_DEF( x, def )     ( OSS_MAX( ( def ) , ( x ) ) )

   #define OPT_LOG2( x )                     ( log( x ) / 0.693147180559945 )

   // Compare BSON numbers between -99999999.9 and 99999999.9
   #define OPT_BSON_NUM_MAX                  ( 99999999.9 )
   #define OPT_BSON_NUM_MIN                  ( -99999999.9 )
   #define OPT_ROUND_BSON_NUM( x )           OPT_ROUND( x, OPT_BSON_NUM_MIN, OPT_BSON_NUM_MAX )

   // Compare first 20 characters in BSON strings between ' ' to 127
   #define OPT_BSON_STR_MIN_LEN              ( 20 )
   #define OPT_BSON_STR_MIN                  ( (UINT8)' ' )
   #define OPT_BSON_STR_MAX                  ( 127 )

   double optConvertStrToScalar ( const CHAR *pValue, UINT32 valueSize,
                                  UINT8 low, UINT8 high ) ;

   /*
      OPT_PLAN_CACHE_LEVEL define
    */
   enum OPT_PLAN_CACHE_LEVEL
   {
      OPT_PLAN_NOCACHE = 0,
      OPT_PLAN_ORIGINAL,
      OPT_PLAN_NORMALIZED,
      OPT_PLAN_PARAMETERIZED,
      OPT_PLAN_FUZZYOPTR
   } ;

   #define OPT_FIELD_HASH_CODE            "HashCode"
   #define OPT_FIELD_REFERENCE_COUNT      "RefCount"
   #define OPT_FIELD_NORMAIZED_QUERY      "NormalizedQuery"
   #define OPT_FIELD_ORDERBY              "OrderBy"
   #define OPT_FIELD_HINT                 "Hint"
   #define OPT_FIELD_SORTED_IDX_REQURED   "SortedIndexRequired"
   #define OPT_FIELD_PARAM_PLAN_VALID     "ParamPlanValid"
   #define OPT_FIELD_MAINCL_PLAN_VALID    "MainCLPlanValid"
   #define OPT_FIELD_PLAN_PATH            "PlanPath"
   #define OPT_FIELD_PARAM_QUERY          "Query"
   #define OPT_FIELD_PLAN_SCORE           "Score"
   #define OPT_FIELD_TOTAL_QUERY_TIME     "TotalQueryTime"
   #define OPT_FIELD_AVG_QUERY_TIME       "AvgQueryTime"
   #define OPT_FIELD_PLAN_REF_COUNT       "RefCount"
   #define OPT_FIELD_PLAN_ACCESS_COUNT    "AccessCount"

   // Explain for cache status
   #define OPT_FIELD_CACHE_STATUS         "CacheStatus"
   // Not cached
   #define OPT_CACHE_STATUS_NOCACHE       "NoCache"
   // New created into cache
   #define OPT_CACHE_STATUS_NEWCACHE      "NewCache"
   // Hit cache
   #define OPT_CACHE_STATUS_HITCACHE      "HitCache"

   // Explain for cache level ( --plancachelevel )
   #define OPT_FIELD_CACHELEVEL           "CacheLevel"
   #define OPT_CACHE_NOCACHE_NAME         "OPT_PLAN_NOCACHE"
   #define OPT_CACHE_ORIGINAL_NAME        "OPT_PLAN_ORIGINAL"
   #define OPT_CACHE_NORMALIZED_NAME      "OPT_PLAN_NORMALZIED"
   #define OPT_CACHE_PARAMETERIZED_NAME   "OPT_PLAN_PARAMETERIZED"
   #define OPT_CACHE_FUZZYOPTR_NAME       "OPT_PLAN_FUZZYOPTR"

   // Explain for query activity
   #define OPT_FIELD_MAX_QUERY            "MaxTimeSpentQuery"
   #define OPT_FIELD_MIN_QUERY            "MinTimeSpentQuery"
   #define OPT_FIELD_CONTEXT_ID           FIELD_NAME_CONTEXTID
   #define OPT_FIELD_QUERY_TYPE           "QueryType"
   #define OPT_QUERY_TYPE_SELECT          "SELECT"
   #define OPT_QUERY_TYPE_UPDATE          "UPDATE"
   #define OPT_QUERY_TYPE_DELETE          "DELETE"
   #define OPT_QUERY_TIME_SPENT           FIELD_NAME_QUERYTIMESPENT
   #define OPT_QUERY_START_TIME           FIELD_NAME_STARTTIMESTAMP

}

#endif //OPTCOMMON_HPP__

