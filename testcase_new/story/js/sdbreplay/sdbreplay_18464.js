/************************************************************************
*@Description: seqDB-18464: tables配置多个不同集合，集合间字段包含相同和不同字段 
*@Author: 2019-7-4  xiaoni zhao init
************************************************************************/
function main()
{ 
   if( commIsStandalone( db ) )
   {
      println("\nThe mode is standalone.");
   }
    
   var csName1 = "csName_18464_1";
   var clName1 = "clName_18464_1";
   var csName2 = "csName_18464_2";
   var clName2 = "clName_18464_2";
   var groupNames = getDataGroupNames();
   
   var cl1 = readyCL(csName1, clName1, {Group:groupNames[0]}); 
   var cl2 = readyCL(csName2, clName2, {Group:groupNames[0]}); 
  
   //get minLSN
   var cursor = db.list(SDB_SNAP_SYSTEM,{GroupName:groupNames[0]});
   var svcName = cursor.current().toObj().Group[0].Service[0].Name;
   cursor = db.snapshot(6, {ServiceName:svcName, RawData:true});
   var minLSN = cursor.current().toObj().CompleteLSN;
  
   var expDataArr = [];
   for(var i=0; i<100; i++)
   {
      cl1.insert({a:i, b:i});
      cl1.insert({a:100+i, b:100+i});
      cl1.update({$set:{b:200+i}},{b:i});
      cl1.remove({a:100+i});
      cl2.insert({a:i, c:i});
      cl2.insert({a:100+i, c:100+i});
      cl2.update({$set:{c:200+i}},{c:i});
      cl2.remove({a:100+i});
      
      expDataArr.push('"I","'+i+'","'+i+'"');
      expDataArr.push('"I","'+(100+i)+'","'+(100+i)+'"');
      expDataArr.push('"B","'+i+'","'+i+'"');
      expDataArr.push('"A","'+i+'","'+(200+i)+'"');
      expDataArr.push('"D","'+(100+i)+'","'+(100+i)+'"');
   }
   
   var rtCmd = getRemoteCmd( groupNames[0] );
   initTmpDir( rtCmd );
     
   try
   {     
      var confName = "sdbreplay_18464.conf";
      getOutputConfFile( groupNames[0], csName1, clName1, confName );
      
      var clName1Arr = [csName1 +"."+ clName1];
      var clNameArr = ["\""+ csName1 +"."+ clName1 +"\"", "\""+ csName2 +"."+ clName2 +"\""];
      var filter = filter = '\'{CL: ['+ clNameArr +'], MinLSN:'+ minLSN +' }\'';
      execSdbReplay( rtCmd, groupNames[0], clName1Arr, undefined, undefined, undefined, undefined, undefined, filter);
      
      checkCsvFile( rtCmd, clName1, expDataArr );
      checkCsvFile( rtCmd, clName2, expDataArr );
      
      cleanCL( csName1, clName1 );
      cleanCL( csName2, clName2 );
      cleanFile( rtCmd );
   }catch(e)
   {
      backupFile( rtCmd, clName1 );
      throw e;
   }
}
main();