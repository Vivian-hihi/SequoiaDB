字段结构页面可以查看当前数据表的字段结构及进行添加字段、删除字段、修改字段名、修改字段类型、设置默认值、删除默认值、设置主键和移除主键等操作。

###添加字段
点击页面中的添加字段按钮，填写需要新增的字段名、类型及相关信息。点击弹窗右侧的“+”符号可以添加多个字段。
![添加字段](sac/data_operation/SequoiaSQL-OLTP/add_field.png)

###删除字段
从字段列表中选择需要删除的字段，点击“X”按钮确认无误后点击确定即可删除该字段。
![删除字段](sac/data_operation/SequoiaSQL-OLTP/drop_field.png)

###修改字段名
从字段列表中选择需要修改字段名的字段，点击该行中的编辑图标，选择修改字段名即可对字段名进行修改。
![修改字段名](sac/data_operation/SequoiaSQL-OLTP/rename_field.png)

###修改字段类型
从字段列表中选择需要修改字段类型的字段，点击该行中的编辑图标，选择修改字段类型即可对字段类型进行修改。
![修改字段类型](sac/data_operation/SequoiaSQL-OLTP/retype_field.png)

###设置默认值
从字段列表中选择需要设置默认值的字段，点击该行中的编辑图标，选择设置默认值即可对字段默认值进行设置或者修改。
![设置默认值](sac/data_operation/SequoiaSQL-OLTP/set_default.png)

###删除默认值
从字段列表中选择需要删除默认值的字段，点击该行中的编辑图标，选择删除默认值即可对字段默认值进行删除操作。
![删除默认值](sac/data_operation/SequoiaSQL-OLTP/drop_default.png)

###设置主键
点击页面中的设置主键按钮，选择需要设置为主键的字段，点击确定即可。
![设置主键](sac/data_operation/SequoiaSQL-OLTP/set_primary.png)

###移除主键
点击页面中的移除主键按钮，点击确定即可删除当前表的主键。
![移除主键](sac/data_operation/SequoiaSQL-OLTP/drop_primary.png)

> ***Note：***
>
> 外部表不支持设置主键，只有普通表可以设置。