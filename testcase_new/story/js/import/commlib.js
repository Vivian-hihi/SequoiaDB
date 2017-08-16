/***********************************************************
*@Description : common function for import importOnce
*@auhor       : Liang XueWang
***********************************************************/
// create WORKDIR in local host
commMakeDir( "localhost", WORKDIR ) ;

// js file without return
var withoutRetFile       = WORKDIR + "/withoutRet_11902.js" ; 
// js file with return
var withRetFile          = WORKDIR + "/withRet_11903.js" ;      

function createWithoutRetFile()
{
    try
    {
        var file = new File( withoutRetFile ) ;
        file.write( "function add( a, b ) { return a + b ; }" ) ;
        file.close() ;
    }
    catch( e )
    {
        throw buildException( "createFile", null, "create file " + withoutRetFile,
                              0, e ) ;
    }
}

function createWithRetFile()
{
    try
    {
        file = new File( withRetFile ) ;
        file.write( "function mul( a, b ) { return a * b ; } var tmp = 100 ; mul( 1, 2 ) ;" 
                    + " mul( 2, 3 ) ;" ) ;
        file.close() ;
    }
    catch( e )
    {
        throw buildException( "createFile", null, "create file " + withRetFile,
                              0, e ) ;
    }
}

function removeFile( filename )
{
    try
    {
        File.remove( filename ) ;
    }
    catch( e )
    {
        throw buildException( "removeFile", null, "remove file " + filename, 0, e ) ;
    }
}

function currUser()
{
   var cmd = new Cmd() ;
   var tmp = cmd.run( "whoami" ).split( "\n" ) ;
   var user = tmp[tmp.length-2] ;
   return user ;
}