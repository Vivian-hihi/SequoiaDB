/* *****************************************************************************
@discretion: rename cl
             seqDB-16074
@author：2018-10-15 chensiqin  Init
***************************************************************************** */
main(db);
function main(db)
{
   /*
     1、cs下创建多个cl，覆盖最大cl数（如4096个cl） 
     2、修改cl名，分别覆盖如下场景： 
                  a、修改部分cl，如4个cl只修改2个cl名 
                  b、修改所有cl名（覆盖最大数量4096个cl） 
     3、检查修改结果 
     4、检查cl属性对应cl名信息 
   */
   var csName = CHANGEDPREFIX+"_cs16074";
   var oldClPerfix = CHANGEDPREFIX+"_cl16074_";
   var newClPerfix = CHANGEDPREFIX+"_newcl16074_";
   commDropCS( db, csName, true, "drop CS "+csName );
   var cs = commCreateCS( db, csName, true, "create CS1" );
   for( var i = 1; i <= 4096; i++ )
   {
      commCreateCLByOption( db, csName, oldClPerfix+i, {}, true, false, "create cl in the beginning" );
   }
   //修改部分cl
   for( var i = 1; i <= 100; i++ )
   {
      cs.renameCL(oldClPerfix+i, newClPerfix+i);
   }
   for( var i = 1; i <= 100; i++ )
   {
      checkRenameCLResult( csName, oldClPerfix+i, newClPerfix+i)
   }
   //修改全部cl

   for( var i = 101; i <= 4096; i++ )
   { 
      cs.renameCL(oldClPerfix+i, newClPerfix+i);
   }

   checkRenameAllCL( csName , newClPerfix);
   
   commDropCS( db, csName, true, "drop CS "+csName );
}

function checkRenameAllCL( csName, newClPerfix )
{
   var cur = db.snapshot(SDB_SNAP_COLLECTIONSPACES,{Name:csName},{Collection:1});
   //var clInfos = cur["Collection"].toArray();
   while( cur.next() )
   {
      var ret = cur.current().toObj();
      var num = 0;
      for ( var i = 1; i <= 4096; i++)
      {
         var str = ret.Collection[i-1].Name;
         if(str.indexOf(newClPerfix) != -1)
         {
            num++;
         }
      }
      if ( num != 4096 )
      {
         throw buildException("check datas", null, "check the newcl nums",
                        4096, num);
      }
   }
   cur.close();

}