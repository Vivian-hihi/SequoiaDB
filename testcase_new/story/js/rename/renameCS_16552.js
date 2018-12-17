/************************************
*@Description: 修改cs名，新名参数校验
*@author:      luweikang
*@createdate:  2018.12.13
*@testlinkCase:seqDB-16552
**************************************/

main();

function main()
{
   var csName = COMMCSNAME+"_16552";
   
   var cs = commCreateCS( db, csName, false, "create cs in begine", "");

   // rename cs new name is begin with $
   checkNewCSName( db, csName, "$csName16552", -6 );
   
   // rename cs new name is contains .
   checkNewCSName( db, csName, "csName.16552", -6 );
   
   // rename cs new name is ""
   checkNewCSName( db, csName, "", -6 );
   
   // rename cs new name is long str
   var longStr = "a";
   for(var i=0; i< 1000; i++)
   {
      longStr += "a";
   }
   checkNewCSName( db, csName, longStr, -6 );
   //TODO:2、还需要补充长度边界值的测试点，如超过长度边界值128字节
   // rename cs new name is 127 str
   var shotStr = "";
   for(var i=0; i< 127; i++)
   {
      shotStr += "a";
   } 
   checkNewCSName( db, csName, shotStr, 0 );
   db.renameCS( shotStr, csName );
   
   // rename cs new name is contains ~!@#$%^()_+
   var nameStr = "~!@#$%^()_+"
   checkNewCSName( db, csName, nameStr, 0 );
   db.renameCS( nameStr, csName );
   
   // rename cs new name is begin with SYS
   checkNewCSName( db, csName, "SYScsName16552", -6 );
   
   commDropCS( db, csName, true, false, "clean cs---" );
   
}

function checkNewCSName( db, oldCSName, newCSName, error )
{
   try
   {
      db.renameCS( oldCSName, newCSName );//TODO:1、此处需要增加rename成功判断，如果rename成功则报错
   }
   catch( e )
   {
      if( e != error )
      {
         throw buildException("rename cs new name is error: " + newCSName, e, "rename", error, e);
      }
   }
}

