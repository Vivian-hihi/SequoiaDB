
#include "spdFMPMgr.hpp"
#include "spdFMP.hpp"
#include <gtest/gtest.h>

using namespace engine ;

TEST(spdTest, spdTest_1)
{
   INT32 rc = SDB_OK ;
   spdFMPMgr fmpMgr ;
   rc = fmpMgr.init() ;
   ASSERT_TRUE( rc == SDB_OK ) ;

   spdFMP *fmp = NULL ;
   rc = fmpMgr.getFMP( fmp ) ;
   ASSERT_TRUE( rc == SDB_OK ) ;
   getchar () ;
}
