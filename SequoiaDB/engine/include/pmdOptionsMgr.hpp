/*******************************************************************************
   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = pmdOptionsMgr.hpp

   Descriptive Name = Process MoDel Main

   When/how to use: this program may be used for managing sequoiadb
   configuration.It can be initialized from cmd and configure file.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          10/22/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef PMDOPTIONSMGR_HPP_
#define PMDOPTIONSMGR_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "ossIO.hpp"
#include "ossSocket.hpp"
#include "pmdOptions.hpp"
#include "msgDef.hpp"

#include <string>
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>
#include "../bson/bson.h"
using namespace std;
namespace po = boost::program_options;

using namespace bson ;

namespace engine
{
   #define PMD_MAX_ENUM_STR_LEN        (32)
   #define PMD_MAX_CATLOG_NUM          (7)

   #define PMD_ADD_PARAM_OPTIONS_BEGIN( desc )\
           desc.add_options()
   #define PMD_ADD_PARAM_OPTIONS_END ;
   #define PMD_COMMANDS_STRING( a, b ) (string(a) +string( b)).c_str()

   #define PMD_MIN_LOG_FILE_SZ      64
   #define PMD_MAX_LOG_FILE_SZ      2048
   #define PMD_DFT_LOG_FILE_SZ      PMD_MIN_LOG_FILE_SZ
   #define PMD_DFT_LOG_FILE_NUM     20

   class _SDB_KRCB;

   enum PMD_CFG_STEP
   {
      PMD_CFG_STEP_INIT       = 0,           // initialize
      PMD_CFG_STEP_REINIT,                   // re-init
      PMD_CFG_STEP_CHG                       // change in runtime
   } ;

   enum PMD_CFG_DATA_TYPE
   {
      PMD_CFG_DATA_CMD        = 0,           // command
      PMD_CFG_DATA_BSON                      // BSON
   } ;

   /*
      _pmdCfgExchange define
   */
   class _pmdCfgExchange : public SDBObject
   {
      public:
         _pmdCfgExchange ( const BSONObj &dataObj,
                           BOOLEAN load = TRUE,
                           PMD_CFG_STEP step = PMD_CFG_STEP_INIT ) ;
         _pmdCfgExchange ( po::variables_map *pVMCmd,
                           po::variables_map *pVMFile = NULL,
                           BOOLEAN load = TRUE,
                           PMD_CFG_STEP step = PMD_CFG_STEP_INIT ) ;
         ~_pmdCfgExchange () ;

         PMD_CFG_STEP   getCfgStep() const { return _cfgStep ; }
         BOOLEAN        isLoad() const { return _isLoad ; }
         BOOLEAN        isSave() const { return !_isLoad ; }

         void           setLoad() { _isLoad = TRUE ; }
         void           setSave() { _isLoad = FALSE ; }
         void           setCfgStep( PMD_CFG_STEP step ) { _cfgStep = step ; }

      public:
         INT32 readInt( const CHAR *pFieldName, INT32 &value,
                        INT32 defaultValue ) ;
         INT32 readInt( const CHAR *pFieldName, INT32 &value ) ;

         INT32 readString( const CHAR *pFieldName, CHAR *pValue, UINT32 len,
                           const CHAR *pDefault ) ;
         INT32 readString( const CHAR *pFieldName, CHAR *pValue, UINT32 len ) ;

         INT32 writeInt( const CHAR *pFieldName, INT32 value ) ;
         INT32 writeString( const CHAR *pFieldName, const CHAR *pValue ) ;

         BOOLEAN hasField( const CHAR *pFieldName ) ;

         const CHAR *getData( UINT32 &dataLen ) ;

      private:
         PMD_CFG_STEP            _cfgStep ;
         BOOLEAN                 _isLoad ;

         PMD_CFG_DATA_TYPE       _dataType ;
         //
         BSONObj                 _dataObj ;
         BSONObjBuilder          _dataBuilder ;
         po::variables_map       *_pVMFile ;
         po::variables_map       *_pVMCmd ;
         std::stringstream       _strStream ;
         std::string             _dataStr ;

   } ;
   typedef _pmdCfgExchange pmdCfgExchange ;

   /*
      _pmdCfgRecord define
   */
   class _pmdCfgRecord : public SDBObject
   {
      public:
         _pmdCfgRecord () ;
         virtual ~_pmdCfgRecord () ;

         INT32 getResult () const { return _result ; }
         void  resetResult () { _result = SDB_OK ; }

         INT32 init( po::variables_map *pVMFile, po::variables_map *pVMCMD ) ;
         INT32 restore( const BSONObj &objData,
                        po::variables_map *pVMCMD ) ;

         INT32 toBSON ( BSONObj &objData ) ;
         INT32 toString( std::string &str ) ;

      protected:
         virtual INT32 doDataExchange( pmdCfgExchange *pEX ) = 0 ;
         virtual INT32 postLoaded() ;
         virtual INT32 preSaving() ;

      protected:
         INT32 rdxInt( pmdCfgExchange *pEX, const CHAR *pFieldName,
                       INT32 &value, BOOLEAN required, BOOLEAN allowRunChg,
                       INT32 defaultValue, BOOLEAN hideParam = FALSE ) ;

         INT32 rdxUInt( pmdCfgExchange *pEX, const CHAR *pFieldName,
                        UINT32 &value, BOOLEAN required, BOOLEAN allowRunChg,
                        UINT32 defaultValue, BOOLEAN hideParam = FALSE ) ;

         INT32 rdxShort( pmdCfgExchange *pEX, const CHAR *pFieldName,
                         INT16 &value, BOOLEAN required, BOOLEAN allowRunChg,
                         INT16 defaultValue, BOOLEAN hideParam = FALSE ) ;

         INT32 rdxUShort( pmdCfgExchange *pEX, const CHAR *pFieldName,
                          UINT16 &value, BOOLEAN required, BOOLEAN allowRunChg,
                          UINT16 defaultValue, BOOLEAN hideParam = FALSE ) ;

         INT32 rdxString( pmdCfgExchange *pEX, const CHAR *pFieldName,
                          CHAR *pValue, UINT32 len, BOOLEAN required,
                          BOOLEAN allowRunChg, const CHAR *pDefaultValue,
                          BOOLEAN hideParam = FALSE ) ;

         INT32 rdxPath( pmdCfgExchange *pEX, const CHAR *pFieldName,
                        CHAR *pValue, UINT32 len, BOOLEAN required,
                        BOOLEAN allowRunChg, const CHAR *pDefaultValue,
                        BOOLEAN hideParam = FALSE ) ;

         INT32 rdxBooleanS( pmdCfgExchange *pEX, const CHAR *pFieldName,
                            BOOLEAN &value, BOOLEAN required,
                            BOOLEAN allowRunChg, BOOLEAN defaultValue,
                            BOOLEAN hideParam = FALSE ) ;

         INT32 rdvMinMax( pmdCfgExchange *pEX, UINT32 &value,
                          UINT32 minV, UINT32 maxV,
                          BOOLEAN autoAdjust = TRUE ) ;
         INT32 rdvMinMax( pmdCfgExchange *pEX, UINT16 &value,
                          UINT16 minV, UINT16 maxV,
                          BOOLEAN autoAdjust = TRUE ) ;
         INT32 rdvMaxChar( pmdCfgExchange *pEX, CHAR *pValue,
                           UINT32 maxChar, BOOLEAN autoAdjust = TRUE ) ;
         INT32 rdvNotEmpty( pmdCfgExchange *pEX, CHAR *pValue ) ;

      private:
         std::string                         _curFieldName ;
         INT32                               _result ;

   } ;
   typedef _pmdCfgRecord pmdCfgRecord ;

   /*
      _pmdOptionsMgr define
   */
   class _pmdOptionsMgr : public pmdCfgRecord
   {
      public:
         _pmdOptionsMgr() ;
         ~_pmdOptionsMgr() ;

      public:
         typedef class _pmdAddrPair
         {
            public :
               CHAR _host[ OSS_MAX_HOSTNAME + 1 ] ;
               CHAR _service[ OSS_MAX_SERVICENAME + 1 ] ;
         } pmdAddrPair ;

      protected:
         virtual INT32 doDataExchange( pmdCfgExchange *pEX ) ;
         virtual INT32 postLoaded() ;
         virtual INT32 preSaving() ;

         INT32    _parseCatAddr() ;

      public:
         INT32 readCmd( INT32 argc, CHAR **argv,
                        po::options_description &desc,
                        po::variables_map &vm ) ;

         INT32 readConfigureFile( const CHAR *path,
                                  po::options_description &desc,
                                  po::variables_map &vm );

         INT32 init( INT32 argc, CHAR **argv ) ;

         INT32 setKrcb( _SDB_KRCB *krcb ) ;

         INT32 reflush2File() ;

      private:
         INT32 _joinDir( const CHAR *dir1, const CHAR *dir2,
                         CHAR *path, INT32 size ) ;

         INT32 _mkdir() ;

      public:
         inline const CHAR *krcbConfPath() const
         {
            return _krcbConfPath;
         }
         inline const CHAR *krcbLogPath() const
         {
            return _krcbLogPath;
         }
         inline const CHAR *krcbDbPath() const
         {
            return _krcbDbPath;
         }
         inline const CHAR *krcbIndexPath() const
         {
            return _krcbIndexPath;
         }
         inline const CHAR *krcbBkupPath() const
         {
            return _krcbBkupPath;
         }
         inline const CHAR *krcbDiagLogPath() const
         {
            return _krcbDiagLogPath;
         }
         inline UINT32 krcbMaxPool() const
         {
            return _krcbMaxPool;
         }
         inline UINT16 krcbSvcPort() const
         {
            return _krcbSvcPort;
         }
         inline UINT16 krcbDiagLvl() const
         {
            return _krcbDiagLvl;
         }
         inline const CHAR *krcbRole() const
         {
            return _krcbRole;
         }
         inline const CHAR *replService() const
         {
            return _replServiceName ;
         }
         inline const CHAR *catService() const
         {
            return _catServiceName ;
         }
         inline const CHAR *shardService() const
         {
            return _shardServiceName ;
         }
         inline const CHAR *restService() const
         {
            return _restServiceName ;
         }
         inline const CHAR *krcbService() const
         {
            return _krcbSvcName ;
         }
         inline const _pmdOptionsMgr::_pmdAddrPair *catAddrs() const
         {
            return _cat ;
         }
         inline const CHAR *dmsTmpPath() const
         {
            return _dmsTmpBlkPath ;
         }

         inline UINT32 sortBufSize() const
         {
            return _sortBufSz ;
         }

         inline UINT32 hjBufSize() const
         {
            return _hjBufSz ;
         }

         void clearCatAddr() ;
         void setCatAddr( const CHAR *host, const CHAR *service ) ;

         inline UINT32 catNum() const { return PMD_MAX_CATLOG_NUM ; }
         inline UINT32 groupID() const { return _groupID ; }
         inline UINT16 nodeID() const { return _nodeID ; }
         inline UINT32 logFileSz () const { return _logFileSz ; }
         inline UINT32 logFileNum () const { return _logFileNum ; }
         inline UINT32 numPreLoaders () const { return _numPreLoaders ; }
         inline UINT32 maxPrefPool () const { return _maxPrefPool ; }
         inline UINT32 maxSubQuery () const { return _maxSubQuery ; }
         inline UINT32 maxReplSync () const { return _maxReplSync ; }
         inline INT32  syncStrategy () const { return _syncStrategy ; }
         inline UINT32 replBucketSize () const { return _replBucketSize ; }
         inline BOOLEAN transactionOn () const { return _transactionOn ; }
         inline BOOLEAN memDebugEnabled () const { return _memDebugEnabled ; }
         inline UINT32 memDebugSize () const { return _memDebugSize ; }
         inline UINT32 indexScanStep () const { return _indexScanStep ; }
         inline UINT32 logBuffSize () const { return _logBuffSize ; }
         inline UINT32 preferedReplica () const { return _preferReplica ; }

      protected: // rdx members
         CHAR        _krcbDbPath[ OSS_MAX_PATHSIZE + 1 ] ;
         CHAR        _krcbIndexPath[ OSS_MAX_PATHSIZE + 1 ] ;
         CHAR        _krcbDiagLogPath[ OSS_MAX_PATHSIZE + 1 ] ;
         CHAR        _krcbLogPath[ OSS_MAX_PATHSIZE + 1 ] ;
         CHAR        _krcbBkupPath[ OSS_MAX_PATHSIZE + 1 ] ;
         UINT32      _krcbMaxPool ;
         CHAR        _krcbSvcName[ OSS_MAX_SERVICENAME + 1 ] ;
         CHAR        _replServiceName[ OSS_MAX_SERVICENAME + 1 ] ;
         CHAR        _catServiceName[ OSS_MAX_SERVICENAME + 1 ] ;
         CHAR        _shardServiceName[ OSS_MAX_SERVICENAME + 1 ] ;
         CHAR        _restServiceName[ OSS_MAX_SERVICENAME + 1 ] ;
         UINT16      _krcbDiagLvl ;
         CHAR        _krcbRole[ PMD_MAX_ENUM_STR_LEN + 1 ] ;
         CHAR        _syncStrategyStr[ PMD_MAX_ENUM_STR_LEN + 1 ] ;
         CHAR        _prefReplStr[ PMD_MAX_ENUM_STR_LEN + 1 ] ;
         UINT32      _logFileSz ;
         UINT32      _logFileNum ;
         UINT32      _numPreLoaders ;
         UINT32      _maxPrefPool ;
         UINT32      _maxSubQuery ;
         UINT32      _maxReplSync ;
         UINT32      _replBucketSize ;
         INT32       _syncStrategy ;
         BOOLEAN     _memDebugEnabled ;
         UINT32      _memDebugSize ;
         UINT32      _indexScanStep ;
         BOOLEAN     _dpslocal ;
         BOOLEAN     _traceOn ;
         UINT32      _traceBufSz ;
         BOOLEAN     _transactionOn ;
         UINT32      _sharingBreakTime ;
         UINT32      _startShiftTime ;
         UINT32      _logBuffSize ;
         CHAR        _catAddrLine[ OSS_MAX_PATHSIZE + 1 ] ;
         CHAR        _dmsTmpBlkPath[ OSS_MAX_PATHSIZE + 1 ] ;
         UINT32      _sortBufSz ;
         UINT32      _hjBufSz ;
         UINT32      _preferReplica ;

      private: // other configs
         CHAR        _krcbConfPath[ OSS_MAX_PATHSIZE + 1 ] ;
         pmdAddrPair _cat[ PMD_MAX_CATLOG_NUM ] ;
         UINT16      _krcbSvcPort ;

         UINT32      _groupID ;
         UINT16      _nodeID ;

   } ;

}

#endif //PMDOPTIONSMGR_HPP_

