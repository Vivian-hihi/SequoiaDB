/******************************************************************************
@Description :   seqDB-17069:校验事务增删改操作的提交与回滚
@Modify list :   2019-1-9    xiaoni Zhao  Init
******************************************************************************/
function main()
{
   var clName = "tran_basic_1";
   commDropCL(db, COMMCSNAME, clName, true, true);
   var dbcl = commCreateCL( db, COMMCSNAME, clName );
   
   db.transBegin() ;
   dbcl.insert( { transaction: 1 } ) ;
   db.transCommit() ;
   verifyDate(dbcl, {transaction : 1}, 1) ;

   db.transBegin() ;
   dbcl.insert( { transaction: 2 } ) ;
   db.transRollback() ;
   verifyDate(dbcl, {transaction : 2}, 0) ;
   
   db.transBegin() ;
   dbcl.update({$set:{transaction : -1}}) ;
   db.transCommit() ;
   verifyDate(dbcl, {transaction : -1}, 1) ;
   
   db.transBegin() ;
   dbcl.update({$set:{transaction : 1}}) ;
   db.transRollback() ;
   verifyDate(dbcl, {transaction : 1}, 0) ;
   
   db.transBegin() ;
   dbcl.remove() ;
   db.transRollback() ;
   verifyDate(dbcl, null, 1) ;
   
   db.transBegin() ;
   dbcl.remove() ;
   db.transCommit() ;
   verifyDate(dbcl, null, 0) ;
   
   commDropCL(db, COMMCSNAME, clName, true, true);
}
main();

function verifyDate(dbcl, options, expectValue)
{
   var rc = dbcl .find(options);
   if ( rc.size() != expectValue )
   {
      throw buildException("main()", "cl remvoe count is wrong", "compare", expectValue, rc.size());
   }
}
