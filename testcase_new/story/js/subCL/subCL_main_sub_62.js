/************************************
*@Description: attach和detach同一个子表多次 
*@author:      wangkexin
*@createDate:  2019.3.16
*@testlinkCase: seqDB-62
**************************************/

var csName = COMMCSNAME + "_cs62";
var mainCL_Name = CHANGEDPREFIX + "_maincl62" ;
var subCL_Name = CHANGEDPREFIX + "_subcl62";
var subCLFullName = csName + "." + subCL_Name;
function main()
{
    if(true == commIsStandalone( db ))
    {
      println( "run mode is standalone");
      return;
    }
    
    println("--test start.");
    //创建主表
    var mainCLOption = { ShardingKey:{"a":1}, ShardingType:"range", IsMainCL:true};
    var maincl = commCreateCLByOption( db, csName, mainCL_Name, mainCLOption, true, true);
    
    //创建子表
    var subClOption = {ShardingKey:{"b":1}, ShardingType:"hash", AutoSplit:true, ReplSize:0};
    commCreateCLByOption( db, csName, subCL_Name, subClOption, true, true );
    
    //循环多次attach子表/detach子表
    for(var i = 0 ; i < 100 ; i++)
    {
        attachCL(maincl);
        maincl.detachCL(subCLFullName);
    }
    
    println("--test end.");
    //清除环境
    commDropCS( db, csName, true, "drop CS in the end" );  
}

function attachCL( mainCL )
{
    var options = { LowBound:{ a:0 },UpBound:{ a: 100} };
    mainCL.attachCL( subCLFullName, options );
}
main();