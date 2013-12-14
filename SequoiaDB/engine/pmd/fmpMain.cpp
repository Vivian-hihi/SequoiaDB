#include "fmpController.hpp"
#include <string>

using namespace std ;

INT32 main( INT32 argc, CHAR **argv )
{
/*
   INT32 rc = SDB_OK ;
   _fmpJSVM vm ;
   if ( !vm.ok() )
   {
      return 0 ;
   }

   {
   BSONObj res ;
   BSONObjBuilder builder ;
//   BSONObjBuilder builder1, builder2, builder3 ;
//   builder.appendCode("func", argv[1]) ;
//   builder1.appendCode( "func", "function sum1(x,y){return sum2(x,y);}" ) ;
//   rc = vm.compile( builder1.obj().firstElement() , "sum1") ;
//   rc = vm.eval( builder1.obj(), res ) ;
//   if ( SDB_OK != rc )
//   {
//      cout << "compile sum1 failed" << endl ;
//      return 0 ;
//   }

//   builder2.appendCode( "func", "function sum2(x,y){return x+y;}" ) ;
//   rc = vm.compile( builder2.obj().firstElement() , "sum2") ;
//   rc = vm.eval( builder2.obj(), res ) ;
//   if ( SDB_OK != rc )
//   {
//      cout << "compile sum2 failed" << endl ;
//      return 0 ;
//   }

//   builder3.appendCode("func", "sum1(1,2);") ;
   rc = vm.eval( builder.obj(), res ) ;
   BSONElement type = res.getField( FMP_RES_TYPE ) ;
   if ( type.eoo() || NumberInt != type.type() )
   {
      cout << "sth wrong happened:"<< res.toString() << endl ;
   }
   else if ( FMP_RES_TYPE_RECORDSET != type.Int() )
   {
      cout << "normal res:" << res.toString() << endl ;
   }
   else
   {
      while (TRUE )
      {
         rc = vm.fetch( res ) ;
         if ( SDB_DMS_EOC == rc )
         {
            break ;
         }
         else if ( SDB_OK == rc )
         {
            cout << "fetch:" << res.toString() << endl ;
         }
         else
         {
            cout << "sth wrong happened:"<< res.toString() << endl ;
            break ;
         }
      }
   }
   }
*/

   INT32 rc = SDB_OK ;
   fmpController controller ;
   rc = controller.run() ;

   return rc ;
}
