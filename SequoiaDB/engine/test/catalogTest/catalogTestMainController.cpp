
#include "ossTypes.h"
#include <gtest/gtest.h>

#include "pmdEDU.hpp"
#include "ossErr.h"
#include "pmdEDUMgr.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"

using namespace engine;

TEST(catalogMainControllerTest, init)
{
   pmdEDUMgr *pEduMgr = pmdGetKRCB()->getEDUMgr();
   EDUID agentEDU = PMD_INVALID_EDUID;
   INT32 rc = pEduMgr->startEDU(EDU_TYPE_CATMAINCONTROLLER, NULL, &agentEDU);
   ASSERT_EQ(SDB_OK, rc);
   ASSERT_NE(PMD_INVALID_EDUID, agentEDU);
   boost::xtime sleepTime;
   boost::xtime_get(&sleepTime, boost::TIME_UTC);
   sleepTime.sec += 1;
   boost::thread::sleep(sleepTime);
   pmdGetKRCB()->destroy();
}
