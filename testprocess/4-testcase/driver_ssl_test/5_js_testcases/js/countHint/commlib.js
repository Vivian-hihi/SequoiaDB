/******************************************************************************
*@Description : Test the hint index common function.
*@Modify list :
*               2014-6-12   xiaojun Hu  Init
*               2014-11-10  xiaojun Hu  Change
******************************************************************************/

var hostName = COORDHOSTNAME ;
var coordPort = COORDSVCNAME ;
//var csName = COMMCSNAME ; // If CS don't need change, use it.
//var clName = COMMCLNAME ;
var db = new SecureSdb( hostName, coordPort ) ;

// Insert data to SequoiaDB
function insertData( db, csName, clName )
{
   try
   {
      var cs = db.getCS( csName ) ;
      var cl = cs.getCL( clName ) ;
      // insert 10000 records in CL
      for( var i = 0 ; i < 10000 ; ++i )
      {
         cl.insert( { no:i, score:i, interest:["movie", "photo"],
                      major:"计算机软件与理论", dep:"计算机学院",
                      info:{name:"Holiday", age:22, sex:"男"} } ) ;
      }
      var cnt = 0 ;
      do
      {
         ++cnt ;
         var count = cl.count() ;
         if( 10000 == count )
            break ;
      }while( cnt < 10000 ) ;
      if( 10000 != count )
      {
         println( "Wrong quantity of records, count = " + count ) ;
         throw "ErrNumRecords" ;
      }
      println( "success to insert data : " + count ) ;
   }
   catch ( e )
   {
      println( "Failed to insert data to SDB, rc = " + e  ) ;
      throw e ;
   }
}

// Create index
function createIndex( cl, idxName, idxKeygen, unique, enforced )
{
   if ( undefined == unique ) { unique = false ; }
   if ( undefined == enforced ) { enforced = false ; }
   //if ( undefined == errno ) { errno = "" ; }
   try
   {
      if( undefined == cl || undefined == idxName || undefined == idxKeygen )
      {
         println( "Please check the argument of createIndex" ) ;
         throw "ErrArg" ;
      }
      cl.createIndex( idxName, idxKeygen, unique, enforced ) ;
   }
   catch ( e )
   {
      println( "Failed to create index, rc = "+ e ) ;
      throw e ;
   }
}

// Inspect index
function hintInspectIndex( cl, indexName , indexKey , keyValue ,
                           idxUnique ,idxEnforced )
{
   if ( undefined == idxUnique ) { idxUnique = false ; }
   if ( undefined == idxEnforced ) { idxEnforced = false ; }
   try
   {
      if ( undefined == cl || undefined == indexName || undefined == indexKey ||
           undefined == keyValue )
      {
         println( " wrong argument when inspect index " ) ;
         throw "ErrArg" ;
      }
      var getIndex = new Boolean(true) ;
      getIndex = cl.getIndex( indexName ) ;
      var cnt = 0 ;
      while ( cnt < 20 )
      {
         getIndex = cl.getIndex( indexName ) ;
         if ( undefined != getIndex )
         {
            break ;
         }
         ++cnt ;
      }
      if ( undefined == getIndex )
      {
         println( "Don't have the index, name = "+indexName ) ;
         throw "ErrIdxName" ;
      }
      //println(cl.getIndex( indexName )) ;
      var indexDef = getIndex.toString() ;
      indexDef = eval('('+indexDef+')') ;
      var index = indexDef[ "IndexDef" ] ;
      if ( keyValue != index["key"][indexKey] )
      {
         println( "Wrong index name or key value : "+index["key"][indexKey] ) ;
         throw "ErrIdxValue" ;
      }
      if ( idxUnique != index["unique"] )
      {
         println( "Wrong index unique : "+index["unique"] ) ;
         throw "ErrIdxUnique" ;
      }
      if ( idxEnforced != index["enforced"] )
      {
         println( "Wrong index enforced : "+index["enforced"] ) ;
         throw "ErrIdxEnforced" ;
      }
      println("Success to inspect index : "+indexName) ;
   }
   catch ( e )
   {
      println( "argument value:'"+indexName+"','"+indexKey+
               "','"+keyValue+"','"+idxUnique+"','"+idxEnforced ) ;
      println( "Failed to inspect index : "+indexName+" rc=: "+e ) ;
      throw e ;
   }
}

// get the read data or read index
function snapshotGetValue( db, objKey )
{
   try
   {
      var snap6 = db.snapshot( 6 ) ;
      var snapArray = new Array() ;
      while( snap6.next() )
      {
         snapArray.push( snap6.current().toObj()[ objKey ] ) ;
      }
      return snapArray[0] ;
   }
   catch ( e )
   {
      println( "Failed to get the data read or index read, rc = " + e ) ;
      throw e ;
   }
}

// get index read
function hintSnapshotSession( db, clName )
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
                                     {"SessionID":1, "TotalIndexRead":1,
                                     "TotalDataRead":1 }).toArray() ;
   var dataIdx = new Array( 0, 0 ) ;
   //println( "current session length : " + currentSession.length ) ;
   //println( "get svcnames : " + masNode.length ) ;
   // get total data read and total index read in this loop
   for( var i = 0 ; i < currentSession.length ; ++i )
   {
      var sessionObj = eval( "(" + currentSession[i] + ")" ) ;
      var _sessionID = sessionObj.SessionID ;
      var sessionSplit = _sessionID.split( ":" ) ;  // get host and split
      if( false == commIsStandalone( db ) )   // group
      {
         for( var j = 0 ; j < masNode.length ; ++j )
         {
            if( masNode[j] == sessionSplit[1] && masHost[j] == sessionSplit[0] )
            {
               dataIdx[0] += sessionObj.TotalDataRead ;
               dataIdx[1] += sessionObj.TotalIndexRead ;
               println( "current session : " + currentSession[i] ) ;
               //println( "host : " + masHost[j] + " node : " + masNode[j]) ;
               break ;
            }
         }
      }
      else   // standalone
      {
         dataIdx[0] = sessionObj.TotalDataRead ;
         dataIdx[1] = sessionObj.TotalIndexRead ;
         println( db.snapshot( SDB_SNAP_SESSIONS_CURRENT, {},
                               {"SessionID":1, "TotalIndexRead":1,
                               "TotalDataRead":1 }) ) ;
      }
   }
   //println( j + " times : " + dataIdx[0] + "--" + dataIdx[1] ) ;
   return dataIdx ;
}
