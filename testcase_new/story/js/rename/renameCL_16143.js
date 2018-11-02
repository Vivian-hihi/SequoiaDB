/* *****************************************************************************
@discretion: rename cl
             seqDB-16143
@author：2018-10-15 chensiqin  Init
***************************************************************************** */
main(db);
function main(db)
{
   if (commGetGroupsNum(db) < 2)
   {
      return ;
   }
   var csName = CHANGEDPREFIX+"_cs16143";
   try{
      commDropCS( db, csName, true, "drop CS "+csName );
   }catch( e ){}//review 1：多余的try-catch要去掉
   var cs = commCreateCS( db, csName, true, "create CS1" );
   
   var clName = CHANGEDPREFIX+"_cl16143";
   var newClName = CHANGEDPREFIX+"_newcl16143";
   var groups = commGetGroups(db);
   var groupName1 = groups[0][0].GroupName;
   var groupName2 = groups[1][0].GroupName;
   var varCL = commCreateCLByOption( db, csName, clName , { ShardingKey: { a: 1 }, ShardingType: "hash", Partition: 1024, Group: groupName1}, true, false, "create cl in the beginning" );
   var recordNums = 2000;
   insertData( varCL, recordNums );
   varCL.split(groupName1, groupName2, 50);
   try
   {
      cs.renameCL( clName, newClName );
   }
   catch( e )
   {
      throw buildException("renameCS( clName, newClName ) fail", e, "rename", "success", e); 
   }
   //新cl上执行插入
   varCL = cs.getCL(newClName);
   insertData( varCL, recordNums );
   checkDatas( csName, newClName, 2*recordNums, {no:{$lt:2000}});
   
   //更新
   varCL.update({$set:{no:2}});
   checkDatas( csName, newClName, 2*recordNums, {no:{$et:2}} );
   
   //删除
   varCL.remove({no:{$et:2}});
   checkDatas( csName, newClName, 0, {no:{$et:2}} );
   
   try{
      commDropCS( db, csName, true, "drop CS "+csName );
   }catch( e ){}//review 2：多余的try-catch建议去掉
}

function checkDatas( csName, newClName, expRecordNums, cond)
{   
   try
   {
      //check the record nums      
      var dbcl = db.getCS( csName ).getCL( newClName );
      var count = dbcl.count(cond);
      if( count != expRecordNums )
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