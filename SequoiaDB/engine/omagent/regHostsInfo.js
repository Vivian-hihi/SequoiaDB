// regHostsInfo
if ( typeof(USERNAME) == "undefined" ) { USERNAME = "root" ; }
if ( typeof(PASSWORD) == "undefined" ) { PASSWORD = "sequoiadb" ; }
if ( typeof(IP) == "undefined" ) { IP = "192.168.20.112" ; }
if ( typeof(TIMES) == "undefined" ) { TIMES = 3 ; }
if ( typeof(HOSTSINFO) == "undefined" ) { HOSTSINFO = "hahaha" ; }

var objRet = new Object() ;

objRet.HostName    = null ;
objRet.hasLeftCopy = false ;
objRet.Rc          = 0 ;
objRet.Detail      = null ;

function main()
{
   var ssh = null ;
   var file = null ;

   try
   {
      // ssh and get host name
      ssh = new Ssh( IP, USERNAME, PASSWORD ) ;
      var name = ssh.exec("hostname") ;
      var i = name.indexOf( "\n" ) ;
      var substr = name.substring(0, i);
      objRet.HostName = substr ;

      // get OS info
      var osInfo = System.type() ;
      // backup hosts file
      var remoteFile = "" ;
      if ( "LINUX" == osInfo )
      {
         remoteFile = "/etc/hosts" ;
         // save a copy for rollback
         ssh.exec( "cp /etc/hosts /etc/hostsbak" ) ;
         objRet.hasLeftCopy = true ;
      }
      else if ( "WINDOWS" == osInfo )
      {

      }
      else
      {

      }
      // cp remote hosts file to local
      var localFile = "" ;
      if ( "LINUX" == osInfo )
      {
         localFile = "/tmp/hosts" ;
         ssh.pull( remoteFile, localFile ) ;
      }
      else if ( "WINDOWS" == osInfo )
      {

      }
      else
      {

      }
      // append content to the file
      file = new File( localFile ) ;
      file.seek( 0, 'e' ) ;
      file.write( HOSTSINFO ) ;
      file.close() ;
      // copy file back to remote and delete the local copy
      ssh.push( localFile, remoteFile ) ;
//      File.remove( localFile ) ;
   }
   catch ( e )
   {
      objRet.Rc = e ;
      objRet.Detail = getLastErrMsg() ;
//      objRet.Detail = eval( '(' + getLastErrMsg() + ')' ) ;
      return objRet ;
   }
   return objRet ;
}

// execute
main() ;

