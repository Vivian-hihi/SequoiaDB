/************************************
*@Description: 匹配符$all，匹配不存在的记录 
*@author:      wangkexin
*@createdate:  2019.3.5
*@testlinkCase:seqDB-11706
**************************************/

main();

function main()
{
    var csName = CHANGEDPREFIX + "_11706_CS";
    var clName = CHANGEDPREFIX + "_11706_CL";

    commDropCS(db, csName, true, "drop cs in the begin");
    var cl = commCreateCL( db, csName, clName, null, null, true, false, "create cl in the begin" );

    println("---begin test---");
    cl.insert({a:[Regex("^W","i"),3]});

    var cursor = cl.find({a:{$all:[Regex("^W","i"),Regex("^s","i")]}});
    if(cursor.next()!=null)
    {
        throw buildException("find()",null,"check record", "no data","have data");
    }
    cursor.close();

    commDropCS( db, csName, true, "drop CS in the end" );
}