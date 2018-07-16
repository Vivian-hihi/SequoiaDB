/*******************************************************************
* @Description : create procedure and eval to return
*                Sdb SdbCS SdbCollection SdbCursor SdbQuery
*                SdbReplicaGroup SdbNode SdbDomain CLCount
*                BinData ObjectId Timestamp Regex MinKey MaxKey
*                NumberLong SdbDate
*                seqDB-14662:执行存储过程返回db/cs/cl等对象                
* @author      : Liang XueWang
*                2018-03-10
*******************************************************************/
var clName = COMMCLNAME + "_dbClasses14662" ;

main( db ) ;

function main( db )
{
   if( commIsStandalone( db ) )
   {
      println( "Run mode is standalone" ) ;
      return ;
   }

   commCreateCL( db, COMMCSNAME, clName, 0 ) ;
      
   evalSdb( db ) ;  // can't return Sdb
   evalSdbCS( db ) ;
   evalSdbCollection( db ) ;
   evalSdbCursor( db ) ;
   evalSdbQuery( db ) ;
   evalSdbReplicaGroup( db ) ;
   evalSdbNode( db ) ;
   evalSdbDomain( db ) ;
   evalCLCount( db ) ;
   evalBinData( db ) ;
   evalObjectId( db ) ;
   evalTimestamp( db ) ;
   evalRegex( db ) ;
   evalMinKey( db ) ;
   evalMaxKey( db ) ;
   evalNumberLong( db ) ;
   evalSdbDate( db ) ;
   
   commDropCL( db, COMMCSNAME, clName ) ;
}

function evalSdb( db )
{
   db.createProcedure( function getSdb( host, svc ) { 
                       var sdb = new Sdb( host, svc ) ; 
                       return sdb ; } ) ;
   try
   {
      var sdb = db.eval( "getSdb( \"" + COORDHOSTNAME + "\", \"" + COORDSVCNAME + "\" )" ) ;
      throw 0 ;
   }
   catch( e )
   {
      if( e !== -10 )
      {
         throw buildException( "evalSdb", e, "get sdb", -10, e ) ;
      }
   }
   db.removeProcedure( "getSdb" ) ;
}

function evalSdbCS( db )
{
   db.createProcedure( function getSdbCS( host, svc, csname ) {
                       var sdb = new Sdb( host, svc ) ; 
                       return sdb.getCS( csname ) ; } ) ;
   try
   {
      var cs = db.eval( "getSdbCS( \"" + COORDHOSTNAME + "\", \"" + COORDSVCNAME +
                        "\", \"" + COMMCSNAME + "\" )" ) ;
      println( "cs instanceof SdbCS: " + ( cs instanceof SdbCS ) ) ;
      cs.toString() ;
      var cl = cs.getCL( clName ) ;
   }
   catch( e )
   {
      throw buildException( "evalSdbCS", e, "get sdbcs", 0, e ) ;
   }
   db.removeProcedure( "getSdbCS" ) ;
}

function evalSdbCollection( db )
{
   db.createProcedure( function getSdbCollection( host, svc, csname, clname ) {
                       var sdb = new Sdb( host, svc ) ;
                       return sdb.getCS( csname ).getCL( clname ) ; } ) ;
   try
   {
      var cl = db.eval( "getSdbCollection( \"" + COORDHOSTNAME + "\", \"" + COORDSVCNAME + 
                        "\", \"" + COMMCSNAME + "\", \"" + clName + "\" )" ) ;
      println( "cl instanceof SdbCollection: " + ( cl instanceof SdbCollection ) ) ;
      // cl.toString() ;
      cl.find() ;
   }
   catch( e )
   {
      throw buildException( "evalSdbCollection", e, "get sdbcl", 0, e ) ;
   }
   db.removeProcedure( "getSdbCollection" ) ;
}

function evalSdbCursor( db )
{
   db.createProcedure( function getSdbCursor( host, svc, csname, clname ) {
                       var sdb = new Sdb( host, svc ) ;
                       var collection = db.getCS( csname ).getCL( clname ) ;
                       return collection.find().explain( { Run: true } ) ; } ) ;
   try
   {
      var cursor = db.eval( "getSdbCursor( \"" + COORDHOSTNAME + "\", " + 
                            "\"" + COORDSVCNAME + "\", \"" + COMMCSNAME + "\", \"" + 
                            clName + "\" )" ) ;
      println( "cursor instanceof SdbCursor: " + ( cursor instanceof SdbCursor ) ) ;
      cursor.next() ;
   }
   catch( e )
   {
      throw buildException( "evalSdbCursor", e, "get sdbcursor", 0, e ) ;
   }
   db.removeProcedure( "getSdbCursor" ) ;
}

function evalSdbQuery( db )
{
   db.createProcedure( function getSdbQuery( host, svc, csname, clname ) {
                       var sdb = new Sdb( host, svc ) ;
                       var collection = db.getCS( csname ).getCL( clname ) ;
                       return collection.find() ; } ) ;
   try
   {
      var query = db.eval( "getSdbQuery( \"" + COORDHOSTNAME + "\", " + 
                            "\"" + COORDSVCNAME + "\", \"" + COMMCSNAME + "\", \"" + 
                            clName + "\" )" ) ;
      println( "query instanceof SdbQuery: " + ( query instanceof SdbQuery ) ) ; // SdbCursor
      query.size() ;
   }
   catch( e )
   {
      throw buildException( "evalSdbQuery", e, "get sdbquery", 0, e ) ;
   }
   db.removeProcedure( "getSdbQuery" ) ;
}

function evalSdbReplicaGroup( db )
{
   db.createProcedure( function getSdbReplicaGroup( host, svc, rgname ) {
                       var sdb = new Sdb( host, svc ) ;
                       return sdb.getRG( rgname ) ; } ) ;
   try
   {
      var rg = db.eval( "getSdbReplicaGroup( \"" + COORDHOSTNAME + "\", " + 
                        "\"" + COORDSVCNAME + "\", \"SYSCoord\" )" ) ;
      println( "rg instanceof SdbReplicaGroup: " + ( rg instanceof SdbReplicaGroup ) ) ;
      rg.getDetail() ;
   }
   catch( e )
   {
      throw buildException( "evalSdbReplicaGroup", e, "get sdbReplicaGroup", 0, e ) ;
   }
   db.removeProcedure( "getSdbReplicaGroup" ) ;
}

function evalSdbNode( db )
{
   db.createProcedure( function getSdbNode( host, svc, rgname ) {
                       var sdb = new Sdb( host, svc ) ;
                       var rg = sdb.getRG( rgname ) ;
                       return rg.getSlave() ; } ) ;
   try
   {
      var node = db.eval( "getSdbNode( \"" + COORDHOSTNAME + "\", " + 
                          "\"" + COORDSVCNAME + "\", \"SYSCoord\" )" ) ;
      println( "node instanceof SdbNode: " + ( node instanceof SdbNode ) ) ;
      node.getHostName() ;
   }
   catch( e )
   {
      throw buildException( "evalSdbNode", e, "get sdbNode", 0, e ) ;
   }
   db.removeProcedure( "getSdbNode" ) ;
}

function evalSdbDomain( db )
{
   var groups = getDataGroups( db ) ;
   var domainName = "testDomain14662" ;
   db.createDomain( domainName, groups ) ;
   
   db.createProcedure( function getSdbDomain( host, svc, domainname ) {
                       var sdb = new Sdb( host, svc ) ;
                       return sdb.getDomain( domainname ) ; } ) ;
   try
   {
      var domain = db.eval( "getSdbDomain( \"" + COORDHOSTNAME + "\", " + 
                            "\"" + COORDSVCNAME + "\", \"" + domainName + "\" )" ) ;
   }
   catch( e )
   {
      throw buildException( "evalSdbDomain", e, "get sdbDomain", -6, e ) ;
   }
   db.removeProcedure( "getSdbDomain" ) ;
   
   db.dropDomain( domainName ) ;
}
   
function evalCLCount( db )
{
   db.createProcedure( function getCLCount( host, svc, csname, clname ) {
                       var sdb = new Sdb( host, svc ) ;
                       var cl = sdb.getCS( csname ).getCL( clname ) ;
                       return cl.count() ; } ) ;
   try
   {
      var cnt = db.eval( "getCLCount( \"" + COORDHOSTNAME + "\", " + 
                         "\"" + COORDSVCNAME + "\", \"" + COMMCSNAME + 
                         "\", \"" + clName + "\" )" ) ;
      throw 0 ;
   }
   catch( e )
   {
      if( e !== -6 )
      {
         throw buildException( "evalCLCount", e, "get CLCount", -6, e ) ;
      }
   }
   db.removeProcedure( "getCLCount" ) ;
}
   
function evalBinData( db )
{
   db.createProcedure( function getBinData( data, type ) {
                       return BinData( data, type ) ; } ) ;
   var data = "aGVsbG8gd29ybGQ" ;
   var type = "1" ;
   var bindata = db.eval( "getBinData( \"" + data + "\", \"" + type + "\" )" ) ;
   println( "bindata instanceof BinData: " + ( bindata instanceof BinData ) ) ;
   var expectval = BinData( data, type ) ;
   if( bindata.toString() !== expectval.toString() )
   {
      throw buildException( "evalBinData", null, "get BinData",
            expectval, bindata ) ;
   }
   db.removeProcedure( "getBinData" ) ;
}
   
function evalObjectId( db )
{
   db.createProcedure( function getObjectId( data ) {
                       return ObjectId( data ) ; } ) ;
   var data = "55713f7953e6769804000001" ;
   var oid = db.eval( "getObjectId( \"" + data + "\" )" ) ;
   println( "oid instanceof ObjectId: " + ( oid instanceof ObjectId ) ) ;
   var expectval = ObjectId( data ) ;
   if( oid.toString() !== expectval.toString() )
   {
      throw buildException( "evalObjectId", null, "get ObjectId",
            expectval, oid ) ;
   }
   db.removeProcedure( "getObjectId" ) ;
}
   
function evalTimestamp( db )
{
   db.createProcedure( function getTimestamp( time ) {
                       return Timestamp( time ) ; } ) ;
   var time = "2015-06-05-16.10.33.000000" ;
   var timestamp = db.eval( "getTimestamp( \"" + time + "\" )" ) ;
   println( "timestamp instanceof Timestamp: " + ( timestamp instanceof Timestamp ) ) ;
   var expectval = Timestamp( time ) ;
   if( timestamp.toString() !== expectval.toString() )
   {
      throw buildException( "evalTimestamp", null, "get Timestamp",
            expectval, timestamp ) ;
   }
   db.removeProcedure( "getTimestamp" ) ;
}
   
function evalRegex( db )
{
   db.createProcedure( function getRegex( pattern, options ) {
                       return Regex( pattern, options ) ; } ) ;
   var pattern = "^W" ;
   var options = "i" ;
   var regex = db.eval( "getRegex( \"" + pattern + "\", \"" + options + "\" )" ) ;
   println( "regex instanceof Regex: " + ( regex instanceof Regex ) ) ;
   var expectval = Regex( pattern, options ) ;
   if( regex.toString() !== expectval.toString() )
   {
      throw buildException( "evalRegex", null, "get Regex",
            expectval, regex ) ;
   }
   db.removeProcedure( "getRegex" ) ;
}
   
function evalMinKey( db )
{
   db.createProcedure( function getMinKey() {
                       return MinKey() ; } ) ;
   var minKey = db.eval( "getMinKey()" ) ;
   println( "minKey instanceof MinKey: " + ( minKey instanceof MinKey ) ) ;
   db.removeProcedure( "getMinKey" ) ;
}

function evalMaxKey( db )
{
   db.createProcedure( function getMaxKey() {
                       return MaxKey() ; } ) ;
   var maxKey = db.eval( "getMaxKey()" ) ;
   println( "maxKey instanceof MaxKey: " + ( maxKey instanceof MaxKey ) ) ;
   db.removeProcedure( "getMaxKey" ) ;
}

function evalNumberLong( db )
{
   db.createProcedure( function getNumberLong( number ) {
                       return NumberLong( number ) ; } ) ;
   var number = 2147483648 ;
   var numberLong = db.eval( "getNumberLong( " + number + " )" ) ;
   println( "numberLong instanceof NumberLong: " + ( numberLong instanceof NumberLong ) ) ;
   var expectval = NumberLong( number ) ;
   if( numberLong.toString() !== expectval.toString() )
   {
      throw buildException( "evalNumberLong", null, "get NumberLong", expectval, numberLong ) ;
   }
   db.removeProcedure( "getNumberLong" ) ;
}
   
function evalSdbDate( db )
{
   db.createProcedure( function getSdbDate( date ) {
                       return SdbDate( date ) ; } ) ;
   var date = "2015-03-13" ;
   var sdbDate = db.eval( "getSdbDate( \"" + date + "\" )" ) ;
   println( "sdbDate instanceof SdbDate: " + ( sdbDate instanceof SdbDate ) ) ;
   var expectval = SdbDate( date ) ;
   if( sdbDate.toString() !== expectval.toString() )
   {
      throw buildException( "evalSdbDate", null, "get SdbDate", date, sdbDate ) ;
   }
   db.removeProcedure( "getSdbDate" ) ;
}
