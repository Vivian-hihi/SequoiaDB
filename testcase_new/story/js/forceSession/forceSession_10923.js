/**************************************
 * @Description: 测试用例 seqDB-10923 :: 版本: 1 :: options参数非法校验 
 * @author: ouyangzhongnan 
 * @Date: 2017-01-05
 * @RunDemo:
 * sdb -f "func.js,forceSession_10923.js" -e "var CHANGEDPREFIX='PREFIX';var COORDHOSTNAME='sdbserver01';var COORDSVCNAME='11810'"
 **************************************/
/*
步骤：
1、连接coord节点
2、执行db.forceSession(sessionID,options)终止会话，其中options参数设置非法，分别验证如下情况：
	a、参数非法，如db.forceSession(31,{Host:"test"})
	b、options参数为空，如db.forceSession(31,{})
	c、options参数格式非法，如db.forceSession(31,{1001})
	d、nodeID类型非法，如非int类型
4、查看操作结果
结果:
1、操作失败，返回对应错误信息
 */

main();
function main()
{
	if ( commIsStandalone(db) ) return;
	
	var currentSession = db.list( SDB_LIST_SESSIONS_CURRENT, {Global:false} ).next().toObj();

	// TODO 2.a 
	try
	{
		db.forceSession( currentSession.SessionID, {Host:"test"} );	
	}
	catch(e)
	{	
		if (e != -16) 
		{
			throw buildException( "forceSession", e, "forceSession by options is {Host:\"test\"}" );
		}
	}
	

	// TODO 2.b
	try
	{
		db.forceSession( currentSession.SessionID, {} );
	}
	catch(e)
	{
		if (e != -64) 
		{
			throw buildException( "forceSession", e, "forceSession by options is {}" );
		}
	}
	

	// TODO 2.c (这种情况js脚本并不能跑起来，本事语法上就有错误，所以不需要验证)
	// try
	// {
	// 	db.forceSession( currentSession.SessionID, {1001} );
	// }
	// catch(e)
	// {
	// 	println(e);
	// }
	
	// TODO 2.d
	try
	{
		db.forceSession( currentSession.SessionID, {NodeID:"abcd"} );
	}
	catch(e)
	{
		if (e != -64) 
		{
			throw buildException( "forceSession", e, "forceSession by options is {NodeID:\"abcd\"}" );
		}
	}

}