/************************************
*@Description：对range分区组进行范围切分，指定分区键为string类型
*@author：2016-3-11 wuyan  Init
**************************************/
var clName = CHANGEDPREFIX + "_stringKey";
function main()
{	
	try
	{
	   //@ clean before
	   if( true == commIsStandalone( db ) )
      {
         println( "run mode is standalone" );
         return;
      }     
      //less two groups no split
      var allGroupName = getGroupName2(db,true);         
      if( 1 === allGroupName.length )
      {
         println("--least two groups");
         return ;
      } 
		var dbCL = createSplitCl( clName, {tstr:1},"range");
		var insertNum = 100;
		var docs = insertStrData( dbCL, insertNum );	  
      
      splitGrInfo = ClSplitOneTimes( COMMCSNAME, clName, docs[0], docs[1] );
       
	   //check the datas of the splitgroups,find a data in splitgroups and check count	
		//checkSplitResult( clName, splitGrInfo, 100 );
		
		checkSplitResult( clName, splitGrInfo, docs, insertNum )
		 
		//@ clean end
		commDropCL( db, COMMCSNAME, clName, false, false,"drop CL in the beginning" );
   }
   catch(e)
   {
      throw e;
   }
   finally
   {
      if (undefined !== db)
      {
         db.close();
      }
   }
}
main();

function buildString(len)
{
   var str = "";
   var startCharCode = "0".charCodeAt();
   var range = 126 - startCharCode;
   for (var pos = 0; pos < len; ++pos)
   {
      var charCode = Math.floor(Math.random() * range) + startCharCode;
      str += String.fromCharCode(charCode);
   }
   return str;
}

function insertStrData( dbcl, insertNum )
{
   if( undefined == insertNum ){ insertNum = 100 ; }
   try
   {  
      var docs = [];
      var len1 = Math.ceil(Math.random() * 20);
      var str1 = buildString(len1);
      
      var len2 = Math.ceil(Math.random() * 20);
      var str2 = buildString(len2);
      
      if (str1 === str2)
      {
         str2 = buildString(len2);
      }
      
      if (str1 >  str2)
      {
         var beginStr = str2;
         var endStr = str1;
      }
      else
      {
         var beginStr = str1;
         var endStr = str2;
      }   
      docs.push({tstr:beginStr});
      docs.push({tstr:endStr});
      println("range beginstr:" + beginStr);
      println("range endstr:" + endStr);
      
      for( var i = 0 ; i < insertNum - 2 ; ++i )
      {
         var len = Math.ceil(Math.random() * 20);
         var str = buildString(len);
         docs.push({tstr:str});
      }
      dbcl.insert( docs ) ;
   }
   catch ( e )
   {
      throw buildException("insertStrData()", null, "Failed to insert data", "insert success", e );
   }
   return docs;
}

function checkSplitResult( clName, dataNodeInfo, docs, expectResult )
{
   var beginKey = docs[0].tstr;
   var endKey = docs[1].tstr;
   var total = 0;
   for( var i = 0 ; i != dataNodeInfo.length ; ++i )
   {  
      try
      {
         var gdb = new Sdb(dataNodeInfo[i].HostName,dataNodeInfo[i].svcname);
         var cl = gdb.getCS(COMMCSNAME).getCL(clName)         
         var dataMin = cl.find().sort({tstr:1}).limit(1);  
         var objMin = dataMin.next().toObj();
         var dataMax = cl.find().sort({tstr:-1}).limit(1);
         var objMax = dataMax.next().toObj();
         //i=1,check datas to target groups
         if ( i == 1 )
         {
            //check the split range whether or not OK
            if( objMin.tstr < beginKey && objMax.tstr >= endKey )			
   	      {  
               throw buildException("checkSplitResult", "targetGroup data range wrong", "compare targetGroup data","range is ok", objMax.tstr)
   	      }
   	      //find the datas
   	      for ( j = 0; j< docs.length; ++j)
   	      {
   	         if ( docs[j].tstr >= beginKey.tstr && docs[j].tstr < endKey.tstr )
   	         {
   	            var actRecs = cl.find(docs[j]).count();
   	            if ( actRecs == 0)
   	            {
   	               throw buildException("checkSplitResult()", "targetGroup data wrong", "find data",docs[j].tstr, actRecs);  
   	            }
   	         }
   	      }
   	   } 
   	   else 
   	   {
   	      if( objMin.tstr >= beginKey && objMax.tstr < endKey )			
   	      {  
               throw buildException("checkSplitResult", "sourceGroup data range wrong", "compare srcGroup data","range is ok", objMax.tstr)
   	      }
   	   } 
   	   //check datas num of the split groups
   	   var num = cl.count(); 
   	   total += num;   	      
      }
      catch(e)
      {
         throw e;
      }
      finally
      {
         if (gdb !== undefined)
         {
            gdb.close();
         	gdb == undefined;
         } 
      }        
   }
   if( Number(total) !== expectResult )			
	{  
       throw buildException("checkClSplitResult()", "count wrong", "count()",expectResult, total)
	}	
}
   
   
   
   
