var db = new Sdb;
var csName = "test_option";
var flags = new Array();
function main()
{
   //seqDB-18361, comm table
   var clName = "t1";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName);
   var expObj = {ReplSize:1,Attribute:1,AttributeDesc:"Compressed",CompressionType:0,CompressionTypeDesc:"snappy"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t2";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName);
   var expObj = {ReplSize:-1,Attribute:1,AttributeDesc:"Compressed",CompressionType:1,CompressionTypeDesc:"lzw"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t3";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, true);
   var expObj = {ReplSize:-1,Attribute:1,AttributeDesc:"Compressed",CompressionType:1,CompressionTypeDesc:"lzw",GroupName:"group1"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   //seqDB-18362, comm table
   var clName = "t4";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, true, true);
   var expObj = {ReplSize:1, Attribute:11, AttributeDesc:"Compressed | NoIDIndex | StrictDataMode", CompressionType:0, CompressionTypeDesc:"snappy", GroupName:"group1", AutoIncrement:"a"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   //seqDB-18365, split table or comm table
   var clName = "t5";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, false, true);
   var expObj = {ReplSize:1, Attribute:1, AttributeDesc:"Compressed", CompressionType:0, CompressionTypeDesc:"snappy", ShardingKey:"a", EnsureShardingIndex:true, ShardingType:"hash", Partition:1024, AutoSplit:true};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t6";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, false, true);
   var expObj = {ReplSize:-1, Attribute:1, AttributeDesc:"Compressed", CompressionType:1, CompressionTypeDesc:"lzw", ShardingKey:"a", EnsureShardingIndex:false, ShardingType:"range", IsMainCL:true};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t7";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName);
   var expObj = {ReplSize:-1, Attribute:1, AttributeDesc:"Compressed", CompressionType:1, CompressionTypeDesc:"lzw"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   //seqDB-18366, comm table
   var clName = "t8";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, true, true);
   var expObj = {ReplSize:1, Attribute:11, AttributeDesc:"Compressed | NoIDIndex | StrictDataMode", CompressionType:0, CompressionTypeDesc:"snappy", GroupName:"group1", AutoIncrement:"a"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   //seqDB-18372, split table or comm table
   var clName = "t9";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, false, true);
   var expObj = {ReplSize:1, Attribute:1, AttributeDesc:"Compressed", CompressionType:0, CompressionTypeDesc:"snappy", ShardingKey:"a", EnsureShardingIndex:true, ShardingType:"hash", Partition:1024, AutoSplit:true};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t10";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, false, true);
   var expObj = {ReplSize:-1, Attribute:1, AttributeDesc:"Compressed", CompressionType:1, CompressionTypeDesc:"lzw", ShardingKey:"a", EnsureShardingIndex:false, ShardingType:"range", IsMainCL:true};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t11";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName);
   var expObj = {ReplSize:-1, Attribute:1, AttributeDesc:"Compressed", CompressionType:1, CompressionTypeDesc:"lzw"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   //seqDB-18373, comm table
   var clName = "t12";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, true, true);
   var expObj = {ReplSize:1, Attribute:11, AttributeDesc:"Compressed | NoIDIndex | StrictDataMode", CompressionType:0, CompressionTypeDesc:"snappy", GroupName:"group1", AutoIncrement:"a"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   //seqDB-18379, comm table
   var clName = "t13";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName);
   var expObj = {ReplSize:-1, Attribute:1, AttributeDesc:"Compressed", CompressionType:1, CompressionTypeDesc:"lzw"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   //seqDB-18363, comm table
   var clName = "t14";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName);
   var expObj = {ReplSize:1, Attribute:1, AttributeDesc:"Compressed", CompressionType:0, CompressionTypeDesc:"snappy"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t15";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName);
   var expObj = {ReplSize:-1, Attribute:1, AttributeDesc:"Compressed", CompressionType:1, CompressionTypeDesc:"lzw"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t16";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, true);
   var expObj = {ReplSize:-1, Attribute:1, AttributeDesc:"Compressed", CompressionType:1, CompressionTypeDesc:"lzw",GroupName:"group1"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   //seqDB-18364, comm table
   var clName = "t17";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, true, true);
   var expObj = {ReplSize:1, Attribute:11, AttributeDesc:"Compressed | NoIDIndex | StrictDataMode", CompressionType:0, CompressionTypeDesc:"snappy", GroupName:"group1", AutoIncrement:"a"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   //seqDB-18367, split table or comm table
   var clName = "t18";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, false, true);
   var expObj = {ReplSize:1, Attribute:1, AttributeDesc:"Compressed", CompressionType:0, CompressionTypeDesc:"snappy", ShardingKey:"a", EnsureShardingIndex:true, ShardingType:"hash", Partition:1024, AutoSplit:true};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t19";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, false, true);
   var expObj = {ReplSize:-1, Attribute:1, AttributeDesc:"Compressed", CompressionType:1, CompressionTypeDesc:"lzw", ShardingKey:"a", EnsureShardingIndex:false, ShardingType:"range", IsMainCL:true};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t20";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, false, true);
   var expObj = {ReplSize:-1, Attribute:1, AttributeDesc:"Compressed", CompressionType:1, CompressionTypeDesc:"lzw", ShardingKey:"a", EnsureShardingIndex:false, ShardingType:"hash", Partition:4096, AutoSplit: true};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   //seqDB-18368, split table or comm table
   var clName = "t21";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, true, true);
   var expObj = {ReplSize:1, Attribute:11, AttributeDesc:"Compressed | NoIDIndex | StrictDataMode", CompressionType:0, CompressionTypeDesc:"snappy", AutoIncrement:"a", ShardingKey:"a", EnsureShardingIndex:false, Partition:4096, AutoSplit:true, ShardingType:"hash"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t22";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, true, true);
   var expObj = {ReplSize:1, Attribute:11, AttributeDesc:"Compressed | NoIDIndex | StrictDataMode", CompressionType:0, CompressionTypeDesc:"snappy", AutoIncrement:"a", ShardingKey:"a", EnsureShardingIndex:false, Partition:4096, AutoSplit:true, ShardingType:"hash"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t23";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, true);
   var expObj = {ReplSize:1, Attribute:11, AttributeDesc:"Compressed | NoIDIndex | StrictDataMode", CompressionType:0, CompressionTypeDesc:"snappy", AutoIncrement:"a"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t24";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, true);
   var expObj = {ReplSize:1, Attribute:11, AttributeDesc:"Compressed | NoIDIndex | StrictDataMode", CompressionType:0, CompressionTypeDesc:"snappy", AutoIncrement:"a"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t25";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, true, true);
   var expObj = {ReplSize:1, Attribute:11, AttributeDesc:"Compressed | NoIDIndex | StrictDataMode", CompressionType:0, CompressionTypeDesc:"snappy", AutoIncrement:"a", ShardingKey:"a", EnsureShardingIndex:false, Partition:4096, AutoSplit:true, ShardingType:"hash"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   //seqDB-18369, split table or comm table
   var clName = "t26";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, true, true);
   var expObj = {ReplSize:1, Attribute:11, AttributeDesc:"Compressed | NoIDIndex | StrictDataMode", CompressionType:0, CompressionTypeDesc:"snappy", AutoIncrement:"a", ShardingKey:"a", EnsureShardingIndex:false, Partition:4096, AutoSplit:true, ShardingType:"hash"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t27";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, true, true);
   var expObj = {ReplSize:1, Attribute:11, AttributeDesc:"Compressed | NoIDIndex | StrictDataMode", CompressionType:0, CompressionTypeDesc:"snappy", AutoIncrement:"a", ShardingKey:"a", EnsureShardingIndex:false, Partition:4096, AutoSplit:true, ShardingType:"hash"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t28";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, true);
   var expObj = {ReplSize:1, Attribute:11, AttributeDesc:"Compressed | NoIDIndex | StrictDataMode", CompressionType:0, CompressionTypeDesc:"snappy", AutoIncrement:"a"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t29";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, true);
   var expObj = {ReplSize:1, Attribute:11, AttributeDesc:"Compressed | NoIDIndex | StrictDataMode", CompressionType:0, CompressionTypeDesc:"snappy", AutoIncrement:"a"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   //seqDB-18370, comm table
   var clName = "t30";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, true);
   var expObj = {ReplSize:1, Attribute:11, AttributeDesc:"Compressed | NoIDIndex | StrictDataMode", CompressionType:0, CompressionTypeDesc:"snappy", AutoIncrement:"a"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t31";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, true);
   var expObj = {ReplSize:1, Attribute:11, AttributeDesc:"Compressed | NoIDIndex | StrictDataMode", CompressionType:0, CompressionTypeDesc:"snappy", AutoIncrement:"a"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t32";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, true);
   var expObj = {ReplSize:1, Attribute:11, AttributeDesc:"Compressed | NoIDIndex | StrictDataMode", CompressionType:0, CompressionTypeDesc:"snappy", AutoIncrement:"a"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   //seqDB-18374, split table or comm table
   var clName = "t33";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, false, true);
   var expObj = {ReplSize:1, Attribute:1, AttributeDesc:"Compressed", CompressionType:0, CompressionTypeDesc:"snappy", ShardingKey:"a", EnsureShardingIndex:true, ShardingType:"hash", Partition:1024, AutoSplit:true};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t34";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, false, true);
   var expObj = {ReplSize:-1, Attribute:1, AttributeDesc:"Compressed", CompressionType:1, CompressionTypeDesc:"lzw", ShardingKey:"a", EnsureShardingIndex:false, ShardingType:"range", IsMainCL:true};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t35";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, false, true);
   var expObj = {ReplSize:-1, Attribute:1, AttributeDesc:"Compressed", CompressionType:1, CompressionTypeDesc:"lzw", ShardingKey:"a", EnsureShardingIndex:false, ShardingType:"hash", Partition:4096, AutoSplit: true};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   //seqDB-18375, split table or comm table
   var clName = "t36";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, true, true);
   var expObj = {ReplSize:1, Attribute:11, AttributeDesc:"Compressed | NoIDIndex | StrictDataMode", CompressionType:0, CompressionTypeDesc:"snappy", AutoIncrement:"a", ShardingKey:"a", EnsureShardingIndex:false, Partition:4096, AutoSplit:true, ShardingType:"hash"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t37";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, true, true);
   var expObj = {ReplSize:1, Attribute:11, AttributeDesc:"Compressed | NoIDIndex | StrictDataMode", CompressionType:0, CompressionTypeDesc:"snappy", AutoIncrement:"a", ShardingKey:"a", EnsureShardingIndex:false, Partition:4096, AutoSplit:true, ShardingType:"hash"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t38";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, true);
   var expObj = {ReplSize:1, Attribute:11, AttributeDesc:"Compressed | NoIDIndex | StrictDataMode", CompressionType:0, CompressionTypeDesc:"snappy", AutoIncrement:"a"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t39";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, true);
   var expObj = {ReplSize:1, Attribute:11, AttributeDesc:"Compressed | NoIDIndex | StrictDataMode", CompressionType:0, CompressionTypeDesc:"snappy", AutoIncrement:"a"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t40";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, true, true);
   var expObj = {ReplSize:1, Attribute:11, AttributeDesc:"Compressed | NoIDIndex | StrictDataMode", CompressionType:0, CompressionTypeDesc:"snappy", AutoIncrement:"a", ShardingKey:"a", EnsureShardingIndex:false, Partition:4096, AutoSplit:true, ShardingType:"hash"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   //seqDB-18376, split table or comm table
   var clName = "t41";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, true, true);
   var expObj = {ReplSize:1, Attribute:11, AttributeDesc:"Compressed | NoIDIndex | StrictDataMode", CompressionType:0, CompressionTypeDesc:"snappy", AutoIncrement:"a", ShardingKey:"a", EnsureShardingIndex:false, Partition:4096, AutoSplit:true, ShardingType:"hash"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t42";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, true, true);
   var expObj = {ReplSize:1, Attribute:11, AttributeDesc:"Compressed | NoIDIndex | StrictDataMode", CompressionType:0, CompressionTypeDesc:"snappy", AutoIncrement:"a", ShardingKey:"a", EnsureShardingIndex:false, Partition:4096, AutoSplit:true, ShardingType:"hash"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t43";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, true);
   var expObj = {ReplSize:1, Attribute:11, AttributeDesc:"Compressed | NoIDIndex | StrictDataMode", CompressionType:0, CompressionTypeDesc:"snappy", AutoIncrement:"a"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t44";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, true);
   var expObj = {ReplSize:1, Attribute:11, AttributeDesc:"Compressed | NoIDIndex | StrictDataMode", CompressionType:0, CompressionTypeDesc:"snappy", AutoIncrement:"a"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   //seqDB-18377, comm table
   var clName = "t45";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, true);
   var expObj = {ReplSize:1, Attribute:11, AttributeDesc:"Compressed | NoIDIndex | StrictDataMode", CompressionType:0, CompressionTypeDesc:"snappy", AutoIncrement:"a"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t46";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, true);
   var expObj = {ReplSize:1, Attribute:11, AttributeDesc:"Compressed | NoIDIndex | StrictDataMode", CompressionType:0, CompressionTypeDesc:"snappy", AutoIncrement:"a"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   //seqDB-18378, comm table
   var clName = "t47";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, true);
   var expObj = {ReplSize:1, Attribute:11, AttributeDesc:"Compressed | NoIDIndex | StrictDataMode", CompressionType:0, CompressionTypeDesc:"snappy", AutoIncrement:"a"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   //seqDB-18380, split table or comm table
   var clName = "t48";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, false, true);
   var expObj = {ReplSize:-1, Attribute:1, AttributeDesc:"Compressed", CompressionType:1, CompressionTypeDesc:"lzw", ShardingKey:"a", EnsureShardingIndex:false, Partition:4096, AutoSplit:true, ShardingType:"hash"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t49";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, false, true);
   var expObj = {ReplSize:-1, Attribute:1, AttributeDesc:"Compressed", CompressionType:1, CompressionTypeDesc:"lzw", ShardingKey:"a", EnsureShardingIndex:false, Partition:4096, AutoSplit:true, ShardingType:"hash"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t50";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName);
   var expObj = {ReplSize:-1, Attribute:1, AttributeDesc:"Compressed", CompressionType:1, CompressionTypeDesc:"lzw"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t51";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName);
   var expObj = {ReplSize:-1, Attribute:1, AttributeDesc:"Compressed", CompressionType:1, CompressionTypeDesc:"lzw"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t52";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, false, true);
   var expObj = {ReplSize:-1, Attribute:1, AttributeDesc:"Compressed", CompressionType:1, CompressionTypeDesc:"lzw", EnsureShardingIndex:false, Partition:4096, AutoSplit:true, ShardingType:"hash", ShardingKey:"a"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   //seqDB-18381, split table or comm table
   var clName = "t53";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, false, true);
   var expObj = {ReplSize:-1, Attribute:1, AttributeDesc:"Compressed", CompressionType:1, CompressionTypeDesc:"lzw", EnsureShardingIndex:false, Partition:4096, AutoSplit:true, ShardingType:"hash", ShardingKey:"a"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t54";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName, false, false, true);
   var expObj = {ReplSize:-1, Attribute:1, AttributeDesc:"Compressed", CompressionType:1, CompressionTypeDesc:"lzw", EnsureShardingIndex:false, Partition:4096, AutoSplit:true, ShardingType:"hash", ShardingKey:"a"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t55";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName);
   var expObj = {ReplSize:-1, Attribute:1, AttributeDesc:"Compressed", CompressionType:1, CompressionTypeDesc:"lzw"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t56";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName);
   var expObj = {ReplSize:-1, Attribute:1, AttributeDesc:"Compressed", CompressionType:1, CompressionTypeDesc:"lzw"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   //seqDB-18382, comm table
   var clName = "t57";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName);
   var expObj = {ReplSize:-1, Attribute:1, AttributeDesc:"Compressed", CompressionType:1, CompressionTypeDesc:"lzw"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   var clName = "t58";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName);
   var expObj = {ReplSize:-1, Attribute:1, AttributeDesc:"Compressed", CompressionType:1, CompressionTypeDesc:"lzw"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
   
   //seqDB-18383, comm table
   var clName = "t59";
   var obj = new Object();
   var clOptions = getCLOptions(csName, clName);
   var expObj = {ReplSize:-1, Attribute:1, AttributeDesc:"Compressed", CompressionType:1, CompressionTypeDesc:"lzw"};
   var flag = compare(clOptions, expObj);
   obj[clName] = flag;
   flags.push(obj);
  
   //println("flags:" + flags);
   return JSON.stringify(flags);
}
main();

function getCLOptions(csName, clName, isGetGroupName, isGetAutoIncrement, isGetShardingKey)
{
   if ( isGetGroupName == undefined ) { isGetGroupName = false ; }
   if ( isGetAutoIncrement == undefined ) { isGetAutoIncrement = false ; }
   if ( isGetShardingKey == undefined ) { isGetShardingKey = false ; }
   var clOptions = db.snapshot(8,{Name:csName + "." + clName}).next().toObj();
   delete clOptions._id;
   delete clOptions.Name;
   delete clOptions.UniqueID;
   delete clOptions.Version;
   delete clOptions.InternalV;
   //delete clOptions.AutoSplit;
   if(!isGetGroupName)
   {
      delete clOptions.CataInfo;
   }else
   {
      var groupName = clOptions.CataInfo[0].GroupName
      clOptions["GroupName"] = groupName
      delete clOptions.CataInfo;
   }
   if(!isGetAutoIncrement)
   {
      delete clOptions.AutoIncrement;
   }else
   {
      var field = clOptions.AutoIncrement[0].Field
      clOptions["AutoIncrement"] = field
      //delete clOptions.AutoIncrement;
   }
   if(!isGetShardingKey)
   {
      delete clOptions.isGetShardingKey;
   }else
   {
      var shardingKey = clOptions.ShardingKey
      clOptions["ShardingKey"] = Object.keys(shardingKey)[0];
      //delete clOptions.AutoIncrement;
   }
   
   return clOptions;
}

function compare(actObj, expObj)
{
    var base = expObj;
    var comp = actObj;
    
    //获取预期的obj对象中的所有key
    var keysInBase = [];
    for(var key in base)
    {
      keysInBase.push(key);
    }
    
    //获取实际的obj对象中的所有key
    var keysInComp = [];
    for(var key in comp)
    {
      keysInComp.push(key);
    }
    
    if(keysInComp.length !== keysInBase.length)
    {
      flag = false;
      return flag;
    }
    
    //比较2个obj，相同key的value值相等
    var len = keysInBase.length;
    var flag = false;
    for( var i = 0; i < len; i++){
        var key = keysInBase[i];
        if(comp.hasOwnProperty(key) && (comp[key] === base[key])){
            flag = true;
        }else{
            flag = false;
            break;
        }
        
    }
    return flag;
}
