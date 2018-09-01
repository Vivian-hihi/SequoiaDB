/************************************
*@Description: Create CappedCL , check Parameter AutoIndexId
*@author:      liuxiaoxuan
*@createdate:  2017.8.15
*@testlinkCase:seqDB-12383
**************************************/

main()

function main()
{
   var csName = COMMCSNAME + "_12383";
   commDropCS( db, csName, true, "drop CS in the beginning" );

   var csOption = {Capped:true};
   commCreateCS( db, csName, false, "", csOption );

   var clName = COMMCLNAME + "_12383";
   
   //Not Exist Parameter AutoIndexId
   var option1 = {Capped:true, Size:1024};
   checkCreateCLOptions(csName,clName,option1, true);
	db.getCS(csName).dropCL(clName);
	
   // AutoIndexId true
   var option2 = {Capped:true, Size:1024, AutoIndexId:true};
   checkCreateCLOptions(csName,clName,option2);
	
   // AutoIndexId is int
   var option3 = {Capped:true, Size:1024, AutoIndexId:123};
   checkCreateCLOptions(csName,clName,option3);
   
   // AutoIndexId is String
   var option4 = {Capped:true, Size:1024, AutoIndexId:"a"};
   checkCreateCLOptions(csName,clName,option4);
   
   // AutoIndexId is null
   var option5 = {Capped:true, Size:1024, AutoIndexId:null};
   checkCreateCLOptions(csName,clName,option5);
   
   // Valid AutoIndexId
   var isValid = true;
   var option6 = {Capped:true, Size:1024, AutoIndexId:false};
   checkCreateCLOptions(csName,clName,option6,isValid);
   
   commDropCS( db, csName, true, "drop CS in the end" );

}

function checkCreateCLOptions( csName, clName, options, isValid)
{
    try
    {
		 db.getCS(csName).createCL(clName,options);
	    if ( isValid == undefined ) 
		 { 
	       throw "NEED_CREATE_FAIL_ERROR";
	    } 
	    println("Create CL with option: " + JSON.stringify(options) + " success!");
    }
   catch(e)
   {
      if( e !== -6)
      {
          throw buildException("Invalid parameter is not -6,error msg is: " + e);
      }
      else
      {
          println("check result success!");
      }
   }
}