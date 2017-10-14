/*******************************************************************
*@Description : test use import importOnce in procedure
*               seqDB-12816:在存储过程中调用import importOnce
*@author      : Liang XueWang 
*******************************************************************/
// js file to import in procedure
var importFile     = WORKDIR + "/procedureImport_12816.js" ; 
// js file to importOnce in procedure    
var importOnceFile = WORKDIR + "/procedureImportOnce_12816.js" ; 

function createImportFile()
{
    try
    {
        var file = new File( importFile ) ;
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
        var file = new File( importOnceFile ) ;
        file.write( "1*2" ) ;
        file.close() ;
    }
    catch( e )
    {
        throw buildException( "createImportOnceFile", null, 
              "create importOnce file " + importOnceFile, 0, e ) ;
    }  
}

/*
// creat import importOnce file
createImportFile() ;
createImportOnceFile() ;

// create procedure to import file and test
try
{
    db.createProcedure( function testImport( file ) { return import( file ) } ) ;
    var result = db.eval( "testImport( \"" + importFile + "\" )" ) ;
    db.removeProcedure( "testImport" ) ;
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
    db.createProcedure( function testImportOnce( file ) { return importOnce( file ) } ) ;
    var result = db.eval( "testImportOnce( \"" + importOnceFile + "\" )" ) ;
    db.removeProcedure( "testImportOnce" ) ;
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
*/