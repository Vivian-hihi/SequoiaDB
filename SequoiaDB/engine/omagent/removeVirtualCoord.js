// installVirtualCoord
if ( typeof(IP) == "undefined" ) { IP = "127.0.0.1" ; }
if ( typeof(USERNAME) == "undefined" ) { USERNAME = "sdbadmin" ; }
if ( typeof(PASSWORD) == "undefined" ) { PASSWORD = "adbadmin" ; }
//if ( typeof(TIMES) == "undefined" ) { TIMES = 3 ; }
// todo::add path in windows
if ( typeof(PROGRAM) == "undefined" ) { PROGRAM_PATH = "/opt/sequoiadb/bin/sdbstop" ; }
//if ( typeof(CONFIG_PATH) == "undefined" ) { CONFIG_PATH = "/tmp/virtualCoord" ; }
//if ( typeof(CONFIG_FILE) == "undefined" ) { CONFIG_FILE = "/tmp/virtualCoord/sdb.conf" ; }
if ( typeof(DB_PATH) == "undefined" ) { CONFIG = "/tmp/virtualCoord" ; }
if ( typeof(LOCAL_SERVICE) == "undefined" ) { LOCAL_SERVICE = "11810" ; }
var objRet = new Object() ;

objRet.Rc = 0 ;
objRet.Detail = "" ;

function main()
{
   try
   {
      // ssh to local
      var ssh = new Ssh( IP, USERNAME, PASSWORD ) ;
      // stop virtual coord
      var cmd = PROGRAM + " -p " + LOCAL_SERVICE ;
      ssh.exec( cmd ) ;
      // remove rubbish files
      File.remove( CONFIG_PATH ) ;
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

