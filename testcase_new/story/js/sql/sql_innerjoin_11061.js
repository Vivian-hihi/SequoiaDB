/************************************
*@Description: 多层嵌套inner join查询
*@author:      wangkexin
*@createdate:  2019.3.4
*@testlinkCase:seqDB-11061
**************************************/

main();

function main()
{
    var csName = CHANGEDPREFIX + "_11061_CS";//检视：无特殊情况，使用公共CS COMMCSNAME
    var clName1 = CHANGEDPREFIX + "_11061_bar1";
    var clName2 = CHANGEDPREFIX + "_11061_bar2";
    var clName3 = CHANGEDPREFIX + "_11061_bar3";
    
    commDropCS(db, csName, true, "drop cs in the begin");
    var cl1 = commCreateCL( db, csName, clName1, null, null, true, false, "create cl1 in the begin" );
    var cl2 = commCreateCL( db, csName, clName2, null, null, true, false, "create cl2 in the begin" );
    var cl3 = commCreateCL( db, csName, clName3, null, null, true, false, "create cl3 in the begin" );
    
    println("---begin test---");
    cl1.insert({a:0});
    cl1.insert({a:1});
    cl1.insert({a:2});
    cl1.insert({a:3});
    cl1.insert({a:4});
    cl1.createIndex("idx_bar1_a",{a:1});//检视：建议使用公共方法中的commCreateIndex

    cl2.insert({a:1,b:1});
    cl2.insert({a:2,b:1});
    cl2.insert({a:3,b:2});
    cl2.insert({a:4,b:2});
    cl2.insert({a:5,b:2});
    cl2.createIndex("idx_bar2_b",{b:1});//检视：建议使用公共方法中的commCreateIndex

    cl3.insert({c:0});
    cl3.insert({c:1});
    cl3.insert({c:2});
    cl3.insert({c:3});
    cl3.insert({c:4});
    cl1.createIndex("idx_bar3_c",{c:1})//检视：建议使用公共方法中的commCreateIndex

    var sql = ' select t1.a, t2.b, t2.cnt from '+csName+'.'+clName1+' as t1 inner join ( select t3.b, t3.cnt, t4.c from ( select count(a) as cnt, b from '+csName+'.'+clName2+' group by b ) as t3 inner join '+csName+'.'+clName3+' as t4 on t3.b = t4.c /*+use_hash() use_index(t4, idx_bar3_c)*/ ) as t2 on t1.a = t2.b /*+use_index(t1, idx_bar1_a) */';
    var cursor = db.exec( sql );
    var expRecs = '[{"a":1,"b":1,"cnt":2},{"a":2,"b":2,"cnt":3}]';
    checkCLData( cursor, expRecs , 2);

    commDropCS( db, csName, true, "drop CS in the end" );
}

function checkCLData( rc, expRecs, expCnt )
{
    println("\n---Begin to check cl data.");
    var recsArray = [];
    while( tmpRecs = rc.next() )
    {
        recsArray.push( tmpRecs.toObj() );
    }
    rc.close();

    var actCnt  = recsArray.length;
    var actRecs = JSON.stringify( recsArray );
    if( actCnt !== expCnt || actRecs !== expRecs )
    {
        throw buildException( "checkCLdata", null, "[find]",
            "[cnt:"+ expCnt +", recs:"+ expRecs +"]",
            "[cnt:"+ actCnt +", recs:"+ actRecs +"]" );
    }
    println( "cl records: "+ actRecs );
}