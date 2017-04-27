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

   Source File Name = coordCommands.hpp

   Descriptive Name = Coord Commands

   When/how to use: this program may be used on binary and text-formatted
   versions of runtime component. This file contains code logic for
   common functions for coordinator node.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/20/2017  XJH Init
   Last Changed =

*******************************************************************************/

#ifndef COORD_COMMANDS_HPP__
#define COORD_COMMANDS_HPP__

#include "coordCommandBase.hpp"
#include "coordFactory.hpp"

using namespace bson ;

namespace engine
{

   /*
      _coordCMDTestCollectionSpace define
   */
   class _coordCMDTestCollectionSpace : public _coordCommandBase
   {
      COORD_DECLARE_CMD_AUTO_REGISTER() ;
      public:
         _coordCMDTestCollectionSpace() ;
         virtual ~_coordCMDTestCollectionSpace() ;

         virtual INT32 execute( MsgHeader *pMsg,
                                pmdEDUCB *cb,
                                INT64 &contextID,
                                rtnContextBuf *buf ) ;
   } ;
   typedef _coordCMDTestCollectionSpace coordCMDTestCollectionSpace ;

   /*
      _coordCMDTestCollection define
   */
   class _coordCMDTestCollection : public _coordCommandBase
   {
      COORD_DECLARE_CMD_AUTO_REGISTER() ;
      public:
         _coordCMDTestCollection() ;
         virtual ~_coordCMDTestCollection() ;

         virtual INT32 execute( MsgHeader *pMsg,
                                pmdEDUCB *cb,
                                INT64 &contextID,
                                rtnContextBuf *buf ) ;
   } ;
   typedef _coordCMDTestCollection coordCMDTestCollection ;

   /*
      _coordCmdWaitTask define
   */
   class _coordCmdWaitTask : public _coordCommandBase
   {
      COORD_DECLARE_CMD_AUTO_REGISTER() ;
      public:
         _coordCmdWaitTask() ;
         virtual ~_coordCmdWaitTask() ;

         virtual INT32 execute( MsgHeader *pMsg,
                                pmdEDUCB *cb,
                                INT64 &contextID,
                                rtnContextBuf *buf ) ;
   } ;
   typedef _coordCmdWaitTask coordCmdWaitTask ;

   /*
      _coordCmdCancelTask define
   */
   class _coordCmdCancelTask : public _coordCommandBase
   {
      COORD_DECLARE_CMD_AUTO_REGISTER() ;
      public:
         _coordCmdCancelTask() ;
         virtual ~_coordCmdCancelTask() ;

         virtual INT32 execute( MsgHeader *pMsg,
                                pmdEDUCB *cb,
                                INT64 &contextID,
                                rtnContextBuf *buf ) ;
   } ;
   typedef _coordCmdCancelTask coordCmdCancelTask ;

   /*
      _coordCMDStatisticsBase define
   */
   class _coordCMDStatisticsBase : public _coordCommandBase
   {
      public:
         _coordCMDStatisticsBase() ;
         virtual ~_coordCMDStatisticsBase() ;

         virtual INT32 execute( MsgHeader *pMsg,
                                pmdEDUCB *cb,
                                INT64 &contextID,
                                rtnContextBuf *buf ) ;
      private:
         virtual INT32 generateResult( rtnContext *pContext,
                                       pmdEDUCB *cb ) = 0 ;

         virtual BOOLEAN openEmptyContext() const { return FALSE ; }
   } ;
   typedef _coordCMDStatisticsBase coordCMDStatisticsBase ;

   /*
      _coordCMDGetIndexes define
   */
   class _coordCMDGetIndexes : public _coordCMDStatisticsBase
   {
      typedef _utilMap< string, BSONObj, 10 >      CoordIndexMap ;

      COORD_DECLARE_CMD_AUTO_REGISTER() ;
      public:
         _coordCMDGetIndexes() ;
         virtual ~_coordCMDGetIndexes() ;

      private :
         virtual INT32 generateResult( rtnContext *pContext,
                                       pmdEDUCB *cb ) ;
   } ;
   typedef _coordCMDGetIndexes coordCMDGetIndexes ;

   /*
      _coordCMDGetCount define
   */
   class _coordCMDGetCount : public _coordCMDStatisticsBase
   {
      COORD_DECLARE_CMD_AUTO_REGISTER() ;
      public:
         _coordCMDGetCount() ;
         virtual ~_coordCMDGetCount() ;

      private :
         virtual INT32 generateResult( rtnContext *pContext,
                                       pmdEDUCB *cb ) ;
         virtual BOOLEAN openEmptyContext() const { return TRUE ; }
   } ;
   typedef _coordCMDGetCount coordCMDGetCount ;

   /*
      _coordCMDGetDatablocks define
   */
   class _coordCMDGetDatablocks : public _coordCMDStatisticsBase
   {
      COORD_DECLARE_CMD_AUTO_REGISTER() ;
      public:
         _coordCMDGetDatablocks() ;
         virtual ~_coordCMDGetDatablocks() ;
      private :
         virtual INT32 generateResult( rtnContext *pContext,
                                       pmdEDUCB *cb ) ;
   } ;
   typedef _coordCMDGetDatablocks coordCMDGetDatablocks ;

   /*
      _coordCMDGetQueryMeta define
   */
   class _coordCMDGetQueryMeta : public _coordCMDGetDatablocks
   {
      COORD_DECLARE_CMD_AUTO_REGISTER() ;
      public:
         _coordCMDGetQueryMeta() ;
         virtual ~_coordCMDGetQueryMeta() ;
   } ;
   typedef _coordCMDGetQueryMeta coordCMDGetQueryMeta ;


   /*
      _coordCMDCrtProcedure define
   */
   class _coordCMDCrtProcedure : public _coordCommandBase
   {
      COORD_DECLARE_CMD_AUTO_REGISTER() ;
      public:
         _coordCMDCrtProcedure() ;
         virtual ~_coordCMDCrtProcedure() ;

         virtual INT32 execute( MsgHeader *pMsg,
                                pmdEDUCB *cb,
                                INT64 &contextID,
                                rtnContextBuf *buf ) ;
   } ;
   typedef _coordCMDCrtProcedure coordCMDCrtProcedure ;

   /*
      _coordCMDEval define
   */
   class _coordCMDEval : public _coordCommandBase
   {
      COORD_DECLARE_CMD_AUTO_REGISTER() ;
      public:
         _coordCMDEval() ;
         virtual ~_coordCMDEval() ;

         virtual INT32 execute( MsgHeader *pMsg,
                                pmdEDUCB *cb,
                                INT64 &contextID,
                                rtnContextBuf *buf ) ;
      private:
         INT32 _buildContext( _spdSession *session,
                              pmdEDUCB *cb,
                              SINT64 &contextID ) ;
   } ;
   typedef _coordCMDEval coordCMDEval ;

   /*
      _coordCMDRmProcedure define
   */
   class _coordCMDRmProcedure : public _coordCommandBase
   {
      COORD_DECLARE_CMD_AUTO_REGISTER() ;
      public:
         _coordCMDRmProcedure() ;
         virtual ~_coordCMDRmProcedure() ;

         virtual INT32 execute( MsgHeader *pMsg,
                                pmdEDUCB *cb,
                                INT64 &contextID,
                                rtnContextBuf *buf ) ;
   } ;
   typedef _coordCMDRmProcedure coordCMDRmProcedure ;

   /*
      _coordCMDSetSessionAttr define
   */
   class _coordCMDSetSessionAttr : public _coordCommandBase
   {
      COORD_DECLARE_CMD_AUTO_REGISTER() ;
      public:
         _coordCMDSetSessionAttr() ;
         virtual ~_coordCMDSetSessionAttr() ;

         virtual INT32 execute( MsgHeader *pMsg,
                                pmdEDUCB *cb,
                                INT64 &contextID,
                                rtnContextBuf *buf ) ;
   } ;
   typedef _coordCMDSetSessionAttr coordCMDSetSessionAttr ;

   /*
      _coordCMDReelection define
   */
   class _coordCMDReelection : public _coordCommandBase
   {
      COORD_DECLARE_CMD_AUTO_REGISTER() ;
      public:
         _coordCMDReelection() ;
         virtual ~_coordCMDReelection() ;

         virtual INT32 execute( MsgHeader *pMsg,
                                pmdEDUCB *cb,
                                INT64 &contextID,
                                rtnContextBuf *buf ) ;
   } ;
   typedef _coordCMDReelection coordCMDReelection ;

   /*
      _coordCMDTruncate define
   */
   class _coordCMDTruncate : public _coordCommandBase
   {
      COORD_DECLARE_CMD_AUTO_REGISTER() ;
      public:
         _coordCMDTruncate() ;
         virtual ~_coordCMDTruncate() ;
         virtual INT32 execute( MsgHeader *pMsg,
                                pmdEDUCB *cb,
                                INT64 &contextID,
                                rtnContextBuf *buf ) ;
   } ;
   typedef _coordCMDTruncate coordCMDTruncate ;

}

#endif // COORD_COMMANDS_HPP__
