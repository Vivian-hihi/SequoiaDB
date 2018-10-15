/* *****************************************************************************
@discretion: rename cl
             seqDB-16113
@author：2018-10-13 chensiqin  Init
***************************************************************************** */
/*
1、创建cs、cl 2、修改cs名，检查结果 3、创建删除cl检查结果 1、修改cs名成功 2、创建删除cl成功
*/
main(db);

function main(db)
{
   var csName1 = CHANGEDPREFIX+"_rename16111_1";
   var csName2 = CHANGEDPREFIX+"_rename16111_2";
   var clName1 = CHANGEDPREFIX+"renamecl16111_1";
   var clName2 = CHANGEDPREFIX+"renamecl16111_2";
   //创建cs cl
   commDropCS( db, csName1, true, "ignoreNotExist is true" );
   commDropCS( db, csName2, true, "ignoreNotExist is true" );
   var varCS1 = commCreateCS( db, csName1, true, "create CS");
   var varCL = commCreateCLByOption( db, csName1, clName1, {}, true, false, "create cl in the beginning" )
   try
   {
      db.renameCS( csName1, csName2 );
   }
   catch( e )
   {
      throw buildException("renameCS( csName1, csName1 ) fail", e, "rename", "success", e); 
   }
   var oldName = csName1;
   var newName = csName2;
   checkRenameCSResult(oldName, newName);
   var cs = db.getCS(csName2);
   commCreateCLByOption( db, csName1, clName2, {}, true, false, "create cl in the beginning" )
   commDropCS( db, csName1, true, "ignoreNotExist is true" );
   commDropCS( db, csName2, true, "ignoreNotExist is true" );
}