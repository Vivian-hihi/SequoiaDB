/************************************
*@Description:capped cl find use sort/limit/skip 
*@author:      zhaoyu
*@createdate:  2017.7.11
*@testlinkCase: seqDB-11802/seqDB-11803/seqDB-11804
**************************************/
function main()
{
   var csName = COMMCSNAME + "_11802";
   commDropCS( db, csName, true, "drop CS in the beginning" );
   
   var csOption = {Capped:true};
   commCreateCS( db, csName, false, "", csOption );
   
   var clName = COMMCLNAME + "_11802";
   var clOption = {Capped:true, Size:1024, AutoIndexId:false};
   var dbcl = commCreateCLByOption( db, csName, clName, clOption, true, true );
   
   insertDatas( dbcl );
   println("--end insert data");
   
   var sortConf1 = {a:1,b:-1,c:1};
   var limitConf1 = 1;
   var expRecs1 = [{a:0,b:2,c:0}];
   checkRecords( dbcl, null, null, sortConf1, limitConf1, null, expRecs1 );
   println("--end check data");
   
   var sortConf2 = {a:-1,b:1,c:-1};
   var skipConf2 = 26;
   var expRecs2 = [{a:0,b:2,c:0}];
   checkRecords( dbcl, null, null, sortConf2, null, skipConf2, expRecs2 );
   println("--end check data");
   
   var sortConf3 = {a:1,b:-1,c:-1};
   var limitConf3 = 1;
   var skipConf3 = 3;
   var expRecs3 = [{a:0,b:1,c:2}];
   checkRecords( dbcl, null, null, sortConf3, limitConf3, skipConf3, expRecs3 );
   println("--end check data");
   
   commDropCS( db, csName, true, "drop CS in the end" );
}

main();

function insertDatas( dbcl )
{
   try
   {
      for(var k = 0; k < 3; k++)
      {
         for(var i = 0; i < 3; i++)
         {
            var doc = [];
            for(var j = 0; j < 3; j++)
            {
               doc.push({a:k,b:i,c:j});
            }
            dbcl.insert(doc);
         }
      }
   }catch( e )
   {
      throw buildException("insertDatas", e, null, null, e);
   }
}

