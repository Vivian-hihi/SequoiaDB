/************************************
*@Description: find操作，匹配符$all，匹配正则和数值 
*@author:      wangkexin
*@createdate:  2019.3.5
*@testlinkCase:seqDB-11072
**************************************/

main();

function main()
{
    var csName = CHANGEDPREFIX + "_11072_CS";
    var clName = CHANGEDPREFIX + "_11072_CL";

    commDropCS(db, csName, true, "drop cs in the begin");
    var cl = commCreateCL( db, csName, clName, null, null, true, false, "create cl in the begin" );

    println("---begin test---");
    cl.insert({_id:1,a:[1,{"$regex":"^W","$options":""}]});
    cl.insert({_id:2,a:1});
    cl.insert({_id:3,a:{"$regex":"^W","$options":""}});

    var cursor = cl.find({a:{$all:[1,{"$regex":"^W","$options":""}]}});
    var expRecs = '[{"_id":1,"a":[1,{"$regex":"^W","$options":""}]}]';
    checkCLData( cursor, expRecs, 1);

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