/************************************************************************
*@Description: 用Rest查询访问计划缓存快照
               对应testlink用例：eqDB-14512:获取访问计划快照
*@Author:  		FanYu  2018/04/03
************************************************************************/
main();

function main()
{  
   var csName = COMMCSNAME + "_14512";
   var clName = COMMCLNAME + "_14512";
    
	try
	{
	   ready( csName );
	   createCS( csName );
	   createCL( csName, clName );
	   //insert 
	   var recs = [];
	   var totalCnt = 1000;
	   for( var i = 0; i < totalCnt; i++ ) 
	   {
	      var rec = { NO:i, Flag:true, Index:"str_"+i.toString() };
	      recs.push( rec );
	   }	   
	   insertRecs( csName, clName, recs );
	  
	  var cond1 = "{NO:{$lt:100}}";
	  var cond2 = "{NO:{$gte:500}}";
	  var cond3 = "{$and:[{NO:{$lt:500}},{NO:{$gte:200}}]}"
	  queryRecs( csName, clName, recs, cond1); 
      queryRecs( csName, clName, recs, cond2);
      queryRecs( csName, clName, recs, cond3);	 
	  var actInfo = snapshot( csName,clName,"accessplans");
      var expInfo = ["{\"MinTimeSpentQuery\":{\"ReturnNum\":100}}", "{\"MinTimeSpentQuery\":{\"ReturnNum\":300}}","{\"MinTimeSpentQuery\":{\"ReturnNum\":500}}"];
	   check(actInfo,expInfo);
	   dropCL( csName, clName );
	   dropCS( csName );
	}
	catch( e )
	{
	   throw e ;
	}
	finally
	{
	}
}

function insertRecs( csName, clName, recs )
{
	println( "\n---Begin to excute " + getFuncName() );
	
	for( var i in recs )
	{
   	var recStr = JSON.stringify( recs[i] );
   		
   	var curlPara = [ 'cmd=insert', 
   	                 'name=' + csName + '.' + clName,
   	                 'insertor=' + recStr ];
   	var expErrno = 0;
   	runCurl( curlPara);
   }
}

function queryRecs( csName, clName, recs,cond)
{
   println( "\n---Begin to excute " + getFuncName() );

	var curlPara = [ 'cmd=query', 
	                 'name=' + csName + '.' + clName,
	                 'filter=' + cond];
	var expErrno = 0;
	runCurl( curlPara );
}

function snapshot( csName,clName,type)
{
	println( "\n---Begin to excute " + getFuncName() );

	var curlPara = [ 'cmd=snapshot ' + type,
	                 'filter={Collection:"'+csName+'.'+clName+'"}',
					 'selector='+"{'MinTimeSpentQuery.ReturnNum':{$include:1}}",
					 'sort=' + "{'MinTimeSpentQuery.ReturnNum':1}" 
	               ];
	var expErrno = 0;
	runCurl( curlPara);
    var resp = infoSplit;
	resp.shift();
	return resp;
}

function check(actInfo,expInfo)
{
  println( "\n---Begin to excute " + getFuncName() );
  if(actInfo.length != expInfo.length)
  {
   throw "actInfo.length  by snapshot is not equal axpInfo.length by self";
  }
  for(var i = 0; i < actInfo.length; i++)
  {
    if(actInfo[i].replace(/\//g,"").replace(/\s+/g,"") != expInfo[i])
    {
      throw "actInfo:" + JSON.stringify(actInfo[i])+"\nexpInfo:" + expInfo[i];
    }
  } 
}

function dropCL( csName, clName )
{
	println( "\n---Begin to excute " + getFuncName() );
	
	var curlPara = [ "cmd=drop collection", "name="+csName+'.'+clName ];
   runCurl( curlPara);
}

function dropCS( csName )
{
	println( "\n---Begin to excute " + getFuncName() );
	
	var curlPara = [ "cmd=drop collectionspace", "name="+csName ]
	runCurl( curlPara);
}

function createCS( csName )
{
	println( "\n---Begin to excute " + getFuncName() );
	
	var curlPara = [ 'cmd=create collectionspace', 'name='+csName ]
	runCurl( curlPara);
}

function createCL( csName, clName, group )
{
	println( "\n---Begin to excute " + getFuncName() );
	
	if ( group == undefined )
	{
   	var curlPara = [ 'cmd=create collection', 
   	                 'name='+csName+'.'+clName,
   	                 'options={ReplSize:-1}' ];
   }
   else
   {
      var curlPara = [ 'cmd=create collection', 
   	                 'name='+csName+'.'+clName,
   	                 'options={ReplSize:-1, Group:"'+group+'"}' ];
   }
	runCurl( curlPara);
}

function ready( csName, clName )
{
   println( "\n---Begin to excute " + getFuncName() );
   commDropCS( db, csName, true, "drop cs in begin" );
}

