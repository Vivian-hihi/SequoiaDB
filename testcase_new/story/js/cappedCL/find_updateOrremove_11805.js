/************************************
*@Description:capped cl findandUpdate/findandRemove
*@author:      zhaoyu
*@createdate:  2017.7.11
*@testlinkCase: seqDB-11805
**************************************/
function main()
{
   var csName = COMMCSNAME + "_11805";
   commDropCS( db, csName, true, "drop CS in the beginning" );
   
   var csOption = {Capped:true};
   commCreateCS( db, csName, false, "", csOption );
   
   var clName = COMMCLNAME + "_11805";
   var clOption = {Capped:true, Size:1073741824, AutoIndexId:false};
   var dbcl = commCreateCLByOption( db, csName, clName, clOption, true, true );
   
   try
   {
      dbcl.find().update({$set:{a:1}}).toArray();
      throw "NEED_THROE_ERROR";
   }catch(e)
   {
      if( e !== -279)
      {
		    throw buildException("find and remove", e, "find and remove", -279, e); 
   	 }
   	 else
   	 {
   	    println("check result is ok!");   		
   	 } 
   }
   
   try{
      dbcl.find().remove().toArray();
      throw "NEED_THROE_ERROR";
   }catch(e)
   {
      if( e !== -279)
      {
		   throw buildException("find and remove", e, "find and remove", -279, e);   
   	}else
   	{
   	   println("check result is ok!");   		
      } 
   }
   
   commDropCS( db, csName, true, "drop CS in the end" );
}

main();