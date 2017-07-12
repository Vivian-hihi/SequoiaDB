/************************************
*@Description: compare actual and expect result,
               they is not the same ,return error ,
               else return ok
*@author:      zhaoyu
*@createDate:  2015.5.20
**************************************/
function checkRec( rc, expRecs )
{				
	//get actual records to array
	var actRecs = [];
   while( rc.next() )
   {
		actRecs.push( rc.current().toObj() );
   }
   //check count
	if( actRecs.length !== expRecs.length )
   {
   	println("\nactual recs in cl= "+JSON.stringify(actRecs)+"\n\nexpect recs= "+JSON.stringify(expRecs));
   	throw buildException("check count", null, "",
									expRecs.length, actRecs.length);
   }
   
   //check every records every fields,expRecs as compare source
   for( var i in expRecs )
   {
   	var actRec = actRecs[i];
   	var expRec = expRecs[i];
   	
   	for ( var f in expRec )
   	{
   		if( JSON.stringify(actRec[f]) !== JSON.stringify(expRec[f]) )
	   	{
	   		println("\nerror occurs in "+(parseInt(i)+1)+"th record, in field '"+f+"'");
	   		println("\nactual record= "+JSON.stringify(actRec)+"\n\nexpect record= "+JSON.stringify(expRec));
	   		println("\nactual recs in cl= "+JSON.stringify(actRecs)+"\n\nexpect recs= "+JSON.stringify(expRecs));   		
	   		throw buildException("checkRec()", "rec ERROR");
	   	}
   	}
   }
   //check every records every fields,actRecs as compare source
   for( var j in actRecs )
   {
   	var actRec = actRecs[j];
   	var expRec = expRecs[j];
   	
   	for ( var f in actRec )
   	{
   	   if(f == "_id")
   	   {
   	      continue;
   	   }
   		if( JSON.stringify(actRec[f]) !== JSON.stringify(expRec[f]) )
	   	{
	   		println("\nerror occurs in "+(parseInt(j)+1)+"th record, in field '"+f+"'");
	   		println("\nactual record= "+JSON.stringify(actRec)+"\n\nexpect record= "+JSON.stringify(expRec));
	   		println("\nactual recs in cl= "+JSON.stringify(actRecs)+"\n\nexpect recs= "+JSON.stringify(expRecs));   		
	   		throw buildException("checkRec()", "rec ERROR");
	   	}
   	}
   }
}

/************************************
*@Description: get actual result and check it 
*@author:      zhaoyu
*@createDate:  2017.7.11
**************************************/
function findBySortLimitSkipAndCheck( dbcl, findConf, selectConf, sortConf, limitConf, skipConf, expRecs )
{
	if( typeof(findConf) == "undefined" ) { findConf = null; }
	if( typeof(selectConf) == "undefined" ) { selectConf = null; }
	if( typeof(sortConf) == "undefined" ) { sortConf = null; }
	if( typeof(limitConf) == "undefined" ) { limitConf = null; }
	if( typeof(skipConf) == "undefined" ) { skipConf = null; }
   var rc = dbcl.find( findConf, selectConf ).sort( sortConf ).limit( limitConf ).skip( skipConf );
   println("--begin to check the data");
   checkRec( rc, expRecs );
   println("--end check the data");
}

/************************************
*@Description: compare actual and expect result,
               they is not the same ,return error ,
               else return ok
*@author:      zhaoyu
*@createDate:  2015.5.20
**************************************/
function checkRec( rc, expRecs )
{				
	//get actual records to array
	var actRecs = [];
   while( rc.next() )
   {
		actRecs.push( rc.current().toObj() );
   }
   //check count
	if( actRecs.length !== expRecs.length )
   {
   	println("\nactual recs in cl= "+JSON.stringify(actRecs)+"\n\nexpect recs= "+JSON.stringify(expRecs));
   	throw buildException("check count", null, "",
									expRecs.length, actRecs.length);
   }
   
   //check every records every fields,expRecs as compare source
   for( var i in expRecs )
   {
   	var actRec = actRecs[i];
   	var expRec = expRecs[i];
   	
   	for ( var f in expRec )
   	{
   		if( JSON.stringify(actRec[f]) !== JSON.stringify(expRec[f]) )
	   	{
	   		println("\nerror occurs in "+(parseInt(i)+1)+"th record, in field '"+f+"'");
	   		println("\nactual record= "+JSON.stringify(actRec)+"\n\nexpect record= "+JSON.stringify(expRec));
	   		println("\nactual recs in cl= "+JSON.stringify(actRecs)+"\n\nexpect recs= "+JSON.stringify(expRecs));   		
	   		throw buildException("checkRec()", "rec ERROR");
	   	}
   	}
   }
   //check every records every fields,actRecs as compare source
   for( var j in actRecs )
   {
   	var actRec = actRecs[j];
   	var expRec = expRecs[j];
   	
   	for ( var f in actRec )
   	{
   	   if(f == "_id")
   	   {
   	      continue;
   	   }
   		if( JSON.stringify(actRec[f]) !== JSON.stringify(expRec[f]) )
	   	{
	   		println("\nerror occurs in "+(parseInt(j)+1)+"th record, in field '"+f+"'");
	   		println("\nactual record= "+JSON.stringify(actRec)+"\n\nexpect record= "+JSON.stringify(expRec));
	   		println("\nactual recs in cl= "+JSON.stringify(actRecs)+"\n\nexpect recs= "+JSON.stringify(expRecs));   		
	   		throw buildException("checkRec()", "rec ERROR");
	   	}
   	}
   }
}


/************************************
*@Description: 调用sdb工具
*@author:      luweikang
*@createDate:  2017.07.05
**************************************/
function command(name)
{
   if ("undefined" === typeof(name))
   {
      throw buildException(command, "name undefined");
   }
   
   this.name = name;
   this.cmd = new Cmd();
}

command.prototype.exec = 
function(newcmdstr)
{
   try
   {
      if ("undefined" !== typeof(newcmdstr))
      {
         var cmdstr = newcmdstr;
      }
      else
      {
         var cmdstr = "undefined" !== typeof(this.options) ?  
                         this.name + " " + this.options: this.name;
      }
      println(cmdstr);
      var result = this.cmd.run(cmdstr);
   }
   catch(e)
   {
      var exceptionMsg = "exec " + cmdstr + e; 
      throw buildException("command.exec", exceptionMsg)
   }
   
   return result;
}

command.prototype.addOption =
function(option)
{
   if ("undefined" === typeof(option))
   {
      throw buildException("command.addOption()", "option is undefined");
   }
   
   if ("undefined" === typeof(this.options))
   {
      this.options = option;
   }
   else
   {
      this.options = this.options + " " + option;
   }
}

/*************************************
*@Description: 检测主备节点数据一致性
*@author:      luweikang
*@createDate:  2017.07.05
**************************************/
function checkData( csName, clName )
{
   var installPath = commGetInstallPath();
   var cmd = new command(installPath + "/bin/sdbinspect");
   var rc  = db.snapshot( SDB_SNAP_COLLECTIONS, {'Name':csName+"."+clName});
   var groupName = rc.next().toObj().Details[0].GroupName;
   cmd.addOption("-g " + groupName);
   cmd.addOption("-d " + this.db.toString());
   cmd.addOption("-c " + csName);
   cmd.addOption("-l " + clName);
   rc.close();
   
   var result = cmd.exec();
   
   if (result.lastIndexOf("inspect done") === 0 &&
       result.lastIndexOf("exit with no records different") !== -1)
   {
      return true;
   }
   else
   {
      println("sdbinspect exec result:" + result );
   }
}

/*************************************
*@Description: 初始化固定集合测试环境
*@author:      luweikang
*@createDate:  2017.07.05
**************************************/
function initCappedCS( csName )
{
   //clean environment before test
   commDropCS( db, csName, true, "drop CS in the beginning" );

   //create cappedCS
   var options = { Capped : true }
   commCreateCS( db, csName, false, "beginning to create cappedCS", options );
}







