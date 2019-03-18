/************************************
*@Description: 批量创建主子表并挂载
*@author:      wangkexin
*@createDate:  2019.3.11
*@testlinkCase: seqDB-20
**************************************/
function main()
{
    var csName = COMMCSNAME + "_cs20"; //TODO: 不需要加COMMCSNAME或CHANGEDPREFIX，其他变量请一并修改
    var mainCL_Name = CHANGEDPREFIX + "_maincl20" ;
    var subCL_Name = CHANGEDPREFIX + "_subcl20";
    var clNum = 4096;
    
    if(true == commIsStandalone( db ))
    {
      println( "run mode is standalone");
      return;
    }
    
    println("--test start.");//TODO: 按规范在主要步骤打印，建议"\n---Begin to create subCL."
    //创建主表
    var mainCLOption = { ShardingKey:{"a":1}, ShardingType:"range", IsMainCL:true};
    var maincl = commCreateCLByOption( db, csName, mainCL_Name, mainCLOption, true, true);
    
    //创建子表
    createSubCL(db, csName, subCL_Name, clNum);
    attachCL(csName, maincl, subCL_Name, clNum);
    
    //随机检查attach的结果  //TODO：每个子表都要检查吧，建议在主表插数据并且每个子表都有存入至少1条记录，主表插入后在子表查询数据正确性
    maincl.insert({a:23});
    var cursor = db.getCS(csName).getCL(subCL_Name+"_2").find();
    if(cursor.next()==null)//TODO: 需要判断整条记录，如果{a:1}要落到子表1那判断=null测不出问题
    {
        throw buildException("attachCL()",null,"check record", "have data", "no data");
    }
    
    println("--test end.");//TODO: 没意义，不用打印
    //清除环境
    commDropCS( db, csName, true, "drop CS in the end" );  
}

function createSubCL( db, csName, subclName , clNum )
{
    for(var i = 0 ; i < clNum; i++)
    {
        var subClOption = {ShardingKey:{"b":1}, ShardingType:"hash", AutoSplit:true, ReplSize:0};
        commCreateCLByOption( db, csName, subclName+"_"+i, subClOption, true, true );
    }
}

function attachCL( csName, mainCL, subCLName, clNum )
{
    var lowBound = 0;
    for(var i = 0 ; i < clNum ; i++ )
    {
        var options = { LowBound:{ a:lowBound },UpBound:{ a: parseInt(lowBound) + 10} };
        mainCL.attachCL( csName + "." + subCLName+"_"+i, options );
        lowBound+=10;
    }
}
main();//TODO: 放最前面（放所有函数前，用例注释后）