/******************************************************************************
*@Description : seqDB-12131:插入数据为特殊字符
                seqDB-12132:插入数据为多层嵌套对象
                seqDB-12137:插入记录值为中文字符                
*@Author      : 2019-5-29  wuyan modify
******************************************************************************/
main();
function main()
{
     var clName = "insert12131";
     var cl = readyCL( clName );
     
     var expRecords = insertRecords( cl);     
     var actRecords = cl.find().sort({no:1}); 
     checkRec( actRecords, expRecords );
     
     cleanCL( clName );   	
}

function insertRecords( cl)
{
   println("---begin to insert records.");
   //test testcase-12131:插入数据为特殊字符
   var insertObj1 = {"no":0,"!@#%^&":"&*()?><"};    
   //test testcase-12132:插入数据为多层嵌套对象 
   var objValue = {"a":{"a":{"a":{"a":{"a":{"a":{"a":{"a":{"a":{"a":100}}}}}}}}}};
   //test testcase-12137:插入数据为中文字符
   var insertObj2 = {"no":2,name:"赵钱孙李陈"}; ;
   
   var doc = [];
   doc.push(insertObj1);
   doc.push({"no": 1, obj: objValue});
   doc.push(insertObj2);
   cl.insert(doc);
     
   return doc;
}
