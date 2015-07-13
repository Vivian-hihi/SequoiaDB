#include "mthSelector.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace std ;
using namespace bson ;
using namespace engine ;

/// simple include
/// select b, a from {a:1,b:2,c:3} -> {a:1,b:2}
TEST( selector, simple_include_test_1 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "b" << BSON( "$include" << 1 ) << "a" << BSON( "$include" << 1 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << 1 << "b" << 2 << "c" << 3 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << 1 << "b" << 2 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
}

/// select b, a from {c:1, b:2} -> {b:2}
TEST( selector, simple_include_test_2 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "b" << BSON( "$include" << 1 ) << "a" << BSON( "$include" << 1 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "c" << 1 << "b" << 2 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "b" << 2 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
}

/// select b, a from {c:1,d:1} -> {}
TEST( selector, simple_include_test_3 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "b" << BSON( "$include" << 1 ) << "a" << BSON( "$include" << 1 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "c" << 1 << "d" << 1 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSONObj() ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
}

/// select a.b from {a:{c:1, b:1}} -> {a:{b:1}} 
TEST( selector, simple_include_test_4 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a.b" << BSON( "$include" << 1 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << BSON( "c" << 1 << "b" << 1 ) ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << BSON( "b" << 1 ) ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
}

/// select a.b from {a:1} -> {}
TEST( selector, simple_include_test_5 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a.b" << BSON( "$include" << 1 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << 1 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSONObj() ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
}

/// select a.b from {a:[{c:1}, {d:1}, {b:1}, {b:2}]} -> {a:[{}, {}, {b:1},{b:2}]}
TEST( selector, simple_include_test_6 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a.b" << BSON( "$include" << 1 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << BSON_ARRAY( BSON( "c" << 1) << BSON("d" << 1) << BSON("b"<< 1) << BSON("b" << 2)) ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << BSON_ARRAY(BSONObj() << BSONObj() <<BSON("b" << 1) << BSON("b" << 2))) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
}

/// select a.b from {a:[{c:1}]} -> {a:[{}]}
TEST( selector, simple_include_test_7 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a.b" << BSON( "$include" << 1 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << BSON_ARRAY( BSON( "c" << 1) ) ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << BSON_ARRAY( BSONObj() ) ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
}

/// select a.b from {a:[1,2]} -> {a:[]}
TEST( selector, simple_include_test_8 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a.b" << BSON( "$include" << 1 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << BSON_ARRAY( 1 << 2 ) ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << BSONArrayBuilder().arr() ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
}

/// exclude b from {a:1,b:1} -> {a:1}
TEST( selector, simple_exclude_test_1 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "b" << BSON( "$include" << 0 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << 1 << "b" << 1 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << 1 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
}

/// exclude a.b from { a:1, b:1} -> {a:1,b:1}
TEST( selector, simple_exclude_test_2 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a.b" << BSON( "$include" << 0 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << 1 << "b" << 1 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << 1 << "b" << 1 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
}

/// exclude a.b from { a:{b:1, c:1}, b:1} -> {a:{c:1},b:1}
TEST( selector, simple_exclude_test_3 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a.b" << BSON( "$include" << 0 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << BSON( "b" << 1 << "c" << 1) << "b" << 1 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << BSON("c" << 1) << "b" << 1 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
}

/// exclude a.b from {a:[1, {b:1, c:1}, {b:1}, {d:1}]} -> {a:[1, {c:1}, {}, {d:1}]}
TEST( selector, simple_exclude_test_4 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a.b" << BSON( "$include" << 0 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << BSON_ARRAY( 1 << BSON("b" << 1 << "c" <<1 ) << BSON("b" << 1 ) << BSON("d" << 1))) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << BSON_ARRAY( 1 << BSON("c" <<1 ) << BSONObj() << BSON("d" << 1)) ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
}

/// exclude a from {b:1} -> {b:1}
TEST( selector, simple_exclude_test_5 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a" << BSON( "$include" << 0 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "b" << 1 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "b" << 1 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
}

/// exclude a.b from {b:1} -> {b:1}
TEST( selector, simple_exclude_test_6 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a.b" << BSON( "$include" << 0 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "b" << 1 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "b" << 1 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
}

/// include a, exclude b --> error
TEST( selector, simple_exclude_test_7 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a" << BSON( "$include" << 1 ) << "b" << BSON("$include" << 0 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_INVALIDARG , rc ) ;
}

/// default a:1 from {b:1 } -> {a:1}
TEST( selector, simple_default_test_1 )
{
   {
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a" << BSON( "$default" << 1 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "b" << 1 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << 1 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }

   {
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a" << 1 ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "b" << 1 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << 1 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }
}


/// default a:1 from { a:2, b:1 } -> {a:2}
TEST( selector, simple_default_test_2 )
{
   {
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a" << BSON( "$default" << 1 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << 2 << "b" << 1 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << 2 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }

   {
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a" << 1 ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << 2 << "b" << 1 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << 2 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }
}

/// default a.b:1 from {a:{c:1, d:1}} -> {a:{b:1}}
TEST( selector, simple_default_test_3 )
{
   {
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a.b" << BSON( "$default" << 1 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << BSON( "c" << 1 << "d" << 1 ) ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << BSON( "b" << 1) ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }
   {
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a.b" << 1 ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << BSON( "c" << 1 << "d" << 1 ) ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << BSON( "b" << 1) ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }
}

/// default a.b:1  from {b:1} -> {a:{b:1}}
TEST( selector, simple_default_test_4 )
{
   {
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a.b" << BSON( "$default" << 1 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "b" << 1) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << BSON( "b" << 1 ) ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }
   {
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a.b" << 1 ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "b" << 1) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << BSON( "b" << 1 ) ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }
}

/// default a.b:1 from {a:[{c:1}, 1, {b:1}] } -> {a:[{b:1}, {b:1}]}
TEST( selector, simple_default_test_5 )
{
   {
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a.b" << BSON( "$default" << 1 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << BSON_ARRAY( BSON("c" << 1) << 1 << BSON("b" << 1) )) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << BSON_ARRAY( BSON("b" << 1) << BSON( "b" << 1 )) ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }
   {
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a.b" << 1 ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << BSON_ARRAY( BSON("c" << 1) << 1 << BSON("b" << 1 )) ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << BSON_ARRAY( BSON("b" << 1) << BSON("b" << 1 )) ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }
}

/// default a.b:1 from {a:1, b:1 } -> {}
TEST( selector, simple_default_test_6 )
{
   {
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a.b" << BSON( "$default" << 1 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << 1 << "b" << 1) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSONObj() ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }
   {
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a.b" << 1 ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << 1 << "b" << 1 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSONObj() ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }
}

/// slice a : 1 from {a:[1,2,3], b:1} ->{a:[1], b:1}
TEST( selector, simple_slice_test_1 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a" << BSON( "$slice" << 1 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << BSON_ARRAY( 1 << 2 << 3 ) << "b" << 1 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << BSON_ARRAY( 1 ) << "b" << 1 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
}

/// slice a.b : 1 from {a:{a:1, b:[1,2,3]}, b:1} ->{a:{a:1, b:[1]}, b:1}
TEST( selector, simple_slice_test_2 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a.b" << BSON( "$slice" << 1 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << BSON( "a" << 1 << "b" << BSON_ARRAY(1 << 2 << 3)) << "b" << 1 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << BSON( "a" << 1 << "b" << BSON_ARRAY( 1 ))<< "b" << 1 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
}

/// slice a.b : 1 from {a:{a:1, b:1}, b:1} ->{a:{a:1, b:[1]}, b:1}
TEST( selector, simple_slice_test_3 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a.b" << BSON( "$slice" << 1 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << BSON( "a" << 1 << "b" << 1 ) << "b" << 1 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << BSON( "a" << 1 << "b" << 1 )<< "b" << 1 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
}

/// slice a : -2 from {a:[1,2,3], b:1} ->{a:[2,3], b:1}
TEST( selector, simple_slice_test_4 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a" << BSON( "$slice" << -2 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << BSON_ARRAY( 1 << 2 << 3 ) << "b" << 1 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << BSON_ARRAY( 2 << 3 )<< "b" << 1 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
}

/// slice a : -2 from {a:[1], b:1} ->{a:[1], b:1}
TEST( selector, simple_slice_test_5 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a" << BSON( "$slice" << -2 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << BSON_ARRAY( 1 ) << "b" << 1 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << BSON_ARRAY( 1 )<< "b" << 1 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
}

/// slice a :[0,2] from {a:[1,2,3], b:1} ->{a:[1,2], b:1}
TEST( selector, simple_slice_test_6 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a" << BSON( "$slice" << BSON_ARRAY( 0 << 2 ) ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << BSON_ARRAY( 1 << 2 << 3 ) << "b" << 1 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << BSON_ARRAY( 1 << 2 )<< "b" << 1 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
}

/// slice a :[2,2] from {a:[1,2,3], b:1} ->{a:[3], b:1}
TEST( selector, simple_slice_test_7 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a" << BSON( "$slice" << BSON_ARRAY( 2 << 2 ) ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << BSON_ARRAY( 1 << 2 << 3 ) << "b" << 1 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << BSON_ARRAY( 3 )<< "b" << 1 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
}

/// slice a.b:1 from {a:[{b:1}, {b:[1,2,3]}, {b:[4,5,6]}, {c:1}], b:1} ->{a:[{b:1}, {b:[1]}, {b:[4]}, {c:1}], b:1}
TEST( selector, simple_slice_test_8 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a.b" << BSON( "$slice" << 1 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << BSON_ARRAY( BSON("b" << 1 ) << BSON( "b" << BSON_ARRAY( 1 << 2 << 3)) << BSON( "b" << BSON_ARRAY(4 << 5 << 6))<< BSON("b" << 1) ) << "b" << 1 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << BSON_ARRAY( BSON( "b" << 1) << BSON("b" << BSON_ARRAY( 1)) << BSON("b" << BSON_ARRAY(4)) << BSON("b" << 1) )<< "b" << 1 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
}

/// elemMatch a:{b:1} from {a:[{b:1}, {b:2}, {b:1}]} -> {a:[{b:1}, {b:1}]}
TEST( selector, simple_elemmatch_test_1 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a" << BSON( "$elemMatch" << BSON( "b" << 1 ) ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << BSON_ARRAY( BSON( "b" << 1 ) << BSON( "b" << 2 ) << BSON( "b" << 1 ) ) ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << BSON_ARRAY( BSON("b" << 1 ) << BSON( "b" << 1 )) ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
}

/// elemMatch a:{c:1} from {a:[{b:1}, {b:2}, {b:1}]} -> {a:[]}
TEST( selector, simple_elemmatch_test_2 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a" << BSON( "$elemMatch" << BSON( "c" << 1 ) ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << BSON_ARRAY( BSON( "b" << 1 ) << BSON( "b" << 2 ) << BSON( "b" << 1 ) ) ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << BSONArrayBuilder().arr() ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
}

/// elemMatch a:{b:1} from {a:1, b:2} -> {b:2}
TEST( selector, simple_elemmatch_test_3 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a" << BSON( "$elemMatch" << BSON( "b" << 1 ) ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << 1 << "b" << 2 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "b" << 2 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
}

/// elemMatchOne a:{b:1} from {a:[{b:1}, {b:2}, {b:1}]} -> {a:[{b:1}]}
TEST( selector, simple_elemmatchone_test_1 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a" << BSON( "$elemMatchOne" << BSON( "b" << 1 ) ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "a" << BSON_ARRAY( BSON( "b" << 1 ) << BSON( "b" << 2 ) << BSON( "b" << 1 ) ) ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << BSON_ARRAY( BSON("b" << 1 ) ) ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
}

/// jira SEQUOIADBMAINSTREAM-915
TEST( selector, jira_915 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a" << BSON( "$default" << 1 << "$include" << 1 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   BSONObj record = BSON( "b" << 2 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << 1 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
}

TEST( selector, simple_abs_test1 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a" << BSON( "$abs" << 1 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;

   {
   BSONObj record = BSON( "a" << 10 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << 10 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }

   {
   BSONObj record = BSON( "a" << -10 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << 10 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }

   {
   BSONObj record = BSON( "a" << 1.123 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << 1.123 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }

   {
   BSONObj record = BSON( "a" << -1.123 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << 1.123 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }

   {
   BSONObjBuilder builder ;
   BSONObj record = BSON( "a" << "") ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = builder.appendNull( "a" ).obj() ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }
}

/// CEILING
TEST( selector, simple_celling_test1 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a" << BSON( "$ceiling" << 1 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;

   {
   BSONObj record = BSON( "a" << 10 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << 10 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }

   {
   BSONObj record = BSON( "a" << 10.1 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << 11 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }

   {
   BSONObj record = BSON( "a" << -10 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << -10 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }

   {
   BSONObj record = BSON( "a" << -10.1 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << -10 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }

   {
   BSONObjBuilder builder ;
   BSONObj record = BSON( "a" << "") ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = builder.appendNull( "a" ).obj() ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }
}

/// floor
TEST( selector, simple_floor_test1 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a" << BSON( "$floor" << 1 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;

   {
   BSONObj record = BSON( "a" << 10 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << 10 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }

   {
   BSONObj record = BSON( "a" << 10.123 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << 10 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }

   {
   BSONObj record = BSON( "a" << -10 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << -10 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }

   {
   BSONObj record = BSON( "a" << -10.123 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << -11 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }

   {
   BSONObjBuilder builder ;
   BSONObj record = BSON( "a" << "abc" ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = builder.appendNull( "a" ).obj() ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }
}

/// mod
TEST( selector, simple_mod_test1 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a" << BSON( "$mod" << 3 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;

   {
   BSONObj record = BSON( "a" << 10 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << 1 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }

   {
   BSONObj record = BSON( "a" << -10 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << -1 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }


/// 10.123 mod 3 -> 1.1229999999999999
/*
   {
   BSONObj record = BSON( "a" << 10.123 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << 1.123 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }

   {
   BSONObj record = BSON( "a" << -10.123 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << -1.123 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }
*/
   {
   BSONObjBuilder builder ;
   BSONObj record = BSON( "a" << "abc" ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = builder.appendNull( "a" ).obj() ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }
}

/// substr {a:{$substr:3}}
TEST( selector, simple_substr_test1 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a" << BSON( "$substr" << 3 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;

   {
   BSONObj record = BSON( "a" << "abcde" ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << "de" ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }

   {
   BSONObj record = BSON( "a" << "a" ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << "" ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }

   {
   BSONObj record = BSON( "a" << "" ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << "" ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }

   {
   BSONObjBuilder builder ;
   BSONObj record = BSON( "a" << 1 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = builder.appendNull( "a" ).obj() ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }
}

/// substr {a:{$substr:[2,3]}}
TEST( selector, simple_substr_test2 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a" << BSON( "$substr" << BSON_ARRAY( 2 << 3 ) ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;

   {
   BSONObj record = BSON( "a" << "abcdef" ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << "cde" ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }

   {
   BSONObj record = BSON( "a" << "abcde" ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << "cde" ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }

   {
   BSONObj record = BSON( "a" << "a" ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << "" ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }
}

/// substr {a:{$substr:[-2,3]}}
TEST( selector, simple_substr_test3 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a" << BSON( "$substr" << BSON_ARRAY( -2 << 3 ) ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;

   {
   BSONObj record = BSON( "a" << "abcdef" ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << "ef" ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }

   {
   BSONObj record = BSON( "a" << "a" ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << "" ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }

   {
   BSONObj record = BSON( "a" << "" ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << "" ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }
}

/// substr {a:{$substr:-2 }}
TEST( selector, simple_substr_test4 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a" << BSON( "$substr" << -2 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;

   {
   BSONObj record = BSON( "a" << "abcdef" ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << "ef" ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }

   {
   BSONObj record = BSON( "a" << "a" ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << "" ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }

   {
   BSONObj record = BSON( "a" << "" ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << "" ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }
}

/// strlen
TEST( selector, simple_strlen_test1 )
{
   INT32 rc = SDB_OK ;
   mthSelector selector ;
   BSONObj rule = BSON( "a" << BSON( "$strlen" << 1 ) ) ;
   rc = selector.loadPattern( rule ) ;
   ASSERT_EQ( SDB_OK , rc ) ;

   {
   BSONObj record = BSON( "a" << "abcdef" ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << 6 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }

   {
   BSONObj record = BSON( "a" << "" ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = BSON( "a" << 0 ) ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }

   {
   BSONObjBuilder builder ;
   BSONObj record = BSON( "a" << 1 ) ;
   BSONObj result ;
   rc = selector.select( record, result ) ;
   ASSERT_EQ( SDB_OK , rc ) ;
   cout << result.toString( FALSE, TRUE ) << endl ;
   BSONObj expect = builder.appendNull( "a" ).obj() ;
   rc = expect.woCompare( result ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   }
}

