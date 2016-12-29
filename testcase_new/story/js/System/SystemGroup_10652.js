/******************************************************************************
*@Description : test js object System function: addGroup delGroup listGroups
*                                               isGroupExist
*               TestLink : 10652 System对象添加、删除用户组
*                          10653 System对象添加用户组，用户组id已存在,isUnique为true
*                          10654 System对象添加用户组，用户组id已存在,isUnique为false
*                          10658 System对象枚举用户组
*                          10661 System对象判断用户组是否存在
*@author      : Liang XueWang
******************************************************************************/

// 测试创建删除用户组
SystemTest.prototype.testAddDelGroup = function( isUnique )
{
   // 检查sdbadmin_group是否存在
   if( !isSdbadminGroupExist( this.hostname, this.svcname ) )
      return ;
   
   this.init() ;
   
   // 检查当前用户和cm用户是否有权限
   var currUser = this.system.getCurrentUser().toObj().user ;
   var cmUser = toolGetSdbcmUser( this.hostname, this.svcname ) ;
   if( this.system == System )
   {
      if( currUser != "root" )
         return ;
   }
   else if( cmUser != "root" )
      return ;
   
   // 获取sdbadmin_group的gid
   var info = this.cmd.run( "cat /etc/group | grep sdbadmin_group" ).split( "\n" )[0] ;
   var tmp = info.split( ":" ) ;
   var gid = tmp[2] ;
   
   var groupObj = {} ;
   groupObj["name"] = "testGroup" ;
   groupObj["id"] = gid ;
   groupObj["isUnique"] = isUnique ;
   groupObj["passwd"] = "testGroup" ;
  
   try
   {
      // 创建用户组
      this.system.addGroup( groupObj ) ;
      if( isUnique )
      {
         throw "create unique group with sdbadmin_group gid should be failed" ;
      }
      // 检查用户组
      checkGroup( this.cmd, groupObj ) ;
      // 删除用户组，需要更改gid和sdbadmin用户的主用户组
      var gid = getIdleGID( this.cmd ) ;
      this.cmd.run( "groupmod -g " + gid + " " + groupObj.name ) ;   // 更改gid
      this.cmd.run( "usermod -g sdbadmin_group sdbadmin" ) ;         // 更改sdbadmin用户的主组
      this.system.delGroup( groupObj.name ) ;
   }
   catch( e )
   {
      if( e == 4 && isUnique )
         ;
      else
         throw buildException( "testAddDelGroup", null, 
               "add del group " + this, "0 4", e ) ;
   }
   
   this.release() ;
}

// 测试枚举用户组
SystemTest.prototype.testListGroups = function()
{
   this.init() ;
   
   var groups = this.system.listGroups( { detail: true } ).toArray() ;
   var info = this.cmd.run( "cat /etc/group | awk -F : '{print $1,$3,$4}'" ).split( "\n" ) ;
   for( var i = 0;i < groups.length;i++ )
   {
      var groupObj = JSON.parse( groups[i] ) ;
      var tmp = info[i].split( " " ) ;
      var name = tmp[0] ;        // 用户组名
      var gid = tmp[1] ;         // GID
      var members = tmp[2] ;     // 用户组成员
      if( name != groupObj.name || gid != groupObj.gid || members != groupObj.members )
      {
         throw buildException( "testListGroups", null, 
               "test group info " + this, tmp, groups[i] ) ;
      }
   }
   
   this.release() ;
}

// 测试判断用户组是否存在
SystemTest.prototype.testIsGroupExist = function()
{
   this.init();
   
   var result = this.system.isGroupExist( "root" ) ;
   if( result != true )
   {
      throw buildException( "testIsGroupExist", null, 
            "test root " + this, true, result ) ;
   }
   result = this.system.isGroupExist( "!@#$%" ) ;
   if( result != false )
   {
      throw buildException( "testIsGroupExist", null, 
            "test !@#$% group " + this, false, result ) ;
   }
   
   this.release() ;
}

/******************************************************************************
*@Description : check group after create
*@author      : Liang XueWang            
******************************************************************************/
function checkGroup( cmd, groupObj )
{
   var info = cmd.run( "cat /etc/group | grep " + groupObj.name ).split( "\n" )[0] ;
   var tmp = info.split( ":" ) ;
   var groupName = tmp[0] ;
   var gid = tmp[2] ;
   if( groupName != groupObj.name || gid != groupObj.id )
   {
      throw buildException( "checkGroup", null, "check group info", 
                            tmp, JSON.stringify( groupObj ) ) ;
   }
}

/******************************************************************************
*@Description : get a idle gid from 1000-
*@author      : Liang XueWang            
******************************************************************************/
function getIdleGID( cmd )
{
   var gids = cmd.run( "cat /etc/group | awk -F : '{print $3}'" ).split( "\n" ) ;
   var gid = "1000" ;
   while( true )
   {
      if( gids.indexOf( gid ) == -1 )
      {
         break ;
      }
      gid = gid*1 + 1 + "" ;
   }
   return gid ;   
}


function main()
{
   // 获取本地主机和远程主机
   var localhost = toolGetLocalhost() ;
   var remotehost = toolGetRemotehost() ;
   
   var st1 = new SystemTest( localhost, CMSVCNAME ) ;
   var st2 = new SystemTest( remotehost, CMSVCNAME ) ;
   var sts = [ st1, st2 ] ;
   
   for( var i = 0;i < sts.length;i++ )
   {
      // 测试创建删除用户组，gid唯一
      sts[i].testAddDelGroup( true ) ;
      
      // 测试创建删除用户组，gid不唯一
      sts[i].testAddDelGroup( false ) ;
      
      // 测试枚举用户组
      sts[i].testListGroups() ;
      
      // 测试判断用户组是否存在
      sts[i].testIsGroupExist() ;
   }
}
   
main()