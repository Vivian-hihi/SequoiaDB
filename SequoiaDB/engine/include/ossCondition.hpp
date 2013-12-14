/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = ossMmap.cpp

   Descriptive Name = Operating System Services Memory Map

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains functions for Memory Mapping
   Files.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef OSSCONDITION_H_
#define OSSCONDITION_H_

#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>

namespace engine
{
   typedef boost::condition_variable _ossCondition;
   typedef boost::mutex _ossConditionMutex;
}

#define QNIQUE_LOCK boost::unique_lock<boost::mutex>

#endif
