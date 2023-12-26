/************************************
*@Description: seqDB-33912 设置字符集为GB18030，执行lob操作
*@author:      chenzejia
*@createDate:  2023.12.16
**************************************/

main( test );
function test ()
{
   db.setCharsets( "GB18030" );
   var csName = "测试空间_33912";
   var clName = "集合_33912";
   var basePath = "testcase_new/story/js/charset_gb18030/libs/";
   var lob1Path = basePath + "grade1.js";
   var lob2Path = basePath + "grade2.js";
   var lob3Path = basePath + "grade3.js";
   var getLobPath1 = basePath + "lob1.js";
   var getLobPath2 = basePath + "lob2.js";
   var getLobPath3 = basePath + "lob3.js";

   commDropCL( db, csName, clName );
   var cl = commCreateCL( db, csName, clName );

   // putlob
   var lob1 = cl.putLob( lob1Path );
   var lob2 = cl.putLob( lob2Path );
   var lob3 = cl.putLob( lob3Path );

   // listLobs
   var cursor = cl.listLobs();
   var oids = [];
   while( cursor.next() )
   {
      var oid = cursor.current().toObj();
      oids.push( oid.Oid.$oid );
   }
   cursor.close();
   if( oids.indexOf( lob1 ) == -1 || oids.indexOf( lob2 ) == -1 || oids.indexOf( lob3 ) == -1 )
   {
      throw new Error( "check listLobs failed,oids: " + oids );
   }


   // getlob
   cl.getLob( lob1, getLobPath1 );
   cl.getLob( lob2, getLobPath2 );
   cl.getLob( lob3, getLobPath3 );

   // deleteLob
   cl.deleteLob( lob1 );
   cl.deleteLob( lob2 );
   cl.deleteLob( lob3 );

   // check file and delete
   var cmd = new Cmd();
   cmd.run( "diff " + lob1Path + " " + getLobPath1 );
   assert.equal( cmd.getLastOut(), "", "check lob1 failed" );
   cmd.run( "diff " + lob2Path + " " + getLobPath2 );
   assert.equal( cmd.getLastOut(), "", "check lob2 failed" );
   cmd.run( "diff " + lob3Path + " " + getLobPath3 );
   assert.equal( cmd.getLastOut(), "", "check lob3 failed" );

   cmd.run( "rm -f " + getLobPath1 + " " + getLobPath2 + " " + getLobPath3 );
   commDropCS( db, csName );
}