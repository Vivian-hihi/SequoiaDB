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

    if( true == commIsStandalone( db ) )
    {
        println( "run mode is standalone" );
        return;
    }     
    
    //less two groups to split
    var allGroupName = commGetGroups(db);
    if( 2 > allGroupName.length )
    {
        println("--least two groups");
        return ;
    }
    
    var clOpt = {ShardingKey:{a:1},ShardingType:'range',ReplSize:0};
    var rangeCL = commCreateCLByOption( db, csName, clName, clOpt );
    getTwoGroupSplit( db, csName, clName, {a:5000}, {a:10000} );

    loadData(rangeCL, rownums);
    println("insert data finished!");
    //query 1 使用切分键执行查询 sort
    var sel_1 = rangeCL.find({},{a:""}).sort({a:1});
    checkResult_sort(sel_1, "a", true, rownums);

    //query 2 使用切分键执行查询 limit
    var sel_2 = rangeCL.find({},{a:""}).limit(500);
    checkResult_limit(sel_2, "a", 500);

    //query 3 使用切分键执行查询 skip 不小于1000
    var sel_3 = rangeCL.find({},{a:""}).skip(2000);
    checkResult_skip(sel_3, "a", rownums-2000);

    //query 4 使用切分键执行查询 sort+limit+skip  skip不小于1000
    var sel_4 = rangeCL.find({},{a:""}).sort({a:-1}).limit(1500).skip(3000);
    checkResult_sort_limit_skip(sel_4, "a", false, 1500, 3000, rownums);

    //query 5 使用非切分键执行查询 sort
    var sel_5 = rangeCL.find({},{b:""}).sort({b:-1});
    checkResult_sort(sel_5, "b", false, rownums);

    //query 6 使用非切分键执行查询 limit
    var sel_6 = rangeCL.find({},{b:""}).limit(200);
    checkResult_limit(sel_6, "b", 200);

    //query 7 使用非切分键执行查询 skip 不小于1000
    var sel_7 = rangeCL.find({},{b:""}).skip(1500);
    checkResult_skip(sel_7, "b", rownums-1500);

    //query 8 使用非切分键执行查询 sort+limit+skip  skip不小于1000
    var sel_8 = rangeCL.find().sort({b:-1}).limit(1500).skip(5000);
    checkResult_sort_limit_skip(sel_8, "b", false, 1500, 5000, rownums);

    //drop cl
    commDropCL( db, csName, clName, true, true, "drop cl in the end" ) ;
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