/*******************************************************************************
   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sqlGrammarBase.hpp

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

#ifndef SQLGRAMMARBASE_HPP_
#define SQLGRAMMARBASE_HPP_

#include "sqlDef.hpp"
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_ast.hpp>

using namespace boost ;
using namespace BOOST_SPIRIT_CLASSIC_NS ;
using namespace boost::spirit ;

namespace engine
{

typedef tree_match<CHAR const *>::container_t SQL_CONTAINER ;

typedef tree_parse_info<> SQL_AST ;

#define SQL_PARSE( sql, rule )\
        ( ast_parse( sql, rule, space_p ) )

#define SQL_RULE_FIELD \
        (\
           lexeme_d[!ch_p('_') >> ( +alnum_p % ch_p('.') )]\
        )


#define SQL_RULE_DIGITAL \
        (\
        lexeme_d[!(ch_p('+')|ch_p('-'))>>+digit_p >>\
        !('.'>>+digit_p)>>!((ch_p('e')|ch_p('E')\
        >> !(ch_p('+')|ch_p('-')))>>+digit_p)]\
        )

#define SQL_RULE_UPDATE \
        ( lexeme_d[as_lower_d[str_p("update")]] )

#define SQL_RULE_SELECT \
        ( lexeme_d[as_lower_d[str_p("select")]] )

#define SQL_RULE_WHERE \
        ( lexeme_d[as_lower_d[str_p("where")]] )

#define SQL_RULE_FROM \
        ( lexeme_d[as_lower_d[str_p("from")]] )

#define SQL_RULE_INSERT \
        ( lexeme_d[as_lower_d[str_p("insert into")]] )

#define SQL_RULE_ORDER \
        ( lexeme_d[as_lower_d[str_p("order by")]] )

#define SQL_RULE_DELETE \
        ( lexeme_d[as_lower_d[str_p("delete from")]] )

#define SQL_RULE_CRTTABLE \
        ( lexeme_d[as_lower_d[str_p("create table")]] )

#define SQL_RULE_DROPTABLE \
        ( lexeme_d[as_lower_d[str_p("drop table")]] )

#define SQL_RULE_CRTSPACE \
        ( lexeme_d[as_lower_d[str_p("create collectionspace")]] )

#define SQL_RULE_DROPSPACE \
        ( lexeme_d[as_lower_d[str_p("drop collectionspace")]] )

#define SQL_RULE_LIST_SPACE \
        ( lexeme_d[as_lower_d[str_p("list collectionspaces")]] )

#define SQL_RULE_LIST_TABLE \
        ( lexeme_d[as_lower_d[str_p("list tables")]] )

#define SQL_RULE_CREATE \
        ( lexeme_d[as_lower_d[str_p("create")]] )

#define SQL_RULE_INDEX \
        ( lexeme_d[as_lower_d[str_p("index")]] )

#define SQL_RULE_DROPINDEX \
        ( lexeme_d[as_lower_d[str_p("drop index")]] )

#define SQL_RULE_DESC \
        ( lexeme_d[as_lower_d[str_p("desc")]] )

#define SQL_RULE_ASC \
        ( lexeme_d[as_lower_d[str_p("asc")]] )

#define SQL_RULE_VALUES \
        ( lexeme_d[as_lower_d[str_p("values")]] )

#define SQL_RULE_SET \
        ( lexeme_d[as_lower_d[str_p("set")]] )

#define SQL_RULE_ON \
        ( lexeme_d[as_lower_d[str_p("on")]] )

#define SQL_RULE_UNIQUE \
        ( lexeme_d[as_lower_d[str_p("unique")]] )

#define SQL_RULE_FULLNAME \
        ( lexeme_d[+alnum_p >> ch_p('.') >> +alnum_p] )

}

#endif

