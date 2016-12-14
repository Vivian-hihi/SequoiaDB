##语法##
***rg.getMaster()***

获取当前分区组的主节点。

##返回值##

返回当前分区组的主节点，类型为 Object 对象。

##错误##

[错误码](reference/Sequoiadb_error_code.md)

##示例##

获取 group1 分区组的主节点

```lang-javascript
> var rg = db.getRG("group1")
> rg.getMaster()
hostname1:11830
```
