/************************************
*@Description: seqDB-33903 设置字符集为GB18030，执行库表操作
*@author:      chenzejia
*@createDate:  2023.12.16
**************************************/
import( "../lib/rename_commlib.js" );

main( test );
function test ()
{
   var csName = "数据库_33903";
   var clName = "集合_33903";
   var newCSName = "新数据库_33903";
   var newCLName = "新集合_33903";
   db.setCharsets( "GB18030" );

   // create collectionSpace
   commCreateCS( db, csName, false );
   // create collection
   commCreateCL( db, csName, clName, {}, false, false );
   // rename collection
   db.getCS( csName ).renameCL( clName, newCLName );
   checkRenameCLResult( csName, clName, newCLName )
   // rename collectionSpace
   db.renameCS( csName, newCSName );
   checkRenameCSResult( csName, newCSName );

   // dropCL
   commDropCL( db, newCSName, newCLName, false, false );
   assert.tryThrow( SDB_DMS_NOTEXIST, function()
   {
      db.getCS( newCSName ).getCL( newCLName );
   } );
   // dropCS
   commDropCS( db, newCSName, false );
   assert.tryThrow( SDB_DMS_CS_NOTEXIST, function()
   {
      db.getCS( newCSName );
   } );
}