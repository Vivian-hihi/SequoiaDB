/************************************
*@Description: 批量创建主子表并挂载
*@author:      wangkexin
*@createDate:  2019.3.11
*@testlinkCase: seqDB-20
**************************************/
main();
function main()
{
    var mainCSName = "maincs20";
    var subCSName = "subcs20";
    var mainCL_Name = "maincl20";
    var subCL_Name = "subcl20";
    var clNum = 4096;
    
    if( true == commIsStandalone( db ) )
    {
      println( "run mode is standalone");
      return;
    }
    
    //创建主表
    var mainCLOption = { ShardingKey:{"a":1}, ShardingType:"range", IsMainCL:true};
    var maincl = commCreateCLByOption( db, mainCSName, mainCL_Name, mainCLOption, true, true);
    
    //创建子表
    println("\n---Begin to create subCL.");
    createSubCL(db, subCSName, subCL_Name, clNum);
    attachCL(subCSName, maincl, subCL_Name, clNum);
    
    //检查attach的结果，保证每个子表都存在一条记录
    for( var i = 0 ; i < 40960; i+=10 )
    {
        maincl.insert({a:i});
    }
    
    checkResult( subCSName, subCL_Name, clNum );
    
    //清除环境
    commDropCS( db, mainCSName, true, "drop CS in the end" );   
    commDropCS( db, subCSName, true, "drop CS in the end" ); 
}

function createSubCL( db, csName, subclName , clNum )
{
    for( var i = 0 ; i < clNum; i++ )
    {
        var subClOption = {ShardingKey:{"b":1}, ShardingType:"hash", AutoSplit:true, ReplSize:0};
        commCreateCLByOption( db, csName, subclName+"_"+i, subClOption, true, true );
    }
}

function attachCL( csName, mainCL, subclName, clNum )
{
    var lowBound = 0;
    for(var i = 0 ; i < clNum ; i++ )
    {
        var options = { LowBound:{ a:lowBound },UpBound:{ a: parseInt(lowBound) + 10} };
        mainCL.attachCL( csName + "." + subclName+"_"+i, options );
        lowBound+=10;
    }
}

function checkResult( subCSName, subclName, clNum )
{
    var cursor = null;
    for( var i = 0; i < clNum ; i++ )
    {
        var insert_obj = {"a": i*10 };
        var sdbcl = db.getCS(subCSName).getCL(subclName+"_"+i);
        if(sdbcl.count() != 1)
        {
            throw buildException("checkResult()",null,"check collection record number, cl:" + subCSName + "." + subclName+"_"+i, "1", sdbcl.count());
        }
        cursor = sdbcl.find({},{"_id":{"$include":0}});
        var act_obj = cursor.next().toObj();
        if(insert_obj["a"] !== act_obj["a"])
        {
            throw buildException("checkResult()",null,"check collection record, cl:" + subCSName + "." + subclName+"_"+i, insert_obj["a"], act_obj["a"]);
        }
        cursor.close();
    }
}