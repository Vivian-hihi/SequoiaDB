/************************************
*@Description: 使用date()函数更新记录 
*@author:      wangkexin
*@createdate:  2019.3.5
*@testlinkCase:seqDB-11062
**************************************/

main();

function main()
{
    var csName = CHANGEDPREFIX + "_11062_CS";
    var clName = CHANGEDPREFIX + "_11062_CL";

    commDropCS(db, csName, true, "drop cs in the begin");
    var cl = commCreateCL( db, csName, clName, null, null, true, false, "create cl in the begin" );

    println("---begin test---");
    cl.insert({_id:1,a:1});

    var sql = ' update '+csName+'.'+clName+' set a=date("2016-06-01")';
    db.execUpdate( sql );
    var cursor = cl.find();
    var expRecs = '[{"_id":1,"a":{"$date":"2016-06-01"}}]';
    checkCLData( cursor, expRecs , 1);

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