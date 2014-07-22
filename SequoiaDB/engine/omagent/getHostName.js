// getHostName
if ( typeof(USERNAME) == "undefined" ) { USERNAME = "" ; }
if ( typeof(PASSWORD) == "undefined" ) { PASSWORD = "" ; }
if ( typeof(IP) == "undefined" ) { IP = "127.0.0.1" ; }
if ( typeof(TIMES) == "undefined" ) { TIMES = 3 ; }

var objRet = new Object() ;

objRet.HostName   = null ;
objRet.Rc         = 0 ;
objRet.Detail     = null ;

function main()
{
   // ssh and get host name
   try
   {
      var ssh = new Ssh( IP, USERNAME, PASSWORD ) ;
      var name = ssh.exec("hostname") ;
      var i = name.indexOf( "\n" ) ;
      var substr = name.substring(0, i);
      objRet.HostName = substr ;
   }
   catch ( e )
   {
      objRet.Rc = e ;
      objRet.Detail = eval( '(' + getLastErrMsg() + ')' ) ;
   }
   return objRet ;
}

// execute
main() ;

