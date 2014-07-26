/* *****************************************************************************
@description: create data node
@modify list:
   2014-7-26 Zhaobo Tan  Init
***************************************************************************** */
// todo: windows
if ( typeof(COORD_HOSTNAME) == "undefined" ) { COORD_HOSTNAME = "localhost" ; }
if ( typeof(COORD_SERVICE) == "undefined" ) { COORD_SERVICE = "11810" ; }
if ( typeof(DB_USERNAME) == "undefined" ) { DB_USERNAME = "" ; }
if ( typeof(DB_PASSWORD) == "undefined" ) { DB_PASSWORD = "" ; }
if ( typeof(INSTALL_HOSTNAME) == "undefined" ) { INSTALL_HOSTNAME = "localhost" ; }
if ( typeof(INSTALL_SERVICE) == "undefined" ) { INSTALL_SERVICE = "11820" ; }
if ( typeof(INSTALL_PATH) == "undefined" ) { INSTALL_PATH = "/opt/sequoiadb/database/data/11820" ; }
if ( typeof(CONFIG) == "undefined" ) { CONFIG = "{}" ; }
if ( typeof(GROUPNAME) == "undefined" ) {}

var objRet = new Object() ;

objRet.Rc = 0 ;
objRet.Detail = "" ;

function main()
{
   try
   {
      // check argument
      if ( typeof(GROUPNAME == "undefined") )
      {
         objRet.Rc = -6 ;
         objRet.Detail = "Invalid arguments" ;
      }
      // connect to coord
      var db = new Sdb( COORD_HOSTNAME, COORD_SERVICE, DB_USERNAME, DB_PASSWORD ) ;
      // get rg
      var rg = null ;
      try
      {
         var rg = db.getRG( GROUPNAME ) ;
      }
      catch ( e )
      {
         if ( -154 == e )
         {
            objRet.Rc = -154 ;
            objDetail = "group does not exist" ;
            return objRet ;
         }
         else
         {
            throw e ;
         }
      }
      // create data node
      var node  = rg.createNode( INSTALL_HOSTNAME, INSTALL_SERVICE,
                                 INSTALL_PATH, CONFIG ) ;
      // start node
      node.start() ;
// todo:maybe need to check whether the node has been start succeed or not
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

