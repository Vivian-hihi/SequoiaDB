/* *****************************************************************************
@discretion: rename cl
             seqDB-16072
@author：2018-10-15 chensiqin  Init
***************************************************************************** */
main(db);
function main(db)//review 1：main中代码太紧凑，请按对应测试步骤分行
{
   if( true == commIsStandalone( db ) )
   {
      println( "run mode is standalone" );
      return;
   }  
   var csName = CHANGEDPREFIX+"_cs16072";
   try{
      commDropCS( db, csName, true, "drop CS "+csName );
   }catch( e ){}//review 2：多余try-catch建议去掉，公共方法中已经try-catch
   try{
      db.removeProcedure("test16072");
   }catch( e ){}//review 3：如果try-catch，请参考规范写抛出异常
   var cs = commCreateCS( db, csName, true, "create CS1" );
   var clName = CHANGEDPREFIX+"_cl16072";
   var newClName = CHANGEDPREFIX+"_newcl16072";
   var pcdName = "procedure16072"
   var varCL = commCreateCLByOption( db, csName, clName , {}, true, false, "create cl in the beginning" );
   var recordNums = 100;
   insertData(varCL, recordNums);
   db.createProcedure(function test16072(csName, clName){ return db.getCS(csName).getCL(clName).count({a:{$lt:95}});});
   var ret = db.eval( 'test16072("'+csName+'", "'+clName+'")' );//review 4：这里是校验存储过程的结果，建议加上描述，另外这行应该和下面一行代码放一起吧
   
   if (ret != 95)
   {
      throw buildException("check datas", null, "check the cl record nums",
                           95, ret);
   }
   
   try
   {
      cs.renameCL( clName, newClName );
      ret = db.eval( 'test16072("'+csName+'", "'+clName+'")' );
   }
   catch( e )
   { 
      if ( e != -23)
      {
         throw buildException("renameCS( clName, newClName ) fail", e, "rename", "success", e); 
      }
   }

   commCreateCLByOption( db, csName, clName , {}, true, false, "create cl in the beginning" );
   insertData(varCL, 50);
   ret = db.eval( 'test16072("'+csName+'", "'+clName+'")' );
   
   if (ret != 50)
   {
      throw buildException("check datas", null, "check the cl record nums",
                           50, ret);
   }
   
   db.removeProcedure("test16072");
   commDropCS( db, csName, true, "ignoreNotExist is true" );
}

function checkDatas( csName, newCLName, expRecordNums)
{   
   try
   {
      //check the record nums      
      var dbcl = db.getCS( csName ).getCL( newCLName );
      var count = dbcl.count();
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
