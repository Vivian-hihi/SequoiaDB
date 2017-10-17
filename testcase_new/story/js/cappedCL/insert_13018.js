/************************************
*@Description: insert records in capped cl, check count/find and the size of data file
*@author:      liuxiaoxuan
*@createdate:  2017.10.17
*@testlinkCase: seqDB-13018
**************************************/
function main()
{
   var csName = COMMCSNAME + "13018";
   commDropCS( db, csName, true, "drop CS in the beginning" );
   
   var csOption = {Capped:true};
   commCreateCS( db, csName, false, "", csOption );
   
   var clName = COMMCLNAME + "13018";
   var clOption = {Capped:true, Size:1024, AutoIndexId:false};
   var dbcl = commCreateCLByOption( db, csName, clName, clOption, true, true );
   
	//insert 4w records 
	var insertNums = 40000;
	var stringLength = 968;
	var string = 'a';
	insertFixedLengthDatas( dbcl, insertNums, stringLength, string );
	
	//check count 
	var expectCount = 40000;
	checkCount(dbcl, null, expectCount);
	
	//check find
	var expectIDs = [2048,1024,0];
	var sortConf = {_id : -1};
	var limitConf = 3;
	var skipConf = 39997;
	checkLogicalID( dbcl, null, null, sortConf, limitConf, skipConf, expectIDs);
	
	//check data file size
	//if(!commIsStandalone(db))
	//{
	//	var dbpath = getDataFilePath(csName , clName);
   //   var expectFileSize = 149 * 1024 * 1024;//init CL size is 149M
	//   checkDataFileSize(dbpath, expectFileSize);
	//}

	//insert records again
	insertNums = 60000;
	insertFixedLengthDatas( dbcl, insertNums, stringLength, string );
	
   //check count 
	expectCount = 100000;
	checkCount(dbcl, null, expectCount);
	
	//check find
	skipConf = 99997;
	checkLogicalID( dbcl, null, null, sortConf, limitConf, skipConf, expectIDs);

   //commDropCS( db, csName, true, "drop CS in the end" );
}

function getDataFilePath( csName, clName )
{
	var dbpath = null;
	// get CL group name
	var cl_full_name = csName + "." + clName;
	var clGroupArray = commGetCLGroups(db, cl_full_name);
	var clGroupName = clGroupArray[0];
	
	//get data file path for CL
	var dataGroupArrays = commGetGroups(db);
	for(var i = 0; i < dataGroupArrays.length; i++)
	{
		var dataGroupItem = dataGroupArrays[i];
		var dataGroupName = dataGroupItem[0].GroupName;
		
		var primaryPos = dataGroupItem[0].PrimaryPos;
		if(clGroupName == dataGroupItem[0].GroupName)
		{
			println('cl group name: ' + clGroupName + '\ndataGroupName: ' + dataGroupName);
			dbpath = dataGroupItem[primaryPos].dbpath + '/' + csName + '.1.data';
			break;
		}
	}
	
	println('dbpath: ' + dbpath);
	return dbpath;	
}

function getDataFileSize( dbpath )
{
	var fileSize = -1;
	var file;
	try
   {
		file = new File( dbpath )
      fileSize = file.getSize( dbpath ) ;
   }
   catch( e )
   {
      throw buildException( "getDataFileSize()", e, "get data file size", "success", "fail:" + e);
   }
	println('dataFileSize: ' + fileSize);
	return fileSize;
}

function checkDataFileSize( dbpath, expectFileSize )
{
	if(dbpath == null)
	{
		println('dbpath is null');
	}
	else
	{
		var actFileSize = getDataFileSize(dbpath);
		if(actFileSize == -1 || actFileSize > expectFileSize)
		{
			 throw buildException( "checkDataFileSize()", e, "check data file size", expectFileSize, actFileSize);
		}
	}
}
main();



