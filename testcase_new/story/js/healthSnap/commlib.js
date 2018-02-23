/**
 * @discription: get a local node info by tool sdblist.
 * @return: return the info as json.
 */
function getLocalNodeInfo( cmd )
{
   var installPath = commGetInstallPath();
   var infoStr = cmd.run( installPath + "/bin/sdblist -l | grep sequoiadb | head -n 1" );   
   if( infoStr == "" )
   {
      throw "no any node of localhost";
   }
   var infoArr = infoStr.split( /\s+/ );
   var infoJson = {};
   infoJson.Name        = infoArr[0];
   infoJson.SvcName     = infoArr[1];
   infoJson.Role        = infoArr[2];
   infoJson.PID         = infoArr[3];
   infoJson.GID         = infoArr[4];
   infoJson.NID         = infoArr[5];
   infoJson.PRY         = infoArr[6];
   infoJson.GroupName   = infoArr[7];
   infoJson.StartTime   = infoArr[8];
   infoJson.DBPath      = infoArr[9];
   return infoJson;
}

/**
 * @discription: judge equal, no matter embedded json or array or any other.
 */
function isEquals( a, b )
{
   if( a instanceof Object && b instanceof Object )
   {
      var aProps = Object.getOwnPropertyNames( a );
      var bProps = Object.getOwnPropertyNames( b );

      if( aProps.length != bProps.length)
      {
         return false;
      }
      
      for( var i = 0; i < aProps.length; ++i )
      {
         var propName = aProps[i];
         if( !isEquals( a[propName], b[propName] ) )
         {
            return false;
         }
      }

      return true;
   }
   else
   {
      return (a === b);
   }
}
