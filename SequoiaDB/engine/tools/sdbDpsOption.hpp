#ifndef _SDB_DPS_FILTER_HPP_
#define _SDB_DPS_FILTER_HPP_

#include "pmdOptionsMgr.hpp"
#include "sdbDpsFilter.hpp"
#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>
#include <iostream>

#define DPS_LOG_FILTER_HELP "help"
#define DPS_LOG_FILTER_VER  "version"
#define DPS_LOG_FILTER_TYPE "type"
#define DPS_LOG_FILTER_NAME "name"
#define DPS_LOG_FILTER_META "meta"
#define DPS_LOG_FILTER_LSN  "lsn"
#define DPS_LOG_FILTER_NONE "none"
#define DPS_LOG_FILTER_FROM_PATH "source"
#define DPS_LOG_FILTER_TO_PATH   "output"
#define DPS_LOG_LSN_AHEAD   "ahead"
#define DPS_LOG_LSN_BACK    "back"


//        ( DPS_FILTER_COMMANDS_STRING( DPS_LOG_FILTER_NONE, ",n" ), "products all logs without any filter" ) 
#define DPS_FILTER_ADD_OPTIONS_BEGIN( desc ) desc.add_options()
#define DPS_FILTER_ADD_OPTIONS_END ;
#define DPS_FILTER_COMMANDS_STRING( a, b ) \
        ( std::string( a ) + std::string( b ) ).c_str()

#define FILTER_OPTIONS \
        ( DPS_FILTER_COMMANDS_STRING( DPS_LOG_FILTER_HELP, ",h" ), "help" ) \
        ( DPS_FILTER_COMMANDS_STRING( DPS_LOG_FILTER_VER, ",v" ), "show version" ) \
        ( DPS_FILTER_COMMANDS_STRING( DPS_LOG_FILTER_META, ",m" ), "show meta info of logs" ) \
        ( DPS_LOG_FILTER_NONE, "products all logs without any filter" ) \
        ( DPS_FILTER_COMMANDS_STRING( DPS_LOG_FILTER_TYPE, ",t" ), boost::program_options::value< INT32 >(), "filte logs that not match type" ) \
        ( DPS_FILTER_COMMANDS_STRING( DPS_LOG_FILTER_NAME, ",n" ), boost::program_options::value< std::string >(), "filte logs not in cs/cl(s) named like name input" ) \
        ( DPS_FILTER_COMMANDS_STRING( DPS_LOG_FILTER_LSN,  ",l" ), boost::program_options::value< std::string >(), "filte logs around lsn input, -a/-b help" ) \
        ( DPS_FILTER_COMMANDS_STRING( DPS_LOG_FILTER_FROM_PATH, ",s" ), boost::program_options::value<std::string>(), "set source log file path, or current path set default" ) \
        ( DPS_FILTER_COMMANDS_STRING( DPS_LOG_FILTER_TO_PATH, ",o" ), boost::program_options::value< std::string >(), "set output file path, or current path set default " ) \
        ( DPS_FILTER_COMMANDS_STRING( DPS_LOG_LSN_AHEAD, ",a" ), boost::program_options::value< INT32 >(), "show n that assigned logs before lsn" ) \
        ( DPS_FILTER_COMMANDS_STRING( DPS_LOG_LSN_BACK, ",b" ), boost::program_options::value< INT32 >(), "show n that assigned logs after lsn" )

class _dpsFilterOption : public engine::_pmdCfgRecord
{
public:
   _dpsFilterOption() ;
   ~_dpsFilterOption() ;

   const dpsCmdData *getCmdData() const
   {
      return &_cmdData ;
   }

   void displayArgs( po::options_description &desc ) ;

public:
   virtual INT32 doDataExchange( engine::pmdCfgExchange *pEx ) ;
   virtual INT32 postLoaded() ;
   virtual INT32 preSaving() ;

public:
   INT32 init( INT32 argc, CHAR **argv, 
               po::options_description &desc, iFilter *&filter ) ;

private:
   dpsCmdData _cmdData ;
} ;
typedef _dpsFilterOption dpsFilterOption ;
#endif
