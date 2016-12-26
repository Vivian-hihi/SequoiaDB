/******************************************************************************
*@Description : test js object oma function: listNodes
*               TestLink: 10617 Oma枚举节点
*                         10618 Oma枚举节点，type取值为null
*                         10619 Oma枚举节点，type取值为普通字符串abc
*@author      : Liang XueWang
******************************************************************************/

// 测试使用第一种条件枚举节点
OmaTest.prototype.testListNodes1 = function()
{
   this.testInit() ;
   var remote = new Remote( this.hostname, this.svcname ) ;
   var cmd = remote.getCmd() ;
   
   var option = {} ;
   option["type"] = "db" ;
   option["role"] = "coord" ;
   option["mode"] = "run" ;
   option["svcname"] = "" + COORDSVCNAME ;
   option["showalone"] = true ;
   option["expand"] = true ;
   option["displaymode"] = "obj" ;
   var filter = {} ;
   filter["svcname"] = "" + COORDSVCNAME ;
   
   var nodes = this.oma.listNodes( option, filter ) ;
   
   var InstallPath = toolGetSequoiadbDir( this.hostname, this.svcname ) ;
   var command = InstallPath + "/bin/sdblist -t db -r coord -p " + COORDSVCNAME + 
                 " -m run --expand" ;
   try
   {
      var tmpInfo = cmd.run( command ) ;
   }
   catch( e )
   {
      throw buildException( "testListNodes1", e, command + " " + this, 0, e ) ;
   }
   
   checkListNodes( nodes, tmpInfo ) ;
   
   remote.close() ;
   this.oma.close() ;
}

// 测试使用第二种条件枚举节点
OmaTest.prototype.testListNodes2 = function()
{
   this.testInit() ;
   var remote = new Remote( this.hostname, this.svcname ) ;
   var cmd = remote.getCmd() ;
   
   var option = {} ;
   option["type"] = "om" ;
   option["role"] = "om" ;
   option["mode"] = "local" ;
   option["showalone"] = false ;
   option["expand"] = false ;
   option["displaymode"] = "text" ;
   var filter = { $and: [ { role: "om" }, { type: "sdbom" } ] } ;
   
   var nodes = this.oma.listNodes( option, filter ) ;
   
   var InstallPath = toolGetSequoiadbDir( this.hostname, this.svcname ) ;
   var command = InstallPath + "/bin/sdblist -t om -r om -m local" ;
   var tmpInfo ;
   try
   {
      tmpInfo = cmd.run( command ) ;
   }
   catch( e )
   {
      if( e == 1 && !isOmExist( this.hostname, this.svcname ) )
      {
         tmpInfo = "Total: 0" ;
      }
      else
      {
         throw buildException( "testListNodes2", e, command + " " + this, 0, e ) ;
      }
   }
   
   checkListNodes( nodes, tmpInfo ) ;
   
   remote.close() ;
   this.oma.close() ;
}

// 测试使用第三种条件枚举节点
OmaTest.prototype.testListNodes3 = function()
{
   this.testInit() ;
   var remote = new Remote( this.hostname, this.svcname ) ;
   var cmd = remote.getCmd() ;
   
   var option = {} ;
   option["type"] = "all" ;
   option["showalone"] = true ;
   var filter = { $and: [ { $or: [ { role: "data" }, { role: "coord" }, { role: "catalog" }, 
                                   { role: "standalone" } ] }, { type: "sequoiadb" } ] } ;
                                    
   var nodes = this.oma.listNodes( option, filter ) ;
   
   var InstallPath = toolGetSequoiadbDir( this.hostname, this.svcname ) ;
   var command = InstallPath + "/bin/sdblist -t db" ;
   try
   {
      var tmpInfo = cmd.run( command ) ;
   }
   catch( e )
   {
      throw buildException( "testListNodes3", e, command + " " + this, 0, e ) ;
   }
   
   checkListNodes( nodes, tmpInfo ) ; 
                          
   this.oma.close() ;
   remote.close() ;
}

// 测试枚举节点type参数异常
OmaTest.prototype.testListNodesAbnormal = function()
{
   this.testInit() ;
   
   // 测试type:null或type:abc时枚举节点
   var option = [ { type: null }, { type: "abc" } ] ;
   for( var i = 0;i < option.length;i++ )
   {
      try
      {
         this.oma.listNodes( option[i] ) ;
         throw "list nodes with " + JSON.stringify( option[i] ) + " should be failed" ;
      }
      catch( e )
      {
         if( e != -6 )
         {
            throw buildException( "testListNodesAbnormal", e, 
                  "list nodes with " + JSON.stringify( option[i] ) + " " + this, -6, e ) ;
         }
      }
   }
   
   this.oma.close() ;
}

/******************************************************************************
*@Description : check result of listNodes with sdblist
*@author      : Liang XueWang            
******************************************************************************/
function checkListNodes( nodes, info )
{
   var num1 = nodes.toArray().length ;
   var tmpStr = "Total: " ;
   var ind = info.indexOf( tmpStr ) ;
   var num2 = info.slice( ind + tmpStr.length ).split( "\n" )[0] ;
   if( num1 != num2 )
   {
      println( "listnodes: " + JSON.stringify( nodes.toArray() ) ) ;
      println() ;
      println( "sdblist: " + info ) ;
      throw buildException( "checkListNodes", null, "check nodes num", num1, num2 ) ;
   }
}


function main()
{
   // 获取本地和远程主机
   var localhost = toolGetLocalhost() ;
   var remotehost = toolGetRemotehost() ;
   
   var ot1 = new OmaTest( localhost, CMSVCNAME ) ;
   var ot2 = new OmaTest( remotehost, CMSVCNAME ) ;
   var ots = [ ot1, ot2 ] ;
   
   for( var i = 0;i < ots.length;i++ )
   {
      // 测试枚举节点正常
      ots[i].testListNodes1() ;
      ots[i].testListNodes2() ;
      ots[i].testListNodes3() ;
      
      // 测试枚举节点异常
      ots[i].testListNodesAbnormal() ;
   }
}

main()