#include "sdbDpsOption.hpp"
#include "ossUtil.hpp"
#include "ossVer.hpp"
#include "utilParam.hpp"

#define ASSIGNED_FILTER( filter, nextFilter ) \
        if( NULL == filter )                  \
           filter = nextFilter ;              \
        else                                  \
           filter->setNext( nextFilter ) ;

#define CHECK_FILTER( filter )                      \
        if ( NULL == filter )                       \
        {                                           \
           printf( "Failed to allocate filter\n" ); \
           goto error;                              \
        }

using namespace engine;

_dpsFilterOption::_dpsFilterOption()
{
   ossMemset( _cmdData.inputName, 0, OSS_MAX_PATHSIZE + 1 ) ;
   ossMemset( _cmdData.srcPath, 0, OSS_MAX_PATHSIZE + 1 ) ;
   ossMemset( _cmdData.dstPath, 0, OSS_MAX_PATHSIZE + 1 ) ;
}

_dpsFilterOption::~_dpsFilterOption()
{
}

void _dpsFilterOption::displayArgs( po::options_description &desc )
{
   std::cout << desc << std::endl ;
}

INT32 _dpsFilterOption::init( INT32 argc, CHAR **argv,
                              po::options_description &desc, iFilter *&filter )
{
   INT32 rc            = SDB_OK ;
   po::variables_map vm ;
   iFilter *nextFilter = NULL ;

   DPS_FILTER_ADD_OPTIONS_BEGIN( desc )
      FILTER_OPTIONS
   DPS_FILTER_ADD_OPTIONS_END

   rc = utilReadCommandLine( argc, argv, desc, vm ) ;
   if ( SDB_OK != rc )
   {
      goto error ;
   }

   if( vm.count( DPS_LOG_FILTER_HELP ) )
   {
      displayArgs( desc ) ;
      rc = SDB_OK ;
      goto done ;
   }

   if( vm.count( DPS_LOG_FILTER_VER ) )
   {
      ossPrintVersion( "SequoiaDB version" ) ;
      goto done ;
   }

   rc = pmdCfgRecord::init( NULL, &vm ) ;
   if( rc )
   {
      printf( "invalid arguments\n" ) ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   ///< we should deal with lsn filter first
   if( vm.count( DPS_LOG_FILTER_LSN ) )
   {
      filter = dpsFilterFactory::getInstance()
               ->createFilter( SDB_LOG_FILTER_LSN ) ;
      CHECK_FILTER( filter ) ;
      const CHAR *pLsn = vm[DPS_LOG_FILTER_LSN].as<std::string>().c_str() ;
      _cmdData.lsn = strtoull( pLsn, NULL, 0 ) ;
   }

   if( vm.count( DPS_LOG_FILTER_TYPE ) )
   {
      nextFilter = dpsFilterFactory::getInstance()
               ->createFilter( SDB_LOG_FILTER_TYPE ) ;
      CHECK_FILTER( nextFilter ) ;
      ASSIGNED_FILTER( filter, nextFilter ) ;
   }

   if( vm.count( DPS_LOG_FILTER_NAME ) )
   {
      nextFilter = dpsFilterFactory::getInstance()
               ->createFilter( SDB_LOG_FILTER_NAME ) ;
      CHECK_FILTER( nextFilter ) ;
      ASSIGNED_FILTER( filter, nextFilter ) ;
   }

   if( vm.count( DPS_LOG_FILTER_META ) )
   {
      if( NULL != filter )
      {
         printf( "meta command must be used alone!\n" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      filter = dpsFilterFactory::getInstance()
               ->createFilter( SDB_LOG_FILTER_META ) ;
      CHECK_FILTER( filter );
   }

   if( vm.count( DPS_LOG_FILTER_NONE ) )
   {
      if( NULL != filter )
      {
         printf( "none-filter must be used alone!\n" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      filter = dpsFilterFactory::getInstance()
               ->createFilter( SDB_LOG_FILTER_NONE ) ;
      CHECK_FILTER( filter );
   }

   if ( NULL == filter )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }

done:
   return rc ;
error:
   goto done ;
}

INT32 _dpsFilterOption::postLoaded()
{
   return SDB_OK ;
}

INT32 _dpsFilterOption::preSaving()
{
   return SDB_OK ;
}

INT32 _dpsFilterOption::doDataExchange( engine::pmdCfgExchange *pEx )
{
   resetResult() ;
 
   rdxString( pEx, DPS_LOG_FILTER_FROM_PATH,
            _cmdData.srcPath, OSS_MAX_PATHSIZE, FALSE, FALSE, "./" ) ;

   rdxString( pEx, DPS_LOG_FILTER_TO_PATH,
            _cmdData.dstPath, OSS_MAX_PATHSIZE, FALSE, FALSE, "./" ) ;

   rdxString( pEx, DPS_LOG_FILTER_NAME,
            _cmdData.inputName, OSS_MAX_PATHSIZE, FALSE, FALSE, "" ) ;

   rdxUShort( pEx, DPS_LOG_FILTER_TYPE,
              _cmdData.type, FALSE, TRUE, (UINT16)PDWARNING ) ;

   rdxInt( pEx, DPS_LOG_LSN_AHEAD,
            _cmdData.lsnAhead, FALSE, TRUE, 20 ) ;

   rdxInt( pEx, DPS_LOG_LSN_BACK,
            _cmdData.lsnBack, FALSE, TRUE, 20 ) ;

   return getResult() ;
}
