#include "myclass.hpp"
#include "myclass2.hpp"
#include "ossUtil.hpp"


#include <gtest/gtest.h>
#include <iostream>

using namespace std ;

TEST(sptTest, test1)
{
  INT32 rc = SDB_OK ;
  _sptContainer container ;
  _sptScope *scope = container.newScope( SPT_SCOPE_TYPE_SP ) ; 
  ASSERT_TRUE( NULL != scope );

  rc = scope->loadUsrDefObj( &(myclass::__desc) ) ;
  ASSERT_TRUE( SDB_OK == rc ) ;

  rc = scope->loadUsrDefObj( &(myclass2::__desc) ) ;
  ASSERT_TRUE( SDB_OK == rc ) ;

  {
//  const CHAR *code = "var a = new myjsclass(); var b = new myjsclass2();" ;
  const CHAR *code = "function sum(x,y){return x+y;} sum(1,2);"
  bson::BSONObj detail ;
  bson::BSONObj rval ;
  rc = scope->eval( code, ossStrlen(code), rval, detail ) ;
  ASSERT_TRUE( SDB_OK == rc ) ;
  cout << rval.toString() << endl ;
  }
/*
  {
  const CHAR *code = "a.func(); b.func()" ;
  bson::BSONObj detail ;
  rc = scope->eval( code, ossStrlen(code), detail) ;
  ASSERT_TRUE( SDB_OK == rc ) ;
  }
*/
  scope->shutdown() ;
  delete scope ;
}
