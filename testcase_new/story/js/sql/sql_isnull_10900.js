/************************************
*@Description:  执行内置sql，查询条件使用is null
*@author:      wangkexin
*@createdate:  2019.3.2
*@testlinkCase:seqDB-10900
**************************************/

main();

function main()
{
    var csName = CHANGEDPREFIX + "_10900_CS";//检视：无特殊情况，使用公共CS COMMCSNAME
    var clName = CHANGEDPREFIX + "_10900_CL";
    
    commDropCS(db, csName, true, "drop cs in the begin");
    var cl = commCreateCL( db, csName, clName, null, null, true, false, "create cl in the begin" );
    
    println("---begin test---");
    insertData(cl);
    
    //在cl使用内置SQL进行select，where子句中的条件为a is null
    var sql = 'select * from '+csName+"."+clName+' where a is null';
    var cursor = db.exec( sql );
    var expRecs1 = '[{"_id":3,"b":1},{"_id":4,"a":null}]';
    checkCLData( cursor, expRecs1 , 2);
    
    //在cl使用内置SQL进行update，where子句中的条件为a is null
    var sql = 'update '+csName+"."+clName+' set a=123 where a is null';
    db.execUpdate( sql );
    var cursor = cl.find();
    var expRecs2 = '[{"_id":1,"a":1,"b":1},{"_id":2,"a":1},{"_id":3,"a":123,"b":1},{"_id":4,"a":123}]';
    checkCLData( cursor, expRecs2 , 4);
    
    //在cl使用内置SQL进行delete，where子句中的条件为a is null
    cl.truncate();
    insertData(cl);
    var sql = 'delete from '+csName+"."+clName+' where a is null';
    db.execUpdate( sql );
    var cursor = cl.find();
    var expRecs3 = '[{"_id":1,"a":1,"b":1},{"_id":2,"a":1}]';
    checkCLData( cursor, expRecs3 , 2);
    
    commDropCS( db, csName, true, "drop CS in the end" );
}

function insertData( cl )
{
    println("\n---Begin to insert cl data.");
    cl.insert({_id:1,a:1,b:1});
    cl.insert({_id:2,a:1});
    cl.insert({_id:3,b:1});
    cl.insert({_id:4,a:null});
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