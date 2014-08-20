var db = new Sdb('sdbserver3',11810)
db.inspect_cs.createCL('maincl',{ShardingKey:{id:1},ShardingType:'range',ReplSize:0,Compressed:true,IsMainCL:true})
db.inspect_cs.createCL('cl1',{ShardingKey:{id:1},ShardingType:'range',ReplSize:0,Compressed:true,IsMainCL:false})
db.inspect_cs.createCL('cl2',{ShardingKey:{id:1},ShardingType:'range',ReplSize:0,Compressed:true,IsMainCL:false})
db.inspect_cs.maincl.attachCL('inspect_cs.cl1',{LowBound:{id:0},UpBound:{id:50}})
db.inspect_cs.maincl.attachCL('inspect_cs.cl2',{LowBound:{id:50},UpBound:{id:100}})



