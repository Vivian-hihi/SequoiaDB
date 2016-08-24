/*******************************************************************************


   Copyright (C) 2011-2016 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY ; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = sdbDataSource.cpp

   Descriptive Name = SDB Data Source Source File

   When/how to use: this program may be used on sequoiadb data source function.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
         06/30/2016   LXJ Initial Draft

   Last Changed =

*******************************************************************************/

#include "sdbDataSource.hpp"
#include "sdbDataSourceStrategy.hpp"
#include "client.hpp"
#include "sdbDataSourceWorker.hpp"
#include <algorithm>
#include "ossMem.hpp"
#include <iostream>

//#if defined (_DEBUG)
//#include <iostream>
//#endif

namespace sdbclient
{
   // sleep time:0.1s
   #define SDB_DS_SLEEP_TIME        100
   #define SDB_DS_TRYGETCONN_TIME   3
   
   void createConnFunc( void *args )
   {
      ((sdbDataSource*)args)->_createConn() ;
   }
   
   void destroyConnFunc( void *args )
   {
      ((sdbDataSource*)args)->_destroyConn() ;
   }

   void bgTaskFunc( void *args )
   {
      ((sdbDataSource*)args)->_bgTask() ;
   }

   sdbDataSource::~sdbDataSource()
   {
      disable() ;
      // clear strategy
      SAFE_OSS_DELETE(_strategy) ;
   }
   
   // init data source with url(hostname:port) and conf
   INT32 sdbDataSource::init( // TODO: it's better to call  "init(vector, 
                              //sdbDataSourceConf)" to realize
                              // TODO:DONE
      const std::string &url, 
      const sdbDataSourceConf &conf )
   {
      std::vector<std::string> vUrl ;
      vUrl.push_back( url ) ;
      
      return init( vUrl, conf ) ;
   }

   // init data source with url vector and conf
   INT32 sdbDataSource::init( 
      const std::vector<std::string> &vUrls,
      const sdbDataSourceConf &conf )
   {
      INT32 ret = SDB_OK ;
      int validUrlCnt = 0 ;

      if ( TRUE == _isInited )
      {
         goto done ;
      }

      _conf = conf ;
      // check confiture
      if ( !_conf.isValid() )
      {
         ret = SDB_INVALIDARG ;
         goto error ;
      }
      
      // new strategy instance
      ret = _buildStrategy();
      if ( SDB_OK != _buildStrategy() ) // TODO: it's better to return rc than 
                                        // a bool value
                                        // TODO: DONE
      {
         goto error ;
      }
      
      // check address vector
      for ( UINT32 i = 0 ; i < vUrls.size() ; ++i )
      {
         if ( _checkAddrArg( vUrls[i] ) )
         {   
            _strategy->addCoord( vUrls[i] ) ;
            validUrlCnt++ ;
         }
      }
      //if no address is valid, return SDB_INVALIDARG
      if ( 0 == validUrlCnt )
      {
         ret = SDB_INVALIDARG ;
         goto error ;
      }

      _isInited = TRUE ;
   done :
      return ret  ;
   error :
      SAFE_OSS_DELETE(_strategy) ;
      goto done  ;
   }
   
   // get idle connection number
   INT32 sdbDataSource::getIdleConnNum() const
   {
      return _idleSize.peek() ;
   }

   // get used connection number
   INT32 sdbDataSource::getUsedConnNum() const
   {
      return _busySize.peek() ;
   }

   // get the number of normal coord node
   INT32 sdbDataSource::getNormalCoordNum() const
   {
      SDB_ASSERT( _strategy, "_strategy is null" ) ;
      return _strategy->getNormalCoordNum() ;
   }

   // get the number of abnormal coord node
   INT32 sdbDataSource::getAbnormalCoordNum() const
   {  
      SDB_ASSERT( _strategy, "_strategy is null" ) ;
      return _strategy->getAbnormalCoordNum() ;
   }

   // get the number of local coord node
   INT32 sdbDataSource::getLocalCoordNum() const
   {
      SDB_ASSERT( _strategy, "_strategy is null" ) ;
      return _strategy->getLocalCoordNum() ;
   }
   
   // add a coord node
   void sdbDataSource::addCoord( const std::string &url )
   {
      // check address argument
      if ( _isInited && _checkAddrArg(url) )
      {
         SDB_ASSERT( _strategy, "_strategy is null" ) ;
         _strategy->addCoord( url ) ;
      }
   }

   // remove a coord node
   void sdbDataSource::removeCoord( const std::string &url )
   {
      // check address argument
      if ( _isInited && _checkAddrArg(url) )
      {
         SDB_ASSERT( _strategy, "_strategy is null" ) ;
         _strategy->removeCoord( url ) ;
      }
   }

   // enable data source, start background task
   INT32 sdbDataSource::enable()
   {
      INT32 ret = SDB_OK ;
      BOOLEAN isLocked = FALSE ;
      
      if ( _isInited )
      {
         if ( !_isEnabled )
         {
            _globalMutex.get() ;
            isLocked = TRUE ;
            if ( !_isEnabled )
            {
               _toStopWorkers = FALSE ;
               // create init number connection

               _createConnByNum( _conf.getInitConnCount() ) ; // TODO: tanzhaobo 
                                                   //think it's apposite or not
                                                   // TODO: DONE
               // start create connection thread
               _createConnWorker = SDB_OSS_NEW sdbDSWorker( 
                  createConnFunc, this ) ;
               if ( NULL == _createConnWorker )
               {
                  ret = SDB_OOM ;
                  goto error ;
               }
               // start destroy connection thread
               _destroyConnWorker = SDB_OSS_NEW sdbDSWorker( destroyConnFunc, 
                                                             this ) ;
               if ( NULL == _destroyConnWorker )
               {
                  ret = SDB_OOM ;
                  goto error ;
               }
               // start background task thread
               _bgTaskWorker= SDB_OSS_NEW sdbDSWorker( bgTaskFunc, this ) ;
               if ( NULL == _bgTaskWorker )
               {
                  ret = SDB_OOM ;
                  goto error ;
               }
               INT32 rc = SDB_OK ;
               rc = _createConnWorker->start() ;
               if ( SDB_OK != rc )
               {
                  ret = rc ;
                  goto error ;
               }
               rc = _destroyConnWorker->start() ;
               if ( SDB_OK != rc )
               {
                  ret = rc ;
                  goto error ;
               }
               rc = _bgTaskWorker->start() ;
               if ( SDB_OK != rc )
               {
                  ret = rc ;
                  goto error ;
               }

               _isEnabled = TRUE ;
            }
         }
      }
      else
      {
         ret = SDB_DS_NOTINIT_OR_DISABLED ;
      }
   done :
      if ( isLocked )
      {
         _globalMutex.release() ; // TODO: deadlock may happen, when program 
                                  //does not come here
                                  //TODO:DONE
      }
      return ret  ;
   error :
      SAFE_OSS_DELETE(_createConnWorker) ;
      SAFE_OSS_DELETE(_destroyConnWorker) ;
      SAFE_OSS_DELETE(_bgTaskWorker) ;
      goto done  ;
   }

   // disable data source, stop background task
   INT32 sdbDataSource::disable()
   {
      INT32 ret = SDB_OK ;
      BOOLEAN isLocked = FALSE ;
      
      if ( _isEnabled )
      {
         _globalMutex.get() ;
         isLocked = TRUE ;
         if ( _isEnabled )
         {
            _toStopWorkers = TRUE ;

            INT32 rc = SDB_OK ;
            // join
            rc = _createConnWorker->waitStop() ;// TODO: tanzhaobo, how can it 
                                                //stop the running thread??
                                                // TODO: DONE
            if ( SDB_OK != rc )
            {
               ret = rc ;
               goto error ;// TODO:tanzhaobo,  when it failed, we leave the 
                           //other backgroup thread alone?
            }
            rc = _destroyConnWorker->waitStop() ;
            if ( SDB_OK != rc )
            {
               ret = rc ;
               goto error ;
            }
            rc = _bgTaskWorker->waitStop() ;
            if ( SDB_OK != rc )
            {
               ret = rc ;
               goto error ;
            }

            // clear data source
            _clearDataSource() ;

            _toCreateConn = FALSE ;
            _toDestroyConn = FALSE ;
            _toStopWorkers = FALSE ;
            _isEnabled = FALSE ;
         }
      }
   done :
      if ( isLocked )
      {
         _globalMutex.release() ; // TODO: misuse
                                  //TODO:DONE
      }
      return ret  ;
   error :
      goto done  ;
   }

   // get a connection
   // TODO: you do not tell people what's the unit of "timeout", maybe you can
   // rename it like this "timeoutsec" or "seconds"
   // TODO: DONE
   INT32 sdbDataSource::getConnection( sdb*& conn, INT32 timeoutsec/* = 3*/ )
   {
      INT32 ret = SDB_OK ;
      BOOLEAN isGet = FALSE ;

      // TODO: need to check the input arguments? for example, conn should be 
      // null and timeout should >= 0, and when timeout < 0, set it to be 0, 
      // and it means no block ???
      // TODO:DONE
      if ( 0 > timeoutsec )
      {
         timeoutsec = 0 ;
      }
      
      if ( !_isEnabled )
      {
         ret = SDB_DS_NOTINIT_OR_DISABLED ;// TODO: before we call "close", "getConnection"
                                // can return a connection in java,
                                // but here, it behaves different in cpp
         goto error ;
      }
      
      while ( !isGet )
      {
         // if has idle connection
         if ( _idleSize.peek() > 0 )
         {
            isGet = _tryGetConn( conn ) ;
         }
         // if has no idle connection
         else
         {
            // if reach max count, wait for timeout, try again
            if ( ( (UINT32)_conf.getMaxCount() ) <= _busySize.peek() && 
               timeoutsec > 0 )
            {
               INT32 timeCnt = 0 ;
               while ( 1000*timeoutsec > timeCnt && !isGet )
               {
                  ossSleep( SDB_DS_SLEEP_TIME ) ;
                  timeCnt += SDB_DS_SLEEP_TIME ;
                  if ( _idleSize.peek() > 0 )
                  {
                     isGet = _tryGetConn( conn ) ;
                  }
               }
               if ( !isGet )
               {   
                  ret = SDB_DRIVER_DS_RUNOUT ;
                  goto error ;
               }
            }
            // not reach max count
            else
            {
               if ( 0 == _strategy->getNormalCoordNum() )
               {  
                  ret = SDB_DS_NO_COORD ;
                  goto error ;
               }
               else
               {
                  // TODO: when _idleSize == 0, we come here, but we create only
                  // one conn and nobody start the backgroup thread to create 
                  // connections.
                  // TODO: DONE
                  INT32 createNum = _createConnByNum(1);
                  _connMutex.get() ;
                  if ( (0 < createNum) && (0 <  _idleSize.peek()) )
                  {
                     
                     sdb* pConn = _idleList.front() ;
                     _idleList.pop_front() ;
                     _idleSize.dec() ;
                     _busyList.push_back( pConn ) ;
                     _busySize.inc() ;
                     _connMutex.release() ;
                     conn = pConn ;
                     _strategy->sync( pConn, ADDBUSYCONN ) ;
                     isGet = TRUE ;
                  }
                  else
                  {
                     _connMutex.release() ;
                  }
               }
            }
         }
         // if need pre create some connection
         if ( _idleSize.peek() <= SDB_DS_TOPRECREATE_THRESHOLD )
         {
            _toCreateConn = TRUE ;
            //ossSleep( SDB_DS_SLEEP_TIME ) ;// TODO: why we need to sleep 
            // here??
            // TODO: DONE
         }
      }
   done :
      return ret  ;
   error :
      goto done  ;   
   }

   // give back a connection
   void sdbDataSource::releaseConnection( sdb *conn )
   {
      // TODO:  in java, before we call "close", the current function
      // can release connections
      BOOLEAN              isLock = FALSE ;
      INT32                rc = SDB_OK ;
      sdb*                 tmp = NULL ;
      
      if ( _isEnabled )
      {
         std::list<sdb*>::iterator iter ;
         _connMutex.get() ; 
         isLock = TRUE ;
         if ( _isEnabled )
         {
            iter = std::find( _busyList.begin(), _busyList.end(), conn ) ;
            // if find it
            if ( iter != _busyList.end() )
            {
               tmp = *iter ;
               _busyList.erase( iter ) ;
               _busySize.dec() ;
               // if check keep time out, drop it
               if ( 0 != _conf.getKeepAliveTimeout() &&
                  _checkKeepAliveTimeOut( tmp ) )
               {
                  goto error ;
               }
               else
               {
                  // reset session attributions
                  bson::BSONObj obj ;
                  obj = BSON( "PreferedInstance" << "A" ) ;
                  rc = tmp->setSessionAttr( obj ) ;
                  if ( SDB_OK != rc )
                  {
                     goto error ;
                  }
                  // close all cursors
                  rc = tmp->closeAllCursors() ;
                  if ( SDB_OK != rc )
                  {
                     goto error ;
                  }
                  _idleList.push_back( tmp ) ;
                  _idleSize.inc() ;
                  _strategy->sync( tmp, ADDIDLECONN ) ;
               }
            }
         }
         
      }
   done :
      if ( TRUE == isLock )
      {
         _connMutex.release() ;
      }
      return ;
   error :
      _strategy->sync( tmp, DELBUSYCONN ) ;
      if ( tmp )
         tmp->disconnect() ;
      SAFE_OSS_DELETE( tmp ) ;
      goto done ;
   }

   // try to get a connection
   BOOLEAN sdbDataSource::_tryGetConn( sdb*& conn )
   {
      sdb* pConn ;
      BOOLEAN isGet = FALSE ;
      
      _connMutex.get() ;
      if ( 0 == _idleSize.peek() )
      {
         _connMutex.release() ;
         goto error ;
      }
      else
      {
         pConn = _idleList.front() ;
         _idleList.pop_front() ;
         _idleSize.dec() ;
         _busyList.push_back( pConn ) ;
         _busySize.inc() ;
         _connMutex.release() ;
         
         _strategy->sync( pConn, ADDBUSYCONN ) ;
      }
         
      // if check valid
      if ( _conf.getValidateConnection() )
      {
         // if not valid, destroy it
         if ( !pConn->isValid() )
         {
            std::list<sdb*>::iterator iter ;
               
            _connMutex.get() ; 
            iter = std::find(_busyList.begin(), _busyList.end(), pConn) ;
            // if find it
            if (iter != _busyList.end())
            {
               _busyList.erase(iter) ;
               _busySize.dec() ;
            }
            _connMutex.release() ;
               
            _strategy->sync( pConn, DELBUSYCONN ) ;
            if ( pConn )
               pConn->disconnect() ;
            SAFE_OSS_DELETE( pConn ) ; // TODO: nerver use delete
                                       // TODO: DONE
            goto error ;
         }
      }
      conn = pConn ;
      isGet = TRUE ;
   done :
      return isGet  ;
   error :
      isGet = FALSE ;
      goto done ;
   }

   // check address arguments, if valid, add it
   BOOLEAN sdbDataSource::_checkAddrArg( const std::string &url )
   {
      BOOLEAN rc = TRUE ;

      size_t pos = url.find_first_of( ":" ) ;
      size_t pos1 = url.find_last_of( ":" ) ;
      if ( std::string::npos == pos )
         rc = FALSE ;
      else if ( pos != pos1 )
         rc = FALSE ;
      
      return rc ;
   }

   // new a strategy with config
   INT32 sdbDataSource::_buildStrategy()
   {
      INT32 ret = SDB_OK ;
      
      if ( _strategy )
         SAFE_OSS_DELETE( _strategy ) ;
      switch( _conf.getConnectStrategy() )
      {
      case DS_STY_SERIAL:
         _strategy = SDB_OSS_NEW sdbDSSerialStrategy() ;
         break ;
      case DS_STY_RANDOM:
         _strategy = SDB_OSS_NEW sdbDSRandomStrategy() ;
         break ;
      case DS_STY_LOCAL:
         _strategy = SDB_OSS_NEW sdbDSLocalStrategy() ;
         break ;
      case DS_STY_BALANCE:
         _strategy = SDB_OSS_NEW sdbDSBalanceStrategy() ;
         break ;
      }
      if (NULL == _strategy)
      {
         ret = SDB_OOM ;
      }
      return ret ;
   }


   // clear data source
   void sdbDataSource::_clearDataSource()
   {
      if ( _createConnWorker )
      {
         SAFE_OSS_DELETE( _createConnWorker ) ;
      }   
      if ( _destroyConnWorker )
      {
         SAFE_OSS_DELETE( _destroyConnWorker ) ;
      }   
      if ( _bgTaskWorker )
      {
         SAFE_OSS_DELETE( _bgTaskWorker ) ;
      }
      
      // clear connection list
      std::list<sdbclient::sdb*>::const_iterator iter ;
      sdbclient::sdb* conn = NULL ;// TODO: init it to be NULL
                                   // TODO: DONE

      for ( iter = _idleList.begin() ; iter != _idleList.end() ; ++iter )
      {
         conn = *iter ;
         if ( conn )
         {
            conn->disconnect() ;
            SAFE_OSS_DELETE( conn ) ;
         }
      }  
      _idleList.clear() ;
      _idleSize.init( 0 ) ;

      for ( iter = _busyList.begin() ; iter != _busyList.end() ; ++iter )
      {
         conn = *iter ;
         if ( conn )
         {
            conn->disconnect() ;
            SAFE_OSS_DELETE( conn) ;
         }
      }
      _busyList.clear() ; 
      _busySize.init( 0 ) ;

      for ( iter = _destroyList.begin() ; iter != _destroyList.end() ; ++iter )
      {
         conn = *iter ;
         if (conn)
         {
            conn->disconnect() ;
            SAFE_OSS_DELETE( conn ) ;
         }
     }
      _destroyList.clear() ; 
   }


   // create connections function
   void sdbDataSource::_createConn()
   {
      while ( !_toStopWorkers )
      {
         while ( !_toCreateConn )
         {
            if ( _toStopWorkers )
               return ;
            ossSleep( SDB_DS_SLEEP_TIME ) ;
         }
         _connMutex.get() ;
         INT32 idleNum = _idleSize.peek() ;
         INT32 busyNum = _busySize.peek() ;
         _connMutex.release() ;
         INT32 maxNum = _conf.getMaxCount() ;
         INT32 createNum = 0 ;
         if ( idleNum + busyNum < maxNum )
         {
            INT32 incNum = _conf.getDeltaIncCount() ;
            if ( idleNum + busyNum + incNum < maxNum )
               createNum = incNum ;
            else
               createNum = maxNum - idleNum - busyNum ;
         }
         _createConnByNum( createNum ) ;         
         _toCreateConn = FALSE ;
      }
   }

   // create connection by a number
   INT32 sdbDataSource::_createConnByNum( INT32 num )
   {
      INT32 i = 0 ;
      INT32 rc = SDB_OK ; // TODO:  should init it
      sdb* conn = NULL ;
      while( i < num )
      {
         std::string coord ;
         // sdb is not inherit form SDBObject
         conn = new sdb( _conf.getUseSSL() ) ;// TODO: never use "new" in sdb
         // if no coord can be used
         if ( SDB_OK != _strategy->getNextCoord(coord) )
         {
            INT32  cnt = _checkAbnormalNodesCnt() ;
            // after check, if have not a normal node
            if ( 0 == cnt )
            {
               SAFE_OSS_DELETE( conn ) ;
               break ; // TODO: 1 not out coding style; 2 memory leak 
                       // TODO: DONE
            }
         }
         INT32 pos = coord.find_first_of( ":" ) ;
         rc = conn->connect( 
            coord.substr(0, pos).c_str(), 
            coord.substr(pos+1, coord.length()).c_str(), 
            _conf.getUserName().c_str(),
            _conf.getPasswd().c_str() ) ;

         // if connect failed
         if ( SDB_OK != rc )
         {
            INT32 retryTime = 0 ;
            BOOLEAN toBreak = FALSE ;
            while ( retryTime < SDB_DS_CREATECONN_RETRYTIME )
            {
               rc = conn->connect(
                  coord.substr( 0, pos ).c_str(), 
                  coord.substr( pos+1, coord.length() ).c_str(), 
                  _conf.getUserName().c_str(),
                  _conf.getPasswd().c_str() ) ;
               // retry failed
               if ( SDB_OK != rc )
               {
                  ++retryTime ;
                  ossSleep( SDB_DS_SLEEP_TIME ) ;
                  continue ;
               }
               // retry success
               else
               {
                  // add success
                  if ( _addNewConnSafely(conn, coord) )
                  {
                     ++i ;
                     //#if defined (_DEBUG)
                     //printCreateInfo(coord) ;
                     //#endif
                  }
                  // fail, may be reach max connection count
                  else
                  {
                     SAFE_OSS_DELETE( conn ) ;
                     toBreak = TRUE ;
                  }
                  break ;
               }
            }
            // after retry time, still failed
            if ( retryTime == SDB_DS_CREATECONN_RETRYTIME )
            {
               SAFE_OSS_DELETE( conn ) ;
               _strategy->mvCoordToAbnormal( coord ) ;
            }
            if ( toBreak )
            {
               break;
            }
         }
         // connect success
         else
         {
            if ( _addNewConnSafely(conn, coord) )
            {
               ++i ;
               //#if defined (_DEBUG)
               //printCreateInfo(coord) ;
               //#endif
            }
            // may be reach max connection count
            else
            {
               SAFE_OSS_DELETE( conn ) ;
               break ;
            }
         }
      }
      return i ;
   }

   // add new connection and make sure not reach max connection count
   BOOLEAN sdbDataSource::_addNewConnSafely(sdb *conn, 
                                            const std::string &coord )
   {
      BOOLEAN ret = FALSE ;
      
      INT32 maxNum = _conf.getMaxCount() ;
      
      _connMutex.get() ;
      INT32 idleNum = _idleSize.peek() ;
      INT32 busyNum = _busySize.peek() ;
      if ( idleNum + busyNum < maxNum )
      {
         _idleList.push_back( conn ) ;
         _idleSize.inc() ;
         _strategy->syncAddNewConn( conn, coord ) ;
         ret = TRUE ; 
      }
      _connMutex.release() ;

      return ret ;
   }
/*
#if defined (_DEBUG)
      void sdbDataSource::printCreateInfo(const std::string& coord)
      {
         std::cout << "create a connection at " << coord << std::endl ;
      }
#endif
*/
   //destroy connections function
   void sdbDataSource::_destroyConn()
   {
      while ( !_toStopWorkers )
      {
         while ( !_toDestroyConn )
         {
            if ( _toStopWorkers )
               return ;
            ossSleep( SDB_DS_SLEEP_TIME ) ;
         }
         sdb* conn ;
         _connMutex.get() ;
         while( !_destroyList.empty() )
         {
            conn = _destroyList.front() ;
            _destroyList.pop_front() ;
            if ( conn )
            {
               _strategy->sync( conn, DELIDLECONN ) ;
               conn->disconnect() ;
               SAFE_OSS_DELETE( conn ) ;
            }
         }
         _connMutex.release() ;
         _toDestroyConn = FALSE ;
      }
   }

   // background task function
   void sdbDataSource::_bgTask()
   {
      INT32 syncCoordInterval = _conf.getSyncCoordInterval() ;
      INT32 ckAbnormalInterval = SDB_DS_CHECKUNNORMALCOORD_INTERVAL ;
      INT32 ckConnInterval = _conf.getCheckInterval() ;
      INT32 syncCoordTimeCnt = 0 ;
      INT32 ckAbnormalTimeCnt = 0 ;
      INT32 ckConnTimeCnt = 0 ; 
      while ( !_toStopWorkers )
      {
         ossSleep( SDB_DS_SLEEP_TIME ) ;
         if ( 0 != syncCoordInterval )
            ++syncCoordTimeCnt ;
         ++ckAbnormalTimeCnt ;
         ++ckConnTimeCnt ;
         if ( 0 != syncCoordInterval )
         {
            if ( syncCoordInterval == syncCoordTimeCnt )
            {
               _syncCoordNodes() ;
               syncCoordTimeCnt = 0 ;
            }
         }
         if ( ckAbnormalInterval == ckAbnormalTimeCnt )
         {
            _checkAbnormalNodesCnt() ;
            ckAbnormalTimeCnt = 0 ;
         }
         if ( ckConnInterval == ckConnTimeCnt )
         {
            _checkMaxIdleConn() ;
            ckConnTimeCnt = 0 ;
         }

      }
   }

   // sync coord node info
   void sdbDataSource::_syncCoordNodes()
   {
      INT32 rc ;
      std::string tmp ;
      sdb conn ;
      rc = _strategy->getNextCoord( tmp ) ;
      if (SDB_OK != rc)
         return ;
      INT32 pos = tmp.find_first_of( ":" ) ;
      rc = conn.connect( 
         tmp.substr(0, pos).c_str(), 
         tmp.substr(pos+1, tmp.length()).c_str(), 
         _conf.getUserName().c_str(),
         _conf.getPasswd().c_str() ) ;
      if ( SDB_OK != rc )
      {
         conn.disconnect() ;
         return ;
      }
      else
      {
         // get coord node
         sdbReplicaGroup group ;
         rc = conn.getReplicaGroup( "SYSCoord", group ) ;
         if ( SDB_OK != rc )
         {
            conn.disconnect() ;
            return ;
         }
         bson::BSONObj obj ;
         group.getDetail( obj ) ;

         // get coord group
         bson::BSONElement eleGroup = obj.getField( "Group" ) ;
         bson::BSONObjIterator itr( eleGroup.embeddedObject() ) ;
         // loop through
         while( itr.more() )
         {
            std::string newcoord ;
            // get host name
            bson::BSONObj hostItem  ;
            bson::BSONElement hostElement = itr.next()  ;
            hostItem = hostElement.embeddedObject()  ;
            const CHAR* pname = hostItem.getField( "HostName" ).valuestr() ;
            newcoord.append(pname).append( ":" ) ;
            // port group
            bson::BSONElement serviceEle  = hostItem.getField( "Service" ) ;
            bson::BSONObjIterator itrPort( serviceEle.embeddedObject() )  ;
            if ( itrPort.more() )
            {
               // get port
               bson::BSONObj portItem  ;
               bson::BSONElement portEle = itrPort.next() ;
               portItem = portEle.embeddedObject() ;
               newcoord.append(portItem.getField( "Name" ).valuestr()) ;
               addCoord(newcoord) ;
            }
         }
      }
   }

   // check abnormal node count
   INT32 sdbDataSource::_checkAbnormalNodesCnt()
   {
      // try abnormal coord
      std::string tmp ;
      sdb conn ;
      INT32 rc ;
      INT32 j ;
      INT32 abnormalNum = _strategy->getAbnormalCoordNum() ;
      INT32 count = 0 ;
      for ( j = 0 ; j < abnormalNum ; ++j ) 
      {
         if ( SDB_OK != _strategy->getNextAbnormalCoord( tmp ) )
         {
            break ;  // TODO: not our coding style
                     // TODO: when error happen, it's just return zero, 
                     // shell we tell the caller there has not abnormal nodes??
                     // TODO: DONE
         }
         INT32 pos = tmp.find_first_of( ":" ) ;
         rc = conn.connect( 
            tmp.substr(0, pos).c_str(), 
            tmp.substr(pos+1, tmp.length()).c_str(), 
            _conf.getUserName().c_str(),
            _conf.getPasswd().c_str() ) ;
         if ( SDB_OK == rc )
         {
            ++count ;
            _strategy->mvCoordToNormal( tmp ) ;
         }
         conn.disconnect() ;
      }
      return count ;
   }

   // check keep alive time out
   BOOLEAN sdbDataSource::_checkKeepAliveTimeOut( sdb *conn )
   {
      BOOLEAN ret = FALSE;
      
      time_t nowTime ;
      time( &nowTime ) ;
      INT32 diffTime = difftime( nowTime, conn->getLastAliveTime() ) ;
      if ( diffTime > _conf.getKeepAliveTimeout() )
      {
         ret = TRUE;
      }

      return ret;
   }

   // check max connection number intervally
   void sdbDataSource::_checkMaxIdleConn()
   {
      INT32 maxIdleNum = _conf.getMaxIdleCount() ;
      INT32 freeNum = 0 ;
      INT32 aliveTime = _conf.getKeepAliveTimeout() ;
      sdb* conn ;
      _connMutex.get() ;
      if ( 0 != aliveTime )
      {
         std::list<sdb*>::iterator iter ;
         time_t nowTime ;
         time( &nowTime ) ;
         for ( iter = _idleList.begin() ; iter != _idleList.end() ; )
         {
            INT32 diffTime = difftime( nowTime, (*iter)->getLastAliveTime() ) ;
            if ( diffTime > aliveTime )
            {
               conn = *iter ; ;
               _idleList.erase( iter++ ) ;
               _idleSize.dec() ;
               _destroyList.push_back( conn ) ; 
               // #if defined (_DEBUG)
               // cout << "destroy a connection because of time out" << endl ;
               // #endif
            }
            else
               ++iter ;
         }

      }
      if ( _idleSize.peek() > maxIdleNum )
         freeNum = _idleSize.peek() - maxIdleNum ;
      while ( freeNum > 0 )
      {
         conn = _idleList.front() ;
         _idleList.pop_front() ;
         _idleSize.dec() ;
         _destroyList.push_back( conn ) ;   
         --freeNum ;
      }
      _toDestroyConn = TRUE ;
      _connMutex.release() ;
   }

   // close data source
   void sdbDataSource::close()
   {
      disable() ;
      // clear strategy
      if ( _strategy )
      {
         SAFE_OSS_DELETE( _strategy ) ;
      }
      _isInited = FALSE;
   }
}
