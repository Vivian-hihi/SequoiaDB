/* *****************************************************************************
@discretion: sdb transaction common function 
@modify list:
   2014-4-1 YiBang Ruan  Init
***************************************************************************** */

function dbNew( db )
{
   try
   {
      db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;
   }
   catch( e )
   {
      println( " new  Sdb failed : " + e ) ;
      throw e ;
   }
}

function dbClose( db )
{
   try
   {
      db.close() ;
   }
   catch( e )
   {
      println( " close Sdb failed : " + e ) ;
      throw e ;
   }
}

function dbArrayNew( db )
{
   try
   {
      for( i = 0; i < CONNECTNUM; ++i )
      {
         db[i] = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;
      }
   }
   catch( e )
   {
      println( " new the " + i + "st Sdb failed : " + e ) ;
      throw e ;
   }
}

function dbArrayClose( db )
{
   try
   {
      for( i = 0; i < CONNECTNUM; ++i )
      {
         db[i].close() ;
      }
   }
   catch( e )
   {
      println( " close the" + i + "st Sdb failed : " + e ) ;
      throw e ;
   }
}

