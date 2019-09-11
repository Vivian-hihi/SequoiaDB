/************************************
*@Description: seqDB-19042 主子表进行truncate
*@author:      luweikang
*@createDate:  2019.8.12
**************************************/
try
{
   main();
}
catch(e)
{
   if ( e.constructor === Error )
   {
      println(e.stack) ;  
   }
   throw e ;
}

function main()
{
   if(commIsStandalone( db ))
   {
      println("skip standalone mode");
      return;
   }
   var csName = COMMCSNAME;
   var mainCLName = "cl19042_main";
   var subCLName = "cl19042_sub";
   var filePath = WORKDIR + "/lob19042/";
   var fileName = "file19042";
   var fileFullPath = filePath + fileName;
   var fileMD5 = makeTmpFile( filePath, fileName );
   
   commDropCL(db, csName, mainCLName);
   commDropCL(db, csName, subCLName);
   
   //测试创建LobShardingKeyFormat为YYYYMMDD主表
   var mainCL = createMainCLAndAttachCL( db, csName, mainCLName, subCLName, "YYYYMMDD", 4 );
   var nameArr = mainCL.toString().split(".");
   var mainCLFullName = nameArr[1] + "." + nameArr[2] ;
   var lobOids = insertLob( mainCL, fileFullPath, "YYYYMMDD", 5, 10, 4 );
   checkLobMD5( mainCL, lobOids, fileMD5 );
   
   mainCL.truncate();
   
   for(i in lobOids)
   {
      try
      {
         mainCL.getLob(lobOids[i], WORKDIR + "/checkLob19042_" + i );
         throw 0;
      }
      catch( e )
      {
         if( e !== -4 )
         {
             throw buildException( "check delete lob", e, "gets the deleted lob: " + lobOids[i], -4, e ); 
         }
      }
   }

   //TODO:只是清理环境删除主表，不需要用这个方法
   cleanMainCL(db, csName, mainCLName);
   deleteTmpFile( filePath );
}

