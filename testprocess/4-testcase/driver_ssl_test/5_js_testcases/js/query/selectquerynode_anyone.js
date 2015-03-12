/*******************************************************************************
*@Description : test query ['TotalSelect']. Using db.snapshot(6) inspect
*@Modify list :
*               2014-6-20  xiaojun Hu  Init
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
   var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, 0, true, true, false,
                          "failed to create cl in the beginning" ) ;
   var recordNum = 1 ;
   cl.insert({a:1});
   // get collection group
   var clName = COMMCSNAME + "." + COMMCLNAME ;
   // set session attribution
   db.setSessionAttr({"PreferedInstance":'A'});
   println("SessionAttr({\"PreferedInstance\":\'A\'})");
   // 原来的读次数
   var oldSelectTimes = getTotalSelectNum( db, clName ) ;
   var records = db.getCS(COMMCSNAME).getCL(COMMCLNAME).find({a:1}).toArray();
   println( "records number : " + records.length ) ;
   if (records.length == 0)
      throw "find record failed" ;
   println( "" ) ;
   var newSelectTimes = getTotalSelectNum( db, clName ) ;
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

// Main Running
try
{
   // standalone模式不执行
   if( true == commIsStandalone( db ) )
      throw "StandaloneMode" ;
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
               "failed to drop cl in the beginning" ) ;
   main( db );   // main function body
   commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
               "failed to drop cl in the end, corrent way" ) ;
   db.close() ;
}
catch( e)
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
      println( "run mode is standalone, exit" ) ;
      db.close() ;
   }
}
