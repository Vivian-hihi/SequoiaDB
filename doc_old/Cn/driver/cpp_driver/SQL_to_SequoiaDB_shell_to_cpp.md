SequoiaDB 的查询用 json（bson）对象表示，下表以例子的形式显示了 SQL 语句，SequoiaDB shell 语句和 SequoiaDB C++ 驱动程序语法之间的对照。

+-----------------------------------------------------------------+-----------------------------------------------------+--------------------------------------------------------------------+
| SQL                                                             | SequoiaDB shell                                     | C++ Driver                                                         |
+=================================================================+=====================================================+====================================================================+
| insert into students(a,b) values(1,-1)                          | db.foo.bar.insert({a:1,b:-1})                       |                                                                    |
|                                                                 |                                                     | -    sdbCollection collection ;                                    |
|                                                                 |                                                     | -    BSONObj obj = BSON("a" << 1 << "b" << -1) ;                   |
|                                                                 |                                                     | -    collection.insert( obj ) ;                                    |
+-----------------------------------------------------------------+-----------------------------------------------------+--------------------------------------------------------------------+
| select a,b from students                                        | db.foo.bar.find(null,{a:"",b:""})                   |                                                                    |
|                                                                 |                                                     | -    sdbCollection collection ;                                    |
|                                                                 |                                                     | -    sdbCursor cursor ;                                            |
|                                                                 |                                                     | -    BSONObj selected ;                                            |
|                                                                 |                                                     | -    BSONObj obj ;                                                 |
|                                                                 |                                                     | -    selected = BSON( "a"<<""<<"b"<<"" ) ;                         |
|                                                                 |                                                     | -    collection .query( cursor, obj, selected ) ;                  |
+-----------------------------------------------------------------+-----------------------------------------------------+--------------------------------------------------------------------+
|                                                                 |                                                     |                                                                    |
| select * from students                                          | db.foo.bar.find()                                   | -    sdbCollection collection ;                                    |
|                                                                 |                                                     | -    sdbCursor cursor ;                                            |
|                                                                 |                                                     | -    collection .query ( cursor ) ;                                |
+-----------------------------------------------------------------+-----------------------------------------------------+--------------------------------------------------------------------+
|                                                                 |                                                     |                                                                    |
| select * from students where age=20                             | db.foo.bar.find({age:20})                           | -    sdbCollection collection ;                                    |
|                                                                 |                                                     | -    sdbCursor cursor ;                                            |
|                                                                 |                                                     | -    BSONObj condition ;                                           |
|                                                                 |                                                     | -    condition = BSON("age" << 20) ;                               |
|                                                                 |                                                     | -    collection .query ( cursor, condition ) ;                     |
+-----------------------------------------------------------------+-----------------------------------------------------+--------------------------------------------------------------------+
|                                                                 |                                                     |                                                                    |
| select * from students where age=20 order by name               | db.foo.bar.find({age:20}).sort({name:1})            | -    sdbCollection collection ;                                    |
|                                                                 |                                                     | -    sdbCursor cursor ;                                            |
|                                                                 |                                                     | -    BSONObj obj ;                                                 |
|                                                                 |                                                     | -    BSONObj condition ;                                           |
|                                                                 |                                                     | -    BSONObj orderBy ;                                             |
|                                                                 |                                                     | -    condition = BSON("age"<<20) ;                                 |
|                                                                 |                                                     | -    orderBy = BSON("name"<<1) ;                                   |
|                                                                 |                                                     | -    collection .query (cursor, condition , obj, orderBy , obj ) ; |
+-----------------------------------------------------------------+-----------------------------------------------------+--------------------------------------------------------------------+
|                                                                 |                                                     |                                                                    |
| select * from students where age > 20 and age < 30              | db.foo.bar.find({age:{\$gt:20,$lt:30}})             | -    sdbCollection collection ;                                    |
|                                                                 |                                                     | -    sdbCursor cursor ;                                            |
|                                                                 |                                                     | -    BSONObj condition ;                                           |
|                                                                 |                                                     | -    condition = BSON("age"<<BSON("$gt"<<20<<" $lt"<<30)) ;        |
|                                                                 |                                                     | -    collection .query (cursor, condition ) ;                      |
+-----------------------------------------------------------------+-----------------------------------------------------+--------------------------------------------------------------------+
|                                                                 |                                                     |                                                                    |
| create index testIndex on students(name)                        | db.foo.bar.createIndex("testIndex",{name:1},false)  | -    sdbCollection collection ;                                    |
|                                                                 |                                                     | -    BSONObj obj ;                                                 | 
|                                                                 |                                                     | -    obj = BSON( "name"<<1 ) ;                                     |
|                                                                 |                                                     | -    collection.createIndex ( &obj, "testIndex", FALSE, FALSE );   |
+-----------------------------------------------------------------+-----------------------------------------------------+--------------------------------------------------------------------+
|                                                                 |                                                     |                                                                    |
| select * from students limit 20 skip 10                         | db.foo.bar.find().limit(20).skip(10)                | -    sdbCollection collection ;                                    |
|                                                                 |                                                     | -    BSONObj obj ;                                                 |
|                                                                 |                                                     | -    sdbCursor cursor ;                                            |
|                                                                 |                                                     | -    collection .query (cursor,obj, obj, obj, obj, 10, 20 ) ;      |
+-----------------------------------------------------------------+-----------------------------------------------------+--------------------------------------------------------------------+
|                                                                 |                                                     |                                                                    |
| select count(*) from students where age > 20                    | db.foo.bar.find({age:{$gt:20}}).count()             | -    sdbCollection collection ;                                    |
|                                                                 |                                                     | -    SINT64 count = 0 ;                                            |
|                                                                 |                                                     | -    BSONObj condition ;                                           |
|                                                                 |                                                     | -    Condition = BSON( "age"<<BSON("$gt"<<20)) ;                   |
|                                                                 |                                                     | -    collection.getCount (count, condition );                      |
+-----------------------------------------------------------------+-----------------------------------------------------+--------------------------------------------------------------------+
|                                                                 |                                                     |                                                                    |
| update students set a=a+2 where b=-1                            | db.foo.bar.update({$set:{a:2}},{b:-1})              | -    sdbCollection collection ;                                    |
|                                                                 |                                                     | -    BSONObj condition = BSON( "b"<<1 ) ;                          |
|                                                                 |                                                     | -    BSONObj rule = BSON( "$set"<<BSON("a"<<2) );                  |
|                                                                 |                                                     | -    BSONObj obj ;                                                 |
|                                                                 |                                                     | -    collection.update ( rule, condition, obj ) ;                  |
+-----------------------------------------------------------------+-----------------------------------------------------+--------------------------------------------------------------------+
|                                                                 |                                                     |                                                                    |
| delete from students where a=1                                  | db.foo.bar.remove({a:1})                            | -    sdbCollection collection ;                                    |
|                                                                 |                                                     | -    BSONObj condition = BSON("a"<<1) ;                            |
|                                                                 |                                                     | -    collection.del ( condition ) ;                                |
+-----------------------------------------------------------------+-----------------------------------------------------+--------------------------------------------------------------------+

