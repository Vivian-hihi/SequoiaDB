## 数据类型映射表

MariaDB 支持多种 SQL 数据类型：数值类型、Date 类型、Time 类型和字符串类型等。

从 MariaDB 实例到 SequoiaDB 的 JSON 对象实例之间的数据类型映射关系为：

| MariaDB 实例 | JSON 对象实例| 备注                                       |
| ---------- | ------------ | -------------------------------------------- |
| BIT        | Int/Long     | 超出 Int 范围则按 Long 存储                  |
| BOOL       | Int          |                                              |
| TINYINT    | Int          |                                              |
| SMALLINT   | Int          |                                              |
| MEDIUMINT  | Int          |                                              |
| INT        | Int/Long     | 超出 Int 范围则按 Long 存储                  |
| BIGINT     | Long/Decimal | 超出 Long 范围则按 Decimal 存储              |
| FLOAT      | Double       |                                              |
| DOUBLE     | Double       |                                              |
| DECIMAL    | Decimal      |                                              |
| YEAR       | Int          |                                              |
| DATE       | Date         |                                              |
| TIME       | Decimal      | 'HHMMSS[.fraction]'格式的 Decimal 值         |
| DATETIME   | String       | 'YYYY-MM-DD HH:MM:SS[.fraction]'格式的字符串 |
| TIMESTAMP  | Timestamp    |                                              |
| CHAR       | String       |                                              |
| VARCHAR    | String       |                                              |
| BINARY     | Binary       |                                              |
| VARBINARY  | Binary       |                                              |
| TINYBLOB   | Binary       |                                              |
| BLOB       | Binary       |                                              |
| MEDIUMBLOB | Binary       |                                              |
| LONGBLOB   | Binary       | 最大长度16MB                                 |
| TINYTEXT   | String       |                                              |
| TEXT       | String       |                                              |
| MEDIUMTEXT | String       |                                              |
| LONGTEXT   | String       | 最大长度16MB                                 |
| ENUM       | Int          |                                              |
| SET        | Int          |                                              |
| JSON       | String       |                                              |
| GEOMETRY   | Binary       | 不支持索引（spatial/non-spatial 索引）       |
| NULL       | -            | 不存储                                       |
