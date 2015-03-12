/* *****************************************************************************
@discretion: Prepare before all test-case
@modify list:
   2014-3-1 Jianhui Xu  Init
***************************************************************************** */

var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;

function main( db )
{
   // 1. check nodes
   var groups = commGetGroups( db, "", "", false ) ;
   var errNodes = commCheckBusiness( groups, true ) ;
   if ( errNodes.length == 0 )
   {
   }
   else
   {
      println( "Has " + errNodes.length + " nodes in fault before all test-cases: " ) ;
      commPrint( errNodes ) ;
   }

   // 2. drop CSPREFIX's all collection space
   var cols = commGetCSCL( db, CSPREFIX ) ;
   for ( var i = 0; i < cols.length; ++i )
   {
      try
      {
         commDropCS( db, cols[i].cs, true, " before all test-cases" ) ;
      }
      catch( e )
      {
         println( "Drop " + cols[i].cs + " failed before all test-cases: " + e ) ;
      }
   }

}

try
{
   main( db ) ;
}
catch( e )
{
   println( "Before all test-cases environment prepare failed: " + e ) ;
}
