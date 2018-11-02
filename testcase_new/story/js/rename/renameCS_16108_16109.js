/* *****************************************************************************
@discretion: rename cl ---//review 1：描述信息不正确，和实际用例不符
             seqDB-16108
             seqDB-16109
@author：2018-10-13 chensiqin  Init
***************************************************************************** */

main(db);

function main(db)
{
   if (commIsStandalone( db ))
   {
      return ;
   }
   if (commGetGroupsNum(db) < 2)
   {
      return ;
   }
   var domainName = "domain16108";
   var csName1 = CHANGEDPREFIX+"_rename16108_1";
   var csName2 = CHANGEDPREFIX+"_rename16108_2";
   var csName3 = CHANGEDPREFIX+"_rename16108_3";
   var clName = CHANGEDPREFIX+"rename16108";
   //创建domain  --------//review 2：方法名已经很明确，不需要再增加和方法名相同的说明信息
   var domain = createDomain( db, domainName );
   
   //创建cs cl
   commDropCS( db, csName1, true, "ignoreNotExist is true" );
   commDropCS( db, csName2, true, "ignoreNotExist is true" );
   commDropCS( db, csName3, true, "ignoreNotExist is true" );
   var varCS = commCreateCS( db, csName1, true, "create CS", {Domain:domainName} );
   var varCL = commCreateCLByOption( db, csName1, clName, {}, true, false, "create cl in the beginning" )
   insertData(varCL, 100);
   
   //修改cs名称，快照检查
   testRenameCS16108( db, csName1, csName2 );
   testRenameCS16109( db, domain, csName2, csName3, clName );
   afterClear( db, domainName, csName3 );
}

function testRenameCS16108( db, csName1, csName2 )
{
   var oldName = csName1;
   var newName = csName2;
   db.renameCS( oldName, newName );
   checkRenameCSResult(oldName, newName, 1);
}
/*
  修改cs名，查看domain.listCollectionSpaces() 3、将cs移除域 4、检查结果 
*/
function testRenameCS16109( db, domain, csName2, csName3, clName )
{
   var oldName = csName2;
   var newName = csName3;
   db.renameCS( oldName, newName );
   var csList = domain.listCollectionSpaces().toArray();
   var cs = eval('('+csList[0]+')') ;
   if (cs["Name"] !== csName3) {
      throw buildException("check domain.listCollectionSpaces() fail", "fail", "check", "success", "fail"); 
   }
   checkDatas( csName3, clName);
}

function createDomain( db, domainName)
{
   var groups = commGetGroups(db);
   var groupName1 = groups[0][0].GroupName;
   var groupName2 = groups[1][0].GroupName;
   try 
   {
      var mydomain = db.createDomain( domainName, [ groupName1, groupName2 ], { AutoSplit: true } );
      return mydomain;
   }
   catch( e )
   {
      throw buildException("createDomain fail", e, "create", "success", e); 
   }
}

function checkDatas( csName3, clName)
{   
   try
   {
      //check the record nums      
      var dbcl = db.getCS( csName3 ).getCL( clName );
      var count = dbcl.count();      
      if( count != 100  )
      {
         throw buildException("check datas", null, "check the new cl record nums",
                           100, count);
      }   
      
   }
   catch(e)
   {
      throw buildException("checkDatas", e)
   }  
}

function afterClear( db, domainName, csName3 )
{
   commDropCS( db, csName3, true, "ignoreNotExist is true" );
   try 
   {
      db.dropDomain( domainName );
   }
   catch( e )
   {
      throw buildException("dropDomain fail", e, "drop", "success", e); 
   }
}

 