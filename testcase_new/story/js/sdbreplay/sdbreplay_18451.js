/************************************************************************
*@Description: seqDB-18451:主子表，多张子表其中包括普通表和分区表，重放复制日志到文件   
*@Author: 2019-7-2  xiaoni zhao init
************************************************************************/
function main()
{  
   if( commIsStandalone( db ) )
   {
      println("\nThe mode is standalone.");
   }
    
   var mainCsName = "mainCsName_18451";
   var mainClName = "mainClName_18451";
   var subCsName1 = "subCsName_18451_1";
   var subClName1 = "subClName_18451_1";
   var subCsName2 = "subCsName_18451_2";
   var subClName2 = "subClName_18451_2";
   var groupNames = getDataGroupNames();
   
   var mainCl = readyCL(mainCsName, mainClName, { IsMainCL: true, ShardingKey: { a: 1 }, ShardingType: "range", Group: groupNames[0] }); 
   var subCl1 = readyCL(subCsName1, subClName1, { ShardingKey: {a: 1}, ShardingType: "hash", Group: groupNames[0] });
   var subCl2 = readyCL(subCsName2, subClName2, { Group: groupNames[0] });   
   
   mainCl.attachCL(subCsName1 +"."+ subClName1,{ LowBound: { a: 0 }, UpBound: { a: 200 } });
   mainCl.attachCL(subCsName2 +"."+ subClName2,{ LowBound: { a: 200 }, UpBound: { a: 400 } });
   
   //get minLSN
   var cursor = db.list(SDB_SNAP_SYSTEM,{GroupName:groupNames[0]});
   var svcName = cursor.current().toObj().Group[0].Service[0].Name;
   cursor = db.snapshot(6, {ServiceName:svcName, RawData:true});
   var minLSN = cursor.current().toObj().CompleteLSN;
   
   var expDataArr = [];
   for(var i=0; i<100; i++)
   {
      mainCl.insert({a:i});
      mainCl.insert({a:200+i});
      mainCl.insert({a:300+i});
      mainCl.update({$set:{a:100+i}},{a:200+i});
      mainCl.remove({a:300+i});
      
      expDataArr.push('"I","'+i+'"');
      expDataArr.push('"I","'+(200+i)+'"');
      expDataArr.push('"I","'+(300+i)+'"');
      expDataArr.push('"B","'+(200+i)+'"');
      expDataArr.push('"A","'+(200+i)+'"');
      expDataArr.push('"D","'+(300+i)+'"');
   }
   
   var rtCmd = getRemoteCmd( groupNames[0] );
   initTmpDir( rtCmd );
    
   try
   {    
      var confName = "sdbreplay_18451.conf";
      getOutputConfFile( groupNames[0], mainCsName, mainClName, confName );
      
      var subClNameArr = ["\""+subCsName1 +"."+ subClName1+"\"", "\""+subCsName2 +"."+ subClName2+"\""];
      var mainClNameArr = [mainCsName +"."+ mainClName];
      var confPath = tmpFileDir + mainCsName +"." + mainClName + ".conf";
      var filter = '\'{CL: ['+ subClNameArr +'], MinLSN: '+ minLSN +' }\''; 
      execSdbReplay( rtCmd, groupNames[0], mainClNameArr, "replica", confPath, undefined, undefined, undefined, filter );
      
      checkCsvFile( rtCmd, mainClName, expDataArr );
      
      cleanCL( mainCsName, mainClName );
      cleanFile( rtCmd );
   }catch(e)
   {
      backupFile( rtCmd, mainClName );
      throw e;
   }
}
main();