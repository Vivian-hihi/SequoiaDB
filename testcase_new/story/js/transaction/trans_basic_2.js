/* *****************************************************************************
@discretion: transaction basic: rollback
@modify list:
   						2014-01-30 Pusheng Ding  Init
***************************************************************************** */

try
{
   var db = new Sdb( COORDHOSTNAME, COORDSVCNAME ) ;
}
catch( e )
{
   println( " new Sdb failed. " ) ;
   throw e ;
}

if( !commIsTransEnabled( db ) )
{
  println( "transaction is disabled" ) ;
}
else
{
   var cs = commCreateCS( db, COMMCSNAME, true ) ;
   var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, 0, true, false, true ) ;

   println( "insert and then commit" ) ;
   try
   {
      db.transBegin() ;
   }
   catch( e )
   {
      println( "failed to transBegin") ;
      throw e ;
   }

   try
   {
     cl.insert( { transaction: 1 } ) ;
   }
   catch( e )
   {
     println( " failed to insert data : { transaction: 1 }" ) ;
     throw e ;
   }
   try
   {
      db.transCommit() ;
   }
   catch( e )
   {
      println( "failed to transCommit") ;
      throw e ;
   }
   //verify data
   var rc ;
   try
   {
      rc = cl.find();
   }
   catch ( e )
   {
      println( "failed to query1, rc1= " + e ) ;
      throw e ;
   }
   if ( 1 != rc.size() )
   {
      println( "" ) ;
      throw "verify-data1-failed!" ;
   }

   println( "update and then rollback" ) ;
   try
   {
      db.transBegin() ;
   }
   catch( e )
   {
      println( "failed to transBegin") ;
      throw e ;
   }
   try
   {
     cl.update({$set:{transaction:-1}}) ;
   }
   catch( e )
   {
     println( " failed to update data!" ) ;
     throw e ;
   }
   try
   {
      db.transRollback() ;
   }
   catch( e )
   {
      println( "failed to transRollback") ;
      throw e ;
   }
   //verify data
   var rc ;
   try
   {
      rc = cl.find();
   }
   catch ( e )
   {
      println( "failed to query2, rc2= " + e ) ;
      throw e ;
   }
   if ( 1 != rc.size() )
   {
      println( "" ) ;
      throw "verify-data2-failed!" ;
   }
}
