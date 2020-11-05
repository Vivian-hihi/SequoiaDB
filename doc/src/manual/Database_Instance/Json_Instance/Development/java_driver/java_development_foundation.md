本文档主要介绍如何使用 Java 驱动接口编写使用 SequoiaDB 巨杉数据库的程序。下述为 SequoiaDB 巨杉数据库 Java 驱动的简单示例，示例中的代码可能不完整，用户可在 `/sequoiadb/samples/java` 目录下获取相应的完整代码。

##数据操作##

* 连接数据库

   ```lang-java
   package com.sequoiadb.samples;
   import com.sequoiadb.base.DBCursor;
   import com.sequoiadb.base.Sequoiadb;
   import com.sequoiadb.exception.BaseException;
   public class Sample {
         public static void main(String[] args) {
             String connString = "192.168.1.2:11810";
               try {
     	          // 建立 SequoiaDB 数据库连接
                   Sequoiadb sdb = new Sequoiadb(connString, "", "");
                   // 获取所有 Collection 信息，并打印
                   DBCursor cursor = sdb.listCollections();
                   try {
                       while(cursor.hasNext()) {
                           System.out.println(cursor.getNext());
                       }
                   } finally {
                       cursor.close();
                   }
               } catch (BaseException e) {
                   System.out.println("Sequoiadb driver error, error description:" + e.getErrorType());
               }
          }
   }
   ```
 
   >**Note:**
   >
   >- 本示例连接到本地数据库的 11810 端口，使用的是空的用户名和密码。用户可以根据实际情况配置参数。
   >- SequoiaDB 类为非线程安全类，每个线程必须单独建立自己的 SequoiaDB 对象，不能传递给多个线程同时操作。

* 插入数据

   ```lang-java
   String connString = "192.168.1.2:11810";
   try {
         Sequoiadb sdb = new Sequoiadb(connString, "", "");
         CollectionSpace db = sdb.createCollectionSpace("space");
         DBCollection cl = db.createCollection("collection");
         // 创建一个插入的 bson 对象
         BSONObject obj = new BasicBSONObject();
         obj.put("name", "tom");
         obj.put("age", 24);
         cl.insert(obj);
   } catch (BaseException e) {
 	    System.out.println("Sequoiadb driver error, error description:" + e.getErrorType());
   }
   ```

* 查询

   ```lang-java
   // 定义一个游标对象
   DBCursor cursor;
   BSONObject queryCondition = new BasicBSONObject();
   queryCondition = (BSONObject) JSON.parse("{age:{$ne:20}}");
   // 查询所有记录，并把查询结果放在游标对象中
   cursor = cl.query(queryCondition, null, null, null);
   // 从游标中显示所有记录
   try {
         while (cursor.hasNext()) {
            BSONObject record = cursor.getNext();
            String name = (String) record.get("name");
            System.out.println("name=" +  name);
         } 
   } finally {
      cursor.close();   
   }
   ```

   >**Note:**
   >
   > - 此示例中设置了简单的查询条件，实际上还可以设置筛选条件、排序情况、仅使用默认索引等选项。
   > - 游标对象将数据表中的部分数据缓存在本地进程的内存中，如果本地数据读取完了，游标对象会通过网络从服务器再次获取部分数据缓存在本地。

##集群操作##

* 创建复制组

   ```lang-java
   String connString = "192.168.1.2:11810";
   try {
         Sequoiadb sdb = new Sequoiadb(connString, "", "");
         ReplicaGroup rg = sdb.createRG("group1");
         rg.createNode("dbserver-1", 11820, "/opt/sequoiadb/database/data/11820", null);
         rg.start();
   } catch (BaseException e) {
         System.out.println("Sequoiadb driver error, error description" + e.getErrorType());
   }
   ```

   >**Note:**
   >
   > - rg.createNode() 方法的第一个参数为新增节点所在的主机名（暂时不支持使用 IP 地址），第三个参数为数据文件存放路径，SequoiaDB 将自动新建该目录，但需要确保 SequoiaDB 管理员用户（默认 sdbadmin）有写权限。
   > - rg.start() 方法用于启动一个复制组的所有节点，该函数一般需要等待 10s 左右才可完成。该方法不保证复制组选举完成，为了保证复制组可以正常使用，start 完成后，还需要等待 30s 才可以正常使用新建的复制组。

* 在复制组增加节点

   ```lang-java
   String connString = "192.168.1.2:11810";
   try {
         Sequoiadb sdb = new Sequoiadb(connString,"","");
         ReplicaGroup rg = sdb.getReplicaGroup("group1");
         Node node = rg.createNode("dbserver-1", 11830, "/var/sequoiadb/database/data/11830", null);
         node.start();
   } catch (BaseException e) {
         System.out.println("Sequoiadb driver error, error description" + e.getErrorType());
   }
   ```
