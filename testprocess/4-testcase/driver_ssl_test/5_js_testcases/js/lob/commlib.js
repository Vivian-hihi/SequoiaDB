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

function lobFileIsExist( fileName )
{
   var isExist = false ;
   try
   {
      var cmd = new Cmd() ;
      cmd.run( "ls " + fileName ) ;
      isExist = true ;
   }
   catch( e )
   {
      if( 2 == e ){ isExist = false; }
   }
   return isExist ;
}

function lobAutoFile( fileName, fileLine)
{
   if( undefined == fileLine ){ fileLine = 1000 ; }
   try
   {
      var cnt = 0 ;
      while( true == lobFileIsExist( fileName ) && cnt < 10 )
      {
         File.remove( fileName ) ;
         cnt++ ;
      }
      if( 10 <= cnt )
         throw "failed to remove file: " + fileName ;
      var file = new File( fileName ) ;
      for( var i = 0 ; i < fileLine ; ++i )
      {
         var record = '{ no:'+i+', score:'+i+', interest:["movie", "photo"],' +
                      '  major:"计算机软件与理论", dep:"计算机学院",' +
                      '  info:{name:"Holiday", age:22, sex:"男"} }' ;
         file.write( record ) ;
      }
      if( false == lobFileIsExist( fileName ) )
         throw "NoFile: " + fileName ;
   }
   catch( e )
   {
      println( "faile to auto generate file, rc = " + e ) ;
      throw e ;
   }
}

function lobCreateCS( db, DOMCSNAME, domName )
{
   try
   {
      db.createCS( DOMCSNAME, { "PageSize":4096, "Domain": domName }) ;
   }
   catch( e )
   {
      println( "failed to create collection space attach domain, rc = " + e ) ;
      throw e ;
   }
}

function lobPutLob( cl, lobFile, lobNum )
{
   if( undefined == lobNum ){ lobNum = 10 ; }
   var oid = new Array() ;
   try
   {
      for( var i = 0 ; i < lobNum; ++i )
      {
         oid[i] = cl.putLob( lobFile ) ;
      }
      // verify
      var cursor = cl.listLobs().toArray() ;
      if( lobNum != cursor.length )
      {
         println( "collection have lob: " + cursor.length ) ;
         throw "ErrNumberPutLob" ;
      }
      return oid ;
   }
   catch( e )
   {
      println( "failed to put lob in collection, rc = " +e ) ;
      throw e ;
   }
}

// Insert data to SequoiaDB
function lobInsert( cl, recordNum )
{
   try
   {
      // insert 10000 records in CL
      for( var i = 0 ; i < recordNum ; ++i )
      {
         cl.insert( { no:i, score:i, interest:["movie", "photo"],
                      major:"计算机软件与理论", dep:"计算机学院",
                      info:{name:"Holiday", age:22, sex:"男"} } ) ;
      }
      var cnt = 0 ;
      do
      {
         ++cnt ;
         sleep(1) ;
      }while( recordNum != cl.count() && 1000 < cnt ) ;
      if( recordNum != cl.count() )
      {
         println( "wrong quantity of records, count = " + count ) ;
         throw "ErrNumRecords" ;
      }
   }
   catch ( e )
   {
      println( "failed to insert data to collection, rc = " + e  ) ;
      throw e ;
   }
}

function lobSplit( cl, srcGroup, dstGroup, firstCond, secondCond )
{
   try
   {
      if( undefined == srcGroup ) { throw "NoSourceGroup" ; }
      if( undefined == dstGroup ) { throw "NoDestnationGroup" ; }
      if( undefined == secondCond ) { secondCond = "percent" ; }
      if( "percent" != secondCond )
      {
         println( "first condition: " + JSON.stringify( firstCond ) ) ;
         println( "second condition: " + JSON.stringify( secondCond ) ) ;
         cl.split( srcGroup, dstGroup, firstCond, secondCond ) ;
      }
      else
      {
         println( "percent condition: " + JSON.parse( firstCond ) ) ;
         cl.split( srcGroup, dstGroup, firstCond ) ;
      }
   }
   catch( e )
   {
      println( "failed to split collection, rc = " + e ) ;
      throw e ;
   }
}

function lobGetGroups( db )
{
   try
   {
      var RG = commGetGroups( db ) ;
      var groups = new Array() ;
      for( var i = 0 ; i < RG.length ; ++i )
      {
         groups[i] = RG[i][0].GroupName ;
      }
      return groups ;
   }
   catch( e )
   {
      println( "failed to get groups, rc = " + e ) ;
      throw e ;
   }
}
