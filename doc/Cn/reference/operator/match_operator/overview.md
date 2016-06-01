
##匹配符##

 | 匹配符 | 描述 | 示例 | 
 | ------ | ---- | ---- | 
 | [$gt](SdbDoc_Cn/reference/operator/match_operator/gt.html) | 大于 | db.foo.bar.find({age:{$gt:20}}) | 
 | [$gte](SdbDoc_Cn/reference/operator/match_operator/gte.html) | 大于等于 | db.foo.bar.find({age:{$gte:20}}) | 
 | [$lt](SdbDoc_Cn/reference/operator/match_operator/lt.html) | 小于 | db.foo.bar.find({age:{$lt:20}}) | 
 | [$lte](SdbDoc_Cn/reference/operator/match_operator/lte.html) | 小于等于 | db.foo.bar.find({age:{$lte:20}}) | 
 | [$ne](SdbDoc_Cn/reference/operator/match_operator/ne.html) | 不等于 | db.foo.bar.find({age:{$ne:20}}) | 
 | [$in](SdbDoc_Cn/reference/operator/match_operator/in.html) | 集合内存在 | db.foo.bar.find({age:{$in:[20,21]}}) | 
 | [$nin](SdbDoc_Cn/reference/operator/match_operator/nin.html) | 集合内不存在 | db.foo.bar.find({age:{$nin:[20,21]}}) | 
 | [$all](SdbDoc_Cn/reference/operator/match_operator/all.html) | 全部 | db.foo.bar.find({age:{$all:[20,21]}}) | 
 | [$and](SdbDoc_Cn/reference/operator/match_operator/and.html) | 与 | db.foo.bar.find({$and:[{age:20},{name:"Tom"}]}) | 
 | [$not](SdbDoc_Cn/reference/operator/match_operator/not.html) | 非 | db.foo.bar.find({$not:{age:20},{name:"Tom"}}) | 
 | [$or](SdbDoc_Cn/reference/operator/match_operator/or.html) | 或 | db.foo.bar.find({$or:{age:20},{name:"Tom"}}) | 
 | [$type](SdbDoc_Cn/reference/operator/match_operator/type.html) | 数据类型 | db.foo.bar.find({age:{$type:16}}) | 
 | [$exists](SdbDoc_Cn/reference/operator/match_operator/exists.html) | 存在 | db.foo.bar.find({age:{$exists:1}}) | 
 | [$elemMatch](SdbDoc_Cn/reference/operator/match_operator/elemMatch.html) | 元素匹配 | db.foo.bar.find({age:{$elemMatch:20}}) | 
 | [$+标识符](SdbDoc_Cn/reference/operator/match_operator/identifier.html) | 数组元素匹配 | db.foo.bar.find({"array.$2":10}) | 
 | [$size](SdbDoc_Cn/reference/operator/match_operator/size.html) | 大小 | db.foo.bar.find({array:{$size:3}}) | 
 | [$regex](SdbDoc_Cn/reference/operator/match_operator/regex.html) | 正则表达式 | db.foo.bar.find({str:{$regex:'dh,*fj',$options:'i'}}) | 
