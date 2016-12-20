/******************************************************************************
*@Description : test Oma function listNodes getNodeConfigs setNodeConfigs
*               updateNodeConfigs
*               TestLink: 10617 10618 10619 10620 10621 10622
*@author      : Liang XueWang
******************************************************************************/

// 测试正常枚举节点
OmaTest.prototype.testListNodesNormal = function()
{
   this.testInit() ;
   var remote = new Remote( this.hostname, this.svcname ) ;
   var cmd = remote.getCmd() ;
   
   // 测试选项条件为type:db, role:coord, mode:run, showalone:true, expand:true, displaymode:obj
   // 过滤条件为role:coord时枚举节点
   var option = {} ;
   option["type"] = "db" ;
   option["role"] = "coord" ;
   option["mode"] = "run" ;
   option["svcname"] = "" + COORDSVCNAME ;
   option["showalone"] = true ;
   option["expand"] = true ;
   option["displaymode"] = "obj" ;
   var filter = {} ;
   filter["role"] = "coord" ;
   var nodes = this.oma.listNodes( option, filter ) ;
   
   var InstallPath = commGetInstallPath() ;
   var tmpInfo = cmd.run( InstallPath + "/bin/sdblist -t db -r coord -m run --expand" ) ;
   checkListNodes( nodes, tmpInfo ) ;
   
   // 测试选项条件为type:om, role:om, mode:local, showalone:false, expand:false, displaymode:text
   // 过滤条件为$and时枚举节点 
   var option = {} ;
   option["type"] = "om" ;
   option["role"] = "om" ;
   option["mode"] = "local" ;
   option["showalone"] = false ;
   option["expand"] = false ;
   option["displaymode"] = "text" ;
   var filter = { $and: [ { role: "om" }, { type: "sdbom" } ] } ;
   nodes = this.oma.listNodes( option, filter ) ;
   try
   {
      tmpInfo = cmd.run( InstallPath + "/bin/sdblist -t om -r om -m local" ) ;
   }
   catch( e )
   {
      if( e == 1 )
      {
         tmpInfo = "Total: 0" ;
         println( "sdblist om get nothing,hostname=" + this.hostname ) ;
      }
      else
      {
         throw buildException( "testListNodes", e, "sdblist om", 0, e ) ;
      }
   }
   checkListNodes( nodes, tmpInfo ) ;
   
   // 测试选项条件为type:all,过滤条件为$and $or组合时枚举节点
   var option = {} ;
   option["type"] = "all" ;
   var filter = { $and: [ { $or: [{role:"data"},{role:"coord"},{role:"catalog"}] },{ type: "sequoiadb" } ] } ;
   nodes = this.oma.listNodes( option, filter ) ;
   tmpInfo = cmd.run( InstallPath + "/bin/sdblist -t db" ) ;
   checkListNodes( nodes,tmpInfo ) ; 
                          
   this.oma.close() ;
   remote.close() ;
}

// 测试枚举节点type参数非法
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
         throw 0 ;
      }
      catch( e )
      {
         if( e != -6 )
         {
            throw buildException( "testListNodes", e, 
                  "list nodes " + JSON.stringify( option[i] ), -6, e ) ;
         }
      }
   }
   
   this.oma.close() ;
}

// 测试设置、更新、获取节点配置
OmaTest.prototype.testNodeConfigs = function()
{
   this.testInit() ;
   var remote = new Remote( this.hostname, this.svcname ) ;
   var cmd = remote.getCmd() ;
   var InstallPath = commGetInstallPath() ;
   
   // 测试getNodeConfigs
   try
   {
      var configs = this.oma.getNodeConfigs( COORDSVCNAME ).toObj() ;
      var contents = cmd.run( "cat " + InstallPath + "/conf/local/" + 
                              COORDSVCNAME + "/sdb.conf" ).split( "\n" ) ;
   }
   catch( e )
   {
      throw buildException( "testNodeConfig", e, "get coord node config", 0, e ) ;
   }
   checkResult( configs, contents, "getNodeConfigs" ) ;
   
   // 测试setNodeConfigs
   configs["tmpConf"] = "foo" ;
   this.oma.setNodeConfigs( COORDSVCNAME, configs ) ;
   var tmpConf = this.oma.getNodeConfigs( COORDSVCNAME ).toObj().tmpConf ;
   if( tmpConf != "foo" )
   {
      throw buildException( "testNodeConfigs", 0, "set node config", "foo", tmpConf ) ;
   }
   
   // 测试updateNodeConfigs
   configs["tmpConf"] = "bar" ;
   this.oma.updateNodeConfigs( COORDSVCNAME, configs ) ;
   tmpConf = this.oma.getNodeConfigs( COORDSVCNAME ).toObj().tmpConf ;
   if( tmpConf != "bar" )
   {
      throw ( ">fail to test updateNodeConfigs" ) ;
   }
      
   // 测试完成后，清除配置文件中的tmpConf
   delete configs.tmpConf ;
   this.oma.setNodeConfigs( COORDSVCNAME, configs ) ;
   
   this.oma.close() ;
   remote.close() ;
}

/******************************************************************************
*@Description : check result of listNodes with sdblist
*@author      : Liang XueWang            
******************************************************************************/
function checkListNodes( nodes,info )
{
   var num1 = nodes.toArray().length ;
   var tmpStr = "Total: " ;
   var ind = info.indexOf( tmpStr ) ;
   var num2 = info.slice( ind + tmpStr.length ).split( "\n" )[0] ;
   if( num1 != num2 )
      throw buildException( "checkListNodes", 0, "check nodes num", num1, num2 ) ;
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
      try
      {
         // 测试枚举节点
         ots[i].testListNodesNormal() ;
         ots[i].testListNodesAbnormal() ;
         
         // 测试获取、设置、更新节点配置
         ots[i].testNodeConfigs() ;
      }
      catch( e )
      {
         ots[i].toString() ;
         throw e ;
      }
   }
}

main()