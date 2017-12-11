/************************************
*@Description:capped cl,pop record,logicalID Non_existent
*@author:      zhaoyu
*@createdate:  2017.7.11
*@testlinkCase: seqDB-11790
**************************************/
function main()
{
   var csName = COMMCSNAME + "_11790";
   commDropCS( db, csName, true, "drop CS in the beginning" );
   
   var csOption = {Capped:true};
   commCreateCS( db, csName, false, "", csOption );
   
   var clName = COMMCLNAME + "_11790";
   var clOption = {Capped:true, Size:1024, AutoIndexId:false};
   var dbcl = commCreateCLByOption( db, csName, clName, clOption, true, true );
   
   var recordNum = 10;
   var string = "a";
   var stringLength = 1;
   insertFixedLengthDatas( dbcl, recordNum, stringLength, string );
   
   var sortConf = {_id:-1};
   var limitConf = 1;
   var logicalIDs = getLogicalID(dbcl, null, null, sortConf, limitConf, null);
   
   pop( dbcl, logicalIDs[0], -1 );
   checkPopResult(dbcl, logicalIDs[0], -1, true);
   checkPopResult(dbcl, logicalIDs[0], 1, true);
   
   checkPopResult(dbcl, logicalIDs[0]+1, -1, true);
   checkPopResult(dbcl, logicalIDs[0]+1, 1, true);
	
	//SEQUOIADBMAINSTREAM-2575,补充测试
	//_id: 0,1024,2048,3072,4096 increasing
	removeAllDatas(dbcl);
   stringLength = 968;
   insertFixedLengthDatas( dbcl, recordNum, stringLength, string );
	
	sortConf = {_id:1};
   limitConf = 2;
   logicalIDs = getLogicalID(dbcl, null, null, sortConf, limitConf, null);
	
	//pop from 1024 and check
	pop( dbcl, logicalIDs[1], -1 );
	checkPopResult(dbcl, logicalIDs[1], -1, true);
	
	//pop from 0 and check
	pop( dbcl, logicalIDs[0], -1 );
	checkPopResult(dbcl, logicalIDs[0], -1, true);
	
   commDropCS( db, csName, true, "drop CS in the end" );
}

main();

function checkPopResult(dbcl, logicalID, direction, isSuccess)
{
   try
   {
      dbcl.pop({LogicalID:logicalID,Direction:direction});
	  if(isSuccess == undefined)  throw "NEED_POP_ERROR";
   }catch(e)
   {
	  if(isSuccess == true)
	  {
		 throw buildException("pop", e, "pop", -6, e); 
	  }
      else if(e !== -6)
      {
         throw buildException("pop", e, "pop", -6, e);
      }
   }
}