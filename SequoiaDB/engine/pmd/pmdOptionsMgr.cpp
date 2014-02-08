/*******************************************************************************
   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = pmdOptionsMgr.cpp

   Descriptive Name = Process MoDel Main

   When/how to use: this program may be used for managing sequoiadb
   configuration.It can be initialized from cmd and configure file.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "pmdOptionsMgr.hpp"
#include "pd.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "pmdCommon.hpp"
#include "ossSocket.hpp"
#include "msg.hpp"
#include "msgCatalog.hpp"
#include "ossMem.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"
#include "ossIO.hpp"
#include "ossVer.hpp"

#include "rtn.hpp"
#include "rtnSortDef.hpp"

#include <vector>
#include <boost/algorithm/string.hpp>

using namespace bson ;

namespace engine
{
   #define ALIGNMENT_SIZE sizeof( ossValuePtr )

   const CHAR *CONFIG_FILE_NAME = "sdb.conf" ;

   #define JUDGE_RC( rc ) if ( SDB_OK != rc ) { goto error ; }

   #define PMD_ARTIFICIAL_CAT_GROUP_ID    CAT_CATALOG_GROUPID
   #define PMD_ARTIFICIAL_CAT_NODE_ID     1

   #define PMD_OPTION_CATALOG_ADDR_SPLITER            ","
   #define PMD_OPTION_CATALOG_ADDR_FIELD_SPLITER      ":"

   #define PMD_OPTION_DIAG_PATH        "diaglog"
   #define PMD_OPTION_LOG_PATH         "replicalog"
   #define PMD_OPTION_BK_PATH          "bakfile"
   #define PMD_OPTION_TMPBLK_PATH      "tmp"

   #define PMD_OPTION_BRK_TIME_DEFAULT (5000)
   #define PMD_MAX_PREF_POOL           (200)
   #define PMD_MAX_SUB_QUERY           (10)
   #define PMD_MIN_SORTBUF_SZ          (RTN_SORT_MIN_BUFSIZE)
   #define PMD_DEFAULT_SORTBUF_SZ      (512)
   #define PMD_DEFAULT_HJ_SZ           (128)
   #define PMD_MIN_HJ_SZ               (64)
   #define PMD_DEFAULT_MAX_REPLSYNC    (10)
   #define PMD_DFT_REPL_BUCKET_SIZE    (32)
   #define PMD_DFT_INDEX_SCAN_STEP     (30)

   /*
      _pmdCfgExchange implement
   */
   _pmdCfgExchange::_pmdCfgExchange( const BSONObj &dataObj,
                                     BOOLEAN load,
                                     PMD_CFG_STEP step )
   :_cfgStep( step ), _isLoad( load ), _dataObj( dataObj )
   {
      _dataType   = PMD_CFG_DATA_BSON ;
      _pVMFile    = NULL ;
      _pVMCmd     = NULL ;
   }

   _pmdCfgExchange::_pmdCfgExchange( po::variables_map *pVMCmd,
                                     po::variables_map * pVMFile,
                                     BOOLEAN load,
                                     PMD_CFG_STEP step )
   :_cfgStep( step ), _isLoad( load ), _pVMFile( pVMFile ), _pVMCmd( pVMCmd )
   {
      _dataType   = PMD_CFG_DATA_CMD ;
   }

   _pmdCfgExchange::~_pmdCfgExchange()
   {
      _pVMFile    = NULL ;
      _pVMCmd     = NULL ;
   }

   INT32 _pmdCfgExchange::readInt( const CHAR * pFieldName, INT32 &value )
   {
      INT32 rc = SDB_OK ;

      if ( PMD_CFG_DATA_BSON == _dataType )
      {
         rc = rtnGetIntElement( _dataObj, pFieldName, value ) ;
      }
      else if ( PMD_CFG_DATA_CMD == _dataType )
      {
         if ( _pVMCmd && _pVMCmd->count( pFieldName ) )
         {
            value = (*_pVMCmd)[pFieldName].as<int>() ;
         }
         else if ( _pVMFile && _pVMFile->count( pFieldName ) )
         {
            value = (*_pVMFile)[pFieldName].as<int>() ;
         }
         else
         {
            rc = SDB_FIELD_NOT_EXIST ;
         }
      }
      else
      {
         rc = SDB_SYS ;
      }

      return rc ;
   }

   INT32 _pmdCfgExchange::readInt( const CHAR * pFieldName, INT32 &value,
                                   INT32 defaultValue )
   {
      INT32 rc = readInt( pFieldName, value ) ;
      if ( SDB_FIELD_NOT_EXIST == rc )
      {
         value = defaultValue ;
         rc = SDB_OK ;
      }
      return rc ;
   }

   INT32 _pmdCfgExchange::readString( const CHAR *pFieldName, CHAR *pValue,
                                      UINT32 len )
   {
      INT32 rc = SDB_OK ;

      if ( PMD_CFG_DATA_BSON == _dataType )
      {
         const CHAR *tmpValue = NULL ;
         rc = rtnGetStringElement( _dataObj, pFieldName, &tmpValue ) ;
         if ( tmpValue )
         {
            ossStrncpy( pValue, tmpValue, len ) ;
            pValue[ len - 1 ] = 0 ;
         }
      }
      else if ( PMD_CFG_DATA_CMD == _dataType )
      {
         string tmpValue ;
         if ( _pVMCmd && _pVMCmd->count( pFieldName ) )
         {
            tmpValue = (*_pVMCmd)[pFieldName].as<string>() ;
         }
         else if ( _pVMFile && _pVMFile->count( pFieldName ) )
         {
            tmpValue = (*_pVMFile)[pFieldName].as<string>() ;
         }
         else
         {
            rc = SDB_FIELD_NOT_EXIST ;
         }

         if ( SDB_OK == rc )
         {
            ossStrncpy( pValue, tmpValue.c_str(), len ) ;
            pValue[ len - 1 ] = 0 ;
         }
      }
      else
      {
         rc = SDB_SYS ;
      }

      return rc ;
   }

   INT32 _pmdCfgExchange::readString( const CHAR *pFieldName,
                                      CHAR *pValue, UINT32 len,
                                      const CHAR *pDefault )
   {
      INT32 rc = readString( pFieldName, pValue, len ) ;
      if ( SDB_FIELD_NOT_EXIST == rc && pDefault )
      {
         ossStrncpy( pValue, pDefault, len ) ;
         pValue[ len - 1 ] = 0 ;
         rc = SDB_OK ;
      }
      return rc ;
   }

   INT32 _pmdCfgExchange::writeInt( const CHAR * pFieldName, INT32 value )
   {
      INT32 rc = SDB_OK ;

      if ( PMD_CFG_DATA_BSON == _dataType )
      {
         _dataBuilder.append( pFieldName, value ) ;
      }
      else if ( PMD_CFG_DATA_CMD == _dataType )
      {
         _strStream << pFieldName << " = " << value << "\n" ;
      }
      else
      {
         rc = SDB_SYS ;
      }
      return rc ;
   }

   INT32 _pmdCfgExchange::writeString( const CHAR *pFieldName,
                                       const CHAR *pValue )
   {
      INT32 rc = SDB_OK ;

      if ( PMD_CFG_DATA_BSON == _dataType )
      {
         _dataBuilder.append( pFieldName, pValue ) ;
      }
      else if ( PMD_CFG_DATA_CMD == _dataType )
      {
         _strStream << pFieldName << " = " << pValue << "\n" ;
      }
      else
      {
         rc = SDB_SYS ;
      }
      return rc ;
   }

   BOOLEAN _pmdCfgExchange::hasField( const CHAR * pFieldName )
   {
      if ( PMD_CFG_DATA_BSON == _dataType &&
           !_dataObj.getField( pFieldName ).eoo() )
      {
         return TRUE ;
      }
      else if ( PMD_CFG_DATA_CMD == _dataType )
      {
         if ( ( _pVMCmd && _pVMCmd->count( pFieldName ) ) ||
              ( _pVMFile && _pVMFile->count( pFieldName ) ) )
         {
            return TRUE ;
         }
      }

      return FALSE ;
   }

   const CHAR* _pmdCfgExchange::getData( UINT32 & dataLen )
   {
      if ( PMD_CFG_DATA_BSON == _dataType )
      {
         _dataObj = _dataBuilder.obj() ;
         dataLen = _dataObj.objsize() ;
         return _dataObj.objdata() ;
      }
      else if ( PMD_CFG_DATA_CMD == _dataType )
      {
         _dataStr = _strStream.str() ;
         dataLen = _dataStr.size() ;
         return _dataStr.c_str() ;
      }
      return NULL ;
   }

   /*
      _pmdCfgRecord implement
   */
   _pmdCfgRecord::_pmdCfgRecord ()
   {
      _result = SDB_OK ;
   }
   _pmdCfgRecord::~_pmdCfgRecord ()
   {
   }

   INT32 _pmdCfgRecord::restore( const BSONObj & objData,
                                 po::variables_map *pVMCMD )
   {
      pmdCfgExchange ex( objData, TRUE, PMD_CFG_STEP_INIT ) ;
      INT32 rc = doDataExchange( &ex ) ;
      if ( rc )
      {
         goto error ;
      }
      else if ( pVMCMD )
      {
         pmdCfgExchange ex( pVMCMD, NULL, TRUE, PMD_CFG_STEP_REINIT ) ;
         rc = doDataExchange( &ex ) ;
         if ( rc )
         {
            goto error ;
         }
      }
      rc = postLoaded() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdCfgRecord::init( po::variables_map *pVMFile,
                              po::variables_map *pVMCMD )
   {
      pmdCfgExchange ex( pVMCMD, pVMFile, TRUE, PMD_CFG_STEP_INIT ) ;
      INT32 rc = doDataExchange( &ex ) ;
      if ( rc )
      {
         goto error ;
      }
      rc = postLoaded() ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdCfgRecord::toBSON( BSONObj & objData )
   {
      INT32 rc = preSaving() ;
      if ( SDB_OK == rc )
      {
         pmdCfgExchange ex( BSONObj(), FALSE, PMD_CFG_STEP_INIT ) ;
         rc = doDataExchange( &ex ) ;
         if ( SDB_OK == rc )
         {
            UINT32 dataLen = 0 ;
            try
            {
               objData = BSONObj( ex.getData( dataLen ) ).getOwned() ;
            }
            catch( std::exception &e )
            {
               PD_LOG( PDERROR, "Occur exception: %s", e.what() ) ;
               rc = SDB_SYS ;
            }
         }
      }
      return rc ;
   }

   INT32 _pmdCfgRecord::toString( string & str )
   {
      INT32 rc = preSaving() ;
      if ( SDB_OK == rc )
      {
         pmdCfgExchange ex( NULL, NULL, FALSE, PMD_CFG_STEP_INIT ) ;
         INT32 rc = doDataExchange( &ex ) ;
         if ( SDB_OK == rc )
         {
            UINT32 dataLen = 0 ;
            str = ex.getData( dataLen ) ;
         }
      }
      return rc ;
   }

   INT32 _pmdCfgRecord::postLoaded()
   {
      return SDB_OK ;
   }

   INT32 _pmdCfgRecord::preSaving()
   {
      return SDB_OK ;
   }

   INT32 _pmdCfgRecord::rdxString( pmdCfgExchange *pEX, const CHAR *pFieldName,
                                   CHAR *pValue, UINT32 len, BOOLEAN required,
                                   BOOLEAN allowRunChg,
                                   const CHAR *pDefaultValue,
                                   BOOLEAN hideParam )
   {
      if ( _result )
      {
         goto error ;
      }
      _curFieldName = pFieldName ;

      if ( pEX->isLoad() )
      {
         if ( PMD_CFG_STEP_REINIT == pEX->getCfgStep() &&
              !pEX->hasField( pFieldName ) )
         {
            goto done ;
         }
         else if ( PMD_CFG_STEP_CHG == pEX->getCfgStep() )
         {
            if ( FALSE == allowRunChg )
            {
               if ( pEX->hasField( pFieldName ) )
               {
                  _result = SDB_PERM ;
                  PD_LOG_MSG( PDWARNING, "Field[%s] do not support changing in "
                              "runtime", pFieldName ) ;
                  goto error ;
               }
               goto done ;
            }
            if ( !pEX->hasField( pFieldName ) )
            {
               goto done ;
            }
         }

         if ( required )
         {
            _result = pEX->readString( pFieldName, pValue, len ) ;
         }
         else
         {
            _result = pEX->readString( pFieldName, pValue, len, pDefaultValue ) ;
         }
         if ( SDB_FIELD_NOT_EXIST == _result )
         {
            ossPrintf( "Error: Field[%s] not config\n", pFieldName ) ;
         }
         else if ( _result )
         {
            ossPrintf( "Error: Read field[%s] config failed, rc: %d\n",
                       pFieldName, _result ) ;
         }
      }
      else
      {
         if ( hideParam && 0 == ossStrcmp( pValue, pDefaultValue ) )
         {
            goto done ;
         }
         _result = pEX->writeString( pFieldName, pValue ) ;
         if ( _result )
         {
            PD_LOG( PDWARNING, "Write field[%s] failed, rc: %d",
                    pFieldName, _result ) ;
         }
      }

   done:
      return _result ;
   error:
      goto done ;
   }

   INT32 _pmdCfgRecord::rdxPath( pmdCfgExchange *pEX, const CHAR *pFieldName,
                                 CHAR *pValue, UINT32 len, BOOLEAN required,
                                 BOOLEAN allowRunChg,
                                 const CHAR *pDefaultValue,
                                 BOOLEAN hideParam )
   {
      _result = rdxString( pEX, pFieldName, pValue, len, required, allowRunChg,
                           pDefaultValue, hideParam ) ;
      if ( SDB_OK == _result && pEX->isLoad() && 0 != pValue[0] )
      {
         std::string strTmp = pValue ;
         if ( NULL == ossGetRealPath( strTmp.c_str(), pValue, len ) )
         {
            ossPrintf( "Error: Failed to get real path for %s:%s\n",
                       pFieldName, strTmp.c_str() ) ;
            _result = SDB_INVALIDARG ;
         }
         else
         {
            pValue[ len - 1 ] = 0 ;
         }
      }
      return _result ;
   }

   INT32 _pmdCfgRecord::rdxBooleanS( pmdCfgExchange *pEX,
                                     const CHAR *pFieldName,
                                     BOOLEAN &value,
                                     BOOLEAN required,
                                     BOOLEAN allowRunChg,
                                     BOOLEAN defaultValue,
                                     BOOLEAN hideParam )
   {
      CHAR szTmp[ PMD_MAX_ENUM_STR_LEN + 1 ] = {0} ;
      ossStrcpy( szTmp, value ? "TRUE" : "FALSE" ) ;
      _result = rdxString( pEX, pFieldName, szTmp, sizeof(szTmp), required,
                           allowRunChg, defaultValue ? "TRUE" : "FALSE",
                           hideParam ) ;
      if ( SDB_OK == _result && pEX->isLoad() )
      {
         ossStrToBoolean( szTmp, &value ) ;
      }
      return _result ;
   }

   INT32 _pmdCfgRecord::rdxInt( pmdCfgExchange *pEX, const CHAR *pFieldName,
                                INT32 &value, BOOLEAN required,
                                BOOLEAN allowRunChg, INT32 defaultValue,
                                BOOLEAN hideParam )
   {
      if ( _result )
      {
         goto error ;
      }
      _curFieldName = pFieldName ;

      if ( pEX->isLoad() )
      {
         if ( PMD_CFG_STEP_REINIT == pEX->getCfgStep() &&
              !pEX->hasField( pFieldName ) )
         {
            goto done ;
         }
         else if ( PMD_CFG_STEP_CHG == pEX->getCfgStep() )
         {
            if ( FALSE == allowRunChg )
            {
               if ( pEX->hasField( pFieldName ) )
               {
                  _result = SDB_PERM ;
                  PD_LOG_MSG( PDWARNING, "Field[%s] do not support changing in "
                              "runtime", pFieldName ) ;
                  goto error ;
               }
               goto done ;
            }
            if ( !pEX->hasField( pFieldName ) )
            {
               goto done ;
            }
         }

         if ( required )
         {
            _result = pEX->readInt( pFieldName, value ) ;
         }
         else
         {
            _result = pEX->readInt( pFieldName, value, defaultValue ) ;
         }
         if ( SDB_FIELD_NOT_EXIST == _result )
         {
            ossPrintf( "Error: Field[%s] not config\n", pFieldName ) ;
         }
         else if ( _result )
         {
            ossPrintf( "Error: Read field[%s] config failed, rc: %d\n",
                       pFieldName, _result ) ;
         }
      }
      else
      {
         if ( hideParam && value == defaultValue )
         {
            goto done ;
         }
         _result = pEX->writeInt( pFieldName, value ) ;
         if ( _result )
         {
            PD_LOG( PDWARNING, "Write field[%s] failed, rc: %d",
                    pFieldName, _result ) ;
         }
      }

   done:
      return _result ;
   error:
      goto done ;
   }

   INT32 _pmdCfgRecord::rdxUInt( pmdCfgExchange *pEX, const CHAR *pFieldName,
                                 UINT32 &value, BOOLEAN required,
                                 BOOLEAN allowRunChg, UINT32 defaultValue,
                                 BOOLEAN hideParam )
   {
      INT32 tmpValue = (INT32)value ;
      _result = rdxInt( pEX, pFieldName, tmpValue, required, allowRunChg,
                        (INT32)defaultValue, hideParam ) ;
      if ( SDB_OK == _result && pEX->isLoad() )
      {
         value = (UINT32)tmpValue ;
      }
      return _result ;
   }

   INT32 _pmdCfgRecord::rdxShort( pmdCfgExchange *pEX, const CHAR *pFieldName,
                                  INT16 &value, BOOLEAN required,
                                  BOOLEAN allowRunChg, INT16 defaultValue,
                                  BOOLEAN hideParam )
   {
      INT32 tmpValue = (INT32)value ;
      _result = rdxInt( pEX, pFieldName, tmpValue, required, allowRunChg,
                        (INT32)defaultValue, hideParam ) ;
      if ( SDB_OK == _result && pEX->isLoad() )
      {
         value = (INT16)tmpValue ;
      }
      return _result ;
   }

   INT32 _pmdCfgRecord::rdxUShort( pmdCfgExchange *pEX, const CHAR *pFieldName,
                                   UINT16 &value, BOOLEAN required,
                                   BOOLEAN allowRunChg, UINT16 defaultValue,
                                   BOOLEAN hideParam )
   {
      INT32 tmpValue = (INT32)value ;
      _result = rdxInt( pEX, pFieldName, tmpValue, required, allowRunChg,
                        (INT32)defaultValue, hideParam ) ;
      if ( SDB_OK == _result && pEX->isLoad() )
      {
         value = (UINT16)tmpValue ;
      }
      return _result ;
   }

   INT32 _pmdCfgRecord::rdvMinMax( pmdCfgExchange *pEX, UINT32 &value,
                                   UINT32 minV, UINT32 maxV,
                                   BOOLEAN autoAdjust )
   {
      if ( _result )
      {
         goto error ;
      }

      if ( !pEX->isLoad() )
      {
         goto done ;
      }

      if ( value < minV )
      {
         ossPrintf( "Waring: Field[%s] value[%u] is less than min value[%u]\n",
                    _curFieldName.c_str(), value, minV ) ;
         if ( autoAdjust )
         {
            value = minV ;
         }
         else
         {
            _result = SDB_INVALIDARG ;
            goto error ;
         }
      }
      else if ( value > maxV )
      {
         ossPrintf( "Waring: Field[%s] value[%u] is more than max value[%u]\n",
                 _curFieldName.c_str(), value, maxV ) ;
         if ( autoAdjust )
         {
            value = maxV ;
         }
         else
         {
            _result = SDB_INVALIDARG ;
            goto error ;
         }
      }

   done:
      return _result ;
   error:
      goto done ;
   }

   INT32 _pmdCfgRecord::rdvMinMax( pmdCfgExchange *pEX, UINT16 &value,
                                   UINT16 minV, UINT16 maxV,
                                   BOOLEAN autoAdjust )
   {
      if ( _result )
      {
         goto error ;
      }

      if ( !pEX->isLoad() )
      {
         goto done ;
      }

      if ( value < minV )
      {
         ossPrintf( "Waring: Field[%s] value[%u] is less than min value[%u]\n",
                    _curFieldName.c_str(), value, minV ) ;
         if ( autoAdjust )
         {
            value = minV ;
         }
         else
         {
            _result = SDB_INVALIDARG ;
            goto error ;
         }
      }
      else if ( value > maxV )
      {
         ossPrintf( "Waring: Field[%s] value[%u] is more than max value[%u]\n",
                    _curFieldName.c_str(), value, maxV ) ;
         if ( autoAdjust )
         {
            value = maxV ;
         }
         else
         {
            _result = SDB_INVALIDARG ;
            goto error ;
         }
      }

   done:
      return _result ;
   error:
      goto done ;
   }

   INT32 _pmdCfgRecord::rdvMaxChar( pmdCfgExchange *pEX, CHAR *pValue,
                                    UINT32 maxChar, BOOLEAN autoAdjust )
   {
      UINT32 len = 0 ;

      if ( _result )
      {
         goto error ;
      }
      if ( !pEX->isLoad() )
      {
         goto done ;
      }

      len = ossStrlen( pValue ) ;
      if ( len > maxChar )
      {
         ossPrintf( "Waring: Field[%s] value[%s] length more than [%u]\n",
                    _curFieldName.c_str(), pValue, maxChar ) ;
         if ( autoAdjust )
         {
            pValue[ maxChar ] = 0 ;
         }
         else
         {
            _result = SDB_INVALIDARG ;
            goto error ;
         }
      }

   done:
      return _result ;
   error:
      goto done ;
   }

   INT32 _pmdCfgRecord::rdvNotEmpty( pmdCfgExchange * pEX, CHAR *pValue )
   {
      if ( _result )
      {
         goto error ;
      }
      if ( !pEX->isLoad() )
      {
         goto done ;
      }

      if ( ossStrlen( pValue ) == 0 )
      {
         ossPrintf( "Waring: Field[%s] is empty\n", _curFieldName.c_str() ) ;
         _result = SDB_INVALIDARG ;
         goto error ;
      }

   done:
      return _result ;
   error:
      goto done ;
   }

   /*
      _pmdOptionsMgr implement
   */
   _pmdOptionsMgr::_pmdOptionsMgr()
   {
      // rdx members
      ossMemset( _krcbDbPath, 0, OSS_MAX_PATHSIZE + 1 ) ;
      ossMemset( _krcbIndexPath, 0, OSS_MAX_PATHSIZE + 1 ) ;
      ossMemset( _krcbDiagLogPath, 0, OSS_MAX_PATHSIZE + 1 ) ;
      ossMemset( _krcbLogPath, 0, OSS_MAX_PATHSIZE + 1 ) ;
      ossMemset( _krcbBkupPath, 0, OSS_MAX_PATHSIZE + 1 ) ;
      ossMemset( _krcbSvcName, 0, OSS_MAX_SERVICENAME + 1) ;
      ossMemset( _replServiceName, 0, OSS_MAX_SERVICENAME + 1) ;
      ossMemset( _catServiceName, 0, OSS_MAX_SERVICENAME + 1) ;
      ossMemset( _shardServiceName, 0, OSS_MAX_SERVICENAME + 1) ;
      ossMemset( _restServiceName, 0, OSS_MAX_SERVICENAME + 1) ;
      ossMemset( _krcbRole, 0, PMD_MAX_ENUM_STR_LEN + 1) ;
      ossMemset( _catAddrLine, 0, OSS_MAX_PATHSIZE + 1) ;
      ossMemset( _dmsTmpBlkPath, 0, OSS_MAX_PATHSIZE + 1 ) ;

      _krcbMaxPool         = 0 ;
      _krcbDiagLvl         = (UINT16)PDWARNING ;
      _logFileSz           = 0 ;
      _logFileNum          = 0 ;
      _numPreLoaders       = 0 ;
      _maxPrefPool         = PMD_MAX_PREF_POOL ;
      _maxSubQuery         = PMD_MAX_SUB_QUERY ;
      _maxReplSync         = PMD_DEFAULT_MAX_REPLSYNC ;
      _replBucketSize      = PMD_DFT_REPL_BUCKET_SIZE ;
      _memDebugEnabled     = FALSE ;
      _memDebugSize        = 0 ;
      _indexScanStep       = PMD_DFT_INDEX_SCAN_STEP ;
      _dpslocal            = FALSE ;
      _traceOn             = FALSE ;
      _traceBufSz          = TRACE_DFT_BUFFER_SIZE ;
      _transactionOn       = FALSE ;
      _sharingBreakTime    = PMD_OPTION_BRK_TIME_DEFAULT ;
      _logBuffSize         = DPS_DFT_LOG_BUF_SZ ;
      _sortBufSz           = PMD_DEFAULT_SORTBUF_SZ ;
      _hjBufSz             = PMD_DEFAULT_HJ_SZ ;

      // other configs
      ossMemset( _krcbConfPath, 0, OSS_MAX_PATHSIZE + 1 ) ;
      for ( UINT32 i = 0; i < PMD_MAX_CATLOG_NUM; ++i )
      {
         ossMemset( _cat[i]._host, 0, OSS_MAX_HOSTNAME + 1 ) ;
         ossMemset( _cat[i]._service, 0, OSS_MAX_SERVICENAME + 1 ) ;
      }
      _krcbSvcPort         = PMD_DFT_SVCPORT ;

      _groupID             = 0 ;
      _nodeID              = 0 ;
   }

   _pmdOptionsMgr::~_pmdOptionsMgr()
   {
   }

   INT32 _pmdOptionsMgr::doDataExchange( pmdCfgExchange * pEX )
   {
      resetResult () ;

      // map configs begin
      // --confpath
      rdxPath( pEX, PMD_OPTION_CONFPATH , _krcbConfPath, sizeof(_krcbConfPath),
               FALSE, FALSE, PMD_CURRENT_PATH ) ;
      // --dbpath
      rdxPath( pEX, PMD_OPTION_DBPATH, _krcbDbPath, sizeof(_krcbDbPath),
               FALSE, FALSE, PMD_CURRENT_PATH ) ;
      rdvNotEmpty( pEX, _krcbDbPath ) ;
      // --indexpath
      rdxPath( pEX, PMD_OPTION_IDXPATH, _krcbIndexPath, sizeof(_krcbIndexPath),
               FALSE, FALSE, "" ) ;
      // --diagpath
      rdxPath( pEX, PMD_OPTION_DIAGLOGPATH, _krcbDiagLogPath,
               sizeof(_krcbDiagLogPath), FALSE, FALSE, "" ) ;
      // --logpath
      rdxPath( pEX, PMD_OPTION_LOGPATH, _krcbLogPath, sizeof(_krcbLogPath),
               FALSE, FALSE, "" ) ;
      // --bkuppath
      rdxPath( pEX, PMD_OPTION_BKUPPATH, _krcbBkupPath, sizeof(_krcbBkupPath),
               FALSE, FALSE, "" ) ;
      // --maxpool
      rdxUInt( pEX, PMD_OPTION_MAXPOOL, _krcbMaxPool, FALSE, TRUE, 0 ) ;
      // --svcname
      rdxString( pEX, PMD_OPTION_SVCNAME, _krcbSvcName, sizeof(_krcbSvcName),
                 FALSE, FALSE,
                 boost::lexical_cast<string>(PMD_DFT_SVCPORT).c_str() ) ;
      rdvNotEmpty( pEX, _krcbSvcName ) ;
      // --replname
      rdxString( pEX, PMD_OPTION_REPLNAME, _replServiceName,
                 sizeof(_replServiceName), FALSE, FALSE, "" ) ;
      // --catalogname
      rdxString( pEX, PMD_OPTION_CATANAME, _catServiceName,
                 sizeof(_catServiceName), FALSE, FALSE, "" ) ;
      // --shardname
      rdxString( pEX, PMD_OPTION_SHARDNAME, _shardServiceName,
                 sizeof(_shardServiceName), FALSE, FALSE, "" ) ;
      // --restname
      rdxString( pEX, PMD_OPTION_RESTNAME, _restServiceName,
                 sizeof(_restServiceName), FALSE, FALSE, "" ) ;
      // --diaglevel
      rdxUShort( pEX, PMD_OPTION_DIAGLEVEL, _krcbDiagLvl, FALSE, TRUE,
                 (UINT16)PDWARNING ) ;
      rdvMinMax( pEX, _krcbDiagLvl, PDSEVERE, PDDEBUG, TRUE ) ;
      // --role
      rdxString( pEX, PMD_OPTION_ROLE, _krcbRole, sizeof(_krcbRole),
                 FALSE, FALSE, PMD_KRCB_ROLE_STANDALONE ) ;
      rdvNotEmpty( pEX, _krcbRole ) ;
      // --logfilesz
      rdxUInt( pEX, PMD_OPTION_LOGFILESZ, _logFileSz, FALSE, FALSE,
               PMD_DFT_LOG_FILE_SZ ) ;
      rdvMinMax( pEX, _logFileSz, PMD_MIN_LOG_FILE_SZ, PMD_MAX_LOG_FILE_SZ,
                 TRUE ) ;
      // --logfilenum
      rdxUInt( pEX, PMD_OPTION_LOGFILENUM, _logFileNum, FALSE, FALSE,
               PMD_DFT_LOG_FILE_NUM ) ;
      rdvMinMax( pEX, _logFileNum, 1, 60000, TRUE ) ;
      // --logbuffsize
      rdxUInt( pEX, PMD_OPTION_LOGBUFFSIZE, _logBuffSize, FALSE, FALSE,
               DPS_DFT_LOG_BUF_SZ ) ;
      rdvMinMax( pEX, _logBuffSize, 512, 1024000, TRUE ) ;
      // --numpreload
      rdxUInt( pEX, PMD_OPTION_NUMPRELOAD, _numPreLoaders, FALSE, TRUE, 0 ) ;
      rdvMinMax( pEX, _numPreLoaders, 0, 100, TRUE ) ;
      // --maxprefpool
      rdxUInt( pEX, PMD_OPTION_MAX_PREF_POOL, _maxPrefPool, FALSE, TRUE,
               PMD_MAX_PREF_POOL ) ;
      rdvMinMax( pEX, _maxPrefPool, 0, 1000, TRUE ) ;
      // --maxsubquery
      rdxUInt( pEX, PMD_OPTION_MAX_SUB_QUERY, _maxSubQuery, FALSE, TRUE,
               PMD_MAX_SUB_QUERY ) ;
      rdvMinMax( pEX, _maxSubQuery, 0, _maxPrefPool, TRUE ) ;
      // --maxreplsync
      rdxUInt( pEX, PMD_OPTION_MAX_REPL_SYNC, _maxReplSync, FALSE, TRUE,
               PMD_DEFAULT_MAX_REPLSYNC ) ;
      rdvMinMax( pEX, _maxReplSync, 0, 200, TRUE ) ;
      // --replbucketsize
      rdxUInt( pEX, PMD_OPTION_REPL_BUCKET_SIZE, _replBucketSize, FALSE, FALSE,
               PMD_DFT_REPL_BUCKET_SIZE, TRUE ) ;
      rdvMinMax( pEX, _replBucketSize, 1, 4096, TRUE ) ;
      // --memdebug
      rdxBooleanS( pEX, PMD_OPTION_MEMDEBUG, _memDebugEnabled, FALSE, TRUE,
                   FALSE, TRUE ) ;
      // --memdebugsize
      rdxUInt( pEX, PMD_OPTION_MEMDEBUGSIZE, _memDebugSize, FALSE, TRUE, 0,
               TRUE ) ;
      // --indexscanstep
      rdxUInt( pEX, PMD_OPTION_INDEX_SCAN_STEP, _indexScanStep, FALSE, TRUE,
               PMD_DFT_INDEX_SCAN_STEP, TRUE ) ;
      rdvMinMax( pEX, _indexScanStep, 1, 10000, TRUE ) ;
      // --dpslocal
      rdxBooleanS( pEX, PMD_OPTION_DPSLOCAL, _dpslocal, FALSE, TRUE, FALSE,
                   TRUE ) ;
      // --traceOn
      rdxBooleanS( pEX, PMD_OPTION_TRACEON, _traceOn, FALSE, TRUE, FALSE,
                   TRUE ) ;
      // --traceBufSz
      rdxUInt( pEX, PMD_OPTION_TRACEBUFSZ, _traceBufSz, FALSE, TRUE,
               TRACE_DFT_BUFFER_SIZE, TRUE ) ;
      rdvMinMax( pEX, _traceBufSz, TRACE_MIN_BUFFER_SIZE,
                 TRACE_MAX_BUFFER_SIZE, TRUE ) ;
      // --transactionOn
      rdxBooleanS( pEX, PMD_OPTION_TRANSACTIONON, _transactionOn, FALSE,
                   TRUE, FALSE ) ;
      // --sharingBreak
      rdxUInt( pEX, PMD_OPTION_SHARINGBRK, _sharingBreakTime, FALSE, TRUE,
               PMD_OPTION_BRK_TIME_DEFAULT, TRUE ) ;
      rdvMinMax( pEX, _sharingBreakTime, PMD_OPTION_BRK_TIME_DEFAULT,
                 300000, TRUE ) ;
      // --catalogaddr
      rdxString( pEX, PMD_OPTION_CATALOG_ADDR, _catAddrLine,
                 sizeof(_catAddrLine), FALSE, TRUE, "" ) ;

      // --dmsTmpBlkPath
      rdxPath( pEX, PMD_OPTION_DMS_TMPBLKPATH, _dmsTmpBlkPath,
               sizeof(_dmsTmpBlkPath), FALSE, FALSE, "", TRUE ) ;

      // --sortBufSz
      rdxUInt( pEX, PMD_OPTION_SORTBUF_SIZE, _sortBufSz,
               FALSE, TRUE, PMD_DEFAULT_SORTBUF_SZ, TRUE ) ;
      rdvMinMax( pEX, _sortBufSz, PMD_MIN_SORTBUF_SZ,
                 -1, TRUE ) ;

      // --hjBufSz
      rdxUInt( pEX, PMD_OPTION_HJ_BUFSZ, _hjBufSz,
               FALSE, TRUE, PMD_DEFAULT_HJ_SZ, TRUE ) ;
      rdvMinMax( pEX, _sortBufSz, PMD_MIN_HJ_SZ,
                 -1, TRUE ) ;

      // end map

      return getResult () ;
   }

   INT32 _pmdOptionsMgr::postLoaded ()
   {
      INT32 rc = SDB_OK ;

      ossSocket::getPort ( _krcbSvcName, _krcbSvcPort ) ;
      if ( 0 == _krcbSvcPort )
      {
         std::cerr << "Invalid svcname: " << _krcbSvcName << endl ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      // logbuffsize check
      if ( _logBuffSize > _logFileNum * _logFileSz *
           ( 1048576 / DPS_DEFAULT_PAGE_SIZE ) )
      {
         std::cerr << "log buff size more than all log file size: "
                   << _logFileNum * _logFileSz << "MB" << std::endl ;
         _logBuffSize = _logFileNum * _logFileSz *
                        ( 1048576 / DPS_DEFAULT_PAGE_SIZE ) ;
      }

      // dbrole check
      if ( 0 != ossStrcmp( PMD_KRCB_ROLE_DATA, _krcbRole ) &&
           0 != ossStrcmp( PMD_KRCB_ROLE_COORD, _krcbRole ) &&
           0 != ossStrcmp( PMD_KRCB_ROLE_AUTH, _krcbRole ) &&
           0 != ossStrcmp( PMD_KRCB_ROLE_STANDALONE, _krcbRole ) &&
           0 != ossStrcmp( PMD_KRCB_ROLE_CATALOG, _krcbRole ) )
      {
         std::cerr << "db role: " << _krcbRole << " error" << std::endl ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      // replbucketsize check
      if ( !ossIsPowerOf2( _replBucketSize ) )
      {
         _replBucketSize = PMD_DFT_REPL_BUCKET_SIZE ;
      }

      if ( 0 == ossStrlen( _replServiceName ) )
      {
         string port = boost::lexical_cast<string>( _krcbSvcPort
                                                    + PMD_REPL_PORT ) ;
         ossMemcpy( _replServiceName, port.c_str(), port.size() ) ;
      }
      if ( 0 == ossStrlen( _shardServiceName ) )
      {
         string port = boost::lexical_cast<string>( _krcbSvcPort
                                                    + PMD_SHARD_PORT ) ;
         ossMemcpy( _shardServiceName, port.c_str(), port.size() ) ;
      }
      if ( 0 == ossStrlen( _catServiceName ) )
      {
         string port = boost::lexical_cast<string>( _krcbSvcPort
                                                    + PMD_CAT_PORT ) ;
         ossMemcpy( _catServiceName, port.c_str(), port.size() ) ;
      }
      if ( 0 == ossStrlen( _restServiceName ) )
      {
         string port = boost::lexical_cast<string>( _krcbSvcPort
                                                    + PMD_REST_PORT ) ;
         ossMemcpy( _restServiceName, port.c_str(), port.size() ) ;
      }

      if ( 0 == _krcbIndexPath[0] )
      {
         ossStrcpy( _krcbIndexPath, _krcbDbPath ) ;
      }
      if ( 0 == _krcbDiagLogPath[0] )
      {
         if ( SDB_OK != _joinDir( _krcbDbPath, PMD_OPTION_DIAG_PATH,
                                  _krcbDiagLogPath, OSS_MAX_PATHSIZE ) )
         {
            std::cerr << "diaglog path is too long!" << endl ;
            rc = SDB_INVALIDPATH ;
            goto error ;
         }
      }
      if ( 0 == _krcbLogPath[0] )
      {
         if ( SDB_OK != _joinDir( _krcbDbPath, PMD_OPTION_LOG_PATH,
                                  _krcbLogPath, OSS_MAX_PATHSIZE ) )
         {
            std::cerr << "repicalog path is too long!" << endl ;
            rc = SDB_INVALIDPATH ;
            goto error ;
         }
      }
      if ( 0 == _krcbBkupPath[0] )
      {
         if ( SDB_OK != _joinDir( _krcbDbPath, PMD_OPTION_BK_PATH,
                                  _krcbBkupPath, OSS_MAX_PATHSIZE ) )
         {
            std::cerr << "bakup path is too long!" << endl ;
            rc = SDB_INVALIDPATH ;
            goto error ;
         }
      }

      if ( 0 == _dmsTmpBlkPath[0] )
      {
         if ( SDB_OK != _joinDir( _krcbDbPath, PMD_OPTION_TMPBLK_PATH,
                                  _dmsTmpBlkPath, OSS_MAX_PATHSIZE ) )
         {
            std::cerr << "diaglog path is too long!" << endl ;
            rc = SDB_INVALIDPATH ;
            goto error ;
         }
      }
      else
      {
         if ( 0 == ossStrcmp( _krcbDbPath, _dmsTmpBlkPath))
         {
            std::cerr << "tmp path and data path should not be the same" << endl ;
            rc = SDB_INVALIDPATH ;
            goto error ;
         }
         if ( 0 == ossStrcmp( _krcbIndexPath, _dmsTmpBlkPath))
         {
            std::cerr << "tmp path and index path should not be the same" << endl ;
            rc = SDB_INVALIDPATH ;
            goto error ;
         }
         if ( 0 == ossStrcmp( _krcbLogPath, _dmsTmpBlkPath))
         {
            std::cerr << "tmp path and log path should not be the same" << endl ;
            rc = SDB_INVALIDPATH ;
            goto error ;
         }
         if ( 0 == ossStrcmp( _krcbBkupPath, _dmsTmpBlkPath))
         {
            std::cerr << "tmp path and bkup path should not be the same" << endl ;
            rc = SDB_INVALIDPATH ;
            goto error ;
         }
         if ( 0 == ossStrcmp( _krcbDiagLogPath, _dmsTmpBlkPath))
         {
            std::cerr << "tmp path and diaglog path should not be the same" << endl ;
            rc = SDB_INVALIDPATH ;
            goto error ;
         }
      }

      if ( _memDebugSize != 0 )
      {
         _memDebugSize = OSS_MIN ( _memDebugSize, SDB_MEMDEBUG_MAXGUARDSIZE ) ;
         _memDebugSize = OSS_MAX ( _memDebugSize, SDB_MEMDEBUG_MINGUARDSIZE ) ;
      }

      rc = _parseCatAddr () ;
      if ( rc )
      {
         goto error ;
      }

      if ( _traceOn && _traceBufSz != 0 )
      {
         pmdGetKRCB()->getTraceCB()->start ( (UINT64)_traceBufSz, 0xFFFFFFFF ) ;
      }

      rc = _mkdir() ;
      if ( rc )
      {
         goto error ;
      }

      // enforced all config
      rc = setKrcb( pmdGetKRCB() ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to enforce all configs, rc: %d", rc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdOptionsMgr::_parseCatAddr ()
   {
      INT32 rc = SDB_OK ;
      vector<string> addrs ;
      UINT32 i = 0 ;

      boost::algorithm::split( addrs, _catAddrLine,
                               boost::algorithm::is_any_of(
                               PMD_OPTION_CATALOG_ADDR_SPLITER ) ) ;
      if ( PMD_MAX_CATLOG_NUM < addrs.size() )
      {
         std::cerr << "catalog addr more than max cat number" << endl ;
         goto error ;
      }

      for ( vector<string>::iterator itr = addrs.begin() ;
            itr != addrs.end() ;
            ++itr )
      {
         vector<string> pair ;
         string tmp = *itr ;
         boost::algorithm::trim( tmp ) ;
         boost::algorithm::split( pair, tmp,
                                  boost::algorithm::is_any_of(
                                  PMD_OPTION_CATALOG_ADDR_FIELD_SPLITER ) ) ;
         if ( pair.size() != 2 )
         {
            continue ;
         }
         UINT32 cpLen = pair.at(0).size() < OSS_MAX_HOSTNAME ?
                        pair.at(0).size() : OSS_MAX_HOSTNAME ;
         ossMemcpy( _cat[i]._host, pair.at(0).c_str(), cpLen ) ;
         (_cat[i]._host)[cpLen] = '\0' ;
         cpLen = pair.at(1).size() < OSS_MAX_SERVICENAME ?
                 pair.at(1).size() : OSS_MAX_SERVICENAME ;
         ossMemcpy( _cat[i]._service, pair.at(1).c_str(),cpLen ) ;
         (_cat[i]._service)[cpLen] = '\0' ;
         ++i ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdOptionsMgr::preSaving ()
   {
      UINT32 count = 0 ;
      stringstream ss ;
      for ( UINT32 i = 0; i < PMD_MAX_CATLOG_NUM ; ++i )
      {
         if ( '\0' != _cat[i]._host[0] )
         {
            if ( 0 != count )
            {
               ss << "," ;
            }
            ss << _cat[i]._host << ":" << _cat[i]._service ;
            ++count ;
         }
         else
         {
            break ;
         }
      }
      string catAddr = ss.str() ;
      ossStrncpy( _catAddrLine, catAddr.c_str(), OSS_MAX_PATHSIZE ) ;
      _catAddrLine[ OSS_MAX_PATHSIZE ] = 0 ;

      return SDB_OK ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDOPTMGR_INIT, "_pmdOptionsMgr::init" )
   INT32 _pmdOptionsMgr::init( INT32 argc, CHAR **argv )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB__PMDOPTMGR_INIT ) ;

      CHAR cfgTempPath[ OSS_MAX_PATHSIZE + 1 ] = {0} ;
      po::options_description all ( "Command options" ) ;
      po::options_description display ( "Command options (display)" ) ;
      po::variables_map vm ;
      po::variables_map vm2 ;

      PMD_ADD_PARAM_OPTIONS_BEGIN( all )
         PMD_COMMANDS_OPTIONS
         PMD_HIDDEN_COMMANDS_OPTIONS
      PMD_ADD_PARAM_OPTIONS_END

      PMD_ADD_PARAM_OPTIONS_BEGIN( display )
         PMD_COMMANDS_OPTIONS
      PMD_ADD_PARAM_OPTIONS_END

      rc = readCmd( argc, argv, all, vm );
      JUDGE_RC( rc )

      /// read cmd first
      if ( vm.count( PMD_OPTION_HELP ) )
      {
         std::cout << display << std::endl ;
         rc = SDB_PMD_HELP_ONLY ;
         goto done ;
      }
      if ( vm.count( PMD_OPTION_VERSION ) )
      {
         INT32 version, subVersion, release ;
         const CHAR *pBuild = NULL ;
         ossGetVersion ( &version, &subVersion, &release, &pBuild ) ;
         std::cout << "SequoiaDB version: " << version << "."
         << subVersion << std::endl ;
         std::cout << "Release: " << release << std::endl ;
         std::cout << "Build: " << pBuild <<std::endl ;
         rc = SDB_PMD_VERSION_ONLY ;
         goto done ;
      }

      // --confpath
      if ( vm.count( PMD_OPTION_CONFPATH ) )
      {
         CHAR *p = ossGetRealPath( vm[PMD_OPTION_CONFPATH].as<string>().c_str(),
                                   cfgTempPath, OSS_MAX_PATHSIZE ) ;
         if ( NULL == p )
         {
            std::cerr << "ERROR: Failed to get real path for "
                      <<  vm[PMD_OPTION_CONFPATH].as<string>() << endl ;
            rc = SDB_INVALIDPATH ;
            goto error;
         }
      }
      else
      {
         CHAR *p = ossGetRealPath( PMD_CURRENT_PATH, cfgTempPath,
                                   OSS_MAX_PATHSIZE ) ;
         if ( NULL == p )
         {
            SDB_ASSERT( FALSE, "impossible" ) ;
            rc = SDB_INVALIDPATH ;
            goto error ;
         }
      }

      rc = readConfigureFile( cfgTempPath, all, vm2 ) ;
      if ( SDB_OK != rc )
      {
         //if user set  configure file,  but cann't read the configure file.
         //then we should exit.
         if ( vm.count( PMD_OPTION_CONFPATH ) )
         {
            std::cerr << "Read config file: " << cfgTempPath
                      << " failed, rc: " << rc << std::endl ;
            goto error ;
         }
         // we should avoid exit when config file is not found or I/O error
         // we should continue run but use other arguments that uses input
         // from command line
         else if ( SDB_IO != rc )
         {
            std::cerr << "Read default config file: " << cfgTempPath
                      << " failed, rc: " << rc << std::endl ;
            goto error ;
         }
         else
         {
            std::cout << "Using default config" << std::endl ;
            rc = SDB_OK ;
         }
      }

      rc = pmdCfgRecord::init( &vm2, &vm ) ;
      if ( rc )
      {
         goto error ;
      }
      else
      {
         ossStrcpy( _krcbConfPath, cfgTempPath ) ;
         BSONObj confObj ;
         pmdCfgRecord::toBSON( confObj ) ;
         PD_LOG( PDEVENT, "All configs: %s", confObj.toString().c_str() ) ;
      }

   done:
      PD_TRACE_EXITRC ( SDB__PMDOPTMGR_INIT, rc );
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDOPTMGR__MKDIR, "_pmdOptionsMgr::_mkdir" )
   INT32 _pmdOptionsMgr::_mkdir()
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__PMDOPTMGR__MKDIR );

      rc = ossMkdir( _krcbDbPath, OSS_CREATE|OSS_READWRITE ) ;
      if ( rc && SDB_FE != rc )
      {
         std::cerr << "Failed to create database dir: " << _krcbDbPath <<
                      ", rc = " << rc << std::endl ;
         goto error ;
      }
      rc = ossMkdir( _krcbIndexPath, OSS_CREATE|OSS_READWRITE ) ;
      if ( rc && SDB_FE != rc )
      {
         std::cerr << "Failed to create index dir: " << _krcbIndexPath <<
                      ", rc = " << rc << std::endl ;
         goto error ;
      }
      rc = ossMkdir( _krcbDiagLogPath, OSS_CREATE|OSS_READWRITE ) ;
      if ( rc && SDB_FE != rc )
      {
         std::cerr << "Failed to create diaglog dir: " << _krcbDiagLogPath <<
                      ", rc = " << rc << std::endl ;
         goto error ;
      }

      rc = ossMkdir( _krcbLogPath, OSS_CREATE|OSS_READWRITE ) ;
      if ( rc && SDB_FE != rc )
      {
         std::cerr << "Failed to create log dir: " << _krcbLogPath <<
                      ", rc = " << rc << std::endl ;
         goto error ;
      }

      rc = ossMkdir( _krcbBkupPath, OSS_CREATE|OSS_READWRITE ) ;
      if ( rc && SDB_FE != rc )
      {
         PD_LOG ( PDERROR, "Failed to create backup dir: %s, rc = %d",
                  _krcbBkupPath, rc ) ;
         goto error ;
      }

      rc = ossMkdir( _dmsTmpBlkPath, OSS_CREATE|OSS_READWRITE ) ;
      if ( rc && SDB_FE != rc )
      {
         PD_LOG ( PDERROR, "Failed to create tmp dir: %s, rc = %d",
                  _krcbBkupPath, rc ) ;
         goto error ;
      }

      rc = SDB_OK ;
   done:
      PD_TRACE_EXITRC ( SDB__PMDOPTMGR__MKDIR, rc );
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDOPTMGR_RDCMD, "_pmdOptionsMgr::readCmd" )
   INT32 _pmdOptionsMgr::readCmd( INT32 argc, CHAR **argv,
                                  po::options_description &desc,
                                  po::variables_map &vm )
   {
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB__PMDOPTMGR_RDCMD );

      try
      {
         po::store ( po::command_line_parser( argc, argv).options(
                     desc ).allow_unregistered().run(), vm ) ;
         po::notify ( vm ) ;
      }
      catch ( po::unknown_option &e )
      {
         std::cerr <<  "Unknown argument: "
                   << e.get_option_name () << std::endl ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      catch ( po::invalid_option_value &e )
      {
         std::cerr <<  "Invalid argument: "
                   << e.get_option_name () << std::endl ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      catch( po::error &e )
      {
         std::cerr << e.what () << std::endl ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC ( SDB__PMDOPTMGR_RDCMD, rc );
      return rc;
   error:
      goto done;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDOPTMGR_RDCONFFILE, "_pmdOptionsMgr::readConfigureFile" )
   INT32 _pmdOptionsMgr::readConfigureFile( const CHAR *path,
                                            po::options_description &desc,
                                            po::variables_map &vm )
   {
      SDB_ASSERT( NULL != path, "path should not be NULL" )
      INT32 rc = SDB_OK;
      PD_TRACE_ENTRY ( SDB__PMDOPTMGR_RDCONFFILE );
      CHAR conf[OSS_MAX_PATHSIZE + 1] = {0} ;

      rc = pmdBuildFullPath( path, CONFIG_FILE_NAME,
                             OSS_MAX_PATHSIZE, conf ) ;
      if ( SDB_OK != rc )
      {
         std::cerr << "configure file path is too long!" << std::endl ;
         goto error;
      }

      try
      {
         po::store ( po::parse_config_file<char> ( conf, desc, TRUE ), vm ) ;
         po::notify ( vm ) ;
      }
      catch( po::reading_file )
      {
         std::cerr << "Failed to open config file: "
                   <<( std::string ) conf << std::endl;
         rc = SDB_IO ;
         goto error ;
      }
      catch ( po::unknown_option &e )
      {
         std::cerr << "Unknown config element: "
                   << e.get_option_name () << std::endl ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      catch ( po::invalid_option_value &e )
      {
         std::cerr << ( std::string ) "Invalid config element: "
                   << e.get_option_name () << std::endl ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      catch( po::error &e )
      {
         std::cerr << e.what () << std::endl ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC ( SDB__PMDOPTMGR_RDCONFFILE, rc );
      return rc;
   error:
      goto done;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDOPTMGR_REFLUSH2FILE, "_pmdOptionsMgr::reflush2file" )
   INT32 _pmdOptionsMgr::reflush2File()
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY (SDB__PMDOPTMGR_REFLUSH2FILE ) ;
      std::string line ;
      _OSS_FILE file ;
      CHAR conf[ OSS_MAX_PATHSIZE + 1 ] = {0} ;
      BOOLEAN opened = FALSE ;

      rc = pmdCfgRecord::toString( line ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to get the line str:%d", rc ) ;
         goto error ;
      }

      rc = pmdBuildFullPath( _krcbConfPath, CONFIG_FILE_NAME,
                             OSS_MAX_PATHSIZE, conf ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to build full path of configure file, "
                 "rc: %d", rc ) ;
         goto error;
      }

      /// TODO: backup first. rollback when failed flush file.
      rc = ossOpen ( conf, OSS_READWRITE|OSS_SHAREWRITE|OSS_REPLACE,
                     OSS_RWXU, file ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to open file[%s]:%d", conf, rc ) ;
         goto error ;
      }

      opened = TRUE ;

      {
         SINT64 written = 0 ;
         SINT64 len = line.size() ;
         while ( 0 < len )
         {
            SINT64 tmpWritten = 0 ;
            rc = ossWrite( &file, line.c_str() + written , len, &tmpWritten ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "Failed to write file[%s]:%d", conf, rc ) ;
               goto error ;
            }
            written += tmpWritten ;
            len -= tmpWritten ;
         }
      }

   done:
      if ( opened )
      {
         ossClose( file ) ;
      }
      PD_TRACE_EXIT ( SDB__PMDOPTMGR_REFLUSH2FILE) ;
      return rc ;
   error:
      if ( opened )
      {
         ossClose( file ) ;
         ossDelete( conf ) ;
         opened = FALSE ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMGOPTMGR_SETKRCB, "_pmdOptionsMgr::setKrcb" )
   INT32 _pmdOptionsMgr:: setKrcb( _SDB_KRCB *krcb )
   {
      SDB_ASSERT( NULL != krcb, "krcb should not be NULL" )
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__PMGOPTMGR_SETKRCB ) ;
      krcb->enforceDiagLevel( (PDLEVEL)_krcbDiagLvl ) ;
      rc = krcb->enforceDiagLogPath() ;
      JUDGE_RC( rc )
      rc = krcb->enforceConfPath() ;
      JUDGE_RC( rc )
      krcb->enforceDBRole( pmdGetRoleEnum(this->_krcbRole ) );
      krcb->enforceLogFileSz ( _logFileSz ) ;
      krcb->enforceLogFileNum ( _logFileNum ) ;

      _MsgRouteID id ;
      id.columns.groupID = _groupID ;
      id.columns.nodeID = _nodeID ;
      id.columns.serviceID = 0 ;
      {
         CHAR host[OSS_MAX_HOSTNAME+1] = {0} ;
         rc = ossSocket::getHostName( host, OSS_MAX_HOSTNAME ) ;
         JUDGE_RC( rc )

         krcb->enforceNodeInfo ( id, host ) ;
         krcb->enforceReplAddr ( MSG_ROUTE_REPL_SERVICE, _replServiceName ) ;
         krcb->enforceShardAddr ( MSG_ROUTE_SHARD_SERVCIE , _shardServiceName ) ;

         //catalog-service
         id.columns.serviceID = MSG_ROUTE_CAT_SERVICE ;
         krcb->enforceCatAddr( id, host, _catServiceName ) ;
      }

      {
         UINT32 catGID = PMD_ARTIFICIAL_CAT_GROUP_ID ;
         UINT16 catNID = PMD_ARTIFICIAL_CAT_NODE_ID ;
         for ( UINT32 i = 0; i < PMD_MAX_CATLOG_NUM ; i++ )
         {
            if ( 0 == ossStrlen( _cat[i]._host ) )
            {
               break ;
            }
            id.columns.groupID = catGID ;
            id.columns.nodeID = catNID++ ;
            id.columns.serviceID = MSG_ROUTE_CAT_SERVICE ;
            krcb->enforceCataLogGrpAddrs( id, _cat[i]._host,
                                          _cat[i]._service ) ;
         }
      }

      if ( krcb->getDBRole() == SDB_ROLE_DATA
         || krcb->getDBRole() == SDB_ROLE_STANDALONE )
      {
         krcb->getTransCB()->setTransSwitch( _transactionOn ) ;
      }

      ossMemDebugEnabled      = _memDebugEnabled ;
      ossMemDebugSize         = _memDebugSize ;

      krcb->getDPSCB()->setLogLocal( _dpslocal ) ;
      krcb->getBPSCB()->setNumPreLoads( _numPreLoaders ) ;
      krcb->getBPSCB()->setMaxPrefPool( _maxPrefPool ) ;
      CLS_SHARING_BRK_TIME    = _sharingBreakTime ;

      if ( krcb->getReplCB() )
      {
         krcb->getReplCB()->getBucket()->enforceMaxReplSync( _maxReplSync ) ;
      }

   done:
      PD_TRACE_EXITRC ( SDB__PMGOPTMGR_SETKRCB, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDOPTMGR__JNDIR, "_pmdOptionsMgr::_joinDir" )
   INT32 _pmdOptionsMgr::_joinDir( const CHAR *dir1, const CHAR *dir2,
                                   CHAR *path, INT32 size )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__PMDOPTMGR__JNDIR );
      INT32 dir1Len = 0 ;
      INT32 dir2Len = 0 ;

      if ( NULL == dir1 || NULL == dir2 ||
           NULL == path )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      dir1Len = ossStrlen( dir1 ) ;
      dir2Len = ossStrlen( dir2 ) ;
      if ( dir1Len + dir2Len + 1 > size )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      ossStrcpy( path, dir1 ) ;
      if ( dir1Len > 0 && 0 != ossStrcmp(&dir1[dir1Len-1],OSS_FILE_SEP) )
      {
         ossStrncat( path, OSS_FILE_SEP, 1 ) ;
      }
      ossStrncat( path, dir2, dir2Len ) ;

   done:
      PD_TRACE_EXITRC ( SDB__PMDOPTMGR__JNDIR, rc );
      return rc ;
   error:
      goto done ;
   }

   void _pmdOptionsMgr::clearCatAddr()
   {
      for ( UINT32 i = 0 ; i < PMD_MAX_CATLOG_NUM ; ++i )
      {
         _cat[i]._host[0] = '\0' ;
         _cat[i]._service[0] = '\0' ;
      }
   }

   void _pmdOptionsMgr::setCatAddr( const CHAR *host,
                                    const CHAR *service )
   {
      for ( UINT32 i = 0 ; i < PMD_MAX_CATLOG_NUM ; ++i )
      {
         if ( '\0' == _cat[i]._host[0] )
         {
            ossMemcpy( _cat[i]._host, host, ossStrlen(host) + 1 ) ;
            ossMemcpy( _cat[i]._service, service, ossStrlen(service)+1 ) ;
            break ;
         }
      }
   }


}


