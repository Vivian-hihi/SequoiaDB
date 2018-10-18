/* *****************************************************************************
@discretion: rename cl
             seqDB-16116
@author：2018-10-13 chensiqin  Init
***************************************************************************** */
/*
 1、创建主子表在多个cs上，分别考虑两种场景：
       a.多个子表cl分布在不同组上
2、修改cs名，执行数据操作（如：插入）、LOB操作、索引操作等 
3、检查cs、cl快照，数据文件、索引文件、LOB文件、LOB元数据文件
*/
main(db);
function main(db)
{
   if (commGetGroupsNum(db) < 3)
   {
      return ;
   }
   var csName1 = CHANGEDPREFIX+"_maincs16116_1";
   var csName2 = CHANGEDPREFIX+"_maincs16116_2";
   var mainClName = CHANGEDPREFIX+"maincl16116_1";
   
   var csName3 = CHANGEDPREFIX+"_subcs16116_1";
   var csName4 = CHANGEDPREFIX+"_subcs16116_2";
   var clName1 = CHANGEDPREFIX+"subcl16116_1";
   var clName2 = CHANGEDPREFIX+"subcl16116_2";
   var groups = commGetGroups(db);
   var groupName1 = groups[0][0].GroupName;
   var groupName2 = groups[1][0].GroupName;
   var groupName3 = groups[2][0].GroupName;
   try{
      commDropCS( db, csName1, true, "drop CS "+csName1 );
      commDropCS( db, csName2, true, "drop CS "+csName2 );
      commDropCS( db, csName3, true, "drop CS "+csName3 );
      commDropCS( db, csName4, true, "drop CS "+csName4 );
   }catch( e ){}
   var varCS = commCreateCS( db, csName1, true, "create CS" );
   var varCL = varCS.createCL(mainClName, { IsMainCL: true, ShardingKey: { a: 1 }, ShardingType: "range" } );
   //子表1
   var subCS = commCreateCS( db, csName3, true, "create CS1" );
   var subcl1 = subCS.createCL(clName1, { ShardingKey: { a: 1 }, ShardingType: "hash", Partition: 1024, Group: groupName1} );
   //子表2
   var subcl2 = subCS.createCL(clName2, { ShardingKey: { a: 1 }, ShardingType: "hash", Partition: 1024, Group: groupName2} );
   //挂载
   attachCL(varCL, csName3+"."+clName1, { LowBound: { a: 0 }, UpBound: { a: 1000 } }  );
   attachCL(varCL, csName3+"."+clName2, { LowBound: { a: 1000 }, UpBound: { a: 3000 } });

   //修改子表cs name
   try
   {
      db.renameCS( csName3, csName4 );
   }
   catch( e )
   {
      throw buildException("renameCS( csName3, csName4 ) fail", e, "rename", "success", e); 
   }
   var recordNums = 2000;
   insertData( varCL, recordNums );
   varCL.createIndex( "index16116", { no: 1 }, false );
   //check
   checkRenameCSResult(csName3, csName4, 2);
   var cs = db.getCS(csName1);
   var varCL = cs.getCL(mainClName);
   checkDatas( csName1, mainClName, recordNums);
   
   try{
      commDropCS( db, csName1, true, "drop CS "+csName1 );
      commDropCS( db, csName2, true, "drop CS "+csName2 );
      commDropCS( db, csName3, true, "drop CS "+csName3 );
      commDropCS( db, csName4, true, "drop CS "+csName4 );
   }catch( e ){}
   
}

function attachCL( dbcl, subCLName, range )
{
   try
   {
      dbcl.attachCL( subCLName, range ) ;
      println( "--attach cl success" ) ;
   }
   catch(e)
   {
      throw buildException("attachCL()", e, "attach cl", "attach cl success","attach cl fail");
   }
}

function checkDatas( csName, newCLName, expRecordNums)
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
   }
   catch(e)
   {
      throw buildException("checkDatas", e)
   }  
}