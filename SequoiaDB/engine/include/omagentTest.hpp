#ifndef OMAGENT_TEST_HPP__
#define OMAGENT_TEST_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "ossTypes.h"
#include "ossUtil.h"
#include "pd.hpp"
#include "omagentMsgDef.hpp"
#include "../bson/bson.h"

using namespace bson ;

namespace CLSMGR
{
   INT32 testScanHost ( CHAR **ppBuffer, INT32 *bufferSize ) ;

   INT32 testInstallRemoteAgent ( CHAR **ppBuffer, INT32 *bufferSize ) ;

   INT32 testInstallAgentProcess ( CHAR **ppBuffer, INT32 *bufferSize ) ;

}



#endif
