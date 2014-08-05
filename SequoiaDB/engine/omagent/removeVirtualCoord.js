/* *****************************************************************************
@description: remove virtual coord
@modify list:
   2014-7-26 Zhaobo Tan  Init
***************************************************************************** */
if ( typeof(IP) == "undefined" ) { IP = "127.0.0.1" ; }
if ( typeof(USERNAME) == "undefined" ) {}
if ( typeof(PASSWORD) == "undefined" ) {}
// todo::add path in windows
if ( typeof(PROGRAM) == "undefined" ) { PROGRAM_PATH = "/opt/sequoiadb/bin/sdbstop" ; }
//if ( typeof(CONFIG_PATH) == "undefined" ) { CONFIG_PATH = "/tmp/virtualCoord" ; }
//if ( typeof(CONFIG_FILE) == "undefined" ) { CONFIG_FILE = "/tmp/virtualCoord/sdb.conf" ; }
if ( typeof(DB_PATH) == "undefined" ) { CONFIG = "/tmp/virtualCoord" ; }
if ( typeof(COORD_SERVICE) == "undefined" ) { COORD_SERVICE = "11810" ; }
var objRet = new Object() ;

objRet.Rc = 0 ;
objRet.Detail = "" ;

function main()
{
   try
   {
      // check arguments
      if ( typeof(USERNAME) == "undefined" ||
          typeof(PASSWORD) == "undefined" )
      {
         objRet.Rc = -6 ;
         objRet.Detail = "Invalid arguments" ;
         return objRet ;
      }

      // ssh to local
      var ssh = new Ssh( IP, USERNAME, PASSWORD ) ;
      // stop virtual coord
      var cmd = PROGRAM + " -p " + COORD_SERVICE ;
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

