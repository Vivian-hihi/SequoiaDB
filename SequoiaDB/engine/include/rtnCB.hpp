/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = rtnCB.hpp

   Descriptive Name = RunTime Control Block Header

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains structure for Runtime
   Control Block.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef RTNCB_HPP_
#define RTNCB_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "rtnContext.hpp"
#include "ossLatch.hpp"
#include "pd.hpp"
#include "monEDU.hpp"
#include "dmsCB.hpp"
#include "pmdEDU.hpp"
#include <map>
#include <set>

namespace engine
{
   /*
      _SDB_RTNCB define
   */
   class _SDB_RTNCB : public SDBObject
   {
   private :
   #ifdef RTNCB_XLOCK
   #undef RTNCB_XLOCK
   #endif
   #define RTNCB_XLOCK ossScopedLock _lock(&_mutex, EXCLUSIVE) ;
   #ifdef RTNCB_SLOCK
   #undef RTNCB_SLOCK
   #endif
   #define RTNCB_SLOCK ossScopedLock _lock(&_mutex, SHARED) ;
      ossSpinSLatch _mutex ;

      std::map<SINT64, rtnContext *> _contextList ;
      SINT64 _contextHWM ;

   public :
      _SDB_RTNCB() ;
      ~_SDB_RTNCB() ;

      SINT32 contextNew ( RTN_CONTEXT_TYPE type, rtnContext **context,
                          SINT64 &contextID, _pmdEDUCB * pEDUCB ) ;

      void contextDelete ( SINT64 contextID, _pmdEDUCB *cb ) ;

      inline rtnContext *contextFind ( SINT64 contextID )
      {
         RTNCB_SLOCK
         std::map<SINT64, rtnContext*>::const_iterator it ;
         if ( _contextList.end() == (it = _contextList.find(contextID)))
            return NULL ;
         return (*it).second ;
      }

      inline INT32 contextNum ()
      {
         RTNCB_SLOCK
         return _contextList.size() ;
      }

      inline void contextDump ( std::map<UINT64, std::set<SINT64> > &contextList )
      {
         UINT64 eduID = PMD_INVALID_EDUID ;
         INT64  contextID = -1  ;

         RTNCB_SLOCK
         std::map<SINT64, rtnContext*>::const_iterator it ;
         for ( it = _contextList.begin() ; it != _contextList.end(); ++it )
         {
            eduID = (*it).second->eduID() ;
            contextID = (*it).second->contextID() ;

            contextList[ eduID ].insert( contextID ) ;
         }
      }

      inline void monContextSnap ( std::map<UINT64, std::set<monContextFull> >
                                   &contextList )
      {
         UINT64 eduID = PMD_INVALID_EDUID ;
         INT64  contextID = -1  ;
         monContextCB *monCB = NULL ;

         RTNCB_SLOCK
         std::map<SINT64, rtnContext*>::const_iterator it ;
         for ( it = _contextList.begin() ; it != _contextList.end(); it++ )
         {
            eduID = (*it).second->eduID() ;
            contextID = (*it).second->contextID() ;
            monCB = (*it).second->getMonCB() ;

            monContextFull item( contextID, *monCB ) ;
            item._typeDesp = getContextTypeDesp( (*it).second->getType() ) ;
            item._info = (*it).second->toString() ;

            contextList[ eduID ].insert( item ) ;
         }
      }

      inline void monContextSnap( UINT64 eduID,
                                  std::set<monContextFull> &contextList )
      {
         INT64  contextID = -1  ;
         monContextCB *monCB = NULL ;

         RTNCB_SLOCK
         std::map<SINT64, rtnContext*>::const_iterator it ;
         for ( it = _contextList.begin() ; it != _contextList.end() ; it++ )
         {
            if ( (*it).second->eduID() == eduID )
            {
               contextID = (*it).second->contextID() ;
               monCB = (*it).second->getMonCB() ;

               monContextFull item( contextID, *monCB ) ;
               item._typeDesp = getContextTypeDesp( (*it).second->getType() ) ;
               item._info = (*it).second->toString() ;

               contextList.insert( item ) ;
            }
         }
      }

   } ;
   typedef class _SDB_RTNCB SDB_RTNCB ;

}

#endif //RTNCB_HPP_

