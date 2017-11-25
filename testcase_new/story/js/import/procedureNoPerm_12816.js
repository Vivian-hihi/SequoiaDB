/*******************************************************************
*@Description : test use import importOnce in procedure
*               seqDB-12816:在存储过程中调用import importOnce
*@author      : Liang XueWang 
*******************************************************************/
// js file to import/importOnce in procedure, no permission
var noPermFile     = WORKDIR + "/noPermFile_12816.js" ;  

function createNoPermFile()
{
   try
   {
      var cmd = new Cmd() ;
      cmd.run( "rm -rf " + noPermFile ) ;
      var file = new File( noPermFile ) ;
      file.write( "1+2" ) ;
      file.close() ;
      File.chmod( noPermFile, 0000 ) ;
   }
   catch( e )
   {
      throw buildException( "createNoPermFile", null, 
            "create file " + noPermFile, 0, e ) ;
   }
}

function main()
{
   if( commIsStandalone( db ) )
   {
      println( "Run mode is Standalone, can't use procedure" ) ;
      return ;
   }
   if( getCoordUser() === "root" )
   {
      println( COORDHOSTNAME + ":" + COORDSVCNAME + " user is root" ) ;
      return ;
   }

   // creat import importOnce file
   createNoPermFile() ;

   // create procedure to import file and test
   try
   {
      db.createProcedure( function testImport( file ) { return import( file ) } ) ;
      db.eval( "testImport( \"" + noPermFile + "\" )" ) ;
      throw 0 ;
   }
   catch( e )
   {
      if( e !== -3 )
      { 
         throw buildException( "main", e, 
               "test import file " + noPermFile + " in procedure", -3, e ) ;    
      }
   }
   db.removeProcedure( "testImport" ) ;

   // create procedure to importOnce file and test
   try
   {
      db.createProcedure( function testImportOnce( file ) { return importOnce( file ) } ) ;
      db.eval( "testImportOnce( \"" + noPermFile + "\" )" ) ;
      throw 0 ;
   }
   catch( e )
   {
      if( e !== -3 )
      {
         throw buildException( "main", e, 
               "test importOnce file " + noPermFile + " in procedure", -3, e ) ;
      }    
   }
   db.removeProcedure( "testImportOnce" ) ; 
   
   // remove file
   File.chmod( noPermFile, 0755 ) ;  
   removeFile( noPermFile ) ;
}

main()