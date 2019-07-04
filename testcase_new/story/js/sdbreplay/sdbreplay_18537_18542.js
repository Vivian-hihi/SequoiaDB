/************************************************************************
*@Description: seqDB-18537: tables.fields.defaultValue配置默认值，fieldType配置为MAPPING_STRING 
               seqDB-18542: tables.fields.defaultValue配置默认值，fieldType配置不为MAPPING_* 
*@Author: 2019-7-4  xiaoni zhao init
************************************************************************/
function main()
{
   try
   {
      if( commIsStandalone( db ) )
      {
         println("\nThe mode is standalone.");
      }
       
      var csName = "csName_18537_18542";
      var clName = "clName_18537_18542";
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
         cl.insert({a:i});
         cl.insert({a:100+i});
         cl.update({$set:{a:200+i}},{a:i});
         cl.remove({a:100+i});
         
         expDataArr.push('"I","'+i+'","","0"');
         expDataArr.push('"I","'+(100+i)+'","","0"');
         expDataArr.push('"B","'+i+'","","0"');
         expDataArr.push('"A","'+(200+i)+'","","0"');
         expDataArr.push('"D","'+(100+i)+'","","0"');
      }
      
      var rtCmd = getRemoteCmd( groupNames[0] );
      initTmpDir( rtCmd );
      
      var confName = "sdbreplay_18537_18542.conf";
      getOutputConfFile( groupNames[0], csName, clName, confName);   
      
      var clNameArr = [csName +"."+ clName];
      var filter = '\'{CL: ["'+ clNameArr +'"], MinLSN:'+ minLSN +' }\'';
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