/************************************************************************
*@Description: seqDB-18463: tables配置一个集合包含多个相同和不同字段 
*@Author: 2019-7-3  xiaoni zhao init
************************************************************************/
function main()
{
   try
   {
      if( commIsStandalone( db ) )
      {
         println("\nThe mode is standalone.");
      }
       
      var csName = "csName_18463";
      var clName = "clName_18463";
      var groupNames = getDataGroupNames();
      
      var cl = readyCL(csName, clName, {Group:groupNames[0]}); 
      
      //get minLSN
      var cursor = db.list(SDB_SNAP_SYSTEM,{GroupName:groupNames[0]});
      var svcName = cursor.current().toObj().Group[0].Service[0].Name;
      cursor = db.snapshot(6, {ServiceName:svcName, RawData:true});
      var minLSN = cursor.current().toObj().CompleteLSN;
      
      var expDataArr = [];
      for(var i=0; i<100; i++)
      {
         cl.insert({a:i, b:i});
         cl.insert({a:100+i, b:100+i});
         cl.update({$set:{b:200+i}},{b:i});
         cl.remove({a:100+i});
         
         expDataArr.push('"I","'+i+'","'+i+'","'+i+'"');
         expDataArr.push('"I","'+(100+i)+'","'+(100+i)+'","'+(100+i)+'"');
         expDataArr.push('"B","'+i+'","'+i+'","'+i+'"');
         expDataArr.push('"A","'+i+'","'+i+'","'+(200+i)+'"');
         expDataArr.push('"D","'+(100+i)+'","'+(100+i)+'","'+(100+i)+'"');
      }
      
      var rtCmd = getRemoteCmd( groupNames[0] );
      initTmpDir( rtCmd );
      
      var confName = "sdbreplay_18463.conf";
      getOutputConfFile( groupNames[0], csName, clName, confName );
      
      var clNameArr = [csName +"."+ clName];
      var filter = '\'{CL: ["'+ clNameArr +'"], MinLSN:'+ minLSN +'}\'';
      execSdbReplay( rtCmd, groupNames[0], clNameArr, undefined, undefined, undefined, undefined, undefined, filter);
      
      checkCsvFile( rtCmd, clName, expDataArr);
      
      cleanCL( csName, clName );
      cleanFile( rtCmd );
   }catch(e)
   {
      backupFile( rtCmd, clName );
      throw e;
   }
}
main();