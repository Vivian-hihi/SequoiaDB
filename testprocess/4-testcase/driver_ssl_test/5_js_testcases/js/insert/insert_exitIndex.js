/*******************************************************************************
*@Description : first create index for collection, then insert data to this
*               collection.
*@Modify List :
*               2014-11-27   xiaoJun Hu   Changed
*******************************************************************************/
function getPrimaryHostAndNode( csName, clName )
{
   var FULL_CL= csName + "." + clName ;
   var clRG = commGetCLGroups( db, FULL_CL ) ;   // get cl group name
   var dataRG = commGetGroups( db ) ;
   var primHostSvc = new Array() ;
   for( var n = 0 ; n < clRG.length ; ++n )
   {
      for( var i = 0 ; i < dataRG.length ; ++i )
      {
         var rgName = dataRG[i][0].GroupName ;
         if( clRG[n] == rgName )
         {
            var primaryNode = dataRG[i][0].PrimaryNode ;
            primHostSvc[n] = Array() ;
            for( var j = 1 ; j < dataRG[i].length ; ++j )
            {
               var nodeID = dataRG[i][j].NodeID ;
               var primCnt = 0 ;
               if( primaryNode == nodeID )
               {
                  primHostSvc[n][primCnt] = new Object() ;
                  primHostSvc[n][primCnt].HostName = dataRG[i][j].HostName ;
                  primHostSvc[n][primCnt].svcname = dataRG[i][j].svcname ;
                  break ;
               }
            }
         }
         else
         {
            continue ;
         }
      }
   }
   return primHostSvc ;
}

function main( db )
{
   var insertNum = 1000 ;
   var indexName = "noIdx" + CSPREFIX ;
   var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, 0, true, true, false,
                          "create collection in the beginning" ) ;
   // create index and check
   cl.createIndex( indexName, { no: -1 }, true ) ;
   var newDB = 0 ;
   var primaryInfo = getPrimaryHostAndNode( COMMCSNAME, COMMCLNAME ) ;
   for( var i = 0 ; i < primaryInfo.length ; ++i )
   {
      for( var j = 0 ; j < primaryInfo[i].length ; ++j )
      {
         println( "hostname : " + primaryInfo[i][j].HostName ) ;
         println( "svcname : " + primaryInfo[i][j].svcname ) ;
         newDB = new SecureSdb( primaryInfo[i][j].HostName, primaryInfo[i][j].svcname ) ;
         var newCS = newDB.getCS( COMMCSNAME ) ;
         var newCL = newCS.getCL( COMMCLNAME ) ;
         var _getIdx = newCL.getIndex( indexName ) ;
         var getIdx = JSON.parse( _getIdx ) ;
         println( "index name : " + getIdx.IndexDef.name ) ;
         if( indexName == getIdx.IndexDef.name )
         {
            println( "create index '" + indexName + "' success" ) ;
         }
         else
         {
            println( "index name : " + indexName ) ;
            println( cl.listIndexes() ) ;
            throw "failed to create index" ;
         }
      }
   }
   // insert data
   insertAndCheck( cl, insertNum ) ;
}

// Run Main
try
{
   if( false == commIsStandalone( db ) )
   {
      commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
                  "clear collection in the beginning" ) ;
      main( db ) ;   // main
      commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
                  "clear collection in the end, correct" ) ;
   }
   else
   {
      println( "run mode is standalone" ) ;
   }
   db.close() ;
}
catch( e )
{
   commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
               "clear collection in the end, correct" ) ;
   db.close() ;
   throw e ;
}
