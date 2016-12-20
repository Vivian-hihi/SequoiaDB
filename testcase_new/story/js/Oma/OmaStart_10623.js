/******************************************************************************
*@Description : test Oma function start 手工测试
*               TestLink: 10623 10624
*@author      : Liang XueWang
******************************************************************************/

main()

function main()
{
   // testOmaStart() ;
}
   
/******************************************************************************
*@Description : test function start
*               测试oma对象启动cm（静态方法，不具备远程能力）
*@author      : Liang XueWang
******************************************************************************/
function testOmaStart()
{
   // 获取空闲端口
   var svcname = toolGetIdleSvcName( COORDHOSTNAME, CMSVCNAME ) ;
   if( svcname == undefined )
   {
      println( "No idle svcname between RSRVPORTBEGIN and RSRVPORTEND" ) ;
      return ;
   }
   
   // option为port:svcname standalone:true alivetime:2时启动cm
   var option = {} ;
   option["port"] = svcname ;
   option["alivetime"] = 2 ;
   option["standalone"] = true ;
   Oma.start( option ) ;
   
   // 检查cm端口及超时
   var oma = new Oma() ;
   var num = oma.listNodes( { type: "cm", showalone: true } ).toArray().length ;
   if( num != 3 )
   {
      throw buildException( "testOmaStart", 0, "check cm after start a standalone cm", 3, num ) ;
   }
   sleep( 5*1000 ) ;
   num =  oma.listNodes( { type: "cm", showalone: true } ).toArray().length ;
   if( num != 2 )
   {
      throw buildException( "testOmaStart", 0, "check cm after standalone cm timeout", 2, num ) ;
   }
   
   // 启动一个非standalone的cm时，首先需要先停止当前的cm
   var InstallPath = commGetInstallPath() ;
   var cmd = new Cmd() ;
   cmd.run( InstallPath + "/bin/sdbcmtop" ) ;
   
   // option为port:svcname standalone:false alivetime:0时启动cm
   var option = {} ;
   option["port"] = svcname ;
   option["alivetime"] = 0 ;
   option["standalone"] = false ;   
   Oma.start( option ) ;
   
   // 检查cm端口及超时
   oma = new Oma( COORDHOSTNAME, svcname ) ;
   num = oma.listNodes( { type: "cm" } ).toArray().length ;
   if( num != 2 )
   {
      throw buildException( "testOmaStart", 0, "check cm after start new cm", 2, num ) ;
   }
   sleep( 5*1000 ) ;
   num = oma.listNodes( { type: "cm" } ).toArray().length ;
   if( num != 2 )
   {
      throw buildException( "testOmaStart", 0, "check cm when alivetime is 0", 2, num ) ;
   }
   oma.close() ;
   
   // 测试完成后，恢复原有的cm端口
   cmd.run( InstallPath + "/bin/sdbcmtop" ) ;
   var option = {} ;
   option["port"] = CMSVCNAME ;
   option["alivetime"] = 0 ;
   option["standalone"] = false ;   
   Oma.start( option ) ;
      
   // windows下asport参数手工验证
}