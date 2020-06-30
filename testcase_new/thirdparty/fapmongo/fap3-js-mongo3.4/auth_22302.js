/******************************************** 
@description : createUser / getUser / getUsers / auth / dropUser
@testcase    : seqDB-22302
@author      : XiaoNi Huang 2020-02-24
*********************************************/
main();

function main ()
{
   var rc = db.getUsers();
   assert.eq( rc, [] );

   var users = ["auth_22302_1", "auth_22302_2", "auth_22302_3", "auth_22302_4"];
   var pwds = ["sequoiadb", "", "   ", "_test_"];
   var clName = "cl22302";
   var cl = db.getCollection( clName );
   cl.drop();

   try
   {
      // not any user, auth
      var rc = db.getUsers();
      assert.eq( rc, [] );
      db.auth( users[0], pwds[0] );
      assert.eq( rc, 0 );
      var rc = db.getLastError();
      //assert.eq( rc, "Authentication has been disabled" );  ---rc:null, ---SEQUOIADBMAINSTREAM-5952

      // createUser
      for( var i = 0; i < users.length; i++ )
      {
         var rc = db.runCommand( { "createUser": users[i], "pwd": pwds[i], "roles": [] } );
         assert.eq( rc, { "ok": 1 } );
      }

      // not auth, business operations
      //cl.insert( {"a":0} );
      //assert.eq( rc, { "ok" : 1 } );

      // auth, business operations
      for( var i = 0; i < users.length; i++ )
      {
         var rc = db.auth( users[i], pwds[i] );
         assert.eq( rc, 1 );
         cl.insert( { "a": i } );
         assert.eq( cl.count( { "a": i } ), 1 );
      }
      assert.eq( cl.count(), users.length );

      // getUser / getUsers
      /*  SEQUOIADBMAINSTREAM-5974
      var rc = db.getUser( users[1] );
      assert.eq( rc, { "user" : users[1] } );
      */
      var rc = db.getUsers();
      var expRc = [];
      for( var i = 0; i < users.length; i++ )
      {
         expRc.push( { "user": users[i] } );
      }
      assert.eq( rc, expRc );

      // user exist, create the user
      var rc = db.runCommand( { "createUser": users[0], "pwd": pwds[0], "roles": [] } );
      assert.eq( rc, { "ok": 0, "code": -295, "errmsg": "The specified user already exist" } );

      // drop the user
      db.auth( users[0], pwds[0] );
      var rc = db.dropUser( users[0] );
      assert.eq( rc, true );
      // repeat drop the user
      try
      {
         db.dropUser( users[0] );
         throw new Error( "expect fail, but actual success" );
      }
      catch( e )
      {
         assert.eq( e, 'Error: user specified is not exist or password is invalid' );
      }

      // createUser again after drop the user
      var rc = db.runCommand( { "createUser": users[0], "pwd": pwds[0], "roles": [] } );
      assert.eq( rc, { "ok": 1 } );
      var rc = db.getUser( users[0] );
      //assert.eq( rc, { "user" : users[0] } );

      // password error
      var rc = db.auth( users[0], "errorPwd" );
      assert.eq( rc, 0 );
      var rc = db.getLastError();
      assert.eq( rc, "Authority is forbidden" );
   }
   finally
   {
      for( var i = 0; i < users.length; i++ )
      {
         var rc = db.auth( users[i], pwds[i] );
         if( rc === 1 )
         {
            db.dropUser( users[i] );
         }
      }
      var rc = db.getUsers();
      assert.eq( rc, [] );
   }

   cl.drop();
}