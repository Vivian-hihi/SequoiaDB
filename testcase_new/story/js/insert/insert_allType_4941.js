/******************************************************************************
*@Description : seqDB-4941:所有数据类型边界值校验                    
*@Author      : 2019-5-29  wuyan modify
******************************************************************************/
main();
function main()
{
   var clName = "insert4941";
   var cl = readyCL( clName );
     
   var expRecords = insertRecords( cl);     
   var actRecords = cl.find(); 
   checkRec( actRecords, expRecords );    
   deleteRecordsAndCheckResult( cl );
    
   cleanCL( clName );   	
}
     

function insertRecords( cl)
{
   println("---begin to insert records.");
   //TODO : 插入数据类型缺少OID类型
   var values = [ -2147483648,2147483647, {"$numberLong":"-9223372036854775808"}, {"$numberLong":"9223372036854775807"},
                  -1.7E+308, 1.7e+308, "","test_id",{a:1}, true, {"$date":"0000-01-01"},{"$date":"9999-12-31"},
                  { "$timestamp" : "1902-01-01-00.00.00.000000" },{ "$timestamp" : "2037-12-31-23.59.59.999999" },
                  { "$binary" : "aGVsbG8gd29ybGQ=", "$type" : "255" } ,{ "$regex" : "^张", "$options" : "i" } ,
                  [ -2147483648, 0, "def" ],[], null, {"$minKey": 1 },{"$maxKey": 1 }];
   
   var docs = [];
   for(var i = 0 ; i < values.length ; ++i )
   {       
      var fieldValue = values[i];
      var objs = { "no": i, "fieldName":fieldValue};       
      docs.push(objs);         
   }    
   cl.insert( docs );    
   return docs;
}

function deleteRecordsAndCheckResult( cl )
{
   //TODO ：文本用例中步骤3为挨个删除每条记录
   cl.remove();
   
   var actCount = cl.count();
   var expCount = 0;
   if ( Number(expCount) !== Number(actCount) )
   {
      throw buildException( "deleteRecords", null, "cl.count()", expCount, actCount );
   }
}




