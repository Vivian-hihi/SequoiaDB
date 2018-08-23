字段结构页面可以查看当前数据表的字段结构及进行添加字段、删除字段、修改字段名、修改字段类型、设置默认值、删除默认值、设置主键和移除主键等操作。

###添加字段
点击页面中的添加字段按钮，填写需要新增的字段名、类型及相关信息。点击弹窗右侧的“+”符号可以添加多个字段。
![添加字段](sac/data_operation/MySQL/add_field.png)

###删除字段
从字段列表中选择需要删除的字段，点击“X”按钮确认无误后点击确定即可删除该字段。
![删除字段](sac/data_operation/MySQL/drop_field.png)

###修改字段
从字段列表中选择需要修改字段名的字段，点击该行中的编辑图标，选择修改字段即可对字段进行修改。
![修改字段](sac/data_operation/MySQL/rename_field.png)

###设置默认值
从字段列表中选择需要设置默认值的字段，点击该行中的编辑图标，选择设置默认值即可对字段默认值进行设置或者修改。
![设置默认值](sac/data_operation/MySQL/set_default.png)

###删除默认值
从字段列表中选择需要删除默认值的字段，点击该行中的编辑图标，选择删除默认值即可对字段默认值进行删除操作。
![删除默认值](sac/data_operation/MySQL/drop_default.png)

###创建索引
点击页面中的索引操作按钮、创建索引，选择需要设置的字段，点击确定即可。
![创建索引](sac/data_operation/MySQL/create_index.png)

###删除索引
点击页面中的索引操作按钮、删除索引，选择需要删除的索引，点击确定即可删除。
![删除索引](sac/data_operation/MySQL/drop_index.png)

###设置主键
点击页面中的索引操作按钮、设置主键，选择需要设置为主键的字段，点击确定即可。
![设置主键](sac/data_operation/MySQL/set_primary.png)

> **Note:**  
> 如果建表时设置了分区键，设置的主键必须包含分区键的字段。

###移除主键
点击页面中的索引操作按钮、移除主键，点击确定即可删除当前表的主键。
![移除主键](sac/data_operation/MySQL/drop_primary.png)