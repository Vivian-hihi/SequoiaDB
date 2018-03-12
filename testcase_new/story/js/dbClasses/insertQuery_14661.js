/*******************************************************************
* @Description : insert and query Sdb SdbCS SdbCollection SdbCursor
*                SdbQuery SdbReplicaGroup SdbNode SdbDomain CLCount
*                seqDB-14661:插入并查询db/cs/cl等对象                
* @author      : Liang XueWang
*                2018-03-07
*******************************************************************/
var clName = COMMCLNAME + "_dbClasses14661" ;

main( db ) ;

function main( db )
{
   var cl = commCreateCL( db, COMMCSNAME, clName, 0 ) ;
   
   insertQuerySdb( cl ) ;
   insertQuerySdbCS( cl ) ;
   insertQuerySdbCollection( cl ) ;
   insertQuerySdbCursor( cl ) ;
   insertQuerySdbQuery( cl ) ;  // can't insert SdbQuery
   if( !commIsStandalone( db ) )
   {
      insertQuerySdbReplicaGroup( db, cl ) ;
      insertQuerySdbNode( db, cl ) ;
      insertQuerySdbDomain( db, cl ) ;
   }
   insertQueryCLCount( cl ) ;  // can't insert CLCount
   
   // SdbQuery CLCount has _collection inside, cause insert error
   
   commDropCL( db, COMMCSNAME, clName ) ;
}

function insertQuerySdb( cl )
{
   var db1 = new Sdb( COORDHOSTNAME, COORDSVCNAME ) ;
   cl.truncate() ;
   cl.insert( db1 ) ;
   var obj = cl.find().next().toObj() ;
   var host = obj["_host"] ;
   var port = obj["_port"] ;
   if( host !== COORDHOSTNAME ||
       port !== COORDSVCNAME )
   {
      throw buildException( "insertQuerySdb", null, "check sdb",
            COORDHOSTNAME + ":" + COORDSVCNAME, host + ":" + port ) ;
   }
   db1.close() ;
}

function insertQuerySdbCS( cl )
{
   var cs = db.getCS( COMMCSNAME ) ;
   cl.truncate() ;
   cl.insert( cs ) ;
   var obj = cl.find().next().toObj() ;
   var name = obj["_name"] ;
   if( name !== COMMCSNAME )
   {
      throw buildException( "insertQuerySdbCS", null, "check sdbcs",
            COMMCSNAME, name ) ;
   }
}

function insertQuerySdbCollection( cl )
{
   cl.truncate() ;
   cl.insert( cl ) ;
   var obj = cl.find().next().toObj() ;
   var name = obj["_name"] ;
   if( name !== clName )
   {
      throw buildException( "insertQuerySdbCollection", null, "check sdbcl",
            clName, name ) ;
   }
}

function insertQuerySdbCursor( cl )
{
   cl.truncate() ;
   var cursor = cl.find().explain( { Run: true } ) ;
   cursor.toArray() ;
   cl.insert( cursor ) ;
   var info = cl.find().next().toObj()["_arr"][0] ;
   var obj = JSON.parse( info ) ;
   var name = obj["Name"] ;
   if( name !== COMMCSNAME + "." + clName )
   {
      throw buildException( "insertQuerySdbCursor", null, "check sdbcursor",
            COMMCSNAME + "." + clName, name ) ;
   }
}

function insertQuerySdbQuery( cl )
{
   cl.truncate() ;
   var query = cl.find() ;
   try
   {
      cl.insert( query ) ;
      throw 0 ;
   }
   catch( e )
   {
      if( e !== -6 )
      {
         throw buildException( "insertQuerySdbQuery", e, "check sdbquery",
               -6, e ) ;
      }
   }
}

function insertQuerySdbReplicaGroup( db, cl )
{
   var rg = db.getRG( "SYSCoord" ) ;
   cl.truncate() ;
   cl.insert( rg ) ;
   var obj = cl.find().next().toObj() ;
   var name = obj["_name"] ;
   if( name !== "SYSCoord" )
   {
      throw buildException( "insertQuerySdbReplicaGroup", null, "check sdbReplicaGroup",
            "SYSCoord", name ) ;
   }
}

function insertQuerySdbNode( db, cl )
{
   var rg = db.getRG( "SYSCoord" ) ;
   var node = rg.getSlave() ;
   var host = node.getHostName() ;
   var svc = node.getServiceName() ;
   
   cl.truncate() ;
   cl.insert( node ) ;
   var obj = cl.find().next().toObj() ;
   var host1 = obj["_hostname"] ;
   var svc1 = obj["_servicename"] ;
   if( host !== host1 || svc !== svc1 )
   {
      throw buildException( "insertQuerySdbNode", null, "check sdbNode",
            host + ":" + svc, host1 + ":" + svc1 ) ;
   }
}

function insertQuerySdbDomain( db, cl )
{
   var groups = getDataGroups( db ) ;
   var domainName = "testDomain14661" ;
   var domain = db.createDomain( domainName, groups ) ;
   
   cl.truncate() ;
   cl.insert( domain ) ;
   var obj = cl.find().next().toObj() ;
   var name = obj["_domainname"] ;
   if( name !== domainName )
   {
      throw buildException( "insertQuerySdbDomain", null, "check sdbDomain",
            domainName, name ) ;
   }
   db.dropDomain( domainName ) ;
}

function insertQueryCLCount( cl )
{
   var cnt = cl.find().count() ;
   try
   {
      cl.insert( cnt ) ;
      throw 0 ;
   }
   catch( e )
   {
      if( e !== -6 )
      {
         throw buildException( "insertQueryCLCount", e, "check clcount",
               -6, e ) ;
      }
   }
}