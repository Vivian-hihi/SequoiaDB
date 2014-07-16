// installAgentProcess
if ( typeof(USERNAME) == "undefined" ) { USERNAME = "" ; }
if ( typeof(PASSWORD) == "undefined" ) { PASSWORD = "" ; }
if ( typeof(IP) == "undefined" ) { IP = "127.0.0.1" ; }
if ( typeof(TIMES) == "undefined" ) { TIMES = 3 ; }
// todo: modify default value
// *********************************************************
if ( typeof(LOCAL_PACKET_PATH) == "undefined" ) { LOCAL_PACKET_PATH = "/home/users/tanzhaobo/sequoiadb/bin/sdb" ; }
if ( typeof(REMOTE_PACKET_PATH) == "undefined" ) { REMOTE_PACKET_PATH = "/tmp/sdb" ; }


var objRet = new Object() ;

objRet.Rc = 0 ;
objRet.Detail = "" ;
objRet.HasPush = false ;
objRet.HasRunning = false ;
objRet.HostName = "" ;

function main()
{
   try
   {
      // ssh
      var ssh = new Ssh( IP, USERNAME, PASSWORD ) ;
      objRet.HostName = ssh.exec("hostname") ;
      // push packet
      ssh.push( LOCAL_PACKET_PATH, REMOTE_PACKET_PATH ) ;
      objRet.HasPush = true ;
      // start the process
      ssh.exec( REMOTE_PACKET_PATH ) ;
      objRet.HasRunning = true ;

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

