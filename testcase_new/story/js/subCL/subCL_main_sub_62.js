/************************************
*@Description: attach和detach同一个子表多次 
*@author:      wangkexin
*@createDate:  2019.3.16
*@testlinkCase: seqDB-62
**************************************/
//TODO: 变量统一全部放到main函数里面
var csName = COMMCSNAME + "_cs62";//TODO: 不需要加COMMCSNAME或CHANGEDPREFIX，其他变量请一并修改
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
    
    println("--test start.");//TODO: 按规范在主要步骤打印，建议"\n---Begin to create subCL."
    //创建主表
    var mainCLOption = { ShardingKey:{"a":1}, ShardingType:"range", IsMainCL:true};
    var maincl = commCreateCLByOption( db, csName, mainCL_Name, mainCLOption, true, true);
    
    //创建子表
    var subClOption = {ShardingKey:{"b":1}, ShardingType:"hash", AutoSplit:true, ReplSize:0};
    commCreateCLByOption( db, csName, subCL_Name, subClOption, true, true );
    
    //循环多次attach子表/detach子表  //TODO：每挂载一次插入一条不同的记录，并在for循环完成后检查记录正确性
    for(var i = 0 ; i < 100 ; i++)//TODO：建议for循环放到attachCL函数里面，或者attachCL操作放到该for函数里面（options不需要重新赋值则放for函数外面）
    {
        attachCL(maincl);
        maincl.detachCL(subCLFullName);
    }
    
    println("--test end.");//TODO: 无意义，删除
    //清除环境
    commDropCS( db, csName, true, "drop CS in the end" );  
}

function attachCL( mainCL )
{
    var options = { LowBound:{ a:0 },UpBound:{ a: 100} };
    mainCL.attachCL( subCLFullName, options );
}
main();//TODO: 放最前面（放所有函数前，用例注释后）