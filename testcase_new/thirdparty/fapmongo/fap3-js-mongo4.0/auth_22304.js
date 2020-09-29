/******************************************** 
@description : seqDB-22304:createUser/auth配置鉴权算法（mongo4.0及以上版本）
@testcase    : seqDB-22304
@author      : XiaoNi Huang 2020-09-01
*********************************************/
main();

function main ()
{
   var rc = db.getUsers();
   assert.eq( rc, [] );

   var user = "auth_22304";
   var pwd = "sequoiadb";
   var clName = "cl22304";
   var cl = db.getCollection( clName );
   cl.drop();

   try
   {
      // createUser, mechanisms: ["SCRAM-SHA-1"]
      var rc = db.runCommand( { "createUser": user, "pwd": pwd, "roles": [], mechanisms: ["SCRAM-SHA-1"] } );
      assert.eq( rc, { "ok": 1 } );
      var rc = db.getUsers();
      assert.eq( rc, { "user": [user] } );
      // auth and exec business operations
      var rc = db.auth( user, pwd );
      assert.eq( rc, 1 );
      cl.insert( { "a": i } );
      assert.eq( cl.count( { "a": i } ), 1 );
      // drop the user
      var rc = db.dropUser( user );
      assert.eq( rc, true );


      // mechanisms error
      var rc = db.runCommand( { "createUser": user, "pwd": pwd, "roles": [], mechanisms: ["SCRAM-SHA-123"] } );
      assert.eq( rc, 0 );
      var rc = db.getLastError();
      assert.eq( rc, "Authority is forbidden" );
   }
   finally
   {
      db.auth( user, pwd );
      db.dropUser( user );
   }

   cl.drop();
}