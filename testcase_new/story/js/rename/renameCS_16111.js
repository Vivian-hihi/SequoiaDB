/* *****************************************************************************
@discretion: rename cl
             seqDB-16111
@author：2018-10-13 chensiqin  Init
***************************************************************************** */
main(db);

function main(db)
{
   /*
    1、创建cs 2、修改cs名为原名 3、修改cs名为已存在的cs 4、检查结果 
   */
   var csName1 = CHANGEDPREFIX+"_rename16111_1";
   var csName2 = CHANGEDPREFIX+"_rename16111_2";
   //创建cs cl
   commDropCS( db, csName1, true, "ignoreNotExist is true" );
   commDropCS( db, csName2, true, "ignoreNotExist is true" );
   var varCS1 = commCreateCS( db, csName1, true, "create CS");
   var varCS2 = commCreateCS( db, csName2, true, "create CS");
   try
   {
      db.renameCS( csName1, csName1 );
   }
   catch( e )
   {
      if ( e !== -33) {
         throw buildException("renameCS( csName1, csName1 ) fail", e, "rename", "success", e); 
      }
   }
   try
   {
      db.renameCS( csName1, csName2 );
   }
   catch( e )
   {
      if ( e !== -33) {
         throw buildException("renameCS( csName1, csName1 ) fail", e, "rename", "success", e); 
      }
   }
   commDropCS( db, csName1, true, "ignoreNotExist is true" );
   commDropCS( db, csName2, true, "ignoreNotExist is true" );
}