/*******************************************************************************
*@Description:   seqDB-13718:rang分区表使用切分键/非切分键sort+limit+skip执行查询
*@Author:        2019-2-26  wangkexin
********************************************************************************/
rownums = 10000;
var mainclName = COMMCLNAME + "_maincl13718"; 
var subclName1 = COMMCLNAME + "_subcl13718a";
var subclName2 = COMMCLNAME + "_subcl13718b";

main();
function main()
{
	try
   {
	   if( commIsStandalone(db) )
	   {
		   println(" Deploy mode is standalone!");
		   return;
	   }
	   var opt = new Object();
	   var subopt = new Object();
	   subopt.a = 1;
	   opt.ReplSize = 0;
	   var subcl1 = commCreateCLByOption(db, COMMCSNAME, subclName1, opt, true);
	   var subcl2 = commCreateCLByOption(db, COMMCSNAME, subclName2, opt, true);
	   
	   opt.IsMainCL = true;
	   opt.ShardingType = "range";
	   opt.ShardingKey = subopt;
	   
	   //subcl1:{a:[999-0]}, subcl2:{a:[9999-1000]}
	   var maincl = commCreateCLByOption(db, COMMCSNAME, mainclName, opt ,true);
	   maincl.attachCL(COMMCSNAME + '.' + subclName1,{LowBound:{a:0},UpBound:{a:1000}})
	   maincl.attachCL(COMMCSNAME + '.' + subclName2,{LowBound:{a:1000},UpBound:{a:10000}})
	   
	   loadData(maincl, rownums);
	   println("insert data finished!");
	   //query 1 使用主表分区键执行查询 sort
	   var sel_1 = maincl.find({},{a:""}).sort({a:1});
	   checkResult(sel_1, "a", rownums, 0, null, true, "query1");
	   
	   //query 2 使用主表分区键执行查询 limit 覆盖值小于等于1000、大于1000
	   var sel_2_1 = maincl.find({},{a:""}).limit(500);
	   checkResultNum(sel_2_1, 500, "query2_1");
	   
	   var sel_2_2 = maincl.find({},{a:""}).limit(1000);
	   checkResultNum(sel_2_2, 1000, "query2_2");
	   
	   var sel_2_3 = maincl.find({},{a:""}).limit(1500);
	   checkResultNum(sel_2_3, 1500, "query2_3");
	   
	   //query 3 使用主表分区键执行查询 skip 覆盖值小于等于1000、大于1000
	   var sel_3_1 = maincl.find({},{a:""}).skip(500);
	   checkResultNum(sel_3_1, rownums-500, "query3_1");
	   
	   var sel_3_2 = maincl.find({},{a:""}).skip(1000);
	   checkResultNum(sel_3_2, rownums-1000, "query3_2");
	   
	   var sel_3_3 = maincl.find({},{a:""}).skip(1500);
	   checkResultNum(sel_3_3, rownums-1500, "query3_3");
	   
	   //query 4 使用主表分区键执行查询 sort+limit+skip  limit/skip覆盖值小于等于1000、大于1000
	   var sel_4_1 = maincl.find({},{a:""}).sort({a:1}).limit(500).skip(200);
	   checkResult(sel_4_1, "a", 500, 200, null, true, "query4_1");
	   
	   var sel_4_2 = maincl.find({},{a:""}).sort({a:1}).limit(1000).skip(1000);
	   checkResult(sel_4_2, "a", 1000, 1000, null, true, "query4_2");
	   
	   var sel_4_3 = maincl.find({},{a:""}).sort({a:1}).limit(1500).skip(1200);
	   checkResult(sel_4_3, "a", 1500, 1200, null, true, "query4_3") 
	   
	   
	   //query 5 使用子表分区键执行查询 sort
	   var sel_5_1 = subcl1.find({},{a:""}).sort({a:1});
	   checkResult(sel_5_1, "a", 1000, 0, null, true, "query5_1");
	   
	   var sel_5_2 = subcl2.find({},{a:""}).sort({a:1});
	   checkResult(sel_5_2, "a", 9000, 1000, null, true, "query5_2");
	   
	   //query 6 使用子表分区键执行查询 limit 覆盖值小于等于1000、大于1000
	   var sel_6_1 = subcl1.find({},{a:""}).limit(500);
	   checkResult(sel_6_1, "a", 500, null, 999, false, "query6_1");
	   
	   var sel_6_2 = subcl1.find({},{a:""}).limit(1000);
	   checkResult(sel_6_2, "a", 1000, null, 999, false, "query6_2");
	   
	   var sel_6_3 = subcl2.find({},{a:""}).limit(1500);
	   checkResult(sel_6_3, "a", 1500, null, 9999, false, "query6_3");
	   
	   //query 7 使用子表分区键执行查询 skip 覆盖值小于等于1000、大于1000
	   var sel_7_1 = subcl1.find({},{a:""}).skip(500);
	   checkResult(sel_7_1, "a", 500, null, 499, false, "query7_1");
	   
	   var sel_7_2 = subcl2.find({},{a:""}).skip(1000);
	   checkResult(sel_7_2, "a", 8000, null, 8999, false, "query7_2");
	   
	   var sel_7_3 = subcl2.find({},{a:""}).skip(1500);
	   checkResult(sel_7_3, "a", 7500, null, 8499, false, "query7_3");
	   
	   //query 8 使用子表分区键执行查询 sort+limit+skip  limit/skip覆盖值小于等于1000、大于1000
	   var sel_8_1 = subcl1.find({},{a:""}).sort({a:1}).limit(500).skip(200);
	   checkResult(sel_8_1, "a", 500, 200, null, true, "query8_1");
	   
	   var sel_8_2 = subcl2.find({},{a:""}).sort({a:1}).limit(1000).skip(1000);
	   checkResult(sel_8_2, "a", 1000, 2000, null, true, "query8_2");
	   
	   var sel_8_3 = subcl2.find({},{a:""}).sort({a:1}).limit(1500).skip(1200);
	   checkResult(sel_8_3, "a", 1500, 2200, null, true, "query8_3");
	   
	   
	   //query 9 使用非切分键执行查询 sort
	   var sel_9 = maincl.find({},{b:""}).sort({b:1});
	   checkResult(sel_9, "b", rownums, 0, null, true, "query9");
	   
	   //query 10 使用非切分键执行查询 limit 覆盖值小于等于1000、大于1000
	   var sel_10_1 = maincl.find().limit(500);
	   checkResultNum(sel_10_1, 500, "query10_1");
	   
	   var sel_10_2 = maincl.find().limit(1000);
	   checkResultNum(sel_10_2, 1000, "query10_2");
	   
	   var sel_10_3 = maincl.find().limit(1500);
	   checkResultNum(sel_10_3, 1500, "query10_3");
	   
	   //query 11 使用非切分键执行查询 skip 覆盖值小于等于1000、大于1000
	   var sel_11_1 = maincl.find().skip(500);
	   checkResultNum(sel_11_1, rownums-500, "query11_1");
	   
	   var sel_11_2 = maincl.find().skip(1000);
	   checkResultNum(sel_11_2, rownums-1000, "query11_2");
	   
	   var sel_11_3 = maincl.find().skip(1500);
	   checkResultNum(sel_11_3, rownums-1500, "query11_3");
	   
	   //query 12 使用非切分键执行查询 sort+limit+skip  limit/skip覆盖值小于等于1000、大于1000
	   var sel_12_1 = maincl.find().sort({b:1}).limit(500).skip(200);
	   checkResult(sel_12_1, "b", 500, 200, null, true, "query12_1");
	   
	   var sel_12_2 = maincl.find().sort({b:1}).limit(1000).skip(1000);
	   checkResult(sel_12_2, "b", 1000, 1000, null, true, "query12_2");
	   
	   var sel_12_3 = maincl.find().sort({b:1}).limit(1500).skip(1200);
	   checkResult(sel_12_3, "b", 1500, 1200, null, true, "query12_3");
	   
	   //drop cl
	   commDropCL( db, COMMCSNAME, mainclName, false, false, "drop cl in the end" ) ;
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
      for(var i=0;i<rownums;i++){cl.insert({a:rownums-i-1,b:i,c:"abcdefghijkl"+i});}
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
			   println(queryname + " fail! rc="+e);
			   throw e;
		   }
		   else
		   {
			   println(queryname + " fail!");
			   throw e;
		   }
	   }
}

function checkResultNum(sel, expectNum, queryname)
{
	var actNum = sel.size();
	if (actNum != expectNum)
	{
		throw buildException("checkResultNum", null, queryname, expectNum, actNum);
	}
}