#ifndef _SDB_DPS_DUMP_LOG_FILTER_HPP_
#define _SDB_DPS_DUMP_LOG_FILTER_HPP_
   
#include "ossTypes.hpp"
#include "ossMem.hpp"
#include "ossIO.hpp"
#include "ossUtil.hpp"
#include "sdbDpsFilter.hpp"



class _dpsLogFilter : public SDBObject
{
public:
   _dpsLogFilter( const dpsCmdData* data ) ;

   virtual ~_dpsLogFilter() ;

   virtual INT32 doParse() ;

   void setFilter( iFilter *filter ) ;

   const CHAR* getSrcPath() const ;
   const CHAR* getDstPath() const ;

   static const INT32 getFileCount( const CHAR *path ) ;

   static BOOLEAN isFileExisted( const CHAR *path ) ;

   static BOOLEAN isDir( const CHAR *path ) ;

private:
   iFilter *_filter ;
   const dpsCmdData *_cmdData ;
} ;
typedef _dpsLogFilter dpsLogFilter ;
   
#endif
