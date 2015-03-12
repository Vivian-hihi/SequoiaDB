/*******************************************************************************
*@Description : when query contex is exist, drop CS in same session.
*@Test Expect : when query in same session and the cursor of query still
*               exist, we can drop collection space in the same session.
*@Modify List :
*               2014-9-26   xiaojunHu  Init
*******************************************************************************/
function main( db )
{
   var csName = CSPREFIX + "_dropcswhilequery_1" ;
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
   var cursor = cl.find() ;
   println( cursor.next() ) ;
   var tmpArray = new Array() ;
   var snapshotInfo = db.snapshot( SDB_SNAP_CONTEXTS_CURRENT ).toArray() ;
   var index = 0 ;
   println( "<begin to drop cs when current contexts exist>" ) ;
   for( var i = 0 ; i < snapshotInfo.length ; ++i )
   {
      var cntxtObj = eval( "(" + snapshotInfo[i] + ")" ) ;
      tmpArray[index] = Array() ;
      tmpArray[index][0] = new Object() ;
      var groupObj = cntxtObj.Contexts ;
      tmpArray[index][0].Length = groupObj.length ;
      //here check current contexts is exist or not and then drop cs
      try
      {
         for( var j = 0 ; j < groupObj.length ; ++j )
         {
            var contextObj = groupObj[j] ;
            //println( contextObj.Description ) ;
            if( "BufferSize:0" != contextObj.Description )
            {
               try
               {
                  db.dropCS( csName ) ;
                  throw "drop cs over" ;
               }
               catch( e )
               {
                  commPrint( contextObj ) ;
                  println( "drop CS = [" + csName + "], rc = " + e ) ;
                  throw e ;
               }
            }
         }
      }
      catch( e )
      {
         if( "drop cs over" != e )
         {
            println( "failed to drop cs, rc = " + e ) ;
            throw e ;
         }
      }
      ++index ;
   }
}

try
{
   main( db ) ;
   db.close() ;
}
catch( e )
{
   throw e ;
}
