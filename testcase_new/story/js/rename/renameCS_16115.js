/* *****************************************************************************
@discretion: rename cl
             seqDB-16115
@author：2018-10-13 chensiqin  Init
***************************************************************************** */
/*
1、创建cs、cl，插入数据并且分到多个数据组
2、修改cs名，执行数据操作、LOB操作，索引操作等 
3、检查cs、cl快照，数据文件、索引文件、LOB文件、LOB元数据文件
*/
main(db);
function main(db)
{
   var csName1 = CHANGEDPREFIX+"_rename16115_1";
   var csName2 = CHANGEDPREFIX+"_rename16115_2";
   var clName1 = CHANGEDPREFIX+"renamecl16115_1";
   var fileName = CHANGEDPREFIX + "_lobtest16115.file";
   var groups = commGetGroups(db);
   var groupName1 = groups[0][0].GroupName;
   var groupName2 = groups[1][0].GroupName;
   //创建cs cl
   commDropCS( db, csName1, true, "ignoreNotExist is true" );
   commDropCS( db, csName2, true, "ignoreNotExist is true" );
   var varCS1 = commCreateCS( db, csName1, true, "create CS");
   var varCL = commCreateCLByOption( db, csName1, clName1, {ShardingKey:{a:1},ShardingType:"hash",Group:groupName1}, true, false, "create cl in the beginning" );
   var recordNums = 2000;
   insertData( varCL, recordNums );  
   varCL.split(groupName1, groupName2, 50);
   try
   {
      db.renameCS( csName1, csName2 );
   }
   catch( e )
   {
      throw buildException("renameCS( csName1, csName1 ) fail", e, "rename", "success", e); 
   }
   checkRenameCSResult(csName1, csName2, 1);
   var cs = db.getCS(csName2);
   var varCL = cs.getCL(clName1);
   var srcMd5 = createFile( fileName);
   var lobIdArr = putLobs( varCL, fileName );
   checkDatas( csName2, clName1, recordNums, srcMd5, lobIdArr );
   commDropCS( db, csName1, true, "ignoreNotExist is true" );
   commDropCS( db, csName2, true, "ignoreNotExist is true" );
   var cmd = new Cmd();
   cmd.run( "rm -rf *" + fileName );
}

function checkDatas( csName, newCLName, expRecordNums, srcMd5, expLobArr )
{   
   try
   {
      //check the record nums      
      var dbcl = db.getCS( csName ).getCL( newCLName );
      var count = dbcl.count();      
      if( count != expRecordNums  )
      {
         throw buildException("check datas", null, "check the new cl record nums",
                           expRecordNums, count);
      }   
      
      //check the lob
      checkLob( dbcl, expLobArr, srcMd5 );
   }
   catch(e)
   {
      throw buildException("checkDatas", e)
   }  
}