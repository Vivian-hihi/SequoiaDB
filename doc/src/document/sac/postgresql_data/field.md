**表结构** 页面可以对当前数据表添加字段、删除字段、修改字段名、修改字段类型等操作。

添加字段
---

点击 **添加字段**，填好好参数，点击 **确定** 按钮。

点击 **+** 符号可以添加多个字段。

![添加字段](sac/data_operation/PostgreSQL/add_field.png)

删除字段
---

点击表格中字段的 **X**，点击 **确定** 按钮。
![删除字段](sac/data_operation/PostgreSQL/drop_field.png)

修改字段名
---

从字段列表中选择需要修改字段名的字段，点击该行中的编辑图标，选择修改字段名即可对字段名进行修改。
![修改字段名](sac/data_operation/PostgreSQL/rename_field.png)

修改字段类型
---

从字段列表中选择需要修改字段类型的字段，点击该行中的编辑图标，选择修改字段类型即可对字段类型进行修改。
![修改字段类型](sac/data_operation/PostgreSQL/retype_field.png)

###设置默认值
从字段列表中选择需要设置默认值的字段，点击该行中的编辑图标，选择设置默认值即可对字段默认值进行设置或者修改。
![设置默认值](sac/data_operation/PostgreSQL/set_default.png)

###删除默认值
从字段列表中选择需要删除默认值的字段，点击该行中的编辑图标，选择删除默认值即可对字段默认值进行删除操作。
![删除默认值](sac/data_operation/PostgreSQL/drop_default.png)

###创建索引
点击页面中的索引操作按钮、创建索引，选择需要设置的字段，点击确定即可。
![创建索引](sac/data_operation/PostgreSQL/create_index.png)

###删除索引
点击页面中的索引操作按钮、删除索引，选择需要删除的索引，点击确定即可删除。
![删除索引](sac/data_operation/PostgreSQL/drop_index.png)

###设置主键
点击页面中的索引操作按钮、设置主键，选择需要设置为主键的字段，点击确定即可。
![设置主键](sac/data_operation/PostgreSQL/set_primary.png)

###移除主键
点击页面中的索引操作按钮、移除主键，点击确定即可删除当前表的主键。
![移除主键](sac/data_operation/PostgreSQL/drop_primary.png)

> ***Note：***
>
> 外部表不支持索引操作，只有普通表可以设置。