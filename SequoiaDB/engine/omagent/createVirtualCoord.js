// installVirtualCoord
if ( typeof(IP) == "undefined" ) { IP = "127.0.0.1" ; }
if ( typeof(USERNAME) == "undefined" ) { USERNAME = "sdbadmin" ; }
if ( typeof(PASSWORD) == "undefined" ) { PASSWORD = "adbadmin" ; }
//if ( typeof(TIMES) == "undefined" ) { TIMES = 3 ; }
// todo::add path in windows
if ( typeof(PROGRAM) == "undefined" ) { PROGRAM_PATH = "/opt/sequoiadb/bin/sequoiadb" ; }
if ( typeof(CONFIG_PATH) == "undefined" ) { CONFIG_PATH = "/tmp/virtualCoord" ; }
if ( typeof(CONFIG_FILE) == "undefined" ) { CONFIG_FILE = "/tmp/virtualCoord/sdb.conf" ; }
if ( typeof(DB_PATH) == "undefined" ) { CONFIG = "/tmp/virtualCoord" ; }
if ( typeof(LOCAL_SERVICE) == "undefined" ) { LOCAL_SERVICE = "11810" ; }
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
   var confPath = "confpath = " + CONFIG_PATH ;
   var dbPath = "dbpath = " + DB_PATH ;
   var svcName = "svcname = " + LOCAL_SERVICE ;
   // create a new conf file and write contents
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
   try
   {
      // ssh to local
      var ssh = new Ssh( IP, USERNAME, PASSWORD ) ;
      // prepare conf file
      createNewConfFile() ;
      // start virtual coord
      /*
      var cmd = "nohup " + PROGRAM + "--dbpath" + DB_PATH + "--confpath" + CONFIG_PATH +
                "--svcname" + LOCAL_SERVICE + "& 1 > /dev/null 2 > &l"
      Cmd.run( cmd ) ;
      */
// todo: think about windows
      var cmd = "nohup " + PROGRAM + "-c " + CONFIG_PATH +
                " & 1>/dev/null 2>&l";
      ssh.exec( cmd ) ;
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

