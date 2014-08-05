// installCatalog
/*
if ( typeof(USERNAME) == "undefined" ) { USERNAME = "" ; }
if ( typeof(PASSWORD) == "undefined" ) { PASSWORD = "" ; }
if ( typeof(IP) == "undefined" ) { IP = "127.0.0.1" ; }
if ( typeof(TIMES) == "undefined" ) { TIMES = 3 ; }
*/
// todo: modify default value
// *********************************************************
if ( typeof(COORD_HOSTNAME) == "undefined" ) { HOSTNAME = "localhost" ; }
if ( typeof(COORD_SERVICE) == "undefined" ) { SERVICE = "11810" ; }
if ( typeof(DB_USERNAME) == "undefined" ) { SERVICE = "" ; }
if ( typeof(DB_PASSWORD) == "undefined" ) { SERVICE = "" ; }
if ( typeof(INSTALL_HOSTNAME) == "undefined" ) { HOSTNAME = "localhost" ; }
if ( typeof(INSTALL_SERVICE) == "undefined" ) { SERVICE = "11820" ; }
if ( typeof(INSTALL_PATH) == "undefined" ) { INSTALL_PATH = "/opt/sequoiadb/database/data/11820" ; }
if ( typeof(CONFIG) == "undefined" ) { CONFIG = "{}" ; }

var objRet = new Object() ;

objRet.Rc = 0 ;
objRet.Detail = "" ;
objRet.HasInstall = false ;
objRet.NodeName = INSTALL_HOSTNAME + ":" + INSTALL_SERVICE ;

function main()
{
   try
   {
      // connect to coord
      var db = new Sdb( COORD_HOSTNAME, COORD_SERVICE, DB_USERNAME, DB_PASSWORD ) ;
      // create cataRG
      var catarg = db.createCataRG( INSTALL_HOSTNAME, INSTALL_SERVICE,
                                    INSTALL_PATH, CONFIG ) ;
      objRet.HasInstall = true ;
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

