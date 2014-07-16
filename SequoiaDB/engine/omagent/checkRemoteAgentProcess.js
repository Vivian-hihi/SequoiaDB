// check remote agent process
if ( typeof(USERNAME) == "undefined" ) { USERNAME = "" ; }
if ( typeof(PASSWORD) == "undefined" ) { PASSWORD = "" ; }
if ( typeof(IP) == "undefined" ) { IP = "127.0.0.1" ; }
if ( typeof(TIMES) == "undefined" ) { TIMES = 3 ; }
/*
if ( typeof(VERSION) == "undefined" ) { VERSION = "" ; }
if ( typeof(LOCAL_CONF_FILE_PATH) == "undefined" ) { LOCAL_CONF_FILE_PATH = "" ; }
if ( typeof(LOCAL_PACKET_PATH) == "undefined" ) { LOCAL_PACKET_PATH = "" ; }
if ( typeof(REMOTE_PACKET_PATH) == "undefined" ) { REMOTE_PACKET_PATH = "" ; }
*/

var objRet = new Object() ;
objRet.IsRunning     = false ;
objRet.Version       = null ;

objRet.Rc            = 0 ;
objRet.Detail        = "" ;

function main()
{
   try
   {
      // ssh
      var ssh = new Ssh( IP, USERNAME, PASSWORD ) ;

      // test whether omagent is running in remote mechine
      var cmd1 = "ps -e | grep sequoiadb" ;
      var ret1 = ssh.exec( cmd1 ) ;
println( "ret1 is: " + ret1 );
      if ( "" != ret1 )
      {
         objRet.IsRunning = true ;
         // get the process id of remote omagent
         var cmd2 = "ps -ef | grep sequoiadb | grep -v grep | awk '{print \"/proc/\"$2\"/exe\"}' | xargs ls -l | head -1 | awk 'BEGIN{FS=\"-> \"} {print $2}'" ;
         var ret2 = ssh.exec( cmd2 ) ;
println( "ret2 is: " + ret2 ) ;
         // get the version of remote omagent
         var cmd3 = ret2 + "--version | grep \"SequoiaDB version\" | awk 'BEGIN{FS=\": \"} {print $2}'" ;
         var ret3 = ssh.exec( cmd3 ) ;
println( "ret3 is: " + ret3 ) ;
         if ( "" != ret3 )
         {
            objRet.Version = ret3 ;
         }
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

try
{
   main() ;
}
catch( e )
{
   println( "Failed in main: " + e ) ;
//   return objRet ;
}

