/* *****************************************************************************
@discretion: rename cl
             seqDB-16110
@author：2018-10-13 chensiqin  Init
***************************************************************************** */

main(db);

function main(db)
{
   if (commIsStandalone( db ))
   {
      return ;
   }
   if (commGetGroupsNum(db) < 3)
   {
      return ;
   }
   var domainName = "domain16110";
   var csName1 = CHANGEDPREFIX+"_rename16110_1";
   var csName2 = CHANGEDPREFIX+"_rename16110_2";
   var clName = CHANGEDPREFIX+"renamecl16110";
   //创建domain
   var domain = createDomain( db, domainName );
   //创建cs cl
   commDropCS( db, csName1, true, "ignoreNotExist is true" );
   commDropCS( db, csName2, true, "ignoreNotExist is true" );
   var varCS = commCreateCS( db, csName1, true, "create CS");
   var varCL = commCreateCLByOption( db, csName1, clName, {}, true, false, "create cl in the beginning" )
   insertData(varCL, 100);
   testRenameCS16110( db, domainName, domain, csName1, csName2, clName );
   afterClear( db, domainName, csName2 )
}

function testRenameCS16110( db, domainName, domain, csName1, csName2, clName )
{
   var oldName = csName1;
   var newName = csName2;
   db.renameCS( oldName, newName );
   //check
   checkRenameCSResult(oldName, newName, 1);
   var cs = db.getCS(csName2)
   cs.setDomain( { Domain : domainName } )
   var csList = domain.listCollectionSpaces().toArray();
   var cs = eval('('+csList[0]+')') ;
   if (cs["Name"] !== csName2) {
      throw buildException("check domain.listCollectionSpaces() fail", "fail", "check", "success", "fail"); 
   }
   checkDatas( csName2, clName);
}

function checkDatas( csName2, clName)
{   
   try
   {
      //check the record nums      
      var dbcl = db.getCS( csName2 ).getCL( clName );
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

function createDomain( db, domainName)
{
   var groups = commGetGroups(db);
   var groupName1 = groups[0][0].GroupName;
   var groupName2 = groups[1][0].GroupName;
   var groupName3 = groups[2][0].GroupName;
   try 
   {
      var mydomain = db.createDomain( domainName, [ groupName1, groupName2, groupName3 ], { AutoSplit: true } );
      return mydomain;
   }
   catch( e )
   {
      throw buildException("createDomain fail", e, "create", "success", e); 
   }
}

function afterClear( db, domainName, csName2 )
{
   commDropCS( db, csName2, true, "ignoreNotExist is true" );
   try 
   {
      db.dropDomain( domainName );
   }
   catch( e )
   {
      throw buildException("dropDomain fail", e, "drop", "success", e); 
   }
}