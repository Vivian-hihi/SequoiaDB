/*******************************************************************************
*@Description:   seqDB-13718:主子表上sort+limit+skip执行查询
*@Author:        2019-2-26  wangkexin
********************************************************************************/
main();
function main()
{
    rownums = 10000;
    var mainclName = COMMCLNAME + "_maincl13718"; 
    var subclName1 = COMMCLNAME + "_subcl13718a";
    var subclName2 = COMMCLNAME + "_subcl13718b";

    if( commIsStandalone(db) )
    {
       println(" Deploy mode is standalone!");
       return;
    }
    var opt = new Object();
    opt.ReplSize = 0;
    var subcl1 = commCreateCLByOption(db, COMMCSNAME, subclName1, opt, true);
    var subcl2 = commCreateCLByOption(db, COMMCSNAME, subclName2, opt, true);

    opt.IsMainCL = true;
    opt.ShardingType = "range";
    opt.ShardingKey = {a:1};

    //subcl1:{a:[0-4999]}, subcl2:{a:[5000-9999]}
    var maincl = commCreateCLByOption(db, COMMCSNAME, mainclName, opt ,true);
    maincl.attachCL(COMMCSNAME + '.' + subclName1,{LowBound:{a:0},UpBound:{a:5000}})
    maincl.attachCL(COMMCSNAME + '.' + subclName2,{LowBound:{a:5000},UpBound:{a:10000}})

    loadData(maincl, rownums);
    println("insert data finished!");
    //query 1 使用主表分区键执行查询 sort
    var sel_1 = maincl.find({},{a:""}).sort({a:1});
    checkResult_sort(sel_1, "a", true, 0, rownums-1);

    //query 2 使用主表分区键执行查询 limit
    var sel_2 = maincl.find({},{a:""}).limit(500);
    checkResult_limit(sel_2, "a", 500);

    //query 3 使用主表分区键执行查询 skip 不小于1000
    var sel_3 = maincl.find({},{a:""}).skip(2000);
    checkResult_skip(sel_3, "a", rownums-2000);

    //query 4 使用主表分区键执行查询 sort+limit+skip  skip值不小于1000
    var sel_4 = maincl.find({},{a:""}).sort({a:-1}).limit(1500).skip(3000);
    checkResult_sort_limit_skip(sel_4, "a", false, 1500, 6999);

    println("begin to check result from sub cl");
    //query 5 使用子表分区键执行查询 sort
    var sel_5_1 = subcl1.find({},{a:""}).sort({a:1});
    checkResult_sort(sel_5_1, "a", true, 0, 4999);

    var sel_5_2 = subcl2.find({},{a:""}).sort({a:1});
    checkResult_sort(sel_5_2, "a", true, 5000, 9999);

    //query 6 使用子表分区键执行查询 limit
    var sel_6_1 = subcl1.find({},{a:""}).limit(500);
    checkResult_limit(sel_6_1, "a", 500);

    var sel_6_2 = subcl2.find({},{a:""}).limit(1500);
    checkResult_limit(sel_6_2, "a", 1500);

    //query 7 使用子表分区键执行查询 skip 值不小于1000
    var sel_7_1 = subcl1.find({},{a:""}).skip(1500);
    checkResult_skip(sel_7_1, "a", 3500);
    
    var sel_7_2 = subcl2.find({},{a:""}).skip(1800);
    checkResult_skip(sel_7_2, "a", 3200);

    //query 8 使用子表分区键执行查询 sort+limit+skip  skip值不小于1000
    var sel_8_1 = subcl1.find({},{a:""}).sort({a:1}).limit(1500).skip(3000);
    checkResult_sort_limit_skip(sel_8_1, "a", true, 1500, 3000);

    var sel_8_2 = subcl2.find({},{a:""}).sort({a:1}).limit(1000).skip(2000);
    checkResult_sort_limit_skip(sel_8_2, "a", true, 1000, 7000);

    println("begin to check field b from main cl");
    //query 9 使用非切分键执行查询 sort
    var sel_9 = maincl.find({},{b:""}).sort({b:-1});
    checkResult_sort(sel_9, "b", false, rownums-1 , 0);

    //query 10 使用非切分键执行查询 limit
    var sel_10 = maincl.find({},{b:""}).limit(500);
    checkResult_limit(sel_10, "b", 500);

    //query 11 使用非切分键执行查询 skip值不小于1000
    var sel_11 = maincl.find().skip(1500);
    checkResult_skip(sel_11, "b", rownums-1500);

    //query 12 使用非切分键执行查询 sort+limit+skip  skip值不小于1000
    var sel_12 = maincl.find().sort({b:-1}).limit(1500).skip(3000);
    checkResult_sort_limit_skip(sel_12, "b", false, 1500, 6999);

    //drop cl
    commDropCL( db, COMMCSNAME, mainclName, false, false, "drop cl in the end" ) ;
}

function loadData(cl, rownums)
{
    var record = [];
    for( var i = 0; i < rownums; i++ )
    {
        record.push({a:i,b:i,c:i});
    }
    cl.insert(record);
}

function checkResult_sort( sel, field, isAsc, start, end )
{
    var act_resurnednum = 0;
    var exp_resurnednum = Math.abs(end - start) + 1;
    var i = start;
    while(sel.next())
    {
        var ret = sel.current();
        if(ret.toObj()[field] !== i)
        {
           throw buildException("checkResult_sort", null, "check field [" + field + "] data. ", i, ret.toObj()[field]);
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
    if(act_resurnednum !== exp_resurnednum)
    {
       throw buildException("checkResult_sort", null, "check returned num of field [" + field + "]", exp_resurnednum, act_resurnednum);
    }
}

function checkResult_limit( sel, field, limitNum )
{
    var act_resurnednum = sel.size();
    if(act_resurnednum !== limitNum)
    {
       throw buildException("checkResult_limit", null, "check returned num of field [" + field + "]", limitNum, act_resurnednum);
    }
}

function checkResult_skip( sel, field, exp_returnednum )
{
    var act_resurnednum = sel.size();
    if(act_resurnednum !== exp_returnednum)
    {
       throw buildException("checkResult_skip", null, "check returned num of field [" + field + "]", exp_returnednum, act_resurnednum);
    }
}


function checkResult_sort_limit_skip( sel, field, isAsc, limitNum, startNum )
{
    var act_resurnednum = 0;
    var i = startNum;
    while(sel.next())
    {
        var ret = sel.current();
        if(ret.toObj()[field] !== i)
        {
           throw buildException("checkResult_sort_limit_skip", null, "check field [" + field + "] data. ", i, ret.toObj()[field]);
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
    if(act_resurnednum !== limitNum)
    {
       throw buildException("checkResult_sort_limit_skip", null, "check returned num of field [" + field + "]", limitNum, act_resurnednum);
    }
}