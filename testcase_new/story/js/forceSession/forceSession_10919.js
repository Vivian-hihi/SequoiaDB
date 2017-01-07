/**************************************
 * @Description: 连接coord节点指定sessionID和options参数终止会话  测试用例seqDB-10919 
 * @author: ouyangzhongnan 
 * @Date: 2017-01-04
 * @RunDemo:
 * sdb -f "func.js,forceSession_10919.js" -e "var CHANGEDPREFIX='PREFIX';var COORDHOSTNAME='sdbserver01';var COORDSVCNAME='11810'"
 **************************************/
/*
操作步骤：
1、连接coord节点 
2、通过list()获取当前集群session（如db.list( SDB_LIST_CONTEXTS )） 
3、执行db.forceSession(sessionID,options)终止会话，分别验证如下场景： 
	a、options指定NodeID 
	b、options指定HostName 
	c、options指定svcname 
	d、options指定两个参数，如NodeID和hostName 
	e、options指定三个参数 
4、查看指定session是否存在 
期望结果：
1、a、d、e场景，指定session被终止，查看session已不存在 
2、b、c场景，节点上存在指定sessionID被终止，部分节点未匹配到sessionID，系统返回对应错误信息
*/

 main();
 function main()
 {
 	if ( commIsStandalone(db) ) return;
 	
 	// TODO prepare param
 	var dataGroups = commGetGroups( db, true, "", true, true, true );
 	//var x = Math.ceil(Math.random()*10) % dataGroups.length;   //x , y用于随机去取节点
 	//var y = Math.ceil(Math.random()*10) % (dataGroups[x].length-1);
	var node = dataGroups[0][1];
	var hostName = node.HostName;
	var svcName = node.svcname;
	var nodeID = node.NodeID;
	var nodeName = hostName +":"+ svcName;

	var param1 = {"NodeName" : nodeName};
	// TODO 3.a
	doTest( param1, {"NodeID" : nodeID} );
	// TODO 3.b
	doTest( param1, {"HostName" : hostName}, true);
	// TODO 3.c
	doTest( param1, {"svcname" : svcName}, true );
	// TODO 3.d
	doTest( param1, {"NodeID" : nodeID, "HostName" : hostName} );
	// TODO 3.e
	doTest( param1, {"NodeID" : nodeID, "HostName" : hostName, "svcname" : svcName} );
 }

/**
 * 执行具体的测试
 * @param  param1 例如：{NodeName:"sdbserver03:11830"}, 
 * 通过这个获取对应对应的机器对应节点上的session列表，以获取sessionId用于force
 * @param  param2 db.forceSession(<sessionid>[,options]) 的options
 * @param  param3 是否可能抛出-264的错误，true/false
 */
 function doTest(param1, param2, param3)
 {
 	var sessionList = null;
	var forceSessionID = null;
	if (param3 == undefined) param3 = false;
 	// get sessionid to force sesssion
	try
	{
		sessionList = db.list( SDB_LIST_CONTEXTS, param1 );
		forceSessionID = sessionList.next().toObj().SessionID;
	}
	catch(e)
	{
		throw buildException( "someoperate", e, "list session and get a sessionid to be force "+JSON.stringify(param2) );
	}
	// force session
	try
	{
		db.forceSession( forceSessionID, param2 );
	}
	catch(e)
	{	
		if ( param3 == true )
		{
			if ( e != -264 ) 
			{
				throw buildException("forceSession", e, "force session by SessionID=forceSessionID and options is "+JSON.stringify(param2), "throw exception and e = -264", "throw exception, but e != -264");	
			}
		} 
		else 
		{
			println(e);
			throw buildException("forceSession", e, "force session by SessionID=forceSessionID and options is "+JSON.stringify(param2), "force session success", "force session throw exception");
		}
	}
	// check the force session can be list
	sleep( 1000 );
	try
	{
		sessionList = db.list( SDB_LIST_CONTEXTS, {"SessionID" : forceSessionID} ).toArray();
		if ( sessionList.length != 0 ) 
		{
			println("====================================");
			for (var i = 0; i < sessionList.length; i++) {
				println( sessionList[i] );
			}
			println("====================================");
			throw buildException("list session", new Error(), "list session by SessionID=oldSessionId "+JSON.stringify(param2), "count for sessionList equals 0", "count for sessionList not equals 0");
		}
	}
	catch(e)
	{
		throw buildException( "someoperate", e, "list session by SessionID=forceSessionID and check list result "+JSON.stringify(param2) );
	}
 }