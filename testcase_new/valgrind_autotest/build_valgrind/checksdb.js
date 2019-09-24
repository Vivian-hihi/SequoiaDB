/****************************************************************
@decription:   Deploy SequoiaDB

@input:        hosts:      String
               eg: bin/sdb -f checksdb.js -e 'var host="192.168.31.1"'

@author:       Yin Zhen 2019-09-24
****************************************************************/

// check parameter
if ( typeof( host ) === "undefined" )
{
   var host = "localhost" ;
}
else if( host.constructor !== String )
{
   throw "Invalid param[host], should be String" ;
}

// set global variable
var HOST              = host ;
var COORD_SVC         = 11810 ;

// run!
main() ;

function main()
{
   if ( HOST !== "" )
   {
      checkSequoiadb() ;
   }
   else
   {
      println("Do not check sdb, because of host: " + HOST) ;
   }
}

function checkSequoiadb()
{
   while( true )
   {
      sleep( 1000 ) ;
      try
      {
         var db = new Sdb(HOST + ":" + COORD_SVC) ;
      }
      catch( e )
      {
         if ( e != -79 && e != -15 && e != -104 )
         {
            throw "ERROR from new Sdb " + e ;
         }
         else
         {
            continue ;
         }
      }
      break ;
   }
   println( HOST + " Check Sdb ok" ) ;
}
