/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = ossASIO.hpp

   Descriptive Name = Operating System Services ASync IO Header

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains structure for accept/read/write
   from ASIO (epoll/IOCP for Linux/Windows).

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef OSSASIO_HPP__
#define OSSASIO_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "msgReplicator.hpp"
#include "ossLatch.hpp"
#include "ossAtomic.hpp"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <map>
using namespace boost ;
using namespace boost::asio ;
using namespace boost::asio::ip ;
// the caller is taking message as input, and may or may not allocate reply
// buffer. The caller will free reply when it's not NULL and not same as message
// pointer.
typedef INT32 (*ossAsioProcessFunction) ( CHAR *message,
                                          CHAR **reply,
                                          tcp::socket *sock ) ;

class _ossAsioMsgProcessor : public
      boost::enable_shared_from_this<_ossAsioMsgProcessor>,
      public SDBObject
{
private :
   BOOLEAN _startRun ;
   ossAsioProcessFunction _onReceive ;
   ossAsioProcessFunction _onDisconnect ;
   tcp::socket _socket ;
   MsgReplHeader _header ;
   io_service *_ioservice ;
   CHAR *_message ;
   INT32 _bufferLen ;
   CHAR *_reply ;
   void _process() ;
   void _handleReadPacketBody ( const boost::system::error_code &error ) ;
   void _handleReadPacketHead ( const boost::system::error_code &error ) ;
   void _handleWritePacket ( const boost::system::error_code &error ) ;
   void _readPacketHead () ;
public :
   _ossAsioMsgProcessor ( ossAsioProcessFunction onReceive,
                          ossAsioProcessFunction onDisconnect,
                          io_service &ioservice )
         : _socket ( ioservice )
   {
      _startRun = FALSE ;
      _message = NULL ;
      _bufferLen = 0 ;
      _reply = NULL ;
      _ioservice = &ioservice ;
      _onReceive = onReceive ;
      _onDisconnect = onDisconnect ;
   }
   ~_ossAsioMsgProcessor () ;

   void run ()
   {
      _startRun = TRUE ;
      _readPacketHead() ;
   }
   INT32 connect ( CHAR *pHostName, CHAR *pServiceName ) ;

   tcp::socket& socket() {
      return _socket;
   }
} ;
typedef class _ossAsioMsgProcessor ossAsioMsgProcessor ;

class _timerPair : public
      boost::enable_shared_from_this<_timerPair>,
      public SDBObject
{
private :
   deadline_timer _timer ;
   ossAsioProcessFunction _onTimer ;
   UINT32 _timeoutMS ;
public :
   _timerPair ( UINT32 timeoutMS,
                ossAsioProcessFunction onTimer, io_service &ioservice ) :
   _timer(ioservice),
   _onTimer(onTimer),
   _timeoutMS(timeoutMS)
   {}
   ~_timerPair ()
   {
      _timer.cancel() ;
   }
   void check_deadline() ;
   void run () ;
} ;
typedef class _timerPair timerPair ;

class _ossASIO : public SDBObject
{
private :
   ossSpinSLatch _mutex ;
   INT32 _port ;
   io_service _ioservice ;
   ossAtomic32 _maxTimer ;
   std::map<UINT32, timerPair*> _timerList ;
   tcp::endpoint _endpoint ;
   tcp::acceptor _acceptor ;
   ossAsioProcessFunction _onReceive ;
   ossAsioProcessFunction _onDisconnect ;

   void _handleAccept ( boost::shared_ptr<ossAsioMsgProcessor> processor,
                        const boost::system::error_code &error ) ;
   void _accept() ;

public :
   _ossASIO ( INT32 port, ossAsioProcessFunction onReceive,
              ossAsioProcessFunction onDisconnect ) ;
   ~_ossASIO () ;
   void run () ;
   INT32 connect ( CHAR *pHostName, CHAR *pServiceName,
                   tcp::socket **sock ) ;
   INT32 addTimer ( UINT32 timeoutMS,
                    ossAsioProcessFunction onTimer, UINT32 &timerID ) ;
   void removeTimer ( UINT32 timerID ) ;
   io_service *getIOService ()
   {
      return &_ioservice ;
   }
} ;
typedef class _ossASIO ossASIO ;

#endif
