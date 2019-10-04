#include "monMgr.hpp"
#include "gtest/gtest.h"
using namespace engine;


class monitorManagerTest : public ::testing::Test
{ 
public: 
   monitorManagerTest() { 
       // initialization code here
   } 
   
   void SetUp( ) { 
       // code here will execute just before the test ensues 
   }

   void TearDown( ) { 
       // code here will be called just after the test completes
       // ok to through exceptions from here if need be
   }
} ;

// Test register object
TEST (monitorManagerTest, registerObj1)
{
   monMonitorManager mgr ;

   // Turn on monitoring
   mgr.setMonitorStatus( MON_CLASS_QUERY, TRUE ) ;

   monClassQuery *obj = mgr.registerMonitorObject<monClassQuery>() ;

   ASSERT_NE( obj, (monClassQuery*)NULL );
}

// Test register object when object class monitoring is off
TEST (monitorManagerTest, registerObj2)
{
   monMonitorManager mgr ;

   // Turn off monitoring
   mgr.setMonitorStatus( MON_CLASS_QUERY, FALSE ) ;

   monClassQuery *obj = mgr.registerMonitorObject<monClassQuery>() ;

   ASSERT_EQ( obj, (monClassQuery*)NULL );
}

// Test scanning objects
TEST (monitorManagerTest, scanObj1)
{
   monMonitorManager mgr ;

   // Turn on monitoring
   mgr.setMonitorStatus(MON_CLASS_QUERY, TRUE) ;

   monClassQuery *obj = mgr.registerMonitorObject<monClassQuery>() ;
   monClassQuery *obj2 = mgr.registerMonitorObject<monClassQuery>() ;

   monClassReadScanner *scanner = mgr.getReadScanner(MON_CLASS_QUERY, MON_CLASS_ACTIVE_LIST);
   monClassQuery *retObj = (monClassQuery*)scanner->getNext();

   ASSERT_EQ(retObj, obj2) ;

   retObj = (monClassQuery*)scanner->getNext() ;

   ASSERT_EQ(retObj, obj) ;

   retObj = (monClassQuery*)scanner->getNext() ;

   ASSERT_EQ( retObj, (monClassQuery*)NULL ) ;

   delete ( scanner ) ;
}

// Test scanning empty active list
TEST (monitorManagerTest, scanObj2)
{
   monMonitorManager mgr ;

   // Turn on monitoring
   mgr.setMonitorStatus(MON_CLASS_EDU, TRUE) ;

   monClassReadScanner *scanner = mgr.getReadScanner(MON_CLASS_EDU, MON_CLASS_ACTIVE_LIST );
   monClassEDU *retObj = (monClassEDU*)scanner->getNext();

   ASSERT_EQ( retObj, (monClassEDU*)NULL ) ;

   delete ( scanner ) ;
}

// Test scanning empty archive list
TEST (monitorManagerTest, scanObj3)
{
   monMonitorManager mgr ;

   // Turn on monitoring
   mgr.setMonitorStatus(MON_CLASS_EDU, TRUE) ;

   monClassReadScanner *scanner = mgr.getReadScanner(MON_CLASS_EDU, MON_CLASS_ARCHIVED_LIST );
   monClassEDU *retObj = (monClassEDU*)scanner->getNext();

   ASSERT_EQ( retObj, (monClassEDU*)NULL ) ;
   delete ( scanner ) ;
}

// Test deleting objects. If archiving is off, then the object should be removed
TEST (monitorManagerTest, deleteObj1)
{
   monMonitorManager mgr ;
   // Turn on monitoring
   mgr.setMonitorStatus(MON_CLASS_EDU, TRUE) ;

   monClassEDU *obj = mgr.registerMonitorObject<monClassEDU>() ;

   monClassReadScanner *scanner = mgr.getReadScanner(MON_CLASS_EDU, MON_CLASS_ACTIVE_LIST );
   monClassEDU *retObj = (monClassEDU*)scanner->getNext();
   ASSERT_EQ(retObj, obj) ;

   delete ( scanner ) ;

   mgr.removeMonitorObject(obj) ;

   // Can no longer read this from a scan
   scanner = mgr.getReadScanner(MON_CLASS_EDU, MON_CLASS_ACTIVE_LIST ) ;
   retObj = (monClassEDU*)scanner->getNext() ;
   ASSERT_EQ( retObj, (monClassEDU*)NULL ) ;
   delete ( scanner ) ;

   // Object should be in pending delete state
   ASSERT_TRUE(obj->isPendingDelete()) ;
   ASSERT_FALSE(obj->isPendingArchive()) ;
}

// Test deleting objects. If archiving is on, can still see it when reading archive list
TEST (monitorManagerTest, deleteObj2)
{
   monMonitorManager mgr ;
   // Turn on monitoring
   mgr.setMonitorStatus(MON_CLASS_QUERY, TRUE) ;

   monClassQuery *obj = mgr.registerMonitorObject<monClassQuery>() ;

   monClassReadScanner *scanner = mgr.getReadScanner(MON_CLASS_QUERY, MON_CLASS_ACTIVE_LIST );
   monClassQuery *retObj = (monClassQuery*)scanner->getNext();
   ASSERT_EQ(retObj, obj) ;

   delete ( scanner ) ;
   mgr.removeMonitorObject(obj) ;

   // Can no longer read this from a active list scan
   scanner = mgr.getReadScanner(MON_CLASS_QUERY, MON_CLASS_ACTIVE_LIST ) ;
   retObj = (monClassQuery*)scanner->getNext() ;
   ASSERT_EQ( retObj, (monClassQuery*)NULL ) ;
   delete ( scanner ) ;
   // Object should be in pending archive state
   ASSERT_TRUE(obj->isPendingArchive()) ;

   // Can read this from an archive list scan
   scanner = mgr.getReadScanner(MON_CLASS_QUERY, MON_CLASS_ARCHIVED_LIST ) ;
   retObj = (monClassQuery*)scanner->getNext() ;
   ASSERT_EQ( retObj, obj ) ;
   delete ( scanner ) ;
}
int main(int argc, char **argv) 
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

