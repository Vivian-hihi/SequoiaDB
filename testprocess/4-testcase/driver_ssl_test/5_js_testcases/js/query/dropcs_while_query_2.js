/*******************************************************************************
*@Description : when query contex is exist, drop CS in different session.
*@Test Expect : when the cursor of query still exis in session 1, we drop
*               collection space in session 2. we cannot drop collection space
*               and will throw [error: -147]
*@Modify List :
*               2014-9-26   xiaojunHu  Init
*******************************************************************************/

function main( db )
{
   var csName = CSPREFIX + "_dropcswhilequery_2" ; // create new CL, not common
   var fullCL = csName + "." + COMMCLNAME ;
   commDropCL( db, csName, COMMCLNAME, true, true,
               "failed to drop cl in the beginning" ) ;
   var cl = commCreateCL( db, csName, COMMCLNAME, -1, true, true, false,
                          "failed to create collection in the beginning" ) ;
   for( var i = 0 ; i < 1000 ; ++i )
   {
      cl.insert( {no:i, num:2000-i, description:"when query contex is exist," +
                  "drop CS in same session." } ) ;
   }
   var cnt = 0 ;
   var count = 0 ;
   println( "success to insert data" ) ;
   println( db.list(3) ) ;
   do
   {
      count = cl.count() ;
      ++cnt ;
   }while( 1000 != count || cnt < 100 ) ;
   if( 1000 != count )
   {
      println( "insert records number = " + count ) ;
      throw "insert wrong records" ;
   }
   println( "success to count" ) ;
   var newDB, newDb ;
   if( false == commIsStandalone( db ) )   // group
   {
      var csRgName = commGetCLGroups( db, fullCL ) ;   // get CL groups
      if( csRgName.length > 1 )
         throw "collection located in two more groups" ;
      var rgName = csRgName[0] ;
      var rg = db.getRG( rgName ) ;
      var master = rg.getMaster() ;   // get master node
      var _master = String(master).split(":") ;   // is string not object
      // connect to primary node query
      newDB = new SecureSdb( _master[0], _master[1] ) ;
      var cs = newDB.getCS( csName ) ;
      var cl = cs.getCL( COMMCLNAME ) ;
      var cursor = cl.find() ;
      println( cursor.next() ) ;
      var tmpArray = new Array() ;
      var snapshotInfo = db.snapshot( SDB_SNAP_CONTEXTS_CURRENT ).toArray() ;
      var index = 0 ;
      // create new connection and drop CS [TEST POINT]
      newDb = new SecureSdb( _master[0], _master[1] ) ;   // new session
   }
   else  // standalone
   {
      // connect to standalone node
      var cs = db.getCS( csName ) ;
      var cl = cs.getCL( COMMCLNAME ) ;
      var cursor = cl.find() ;
      println( cursor.next() ) ;
      var tmpArray = new Array() ;
      var snapshotInfo = db.snapshot( SDB_SNAP_CONTEXTS_CURRENT ).toArray() ;
      var index = 0 ;
      // create new connection and drop CS [TEST POINT]
      newDb = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;   // new session
   }
   println( "<create new connection and drop cs when current contexts exist>" ) ;
   var desContext = false ;
   for( var i = 0 ; i < snapshotInfo.length ; ++i )
   {
      var cntxtObj = eval( "(" + snapshotInfo[i] + ")" ) ;
      tmpArray[index] = Array() ;
      tmpArray[index][0] = new Object() ;
      var groupObj = cntxtObj.Contexts ;
      tmpArray[index][0].Length = groupObj.length ;
      //here check current contexts is exist or not and then drop cs
      for( var j = 0 ; j < groupObj.length ; ++j )
      {
         var contextObj = groupObj[j] ;
         //println( contextObj.Description ) ;
         if( "BufferSize:0" != contextObj.Description )
         {
            commPrint( contextObj ) ;
            desContext = true ;
            break ;
         }
      }
      ++index ;
   }
   try
   {
      newDb.dropCS( csName ) ;
      throw "<should not be deleted successfully>" ;
   }
   catch( e )
   {
      if( -147 != e )
      {
         println( "failed to run <drop cs>, rc = " + e ) ;
         throw e ;
      }
   }
   if( false == commIsStandalone( db ) )
      newDB.close() ;
   else
      db.close() ;
   newDb.close() ;   // this db do drop cs
   // clean collection space
   var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;
   commDropCS( db, csName, false, "failed to drop cl in the end" ) ;
}

try
{
   main( db ) ;
}
catch( e )
{
   throw e ;
}
