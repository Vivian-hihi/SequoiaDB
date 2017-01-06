/**************************************
 * @Description: 测试用例 seqDB-10918 指定sessionID终止当前节点会话 
 * @author: ouyangzhongnan 
 * @Date: 2017-01-04
 * @RunDemo:
 * sdb -f "func.js,forceSession_10918.js" -e "var CHANGEDPREFIX='PREFIX';var COORDHOSTNAME='sdbserver01';var COORDSVCNAME='11810'"
 **************************************/
/*
步骤：
1、直连当前节点，分别验证如下场景： 
	a、连接coord节点 
	b、连接data节点 
	c、连接catalog节点 
2、通过list()获取当前集群session（如list（3,{Global:false}）） 
3、指定当前节点上的sessionID,执行db.forceSession(sessionID)终止会话 
4、查看指定session是否存在
 */
main();
function main()
{

	if ( commIsStandalone(db) ) return;

	/**
	 * 初始变量值
	 *    保存内容, 例：coord = [sdbserver01:11810,sdbserver02:11810,sdbserver03:11810];
	 */
	var catalog   = [];
	var datagroup = [];
 	var SYSCatalogGroup = commGetGroups( db, true, "SYSCatalogGroup", false, true, true );
 	for (var i = 1; i < SYSCatalogGroup[0].length; i++) 
 	{
 		var url = SYSCatalogGroup[0][i].HostName +":"+ SYSCatalogGroup[0][i].svcname;
 		catalog.push( url );
 	}
 	var DataGroup = commGetGroups( db, true, "", true, true, true );
 	for (var i = 1; i < DataGroup[0].length; i++) 
 	{
 		var url = DataGroup[0][i].HostName +":"+ DataGroup[0][i].svcname;
 		datagroup.push( url );
 	}

 	doTest( COORDHOSTNAME+":"+COORDSVCNAME );
 	doTest( catalog[0] );
 	doTest( datagroup[0] );
}

/**
 * 通过给定的url连接到对应的节点，执行测试用例中指定的操作步骤
 * @param  url 要连接的节点的地址，如sdbserver01:11820
 */
function doTest(url)
{
	var oldSessionId = null;
	try
	{
	 	var conn = new Sdb( url );
	 	oldSessionId = conn.list( 3, {Global:false} ).next().toObj().SessionID;
	}
	catch(e)
	{
		throw buildException( "someoperate", e, "connection sdb and list session" );
	}

 	try
 	{
 		conn.forceSession( oldSessionId );
 		throw buildException("forceSession", new Error(), "force current session", "throw exception", "not throw exception");
 	}
 	catch(e)
 	{
 		if ( e != -16) 
 			throw buildException("forceSession", e, "force current session", "throw exception and e = -16", "throw exception, but e != -16"); 	
 	}

 	sleep(1000);
 	try
 	{
 		var reconn = new Sdb( url );
 		var sessionList = reconn.list( 3, {Global:false,SessionID:oldSessionId} ).toArray();
 		if ( sessionList.length != 0) 
 			throw buildException("list session", new Error(), "list session by SessionID="+forceSessionID, "count for sessionList equals 0", "count for sessionList not equals 0");
 	}
 	catch(e)
 	{
 		throw buildException( "someoperate", e, "connection sdb and list session" );
 	}
}