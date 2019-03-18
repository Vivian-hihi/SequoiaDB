/************************************
*@Description: limit和offset关键字的位置在sql语句中无序 
*@author:      wangkexin
*@createdate:  2019.3.4
*@testlinkCase:seqDB-11105
**************************************/

main();

function main()
{
    var csName = CHANGEDPREFIX + "_11105_CS";//检视：无特殊情况，使用公共CS COMMCSNAME
    var clName = CHANGEDPREFIX + "_11105_CL";

    commDropCS(db, csName, true, "drop cs in the begin");
    var cl = commCreateCL( db, csName, clName, null, null, true, false, "create cl in the begin" );

    println("---begin test---");
    cl.insert({_id:1,a:0});
    cl.insert({_id:2,a:1});
    cl.insert({_id:3,a:2});
    cl.insert({_id:4,a:3});
    cl.insert({_id:5,a:4});

    var sql = ' select * from '+csName+'.'+clName+' offset 1 limit 2';
    var cursor = db.exec( sql );
    var expRecs = '[{"_id":2,"a":1},{"_id":3,"a":2}]';
    checkCLData( cursor, expRecs , 2);
    
    //检视：db.exec("select * from cs.cl limit 2 offset 1")场景未实现自动化

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