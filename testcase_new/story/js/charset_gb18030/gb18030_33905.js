/************************************
*@Description: seqDB-33905 设置字符集为GB18030，执行dcl操作
*@author:      chenzejia
*@createDate:  2023.12.16
**************************************/
testConf.skipStandAlone = true;
main( test );
function test ()
{
   var roleName = "角色_33905";
   var userName = "user_33905";
   var passwd = "123456"
   var csName = "cs_33905";

   db.setCharsets( "GB18030" );
   // create role
   db.createRole( { Role: roleName } );
   // grant privilege
   db.grantPrivilegesToRole( roleName, [{ Resource: { cs: csName, cl: "" }, Actions: ["createCL", "dropCL"] }] )
   // grant role
   db.grantRolesToRole( roleName, ["_" + csName + ".admin"] );
   var roleInfo = db.getRole( roleName ).toObj();
   assert.equal( roleInfo.Role, roleName, "check role failed,roleInfo: " + JSON.stringify( roleInfo ) );
   assert.equal( roleInfo.Roles, ["_" + csName + ".admin"], "check role failed,roleInfo: " + JSON.stringify( roleInfo ) );

   // create user
   db.createUsr( userName, passwd, { Role: "admin" } );
   var authDB = new Sdb( COORDHOSTNAME, COORDSVCNAME, userName, passwd );
   authDB.setCharsets( "GB18030" );
   // grant role
   authDB.grantRolesToUser( userName, [roleName] );
   // check user info
   var userInfo = authDB.getUser( userName ).toObj();
   assert.equal( userInfo.User, userName, "check user failed, userinfo: " + JSON.stringify( userInfo ) );
   assert.equal( userInfo.Roles, ["_root", roleName], "check user failed, userinfo: " + JSON.stringify( userInfo ) );

   // drop role
   authDB.dropRole( roleName );
   assert.tryThrow( SDB_AUTH_ROLE_NOT_EXIST, function()
   {
      authDB.getRole( roleName );
   } );
   // drop user
   authDB.dropUsr( userName, passwd );
   assert.tryThrow( SDB_AUTH_USER_NOT_EXIST, function()
   {
      authDB.getUser( userName );
   } );
   authDB.close();
}