/************************************************************************
*@Description: seqDB-10150: 重放时过滤/指定cl  
*@Author: 2019-7-1  xiaoni zhao init
************************************************************************/
function main()
{ 
   if( commIsStandalone( db ) )
   {
      println("\nThe mode is standalone.");
   }
   
   var csName1 = "csName_10150_1";
   var clName1 = "clName_10150_1" + getRandomInt(0, 100);
   var csName2 = "csName_10150_2";
   var clName2 = "clName_10150_2" + getRandomInt(0, 100);
   var groupNames = getDataGroupNames();
   
   var cl1 = readyCL(csName1, clName1, {Group:groupNames[0]}); 
   var cl2 = readyCL(csName2, clName2, {Group:groupNames[0]}); 
  
   var expDataArr = [];
   for(var i=0; i<100; i++)
   {
      cl1.insert({a:i});
      cl1.insert({a:100+i});
      cl1.update({$set:{a:200+i}},{a:i});
      cl1.remove({a:100+i});
      cl2.insert({b:i});
      cl2.insert({b:100+i});
      cl2.update({$set:{b:200+i}},{b:i});
      cl2.remove({b:100+i});
      
      expDataArr.push('"I","'+i+'"');
      expDataArr.push('"I","'+(100+i)+'"');
      expDataArr.push('"B","'+i+'"');
      expDataArr.push('"A","'+(200+i)+'"');
      expDataArr.push('"D","'+(100+i)+'"');
   }
   
   var rtCmd = getRemoteCmd( groupNames[0] );
   initTmpDir( rtCmd );
   
   try
   {   
      var fieldType = "MAPPING_INT";
      readyOutputConfFile( rtCmd, groupNames[0], csName1, clName1, fieldType );
      
      //重放时指定/过滤cl
      var clNameArray = [csName1+"."+clName1];
      var filter = '\'{CL: ["'+ csName1 +'.'+ clName1+'","'+ csName2 +'.'+ clName2 +'"], ExclCL: ["'+ csName2 +"."+ clName2 +'"] }\'';
      execSdbReplay( rtCmd, groupNames[0], clNameArray, undefined, undefined, undefined, undefined, undefined, filter);
      checkCsvFile( rtCmd, clName1, expDataArr );
      
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