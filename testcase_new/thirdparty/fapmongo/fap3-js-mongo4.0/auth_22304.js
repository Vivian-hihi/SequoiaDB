/******************************************** 
@description : seqDB-22304:createUser/auth配置鉴权算法（mongo4.0及以上版本）
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
      assert.eq( rc, [{ "user": user }] );
      // auth and exec business operations
      var rc = db.auth( { "user": user, "pwd": pwd, mechanisms: "SCRAM-SHA-1" } );
      assert.eq( rc, 1 );
      cl.insert( { "a": 0 } );
      assert.eq( cl.count( { "a": 0 } ), 1 );
      // auth, the mechanisms is different from the mechanisms when it was created, and exec business operations
      var rc = db.auth( { "user": user, "pwd": pwd, mechanisms: "SCRAM-SHA-256" } );
      assert.eq( rc, 1 );
      assert.eq( cl.count( { "a": 0 } ), 1 );
      // drop the user
      var rc = db.dropUser( user );
      assert.eq( rc, true );

      // createUser, mechanisms: ["SCRAM-SHA-256"]
      var rc = db.runCommand( { "createUser": user, "pwd": pwd, "roles": [], mechanisms: ["SCRAM-SHA-256"] } );
      assert.eq( rc, { "ok": 1 } );
      var rc = db.getUsers();
      assert.eq( rc, [{ "user": user }] );
      // auth and exec business operations
      var rc = db.auth( { "user": user, "pwd": pwd, mechanisms: "SCRAM-SHA-256" } );
      assert.eq( rc, 1 );
      assert.eq( cl.count( { "a": 0 } ), 1 );
      // drop the user
      var rc = db.dropUser( user );
      assert.eq( rc, true );

      // createUser, mechanisms: ["SCRAM-SHA-1","SCRAM-SHA-256"]
      var rc = db.runCommand( { "createUser": user, "pwd": pwd, "roles": [], mechanisms: ["SCRAM-SHA-1", "SCRAM-SHA-256"] } );
      assert.eq( rc, { "ok": 1 } );
      var rc = db.getUsers();
      assert.eq( rc, [{ "user": user }] );
      // auth and exec business operations
      var rc = db.auth( { "user": user, "pwd": pwd } );
      assert.eq( rc, 1 );
      assert.eq( cl.count( { "a": 0 } ), 1 );
      // auth, mechanisms: "SCRAM-SHA-1", and exec business operations
      var rc = db.auth( { "user": user, "pwd": pwd, mechanisms: "SCRAM-SHA-1" } );
      assert.eq( rc, 1 );
      assert.eq( cl.count( { "a": 0 } ), 1 );
      // auth, mechanisms: "SCRAM-SHA-256", and exec business operations
      var rc = db.auth( { "user": user, "pwd": pwd, mechanisms: "SCRAM-SHA-256" } );
      assert.eq( rc, 1 );
      assert.eq( cl.count( { "a": 0 } ), 1 );

      // auth, mechanisms is error
      var rc = db.auth( { "user": user, "pwd": pwd, mechanisms: "SCRAM-SHA-111" } );
      assert.eq( rc, 1 );
      assert.eq( cl.count( { "a": 0 } ), 1 );

      // auth, mechanisms is array
      var rc = db.auth( { "user": user, "pwd": pwd, mechanisms: ["SCRAM-SHA-1", "SCRAM-SHA-256"] } );
      assert.eq( rc, 1 );
      assert.eq( cl.count( { "a": 0 } ), 1 );

      // drop the user
      var rc = db.dropUser( user );
      assert.eq( rc, true );

      // createUser, mechanisms error
      var rc = db.runCommand( { "createUser": user, "pwd": pwd, "roles": [], mechanisms: ["SCRAM-SHA-123"] } );
      assert.eq( rc, { "ok": 1 } );
      var rc = db.getLastError();
      assert.eq( rc, null );
   }
   finally
   {
      db.auth( user, pwd );
      db.dropUser( user );
   }

   cl.drop();
}