/*******************************************************************************
*@Description : test query ['db.setSessionAttr({"PreferedInstance":'1-7'})'].
*               then verify query is correct or not
*               Using db.snapshot(6) inspect
*@Modify list :
*               2014-11-22  xiaojun Hu  changed
*******************************************************************************/
function getTotalSelectNum( db, clName )
{
   var masNode = new Array ;
   var masHost = new Array ;
   var clRG = commGetCLGroups( db, clName ) ;
   //println( "collection located in : " + clRG ) ;
   for( var i = 0 ; i < clRG.length ; ++i )
   {
      var group = commGetGroups( db ) ;
      for( var j = 0 ; j < group.length ; ++j )
      {
         if( clRG[i] == group[j][0].GroupName )
         {
            for( var m = 0 ; m < group[j][0].Length ; ++m )
            {
               masNode[m] = group[j][m+1].svcname ;    // get master node
               masHost[m] = group[j][m+1].HostName ;   // get master host
            }
         }
      }
   }
   // the data node of group must larger than 1
   try
   {
      if( 2 > masNode.length )
         throw "NotEnoughDataNode" ;
   }
   catch( e )
   {
      println( "data group don't have enough node" ) ;
      throw e ;
   }
   var currentSession = db.snapshot( SDB_SNAP_SESSIONS_CURRENT, {},
                                     {"SessionID":1, "TotalInsert":1,
                                     "TotalSelect":1 }).toArray() ;
   // totalselect[0] is the number of nodes in the group
   var totalselect = new Array( 0, 0 ) ;
   println( "current session length : " + currentSession.length ) ;
   //println( "get svcnames : " + masNode.length ) ;
   // get total data read and total index read in this loop
   for( var i = 0 ; i < currentSession.length ; ++i )
   {
      var sessionObj = eval( "(" + currentSession[i] + ")" ) ;
      var idxRead = sessionObj.SessionID ;
      totalIdxRead = sessionObj.TotalIndexRead ;
      var sessionSplit = idxRead.split( ":" ) ;
      for( var j = 0 ; j < masNode.length ; ++j )
      {
         if( masNode[j] == sessionSplit[1] && masHost[j] == sessionSplit[0] )
         {
            // When excute snapshot, 'TotalSelect' will add 1 times in every
            // sessions.So need add the number of sessions in this group
            totalselect[1] += sessionObj.TotalSelect ;
            println( "host : " + masHost[j] + " node : " + masNode[j]) ;
            println( currentSession[i] ) ;
            break ;
         }
      }
   }
   totalselect[0] = masNode.length ;
   //println( j + " times : " + totalselect ) ;
   return totalselect;
}

function main( db )
{
   var group = commGetGroups( db, false, "", false, false ) ;
   if( 2 > group.length )
   {
      println( "data group is not enough" ) ;
      return ;
   }
   var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, 0, true, true, false,
                          "failed to create cl in the beginning" ) ;
   var recordNum = 1 ;
   cl.insert({a:1}) ;
   // get collection group
   var clName = COMMCSNAME + "." + COMMCLNAME ;

   // 加个数组是因为直接用parseInt(Math.random()*7)的返回值
   // setSessionAttr会报-6错误
   var idArray = new Array(1,2,3,4,5,6,7) ;
   var id = parseInt(Math.random()*7) ;
   println("select id is: " + idArray[id]) ;
   db.setSessionAttr({"PreferedInstance":idArray[id]}) ;
   // make sure the datanode of group is enough or not
   try
   {
      var oldSelectTimes = getTotalSelectNum( db, clName ) ;
   }
   catch( e )
   {
      if( "NotEnoughDataNode" != e )
      {
         println( "failed to get total select number, rc = " + e ) ;
         throw e ;
      }
      else
      {
         println( "data group don't have enough node" ) ;
         return ;
      }
   }
   println("old select times :" + oldSelectTimes ) ;
   var records = db.getCS( COMMCSNAME ).getCL( COMMCLNAME ).find({a:1}).toArray();
   if ( records.length == 0 )
   {
      println( "find record failed ") ;
   }
   println( "" ) ;
   var newSelectTimes = getTotalSelectNum( db, clName ) ;
   println("new select times: " + newSelectTimes ) ;
   if ( newSelectTimes[1] - oldSelectTimes[1] - oldSelectTimes[0] == 1 )
   {
      println( "success to query and inspect 'TotalSelect'." ) ;
      return;
   }
   else
   {
      println( "old times = " + oldSelectTimes[1] ) ;
      println( "new times = " + newSelectTimes[1] ) ;
      println( "number of nodes in group = " + newSelectTimes[0] ) ;
      throw "queryError" ;
   }
}

// Run Main
try
{
   // standalone模式不执行
   if( true == commIsStandalone( db ) )
      throw "StandaloneMode" ;
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
               "failed to drop cl in the beginning" ) ;
   main( db ) ;
   commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
               "failed to drop cl in the end, corrent way" ) ;
   db.close() ;
}
catch( e )
{
   if( "StandaloneMode" != e )
   {
      commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
                  "failed to drop cl in the end, wrong way" ) ;
      db.close() ;
      throw e ;
   }
   else
   {
      println( "run mode is standalone " ) ;
      db.close() ;
   }
}
