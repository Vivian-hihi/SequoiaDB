/********************************************************************************
@Description : abnormal test : db.CS.CL.createIndex("",{a:1},false)
@Modify list :
               2014-5-20  xiaojun Hu  Modify
********************************************************************************/
function main( db )
{
   // drop collection in the beginning
   commDropCL( db, csName, clName, true, true, "drop collection in the beginning" ) ;

   // create collection
   var idxCL = commCreateCL( db, csName, clName, -1, true, true, false, "create collection" ) ;

   // insert data to SDB
   idxCL.insert( {a:1} ) ;

   // create index
   createIndex( idxCL, "", {a:1}, false, false, -6 ) ;

   // drop collectionspace in clean
   commDropCS( db, csName, true, "drop CS in clean " )
}

try
{
   main( db ) ;
   db.close() ;
}
catch ( e )
{
   throw e ;
}

