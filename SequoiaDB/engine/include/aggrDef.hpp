/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = aggrDef.hpp

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
#ifndef AGGRDEF_HPP__
#define AGGRDEF_HPP__

#define AGGR_KEYWORD_PREFIX               '$'
#define AGGR_CL_DEFAULT_ALIAS             "SYS_AGGR_ALIAS"

#define AGGR_KEYWORD_PREFIX_STR           "$"
#define AGGR_GROUP_PARSER_NAME            AGGR_KEYWORD_PREFIX_STR"group"
#define AGGR_MATCH_PARSER_NAME            AGGR_KEYWORD_PREFIX_STR"match"
#define AGGR_SKIP_PARSER_NAME             AGGR_KEYWORD_PREFIX_STR"skip"
#define AGGR_LIMIT_PARSER_NAME            AGGR_KEYWORD_PREFIX_STR"limit"
#define AGGR_SORT_PARSER_NAME             AGGR_KEYWORD_PREFIX_STR"sort"
#define AGGR_PROJECT_PARSER_NAME          AGGR_KEYWORD_PREFIX_STR"project"

#endif