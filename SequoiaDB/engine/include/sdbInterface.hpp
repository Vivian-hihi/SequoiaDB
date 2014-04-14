/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sdbInterface.hpp

   Descriptive Name = Process MoDel Engine Dispatchable Unit Event Header

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains structure for events that
   used as inter-EDU communications.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef SDB_INTERFACE_HPP__
#define SDB_INTERFACE_HPP__

#include "core.hpp"
#include "oss.hpp"

namespace engine
{

   /*
      _pmdBaseSession define
   */
   class _ISession : public SDBObject
   {
      public:
         _ISession() {}
         virtual ~_ISession() {}

      public:
         virtual INT32           sessionType() const = 0 ;
         virtual UINT64          identifyID() = 0 ;
         virtual const CHAR*     sessionName() const = 0 ;

      protected:
         virtual void            _onAttach () {}
         virtual void            _onDetach () {}

   } ;
   typedef _ISession ISession ;

}

#endif // SDB_INTERFACE_HPP__

