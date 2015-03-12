/* *****************************************************************************
@discretion: Insert record normal case
@modify list:
   2014-3-1 Jianhui Xu  Init
***************************************************************************** */

var insertCL1 = COMMCLNAME + "_1" ;
var insertCL2 = COMMCLNAME + "_2" ;

function main( db )
{
   // drop cl
   commDropCL( db, COMMCSNAME, insertCL1, true, true, "drop cl1 in begin" ) ;
   commDropCL( db, COMMCSNAME, insertCL2, true, true, "drop cl2 in begin" ) ;

   // create cl
   var varCL1 = commCreateCL( db, COMMCSNAME, insertCL1, -1, true, true, false, "create cl1 in begin" ) ;
   var varCL2 = commCreateCL( db, COMMCSNAME, insertCL2, -1, false, true, false, "create cl2 in begin" ) ;

   var numArray = new Array( 1, 10, 100, 200 ) ;
   for ( var i = 0 ; i < numArray.length ; ++i )
   {
      if ( i % 2 == 0 )
      {
         insertAndCheck( varCL1, numArray[i], true, true, "cl1: " + numArray[i] ) ;
      }
      else
      {
         insertAndCheck( varCL2, numArray[i], true, true, "cl2: " + numArray[i] ) ;
      }
   }

   // clean - drop cl
   commDropCL( db, COMMCSNAME, insertCL1, false, false, "drop cl1 in clean" ) ;
   commDropCL( db, COMMCSNAME, insertCL2, false, false, "drop cl2 in clean" ) ;
}

// main entry
try
{
   main( db ) ;
   db.close() ;
}
catch( e )
{
   throw e ;
}
