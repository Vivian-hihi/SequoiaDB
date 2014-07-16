// scanHost
/*
if ( typeof(USERNAME) == "undefined" ) { USERNAME = "tanzhaobo" ; }
if ( typeof(PASSWORD) == "undefined" ) { PASSWORD = "tanzb_202" ; }
if ( typeof(IP) == "undefined" ) { IP = "192.168.20.40" ; }
*/
if ( typeof(USERNAME) == "undefined" ) { USERNAME = "" ; }
if ( typeof(PASSWORD) == "undefined" ) { PASSWORD = "" ; }
if ( typeof(IP) == "undefined" ) { IP = "127.0.0.1" ; }
if ( typeof(TIMES) == "undefined" ) { TIMES = 3 ; }

var objRet = new Object() ;
objRet.Ping = false ;
objRet.Ssh = false ;
objRet.HostName = null ;
objRet.Rc = 0 ;
objRet.Detail = "" ;

function main()
{
   try
   {
      // ping
      var ping = System.ping( IP, TIMES ) ;
      if ( null != typeof(ping) && "undefined" != typeof(ping) )
         objRet.Ping = true ;

      // ssh
      var ssh = new Ssh( IP, USERNAME, PASSWORD ) ;
      if ( null != typeof(ssh) && "undefined" != typeof(ssh) )
         objRet.Ssh = true ;

      // hostName
      var name = ssh.exec("hostname") ; // if no host name, what can I do ?

      if ( null != typeof(name) && "undefined" != typeof(name) )
      {
         var i = name.indexOf( "\n" ) ;
         var substr = name.substring(0, i);
         objRet.HostName = substr ;
      }
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

