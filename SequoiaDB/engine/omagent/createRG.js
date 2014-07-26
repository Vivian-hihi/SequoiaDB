/* *****************************************************************************
@description: create replica group
@modify list:
   2014-7-26 Zhaobo Tan  Init
***************************************************************************** */
if ( typeof(COORD_HOSTNAME) == "undefined" ) {}
if ( typeof(COORD_SERVICE) == "undefined" ) {}
if ( typeof(DB_USERNAME) == "undefined" ) {}
if ( typeof(DB_PASSWORD) == "undefined" ) {}
if ( typeof(GROUPNAME) == "undefined" ) {}

var objRet = new Object() ;

objRet.Rc = 0 ;
objRet.Detail = "" ;

function main()
{
   try
   {
      // check arguments
      if ( typeof(COORD_HOSTNAME) == "undefined" ||
           typeof(COORD_SERVICE) == "undefined" ||
           typeof(DB_USERNAME) == "undefined" ||
           typeof(DB_PASSWORD) == "undefined" ||
           typeof(GROUPNAME) == "undefined" )
      {
         objRet.Rc = -6 ;
         objRet.Detail = "Invalid argument" ;
         return objRet ;
      }
      // connect to coord
      var db = new Sdb( COORD_HOSTNAME, COORD_SERVICE, DB_USERNAME, DB_PASSWORD ) ;
      // create replica group
      var rg = db.createRG( GROUPNAME ) ;
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

