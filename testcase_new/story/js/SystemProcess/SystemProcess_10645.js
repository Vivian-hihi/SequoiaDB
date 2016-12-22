/******************************************************************************
*@Description : test js object System function: listProcess isProcExist
*                                               killProcess
*               TestLink : 10645 System对象枚举进程如sdbcm
*                          10646 System对象判断进程是否存在
*                          10647 System对象判断进程是否存在，type取值name/pid，与value不匹配
*                          10648 System对象杀死进程如协调节点11810
*@author      : Liang XueWang
******************************************************************************/

// 测试枚举进程
SystemTest.prototype.testListProcess = function()
{
   this.init() ;
   
   // 测试detail为true时枚举sdbcm进程
   var cmProcs ;
   cmProcs = this.system.listProcess( { detail: true }, 
                                      { cmd: "sdbcm(" + CMSVCNAME + ")" } ).toArray() ;
   if( cmProcs.length != 1 )
   {
      throw buildException( "testListProcess", null, 
                            "list sdbcm process " + this, 1, cmProcs.length ) ;
   }
   var command = "ps aux | grep 'sdbcm(" + CMSVCNAME + ")' | grep -v grep | " +
                 "awk '{print $1,$2,$8,$11}'" ;
   var result = this.cmd.run( command ).split( "\n" )[0].split( " " ) ;
   var user = result[0] ;     // 进程用户
   var pid = result[1] ;      // 进程id
   var stat = result[2] ;     // 进程状态
   var order = result[3] ;    // 进程命令
   var obj = JSON.parse( cmProcs[0] ) ;
   if( user != obj.user || pid != obj.pid || stat != obj.status || order != obj.cmd )
   {
      throw buildException( "testListProcess", null, 
            "list sdbcm process detail true " + this, result, cmProcs[0] ) ;
   }
   
   // 测试detail为false时枚举sdbcm进程
   cmProcs = this.system.listProcess( { detail: false }, 
                                      { cmd: "sdbcm(" + CMSVCNAME + ")" } ).toArray() ;
   obj = JSON.parse( cmProcs[0] ) ;
   if( undefined != obj.user || pid != obj.pid || 
       undefined != obj.stat || order != obj.cmd )
   {
      throw buildException( "testListProcess", null, 
            "list sdbcm process detail false " + this, result, cmProcs[0] ) ;
   }
   
   this.release() ;
}

// 测试判断进程是否存在
SystemTest.prototype.testIsProcExist = function()
{
   this.init() ;
   
   // 测试判断存在的sdbcm进程
   var result ;
   result = this.system.isProcExist( { value: "sdbcm(" + CMSVCNAME + ")", type: "name" } ) ;
   if( result != true )
   {
      throw buildException( "testIsProcExist", null, 
            "test sdbcm exist " + this, true, result ) ;
   }
   // 测试判断sdbcm进程，type与value不匹配
   result = this.system.isProcExist( { value: "sdbcm(" + CMSVCNAME +")", type: "pid" } ) ;
   if( result != false )
   {
      throw buildException( "testIsProcExist", null, 
            "test sdbcm mismatch " + this, false, result ) ;
   }
   // 测试判断不存在的进程
   result = this.system.isProcExist( { value: "sdbcm", type: "name" } ) ;
   if( result != false )
   {
      throw buildException( "testIsProcExist", null, 
            "test sdbcm notexist " + this, false, result ) ;
   }
   
   this.release() ;
}

// 测试杀死进程，协调节点11810进程，强杀
SystemTest.prototype.testKillProcessWithSigKill = function()
{
   this.init() ;
   
   // 首先获取进程id
   var filter = {} ;
   if( this.isStandalone )
      filter = { cmd: "sequoiadb(" + COORDSVCNAME + ")" } ;
   else
      filter = { cmd: "sequoiadb(" + COORDSVCNAME + ") S" } ;
   var process = this.system.listProcess( {}, filter ).toArray() ;
   var obj = JSON.parse( process[0] ) ;
   var pid = obj.pid ;
   
   // 测试强杀进程
   var option = {} ;
   option["pid"] = pid*1 ;
   option["sig"] = "kill" ;
   this.system.killProcess( option ) ;
   process = this.system.listProcess( {}, filter ).toArray() ;
   if( process.length != 0 )
   {
      throw buildException( "testKillProcessWithSigKill", null, 
                            "test kill coord " + this, 0, process.length ) ;
   }
   
   // 测试强杀之后自动重启
   while( process.length != 1 )
   {
      sleep( 100 ) ;
      process = this.system.listProcess( {}, filter ).toArray() ;   
   }
   
   this.release() ; 
}

// 测试杀死进程，协调节点11810进程，普通杀
SystemTest.prototype.testKillProcessWithSigTerm = function()
{
   this.init() ;
   
   // 获取进程id
   var filter = {} ;
   if( this.isStandalone )
      filter = { cmd: "sequoiadb(" + COORDSVCNAME + ")" } ;
   else
      filter = { cmd: "sequoiadb(" + COORDSVCNAME + ") S" } ;
   var process = this.system.listProcess( {}, filter ).toArray() ;
   var obj = JSON.parse( process[0] ) ;
   var pid = obj.pid ;
   
   // 测试普通杀进程
   var option = {} ;
   option["pid"] = pid*1 ;
   option["sig"] = "term" ;
   this.system.killProcess( option ) ;
   process = this.system.listProcess( {}, filter ).toArray() ;
   while( process.length != 0 )
   {
      sleep( 100 ) ;
      process = this.system.listProcess( {}, filter ).toArray() ;
   }
   
   // 测试普通杀之后，不会重启
   sleep( 5000 ) ;
   process = this.system.listProcess( {}, filter ).toArray() ;
   if( process.length != 0 )
   {
      throw buildException( "testKillProcessWithSigTerm", null, 
            "test cannot auto restart " + this, 0, process.length ) ;
   }
   
   // 测试完成后，重启
   var oma = new Oma( this.hostname, CMSVCNAME ) ;
   oma.startNode( COORDSVCNAME*1 ) ;
   oma.close() ;
   while( process.length != 1 )
   {
      sleep( 100 ) ;
      process = this.system.listProcess( {}, filter ).toArray() ;
   }
   
   this.release() ;
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
      // 测试枚举进程
      sts[i].testListProcess() ;
      
      // 测试判断进程是否存在
      sts[i].testIsProcExist() ;
      
      // 测试强杀进程
      sts[i].testKillProcessWithSigKill() ;
      
      // 测试普通杀进程
      sts[i].testKillProcessWithSigTerm() ;
   }
}
   
main()