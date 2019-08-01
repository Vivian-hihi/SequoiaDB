/*******************************************************************
*@Description : test use import importOnce in procedure
*               seqDB-12816:在存储过程中调用import importOnce
*@author      : Liang XueWang 
*******************************************************************/
// js file to import in procedure
var importFile     = WORKDIR + "/procedureImport_12816.js" ; 
// js file to importOnce in procedure    
var importOnceFile = WORKDIR + "/procedureImportOnce_12816.js" ; 

main();
function main()
{
    if( commIsStandalone( db ) )
    {
        println( "Run mode is Standalone, cannot use procedure" ) ;
        return ;
    }

    // creat import importOnce file
    createImportFile() ;
    createImportOnceFile() ;

    // create procedure to import file and test
    try
    {
        checkProcedure( "testImport12816" );
        db.createProcedure( function testImport12816( file ) { return import( file ) } ) ;
        var result = db.eval( "testImport12816( \"" + importFile + "\" )" ) ;
        db.removeProcedure( "testImport12816" ) ;
    }
    catch( e )
    {
        throw buildException( null, null, "test import in procedure", 0, e ) ;    
    }
    if( result !== 3 )
    {
        throw buildException( null, null, "test procedure result", 3, result ) ;
    }

    // create procedure to importOnce file and test
    try
    {
        checkProcedure( "testImportOnce12816" );
        db.createProcedure( function testImportOnce12816( file ) { return importOnce( file ) } ) ;
        var result = db.eval( "testImportOnce12816( \"" + importOnceFile + "\" )" ) ;
        db.removeProcedure( "testImportOnce12816" ) ;
    }
    catch( e )
    {
        throw buildException( null, null, "test importOnce in procedure", 0, e ) ;    
    }
    if( result !== 2 )
    {
        throw buildException( null, null, "test procedure result", 2, result ) ;
    }

    // remove file   
    removeFile( importFile ) ;
    removeFile( importOnceFile ) ;
}

function createImportFile()
{
    try
    {
        var cmd = new Cmd() ;
        cmd.run( "rm -rf " + importFile ) ;
        var file = new File( importFile, 0744 ) ;
        file.write( "1+2" ) ;
        file.close() ;
    }
    catch( e )
    {
        throw buildException( "createImportFile", null, 
              "create import file " + importFile, 0, e ) ;
    }
}

function createImportOnceFile()
{
    try
    {
        var cmd = new Cmd() ;
        cmd.run( "rm -rf " + importOnceFile ) ;
        var file = new File( importOnceFile, 0744 ) ;
        file.write( "1*2" ) ;
        file.close() ;
    }
    catch( e )
    {
        throw buildException( "createImportOnceFile", null, 
              "create importOnce file " + importOnceFile, 0, e ) ;
    }  
}

function checkProcedure( procedureName )
{
    var cursor = db.listProcedures({name:procedureName});
    if(cursor.next())
    {
        db.removeProcedure( procedureName ) ;
    }
}