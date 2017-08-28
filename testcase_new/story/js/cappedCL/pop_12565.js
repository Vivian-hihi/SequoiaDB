/************************************
*@Description: pop empty cappedCL
*@author:      liuxiaoxuan
*@createdate:  2017.8.28
*@testlinkCase: seqDB-12565
**************************************/
function main()
{
   var csName = COMMCSNAME + "_12565";
   commDropCS( db, csName, true, "drop CS in the beginning" );
   
   var csOption = {Capped:true};
   commCreateCS( db, csName, false, "", csOption );
   
   var clName = COMMCLNAME + "_12565";
   var clOption = {Capped:true, Size:1024, AutoIndexId:false};
   var dbcl = commCreateCLByOption( db, csName, clName, clOption, true, true );
    
   checkPopResult(dbcl, 0, 1);
   checkPopResult(dbcl, 0, -1);
   checkPopResult(dbcl, 100, -1);
   
   commDropCS( db, csName, true, "drop CS in the end" );
}
main();

function checkPopResult(dbcl, logicalID, direction)
{
   try
   {
      dbcl.pop({LogicalID:logicalID,Direction:direction}).toArray();
      throw "NEED_POP_ERROR";
   }catch(e)
   {
      if(e !== -6)
      {
         throw buildException("pop", e, "pop", -6, e);
      }
   }
}