分区功能使得一张表的存储能分散到多个物理位置，达到更好的并发读写效果。在数据量大时，效果尤为明显。MySQL 提供了四种分区的方式：RANGE 分区、LIST 分区、HASH 分区和KEY 分区。还支持复合分区。

## RANGE 分区
+ **RANGE(\<expression\>)**

    根据表达式进行范围分区。在这种分区中，记录将根据表达式的值的范围，决定坐落的分区。注意，根据表达式的值必须是整数。 例如，将 goods 表根据生产年份划分。

    ```lang-sql
    CREATE TABLE goods (
        id INT NOT NULL,
        produced_date DATE,
        name VARCHAR(100),
        company VARCHAR(100)
    )
    PARTITION BY RANGE (YEAR(produced_date)) (
        PARTITION p0 VALUES LESS THAN (1990),
        PARTITION p1 VALUES LESS THAN (2000),
        PARTITION p2 VALUES LESS THAN (2010),
        PARTITION p3 VALUES LESS THAN (2020)
    );
    ```

+ **RANGE COLUMNS(\<column_list\>)**

    根据列进行范围分区。在这种分区中，记录将根据列的值计算分区。可以指定一个列或多个列，不限制列的类型。一般推荐使用 `RANGE COLUMS(<column_list>)` 代替 `RANGE(<expression>)` 分区。因为它可以达到更好的性能。例如，依然是将 goods 表按生产年份划分。

    ```lang-sql
    CREATE TABLE goods (
        id INT NOT NULL,
        produced_date DATE,
        name VARCHAR(100),
        company VARCHAR(100)
    )
    PARTITION BY RANGE COLUMNS (produced_date) (
        PARTITION p0 VALUES LESS THAN ('1990-01-01'),
        PARTITION p1 VALUES LESS THAN ('2000-01-01'),
        PARTITION p2 VALUES LESS THAN ('2010-01-01'),
        PARTITION p3 VALUES LESS THAN ('2020-01-01')
    );
    ```

    指定多个列时，可以这样描述。

    ```lang-sql
    CREATE TABLE simple (
        a INT,
        b INT,
        c CHAR(3),
        d CHAR(10)
    )
    PARTITION BY RANGE COLUMNS (a, b, c) (
        PARTITION p0 VALUES LESS THAN (10, 100, 'aaa'),
        PARTITION p1 VALUES LESS THAN (20, 200, 'fff'),
        PARTITION p2 VALUES LESS THAN (30, 300, 'lll'),
        PARTITION p3 VALUES LESS THAN (MAXVALUE, MAXVALUE, MAXVALUE)
    );
    ```

## LIST 分区
+ **LIST(\<expression\>)**
    
    根据表达式进行枚举值分区。这种分区中，表达式返回值必须是整数。例如，根据业务标签进行分区。

    ```lang-sql
    CREATE TABLE business (
        id INT NOT NULL,
        start DATE NOT NULL DEFAULT '1970-01-01',
        end DATE NOT NULL DEFAULT '9999-12-31',
        COMMENT VARCHAR(255),
        flag INT
    )
    PARTITION BY LIST (flag) (
        PARTITION p0 VALUES IN (1, 3),
        PARTITION p1 VALUES IN (2, 5, 7),
        PARTITION p2 VALUES IN (4, 6)
    );
    ```

+ **LIST COLUMNS(\<column_list\>)**

    根据列进行枚举值分区。这种分区中，可以指定一个列或多个列，不限定列的类型。例如，根据业务得分进行分区。

    ```lang-sql
    CREATE TABLE business (
        id INT NOT NULL,
        start DATE NOT NULL DEFAULT '1970-01-01',
        end DATE NOT NULL DEFAULT '9999-12-31',
        COMMENT VARCHAR(255),
        score CHAR(1)
    )
    PARTITION BY LIST COLUMNS (score) (
        PARTITION good VALUES IN ('S', 'A'),
        PARTITION normal VALUES IN ('B', 'C'),
        PARTITION fail VALUES IN ('D')
    );
    ```

## HASH 分区
+ **HASH(\<expression\>)**

    根据表达式中的值计算 hash 值从而均匀打散记录的一种分区。表达式的返回值必须为整数。由于 SequoiaDB 有独立的 hash 算法。所以这种分区下，SequoiaDB 只取表达式中的列进行计算。一般推荐使用 `KEY(<column_list>)` 代替 `HASH(<expression>)` 语法。例如这个语句

    ```lang-sql
    CREATE TABLE goods (
        id INT NOT NULL,
        produced_date DATE,
        name VARCHAR(100),
        company VARCHAR(100)
    )
    PARTITION BY HASH (YEAR(produced_date))
    PARTITIONS 4;
    ```

    等价于对应的 KEY 分区

    ```lang-sql
    CREATE TABLE goods (
        id INT NOT NULL,
        produced_date DATE,
        name VARCHAR(100),
        company VARCHAR(100)
    )
    PARTITION BY KEY (produced_date)
    PARTITIONS 4;
    ```

    `PARTITION BY LINEAR HASH` 与 `PARTITION BY HASH` 效果等同。例子中 `PARTITION 4` 参数是无意义的。分区实际是按 SequoiaDB 的规则自动切分到对应的数据组中。

## KEY 分区
+ **KEY(\<column_list\>)**

    根据指定的列计算 hash 值，从而均匀打散记录的一种分区。可以指定一个列或多个列，不限制列的类型。例如，根据货物 id 进行分区。

    ```lang-sql
    CREATE TABLE goods (
        id INT NOT NULL,
        produced_date DATE,
        name VARCHAR(100),
        company VARCHAR(100)
    )
    PARTITION BY KEY (id)
    PARTITIONS 4;
    ```

    由于 SequoiaDB 引擎有独立的 hash 算法。`PARTITION BY LINEAR KEY` 与 `PARTITION BY KEY` 效果等同，均是使用 SequoiaDB 的算法。而例子中 `PARTITION 4` 参数是无意义的。分区实际是按 SequoiaDB 的规则自动切分到对应的数据组中。上一个例子等同于以下的语句。

    ```lang-sql
    CREATE TABLE goods (
        id INT NOT NULL,
        produced_date DATE,
        name VARCHAR(100),
        company VARCHAR(100)
    ) 
    COMMENT='sequoiadb:{ table_options: { ShardingKey: { id: 1 }, ShardingType: "hash", AutoSplit: true } }'
    ```

## 复合分区

复合分区中，上层分区必须使用 RANGE 或者 LIST 分区，而下层分区则必须使用 HASH 或者 KEY 分区。例如，在 goods 表上，先根据 produced_date 做范围分区，再对每个范围分区按 id 做哈希分区。

```lang-sql
CREATE TABLE goods (
    id INT NOT NULL,
    produced_date DATE,
    name VARCHAR(100),
    company VARCHAR(100)
)
PARTITION BY RANGE COLUMNS (produced_date)
SUBPARTITION BY KEY (id)
SUBPARTITIONS 2 (
    PARTITION p0 VALUES LESS THAN ('1990-01-01'),
    PARTITION p1 VALUES LESS THAN ('2000-01-01'),
    PARTITION p2 VALUES LESS THAN ('2010-01-01'),
    PARTITION p3 VALUES LESS THAN ('2020-01-01')
);
```

## 约束与限制

* 不支持指定特定 HASH 分区的操作。

* 不支持从 INFORMATION_SCHEMA.PARTITIONS 表查询 HASH 各个分区具体记录数。

* 不支持自增字段做 LIST / RANGE 的分区字段

* 不支持 EXCHANGE PARTITION 操作。

* 不支持 CHECK PARTITION 和 REPAIR PARTITION 操作。

* RANGE COLUMNS 有多个列时，不能指定分区操作。
