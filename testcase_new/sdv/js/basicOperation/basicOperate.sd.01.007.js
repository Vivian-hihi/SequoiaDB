/****************************************************
@description: 对多条lob（700条）进行插入查询删除操作
@author:
              2015-10-27 TingYU
****************************************************/
main();

function main()
{	
	try
	{  
		var cl = readyCL({ReplSize:0});
		
		//generate a 1MB lob file
      var fileName = CHANGEDPREFIX + "_lobtest1.file";
      var fileSize = "1M";
      var srcMd5 = create1File( fileName, fileSize );//md5sum of source file
      
      //put lob and check
      var lobNum = 700;
	   var lobIdArr = putSomeLobs( cl, fileName, lobNum );
	   var expLobArr = lobIdArr;
	   checkLob( cl, expLobArr, srcMd5 );
	   
	   //delete lob and check
	   deleteSomeLobs( cl, lobIdArr );
	   var expLobArr = [];
      checkLob( cl, expLobArr, "" );
		
		//generate a 10MB lob file
      var fileName = CHANGEDPREFIX + "_lobtest2.file";
      var fileSize = "10M";
      var srcMd5 = create1File( fileName, fileSize );
      
		//put lob and check
      var lobNum = 10;
	   var lobIdArr = putSomeLobs( cl, fileName, lobNum );
	   var expLobArr = lobIdArr;
	   checkLob( cl, expLobArr, srcMd5 );
	   		
		clean();
	}
	catch(e)
	{
		throw e;
	}
	finally
	{
	   var cmd = new Cmd();
	   cmd.run( "rm -rf *" + CHANGEDPREFIX + "*.file" );
	}
}