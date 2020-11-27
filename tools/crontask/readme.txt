数据重分布时间定制工具

1、介绍
用于定时执行数据均衡或集群缩容等任务。
在tools/crontask目录下有三个文件：

1.1、sdbtaskdaemon
执行定时任务的守护进程。它会每分钟检查是否有要执行的任务，如果有则调用js脚本后台执行任务。

1.2、sdbtaskctl
管理定时任务的工具。它用于创建、删除、查看定时任务。目前可指定两种类型的任务：数据均衡和集群缩容。

1.2.1、数据均衡
使集合上的数据均匀地分布在对应的数据组上。集合对应的集合空间，必须落在某个域上，域对应的一个或多个数据组，集合上的数据要在这些数据组上均衡。
数据均衡的条件：range 分区表，各个数据组之间的数据大小小于均衡单位（默认 100 MB）；hash 分区表，各个数据组之间的 partition 个数小于等于 2。

1.2.2、集群缩容
使指定主机上的数据迁移到其他主机上，以支持用户后续继续做集群缩容。它只迁移数据，并不会把对应的数据组从集群中删除。

1.2.3、dataRebalance.js
daemon进程执行任务的脚本。可用于数据均衡和集群缩容。

2、使用

切换到工具的目录下
cd /opt/sequoiadb/tools/crontask

运行守护进程
setsid ./sdbtaskdaemon >> ./sdbtaskdaemon.log 2>&1

创建重分布任务，指定每天晚上 11 点定时做数据均衡
./sdbtaskctl create task1 -t rebalance -b 23:00 -f daily -l foo.bar

创建缩容任务，指定运行时间为晚上 11 点
./sdbtaskctl create task2 -t shrink -b 23:00 -f once --hosts host1,host2,host3

查看未执行的任务
./sdbtaskctl list

查看执行过的任务，从daemon的日志中查看：
vi sdbtaskdaemon.log

查看具体任务 task1 的执行情况，从任务日志中查看：
vi /opt/sequoiadb/conf/log/crontask/task1.log