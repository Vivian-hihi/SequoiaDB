// uninstallAgentProcess
if ( typeof(USERNAME) == "undefined" ) { USERNAME = "" ; }
if ( typeof(PASSWORD) == "undefined" ) { PASSWORD = "" ; }
if ( typeof(IP) == "undefined" ) { IP = "127.0.0.1" ; }
if ( typeof(TIMES) == "undefined" ) { TIMES = 3 ; }
// todo: modify default value
// *********************************************************
if ( typeof(REMOTE_PACKET_PATH) == "undefined" ) { REMOTE_PACKET_PATH = "/tmp/sdbomagent" ; }


var objRet = new Object() ;

objRet.Rc = 0 ;
objRet.Detail = "" ;
objRet.HasRemove = false ;
objRet.HostName = "" ; // no need, just for test, remember to remove it

function main()
{
   try
   {
      // ssh
      var ssh = new Ssh( IP, USERNAME, PASSWORD ) ;
      objRet.HostName = ssh.exec("hostname") ;
      // remove the agent packet
      var sysType = System.type() ;
      var str = "" ;
      if ( sysType == "LINUX" )
      {
         str = "rm -rf " ;
         ssh.exec( str + REMOTE_PACKET_PATH ) ;
      }
      else
      {
         str = "DEL /Q "
         ssh.exec( str + REMOTE_PACKET_PATH ) ;
      }
      objRet.HasRemove = true ;

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

