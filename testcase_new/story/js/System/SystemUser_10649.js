/******************************************************************************
*@Description : test js object System function: getUserEnv addUser delUser
*               setUserConfigs listLoginUsers listAllUsers getCurrentUser
*               isUserExist
*               TestLink : 10649 System对象获取当前用户环境信息
*                          10650 System对象添加、删除用户
*                          10651 System对象添加用户，已存在用户
*                          10655 System对象设置用户信息
*                          10656 System对象枚举已登录用户
*                          10657 System对象枚举所有用户
*                          10659 System对象获取当前用户信息
*                          10660 System对象判断用户是否存在
*@author      : Liang XueWang
******************************************************************************/

// 测试获取用户环境变量
SystemTest.prototype.testGetUserEnv = function()
{
   this.init() ;
   
   var environ = this.system.getUserEnv().toObj() ;
   var tmpInfo = this.cmd.run( "env" ).split( "\n" ) ;
   var tmpObj = {} ;
   for( var i = 0;i < tmpInfo.length-1;i++ )
   {
      var index = tmpInfo[i].indexOf( "=" ) ;
      var key = tmpInfo[i].slice( 0, index ) ;
      var value = tmpInfo[i].slice( index + 1 ) ;
      tmpObj[key] = value ;   
   }
   
   for( var k in tmpObj )
   {
      if( tmpObj[k] != environ[k] )
      {
         throw buildException( "testGetUserEnv", null, 
               "check key: " + k + " " + this, tmpObj[k], environ[k] ) ;
      }   
   }
   
   this.release() ;   
}

// 测试创建用户删除用户，createDir取值为true/false
SystemTest.prototype.testAddDelUser = function( createDir )
{
   // 检查cm用户是否为root
   var user = toolGetSdbcmUser( this.hostname, this.svcname ) ;
   if( user != "root" )
   {
      // println( user + " have no permission to create user." ) ;
      return ;
   }
   
   var userObj = {} ;
   userObj["name"] = "createUser" ;          // 用户名
   userObj["passwd"] = "createUser" ;        // 用户密码
   userObj["group"] = "sequoiadb" ;          // 用户组
   userObj["Group"] = "sdbadmin_group" ;     // 附加用户组
   userObj["dir"] = "/home/createUser" ;     // 用户主目录
   userObj["createDir"] = createDir ;        // 是否自动创建用户主目录
   
   try
   {
      // 创建用户
      this.system.addUser( userObj ) ;
   }
   catch( e )
   {
      throw buildException( "testAddDelUser", e, 
                            "add user: " + user + " " + this, 0, e ) ;
   }
   
   // 检查用户
   checkUser( this.cmd, userObj.name ) ;
   
   // 检查用户组和附加用户组
   checkGroup( this.cmd, userObj.group, userObj.name ) ;
   checkGroup( this.cmd, userObj.Group, userObj.name ) ;
   
   // 检查用户主目录
   checkDir( this.cmd, userObj.dir, createDir ) ;
   
   // 测试完成后，删除用户
   var option = {} ;
   option["name"] = userObj.name ;
   option["isRemoveDir"] = createDir ;
   this.system.delUser( option ) ;
   
   this.release() ;
}

// 测试创建已存在用户 sdbadmin
SystemTest.prototype.testAddExistUser = function()
{
   // 检查cm用户是否为root
   var user = toolGetSdbcmUser( this.hostname, this.svcname ) ;
   if( user != "root" )
   {
      // println( user + " have no permission to create user." ) ;
      return ;
   }
   
   this.init() ;
   try
   {
      this.system.addUser( { name: "sdbadmin" } ) ;
      throw "create sdbadmin user should be failed" ;
   }
   catch( e )
   {
      if( e != 9 )
      {
         throw buildException( "testAddExistUser", e, "add exist user " + this, 9, e ) ;
      }   
   }
   this.release() ;
}

// 测试设置用户属性
SystemTest.prototype.testSetUserConfigs = function()
{
   // 检查cm用户是否为root
   var user = toolGetSdbcmUser( this.hostname, this.svcname ) ;
   if( user != "root" )
   {
      println( user + " have no permission to set user configs." ) ;
      return ;
   }
   
   // 首先创建用户
   var userObj = {} ;
   userObj["name"] = "modifyUser" ;
   userObj["passwd"] = "modifyUser" ;
   userObj["dir"] = "/tmp/modifyUser" ;
   userObj["createDir"] = true ;
   this.system.addUser( userObj ) ;
   
   // 设置用户属性
   var option = {} ;
   option["name"] = "modifyUser" ;
   option["group"] = "sequoiadb" ;
   option["Group"] = "sdbadmin_group" ;
   option["isAppend"] = true ;
   option["dir"] = "/home/modifyUser" ;
   option["isMove"] = true ;
   
   try
   {
      this.system.setUserConfigs( option ) ;
   }
   catch( e )
   {
      throw buildException( "testSetUserConfigs", e, 
            "set user: " + option["name"] + " config " + this, 0, e ) ;
   }
   
   // 检查用户组和附加用户组
   checkGroup( this.cmd, option.group, option.name ) ;
   checkGroup( this.cmd, option.Group, option.name ) ;
      
   // 检查用户主目录
   checkDir( this.cmd, option.dir, true ) ;
   
   // 测试完成后，删除用户
   var option = {} ;
   option["name"] = userObj.name ;
   option["isRemoveDir"] = true ;
   this.system.delUser( option ) ;
   
   this.release() ;
}

// 测试枚举登录用户
SystemTest.prototype.testListLoginUsers = function()
{
   this.init() ;
   var users = this.system.listLoginUsers( { detail: true } ).toArray() ;
   var info = this.cmd.run( "who | sed 's/  */ /g'" ).split( "\n" ) ;
   for( var i = 0;i < users.length;i++ )
   {
      var userObj = JSON.parse( users[i] ) ;
      var tmp = info[i].split( " " ) ;
      var username = tmp[0] ;             // 用户名
      var tty = tmp[1] ;                  // 登录终端
      var time = tmp[2] + " " + tmp[3] ;  // 登录时间
      var addr ;                          // 登录的主机名或者ip
      if( tmp[4] == undefined )
      {
         addr = "" ;
      }
      else
      {
         addr = tmp[4].slice( 1, tmp[4].length-1 ) ;
      }  
      if( username != userObj.user || tty != userObj.tty ||
          time != userObj.time || addr != userObj.from )
      {
         throw buildException( "testListLoginUsers", null, 
               "check info " + this, tmp, JSON.stringify( userObj ) ) ;
      }
   }
   this.release() ;
}

// 测试枚举所有用户
SystemTest.prototype.testListAllUsers = function()
{
   this.init() ;
   var users = this.system.listAllUsers( { detail: true } ).toArray() ;
   var info = this.cmd.run( "cat /etc/passwd" ).split( "\n" ) ;
   for( var i = 0;i < users.length;i++ )
   {
      var userObj = JSON.parse( users[i] ) ;
      var tmp = info[i].split( ":" ) ;
      var username = tmp[0] ;    // 用户名
      var groupid = tmp[3] ;     // 用户组id
      var dir = tmp[5] ;         // 用户主目录
      if( username != userObj.user || groupid != userObj.gid || dir != userObj.dir )
      {
         throw buildException( "testListAllUsers", null, 
               "check info " + this, tmp, JSON.stringify( userObj) ) ;
      }   
   }
   this.release() ;
}

// 测试获取当前用户信息
SystemTest.prototype.testGetCurrentUser = function()
{
   this.init() ;
   var userObj = this.system.getCurrentUser().toObj() ;
   var username = userObj.user ;
   var info = this.cmd.run( "cat /etc/passwd | grep -w " + username ).split( "\n" ) ;
   var found = false ;
   for( var i = 0;i < info.length-1;i++ )
   {
      var tmp = info[i].split( ":" ) ;
      var name = tmp[0] ;      // 用户名
      var groupid = tmp[3] ;   // 用户组id
      var dir = tmp[5] ;       // 用户主目录
      if( name == userObj.user && groupid == userObj.gid && dir == userObj.dir )
      {
         found = true ;
         break ;
      }
   }
   if( found == false )
   {
      throw buildException( "testGetCurrentUser", null, 
            "check current user " + this, info, JSON.stringify( userObj ) ) ;
   }
   this.release() ; 
}

// 测试判断用户是否存在
SystemTest.prototype.testIsUserExist = function()
{
   this.init() ;
   
   var result = this.system.isUserExist( "root" ) ;
   if( result != true )
   {
      throw buildException( "testIsUserExist", null, "test user root " + this, 
                            true, result ) ;
   }
   result = this.system.isUserExist( "!@#$%" ) ;
   if( result != false )
   {
      throw buildException( "testIsUserExist", null, "test user !@#$% " + this, 
                            false, result ) ;
   }
   
   this.release() ;  
}


/******************************************************************************
*@Description : check user name  in /etc/passwd
*@author      : Liang XueWang            
******************************************************************************/
function checkUser( cmd, username )
{
   var names = cmd.run( "cat /etc/passwd | grep -w " + username + 
                       " | awk -F : '{print $1}'").split( "\n" ) ;
   if( names.indexOf( username ) == -1 )
   {
      throw buildException( "checkUser", null, "check user name", username, name ) ;   
   } 
}

/******************************************************************************
*@Description : check group and Group of user with id command
*@author      : Liang XueWang            
******************************************************************************/
function checkGroup( cmd, groupname, username )
{
   var info = cmd.run( "id " + username ).split( "\n" )[0] ; 
   if( info.indexOf( groupname ) == -1 )
   {
      throw buildException( "checkGroup", null, "check group: " + groupname + 
                            " of user: " + username, groupname, info ) ;
   }
}

/******************************************************************************
*@Description : check user dir
*@author      : Liang XueWang            
******************************************************************************/
function checkDir( cmd, dir, createDir )
{
   try
   {
      cmd.run( "ls -al " + dir ) ;
   }
   catch( e )
   {
      if( !createDir && e == 2 )
         ;
      else
         throw buildException( "checkDir", null, "check user dir", 0, e ) ;
   }
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
      // 测试获取用户环境
      sts[i].testGetUserEnv() ;
      
      // 测试创建删除用户，自动创建删除用户主目录
      sts[i].testAddDelUser( true ) ;
      
      // 测试创建删除用户，不创建删除用户主目录
      sts[i].testAddDelUser( false ) ;
      
      // 测试创建已存在用户
      sts[i].testAddExistUser() ;
      
      // 测试修改用户属性
      sts[i].testSetUserConfigs() ;
      
      // 测试枚举登录用户
      // sts[i].testListLoginUsers() ;
      
      // 测试枚举所有用户
      sts[i].testListAllUsers() ;
      
      // 测试获取当前用户信息
      sts[i].testGetCurrentUser() ;
      
      // 测试判断用户是否存在
      sts[i].testIsUserExist() ;
   }
}
   
main()