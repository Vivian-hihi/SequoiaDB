/*******************************************************************************
*@Description:   seqDB-13716:rang分区表使用切分键/非切分键sort+limit+skip执行查询
*@Author:        2019-2-26  wangkexin
********************************************************************************/
main();
function main()
{
    var rownums = 10000;
    var csName = COMMCSNAME;
    var clName = "cl13716";

    var clOpt = {ShardingKey:{a:1},ShardingType:'range',ReplSize:0};
    var rangeCL = commCreateCLByOption( db, csName, clName, clOpt );

    loadData(rangeCL, rownums);
    println("insert data finished!");
    //query 1 使用切分键执行查询 sort
    var sel_1 = rangeCL.find({},{a:""}).sort({a:1});
    checkResult_sort(sel_1, "a", true, rownums);

    //query 2 使用切分键执行查询 limit 覆盖值小于等于1000、大于1000
    var sel_2_1 = rangeCL.find({},{a:""}).limit(500);
    checkResult_limit(sel_2_1, "a", 500);

    var sel_2_2 = rangeCL.find({},{a:""}).limit(1000);
    checkResult_limit(sel_2_2, "a", 1000);

    var sel_2_3 = rangeCL.find({},{a:""}).limit(1500);
    checkResult_limit(sel_2_3, "a", 1500);

    //query 3 使用切分键执行查询 skip 覆盖值小于等于1000、大于1000
    var sel_3_1 = rangeCL.find({},{a:""}).skip(500);
    checkResult_skip(sel_3_1, "a", 500, rownums-500);

    var sel_3_2 = rangeCL.find({},{a:""}).skip(1000);
    checkResult_skip(sel_3_2, "a", 1000, rownums-1000);

    var sel_3_3 = rangeCL.find({},{a:""}).skip(1500);
    checkResult_skip(sel_3_3, "a", 1500, rownums-1500);

    //query 4 使用切分键执行查询 sort+limit+skip  limit/skip覆盖值小于等于1000、大于1000
    var sel_4_1 = rangeCL.find({},{a:""}).sort({a:1}).limit(500).skip(200);
    checkResult_sort_limit_skip(sel_4_1, "a", true, 500, 200, rownums);

    var sel_4_2 = rangeCL.find({},{a:""}).sort({a:1}).limit(1000).skip(1000);
    checkResult_sort_limit_skip(sel_4_2, "a", true, 1000, 1000, rownums);

    var sel_4_3 = rangeCL.find({},{a:""}).sort({a:-1}).limit(1500).skip(1200);
    checkResult_sort_limit_skip(sel_4_3, "a", false, 1500, 1200, rownums);

    //query 5 使用非切分键执行查询 sort
    var sel_5 = rangeCL.find({},{b:""}).sort({b:-1});
    checkResult_sort(sel_5, "b", false, rownums);

    //query 6 使用非切分键执行查询 limit 覆盖值小于等于1000、大于1000
    var sel_6_1 = rangeCL.find().limit(500);
    checkResult_limit(sel_6_1, "b", 500);

    var sel_6_2 = rangeCL.find().limit(1000);
    checkResult_limit(sel_6_2, "b", 1000);

    var sel_6_3 = rangeCL.find().limit(1500);
    checkResult_limit(sel_6_3, "b", 1500);

    //query 7 使用非切分键执行查询 skip 覆盖值小于等于1000、大于1000
    var sel_7_1 = rangeCL.find({},{b:""}).skip(500);
    checkResult_skip(sel_7_1, "b", 500, rownums-500);

    var sel_7_2 = rangeCL.find({},{b:""}).skip(1000);
    checkResult_skip(sel_7_2, "b", 1000, rownums-1000);

    var sel_7_3 = rangeCL.find({},{b:""}).skip(1500);
    checkResult_skip(sel_7_3, "b", 1500, rownums-1500);

    //query 8 使用非切分键执行查询 sort+limit+skip  limit/skip覆盖值小于等于1000、大于1000
    var sel_8_1 = rangeCL.find().sort({b:1}).limit(500).skip(200);
    checkResult_sort_limit_skip(sel_8_1, "b", true, 500, 200, rownums);

    var sel_8_2 = rangeCL.find().sort({b:1}).limit(1000).skip(1000);
    checkResult_sort_limit_skip(sel_8_2, "b", true, 1000, 1000, rownums);

    var sel_8_3 = rangeCL.find().sort({b:-1}).limit(1500).skip(1200);
    checkResult_sort_limit_skip(sel_8_3, "b", false, 1500, 1200, rownums);

    //drop cl
    commDropCL( db, csName, clName, true, true, "drop cl in the end" ) ;
}

function loadData(cl, rownums)
{
    var record = [];
    for( var i = 0; i < rownums; i++ )
    {
        record.push({a:i,b:i,c:"abcdefghijkl"+i});
    }
    cl.insert(record);
}

function checkResult_sort( sel, field, isAsc, rownums )
{
    var act_resurnednum = 0;
    if(isAsc)
    {
       var i = 0;
    }else
    {
       var i = rownums - 1 ;
    }
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
    if(act_resurnednum !== rownums)
    {
       throw buildException("checkResult_sort", null, "check returned num of field [" + field + "]", rownums, act_resurnednum);
    }
}

function checkResult_limit( sel, field, limitNum )
{
    var i = 0;
    while(sel.next())
    {
        var ret = sel.current();
        if(ret.toObj()[field] !== i)
        {
           throw buildException("checkResult_limit", null, "check field [" + field + "] data. ", i, ret.toObj()[field]);
        }
        i++;
    }
    if(i !== limitNum)
    {
       throw buildException("checkResult_limit", null, "check returned num of field [" + field + "]", limitNum, i);
    }
}

function checkResult_skip( sel, field, skipNum, exp_returnednum )
{
    var act_resurnednum = 0;
    var i = skipNum;
    while(sel.next())
    {
        var ret = sel.current();
        if(ret.toObj()[field] !== i)
        {
           throw buildException("checkResult_skip", null, "check field [" + field + "] data. ", i, ret.toObj()[field]);
        }
        i++;
        act_resurnednum++;
    }
    if(act_resurnednum !== exp_returnednum)
    {
       throw buildException("checkResult_skip", null, "check returned num of field [" + field + "]", exp_returnednum, act_resurnednum);
    }
}


function checkResult_sort_limit_skip( sel, field, isAsc, limitNum, skipNum, rownums )
{
    var act_resurnednum = 0;
    if(isAsc)
    {
       var i = skipNum;
    }else
    {
       var i = (rownums - skipNum) - 1;
    }
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