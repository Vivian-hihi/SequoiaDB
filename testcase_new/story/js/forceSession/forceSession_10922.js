/**************************************
 * @Description: 测试用例 seqDB-10922 :: 版本: 1 :: 指定sessionID不存在
 * @author: ouyangzhongnan 
 * @Date: 2017-01-05
 * @RunDemo:
 * sdb -f "func.js,forceSession_10922.js" -e "var CHANGEDPREFIX='PREFIX';var COORDHOSTNAME='sdbserver01';var COORDSVCNAME='11810'"
 **************************************/
 
 /*
 步骤：
 1、连接coord节点 
 2、执行db.forceSession(sessionID,options)终止会话，其中sessionID在集群中不存在，分别验证如下两种情况： 
 	a、sessionID不存在 
 	b、sessionID存在，指定options不匹配，如hostname、nodeID和sessionID不匹配 
 3、查看操作结果
 结果：
 1、操作失败，返回对应错误信息（如指定sessionID不存在等相关错误信息）
  */

main();
function main()
{
	if ( commIsStandalone(db) ) return;
	
	// var sessions = db.list( SDB_LIST_SESSIONS );
	// println( sessions );

	var session_id_1 = null;   // 用于保存一个不存在的sessionid
	var session_id_2 = null;   // 用于保存一个存在的sessionid
	// 获取一个[不存在&存在]的sessionid;
	while(true)
	{
		var temp = Math.ceil(Math.random()*100);
		var tempRes = db.list( SDB_LIST_SESSIONS, {SessionID:temp} ).toArray();
		if(tempRes.length == 0)
		{
			session_id_1 = temp;	
		} 
		else
		{
			session_id_2 = temp;
		}
		if (session_id_1 != null && session_id_2 != null) 
		{
			break;
		}
	}

	// TODO 2.a: force 一个集群中不存在sessionid
	try
	{
		db.forceSession( session_id_1, {Global:true} );	
		throw buildException( "forceSession", new Error(), "forceSession to do 2.a", "forceSession fail and throw exception", "forceSession pass" );
	}
	catch(e)
	{
		if (e != -264) 
		{
			throw buildException( "forceSession", new Error(), "forceSession to do 2.a", "forceSession fail and throw exception=-155", "forceSession fail but throw exception!=-155" );
		}
	}
	
	// TODO 2.b： force 一个集群中存在的sessionid，但是options不存在
	try
	{
		db.forceSession( session_id_2, {HostName:"sdbserver01",svcname:"21810"} );	
		throw buildException( "forceSession", new Error(), "forceSession to do 2.b", "forceSession fail and throw exception", "forceSession pass" );
	}
	catch(e)
	{
		if (e != -155) 
		{
			throw buildException( "forceSession", new Error(), "forceSession to do 2.b", "forceSession fail and throw exception=-155", "forceSession fail but throw exception!=-155" );
		}
	}
	
 
}