/* *****************************************************************************
@description: create virtual coord
@modify list:
   2014-7-26 Zhaobo Tan  Init
***************************************************************************** */
// todo::add path in windows
if ( typeof(PROGRAM) == "undefined" ) { PROGRAM_PATH = "/opt/sequoiadb/bin/sequoiadb" ; }
if ( typeof(COORD_SERVICE) == "undefined" ) { COORD_SERVICE = "11810" ; }
if ( typeof(CONFIG_PATH) == "undefined" ) { CONFIG_PATH = "/tmp/virtualCoord" ; }
if ( typeof(CONFIG_FILE) == "undefined" ) { CONFIG_FILE = "/tmp/virtualCoord/sdb.conf" ; }
if ( typeof(DB_PATH) == "undefined" ) { DB_PATH = "/tmp/virtualCoord/coord" ; }

var objRet = new Object() ;

objRet.Rc = 0 ;
objRet.Detail = "" ;

function createNewConfFile()
{
   // remove the old config folder
   try
   {
      File.remove( CONFIG_PATH ) ;
   }
   catch ( e )
   {
      if ( -4 != e )
         throw e ;
   }
   // create a new folder
// todo:think about windows
   var cmd = " mkdir " + CONFIG_PATH ;
   Cmd.run( cmd ) ;
   // contents
   // todo: windows
   var confPath = "confpath = " + CONFIG_PATH + "\n" ;
   var dbPath = "dbpath = " + DB_PATH + "\n" ;
   var svcName = "svcname = " + COORD_SERVICE + "\n" ;
   try
   {
      var file = new File( CONFIG_FILE ) ;
      file.write( confPath ) ;
      file.write( dbPath ) ;
      file.write( svcName ) ;
      file.close() ;
   }
   catch ( e )
   {
      throw e
   }
}

function main()
{
println( "PROGRAM is " + PROGRAM ) ;
println( "COORD_SERVICE is " + COORD_SERVICE ) ;
println( "CONFIG_PATH is " + CONFIG_PATH ) ;
println( "CONFIG_FILE is " + CONFIG_FILE ) ;
println( "DB_PATH is " + DB_PATH ) ;

   try
   {
      // prepare conf file
      createNewConfFile() ;
      // start virtual coord
      var cmd = PROGRAM + " -c " + CONFIG_PATH
      Cmd.run( cmd ) ;
      return objRet ;
   }
   catch ( e )
   {
      objRet.Rc = e ;
      objRet.Detail = getLastErrMsg() ;
      return objRet ;
   }
}

// execute
   main() ;

