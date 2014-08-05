/* *****************************************************************************
@description: create catalog
@modify list:
   2014-7-26 Zhaobo Tan  Init
***************************************************************************** */
if ( typeof(COORD_HOSTNAME) == "undefined" ) { COORD_HOSTNAME = "localhost" ; }
if ( typeof(COORD_SERVICE) == "undefined" ) { COORD_SERVICE = "11810" ; }
if ( typeof(DB_USERNAME) == "undefined" ) { DB_USERNAME = "" ; }
if ( typeof(DB_PASSWORD) == "undefined" ) { DB_PASSWORD = "" ; }
/*
if ( typeof(INSTALL_HOSTNAME) == "undefined" ) { INSTALL_HOSTNAME = "localhost" ; }
if ( typeof(INSTALL_SERVICE) == "undefined" ) { INSTALL_SERVICE = "11800" ; }
if ( typeof(INSTALL_PATH) == "undefined" ) { INSTALL_PATH = "/opt/sequoiadb/database/cata/11800" ; }
if ( typeof(CONFIG) == "undefined" ) { CONFIG = "{}" ; }
*/
var objRet = new Object() ;

objRet.Rc = 0 ;
objRet.Detail = "" ;
objRet.HasInstall = false ;
objRet.NodeName = INSTALL_HOSTNAME + ":" + INSTALL_SERVICE ;

function main()
{
   try
   {
      // check arguments
      if ( typeof(INSTALL_HOSTNAME) == "undefined" ||
           typeof(INSTALL_SERVICE) == "undefined" ||
           typeof(INSTALL_PATH) == "undefined" ||
           typeof(CONFIG) == "undefined" )
      {
         objRet.Rc = -6 ;
         objRet.Detail = "Invalid arguments for js to create catalog" ;
         return objRet ;
      }
print("INSTALL_HOSTNAME is: " + INSTALL_HOSTNAME + '\n') ;
print("INSTALL_SERVICE is: " + INSTALL_SERVICE + '\n') ;
print("INSTALL_PATH is: " + INSTALL_PATH + '\n') ;
print("CONFIG is: " + CONFIG + '\n') ;
      // connect to coord
      var db = new Sdb( COORD_HOSTNAME, COORD_SERVICE, DB_USERNAME, DB_PASSWORD ) ;
print("00000000000000\n") ;
      // create cataRG
      var catarg = db.createCataRG( INSTALL_HOSTNAME, INSTALL_SERVICE,
                                    INSTALL_PATH, CONFIG ) ;
      objRet.HasInstall = true ;
print("aaaaaaaaaaaaaaaaaaaaaaaaaa\n") ;
      return objRet ;
   }
   catch ( e )
   {
      objRet.Rc = e ;
      objRet.Detail = getLastErrMsg() ;
print("ddddddddddddddddd " + objRet.Rc + " " + objRet.Detail) ;
      return objRet ;
   }
}

// execute
   main() ;

