/**************************************
 * @Description: 测试用例 seqDB-10938 :: 版本: 1 :: 指定options参数为Global 
 * @author: ouyangzhongnan 
 * @Date: 2017-01-05
 * @RunDemo:
 * sdb -f "func.js,forceSession_10938.js" -e "var CHANGEDPREFIX='PREFIX';var COORDHOSTNAME='sdbserver01';var COORDSVCNAME='11810'"
 **************************************/

/*
步骤：
1、连接coord节点 
2、通过list()获取当前集群session（如db.list(2, {Global:false}) )） 
3、执行db.forceSession(sessionID,options)终止会话，options指定为Global（覆盖true、false取值） 
4、查看指定session是否存在
结果：
1、节点上存在指定sessionID被终止，部分节点未匹配到sessionID，系统返回对应错误信息
 */

main();
function main()
{
	if ( commIsStandalone(db) ) return;
	
	//TODO options = {Global:false}
	doTest(false);
	//TODO options = {Global:true}
	doTest(true);
}

function doTest(flag)
{
	var conn = getConn( COORDHOSTNAME, COORDSVCNAME );
	var sessionList = conn.list( SDB_LIST_SESSIONS, {Global:flag, Type:{$in:["Agent","ShardAgent","CoordAgent","ReplAgent","HTTPAgent"]}} ).toArray();
	var sessionid = JSON.parse(sessionList[Math.ceil(Math.random()*10) % sessionList.length]).SessionID;
	try
	{
		conn.forceSession( sessionid, {Global:flag} );
	}
	catch(e)
	{
		if ( !(e==-16 || e==-264) ) 
		{
			throw buildException("forceSession", e, "forceSession by sessionid and options", "forceSession success and throw exception(-16/-264)", "forceSession throw exception not is(-16/-264)" );
		}
	}
	sleep(3000);
	var reconn = getConn( COORDHOSTNAME, COORDSVCNAME );
	var sessionList = reconn.list( SDB_LIST_SESSIONS, {Global:flag, SessionID:sessionid} ).toArray();
	if ( sessionList.length !=0 )
	{
		println(sessionList.length);
		throw buildException("list session", new Error(), "list by sessionid after forceSession", "result is null", "result is not null");
	}
}

/**
 * 通过url获取连接
 * @param url 例：sdbserver01:11820
 */
function getConn(url)
{
	var conn = null;
	try
	{
		conn = new Sdb(url);
	}
	catch(e)
	{
		throw buildException( "connection to sdb by url["+url+"] error", e);
	}
	return conn;
}