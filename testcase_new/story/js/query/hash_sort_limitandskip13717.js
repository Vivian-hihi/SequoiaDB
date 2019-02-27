/*******************************************************************************
*@Description:   seqDB-13717:rang分区表使用切分键/非切分键sort+limit+skip执行查询
*@Author:        2019-2-26  wangkexin
********************************************************************************/
rownums = 10000;

main();
function main()
{
	try
   {
	   var csName = COMMCSNAME;
	   var clName = COMMCLNAME+"_13717";
	   
	   var varCS = commCreateCS( db, csName, true, "create CS in the beginning" );
	   var hashCL = varCS.createCL(clName,{ShardingKey:{a:1},ShardingType:'hash',ReplSize:0});
	   
	   loadData(hashCL, rownums);
	   println("insert data finished!");
	   //query 1 使用切分键执行查询 sort
	   var sel_1 = hashCL.find({},{a:""}).sort({a:1});
	   checkResult(sel_1, "a", rownums, 1, null, true, "query1");
	   
	   //query 2 使用切分键执行查询 limit 覆盖值小于等于1000、大于1000
	   var sel_2_1 = hashCL.find({},{a:""}).limit(500);
	   checkResult(sel_2_1, "a", 500, null, rownums, false, "query2_1");
	   
	   var sel_2_2 = hashCL.find({},{a:""}).limit(1000);
	   checkResult(sel_2_2, "a", 1000, null, rownums, false, "query2_2");
	   
	   var sel_2_3 = hashCL.find({},{a:""}).limit(1500);
	   checkResult(sel_2_3, "a", 1500, null, rownums, false, "query2_3");
	   
	   //query 3 使用切分键执行查询 skip 覆盖值小于等于1000、大于1000
	   var sel_3_1 = hashCL.find({},{a:""}).skip(500);
	   checkResult(sel_3_1, "a", rownums-500, null, rownums-500, false, "query3_1");
	   
	   var sel_3_2 = hashCL.find({},{a:""}).skip(1000);
	   checkResult(sel_3_2, "a", rownums-1000, null, rownums-1000, false, "query3_2");
	   
	   var sel_3_3 = hashCL.find({},{a:""}).skip(1500);
	   checkResult(sel_3_3, "a", rownums-1500, null, rownums-1500, false, "query3_3");
	   
	   //query 4 使用切分键执行查询 sort+limit+skip  limit/skip覆盖值小于等于1000、大于1000
	   var sel_4_1 = hashCL.find({},{a:""}).sort({a:1}).limit(500).skip(200);
	   checkResult(sel_4_1, "a", 500, 201, null, true, "query4_1");
	   
	   var sel_4_2 = hashCL.find({},{a:""}).sort({a:1}).limit(1000).skip(1000);
	   checkResult(sel_4_2, "a", 1000, 1001, null, true, "query4_2");
	   
	   var sel_4_3 = hashCL.find({},{a:""}).sort({a:1}).limit(1500).skip(1200);
	   checkResult(sel_4_3, "a", 1500, 1201, null, true, "query4_3");
	   
	   
	   
	   //query 5 使用非切分键执行查询 sort
	   var sel_5 = hashCL.find({},{b:""}).sort({b:1});
	   checkResult(sel_5, "b", rownums, 0, null, true, "query5");
	   
	   //query 6 使用非切分键执行查询 limit 覆盖值小于等于1000、大于1000
	   var sel_6_1 = hashCL.find().limit(500);
	   checkResult(sel_6_1, "b", 500, 0, null, true, "query6_1");
	   
	   var sel_6_2 = hashCL.find().limit(1000);
	   checkResult(sel_6_2, "b", 1000, 0, null, true, "query6_2");
	   
	   var sel_6_3 = hashCL.find().limit(1500);
	   checkResult(sel_6_3, "b", 1500, 0, null, true, "query6_3");
	   
	   //query 7 使用非切分键执行查询 skip 覆盖值小于等于1000、大于1000
	   var sel_7_1 = hashCL.find().skip(500);
	   checkResult(sel_7_1, "b", rownums-500, 500, null, true, "query7_1");
	   
	   var sel_7_2 = hashCL.find().skip(1000);
	   checkResult(sel_7_2, "b", rownums-1000, 1000, null, true, "query7_2");
	   
	   var sel_7_3 = hashCL.find().skip(1500);
	   checkResult(sel_7_3, "b", rownums-1500, 1500, null, true, "query7_3");
	   
	   //query 8 使用非切分键执行查询 sort+limit+skip  limit/skip覆盖值小于等于1000、大于1000
	   var sel_8_1 = hashCL.find().sort({b:1}).limit(500).skip(200);
	   checkResult(sel_8_1, "b", 500, 200, null, true, "query8_1");
	   
	   var sel_8_2 = hashCL.find().sort({b:1}).limit(1000).skip(1000);
	   checkResult(sel_8_2, "b", 1000, 1000, null, true, "query8_2");
	   
	   var sel_8_3 = hashCL.find().sort({b:1}).limit(1500).skip(1200);
	   checkResult(sel_8_3, "b", 1500, 1200, null, true, "query8_3");
	   
	   //drop cl
	   commDropCL( db, csName, clName, false, false, "drop cl in the end" ) ;
	   println("drop cl finished")
	}
	catch(e)
	{
		throw e;
	}
}

function loadData(cl, rownums)
{
   var funname = "loadData";
   try
   {
      for(var i=0;i<rownums;i++){cl.insert({a:rownums-i,b:i,c:"abcdefghijkl"+i});}
   }
   catch(e)
   {
      buildException(funname, e);
   }
}

function checkResult(sel, field, exp_returnednum, beginnum, endnum, isAsc, queryname)
{
	try
	   {
		   var act_resurnednum = 0;
		   var flag = true;
		   //The field is in ascending order when checking.
		   if(isAsc)
		   {
			   var i = beginnum;
		   }
		   else{
			   var i = endnum;
		   }
		   while(sel.next())
		   {
			   var ret = sel.current();
			   if(ret.toObj()[field]!=i)
			   {
				   flag = false;
				   println("ret.toObj()['" + field + "']!=i" + "   ret.toObj()['" + field + "']: " + ret.toObj()[field] + ", i: " + i);
				   throw "query-result-incorrect";
			   }
			   if(isAsc)
			   {
				   i++;
			   }else
			   {
				   i--;
			   }
			   act_resurnednum++;
		   }
		   sel.close();
		   if(act_resurnednum != exp_returnednum)
		   {
			   println("flag = " + flag + ", exp_returnednum =" + exp_returnednum + ", act_resurnednum =" + act_resurnednum);
			   throw "query-result-incorrect";
		   }
	   }
	   catch(e)
	   {
		   if(e!="query-result-incorrect")
		   {
			   println("'select * from "+csName+"."+clName+" order by a' fail! rc="+e);
			   throw e;
		   }
		   else
		   {
			   println("'select * from "+csName+"."+clName+" order by a' verify record fail! ");
			   throw e;
		   }
	   }
		println(queryname + " finished!");
}