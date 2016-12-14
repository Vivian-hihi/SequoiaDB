sdbtop 是一个 SequoiaDB 数据库的性能监控工具。通过它，可以监控和查看集群中各个节点的监视信息。

##选项##

参数            缩写   描述
--------------- ------ ---------------------------------------------------------------------------------
--help          -h     返回基本帮助和用法文本
--confpath      -c     sdbtop 的配置文件，sdbtop 界面形态以及输出字段都依赖该文件（缺省使用默认配置文件）
--hostname      -i     指定需要监控的主机名
--servicename   -s     指定监控的端口服务名
--usrname       -u     数据库用户名
--password      -p     数据库密码
--ssl                  使用 SSL 连接。

**Note: **

对于 Ubuntu 等系统，需要安装 Ncurses 库，否则将会提示“Error opening terminal: TERM”

方式一： 联网安装

<pre class="prettyprint lang-javascript">
$ sudo apt-get install libncurses5-dev</pre>

方式二： 源码安装

解压 tar -xvzf ncurses-5.5.tar.gz，进入 ncurses-5.5 目录

<pre class="prettyprint lang-javascript">
$ ./configure
$ sudo make && make install</pre>

##用法##

在下面的例子，sdbtop 使用配置文件为“/opt/sequoiadb/conf/sample/sdbtop.xml”，监控主机名为 sdbserver3，端口服务名为11810，用户名为 test，密码为 test 的数据库集群中的一个节点。

<pre class="prettyprint lang-javascript">
$ sdbtop -c /opt/sequoiadb/conf/sample/sdbtop.xml -i sdbserver3 -s 11810 -u test -p test</pre>

接着进入主窗口：

<pre class="prettyprint lang-diy">
refresh= 3 secs           sdbtop 1.0       snapshotMode: GLOBAL
displayMode: ABSOLUTE     Main Window      snapshotModeInput: NULL
hostname: sdbserver3                       filtering Number: 0
servicename: 11810                         sortingWay: NULL sortingField: NULL
usrName: test                              Refresh: F5, Quit: q, Help: h

 #####  ######  ######  #######  #####  ######   For help type h or ...
#       #     # #     #    #    #     # #     #  sdbtop -h: usage
#       #     # #     #    #    #     # #     #
 #####  #     # ######     #    #     # ######
      # #     # #     #    #    #     # #
      # #     # #     #    #    #     # #
 #####  ######  ######     #     #####  #

SDB Interactive Snapshot Monitor V2.0
Use these keys to navigate:
  m   -  Main Window            s   -  Sessions               c   -  CollectionSpaces
  t   -  System                 d   -  Database               G   -  GLOBAL_SNAPSHOT
  g   -  GROUP_SNAPSHOT         n   -  NODE_SNAPSHOT          r   -  reset refreshInterval
  A   -  Ascending order        D   -  Descending order       C   -  filter condition
  Q   -  no filter condition    N   -  filter number          W   -  no filter number
Licensed Materials - Property of SequoiaDB
Copyright SequoiaDB Corp. 2013-2014 All Rights Reserved.</pre>

**Note:** 在主窗口中按‘**h**’键可以查看所有工具支持的按键

-   主窗口按键说明

参数    描述
------- ---------------------------------------------------
m       返回主窗口
s       列出数据库节点上的所有会话
c       列出数据库节点上的所有集合空间
t       列出数据库节点上的系统资源使用情况
d       列出数据库节点的数据库监视信息
G       global_snapshot，监控所有的数据节点组
g       group_snapshot，指定监控某个数据节点组
n       node_snapshot，列出指定的数据库节点的监视信息
r       设置刷屏的时间间隔，单位秒/s
A       将监视信息按照某列进行顺序排序
D       将监视信息按照某列进行逆序排序
C       将监视信息按照某个条件进行筛选
Q       返回没有使用条件进行筛选前的监视信息
N       将监视信息中对应行号的记录过滤不显示
W       返回没有使用行号进行过滤前的监视信息
h       查看使用帮助
Esc     取消已进入的操作
Enter   返回上一次监视界面，（在已进入 help 帮助输出中有效）
F5      强制刷新后台监视信息
<       向左移动，以查看隐藏的左边列的监视信息
\>      向右移动，以查看隐藏的右边列的监视信息
q       退出程序
Tab     切换数据计算的模式（绝对值，平均值，差值三个模式）

**例如：**

1.进入主窗口后，按‘s’键，列出数据库节点的所有会话信息

<pre class="prettyprint lang-diy">
refresh= 3 secs              sdbtop 1.0         snapshotMode: GLOBAL
displayMode: ABSOLUTE        Sessions           snapshotModeInput: NULL
hostname: sdbserver3                            filtering Number: 0
servicename: 11810                              sortingWay: NULL sortingField: NULL
usrName: test                                   Refresh: F5, Quit: q, Help: h

SessionID                           TID Type               Name
&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;   &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;    &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;    &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;
1  sdbserver3:11820:1                10732 LogWriter          ""
2  sdbserver3:11820:10               10741 Task               Job[Prefetcher]
3  sdbserver3:11820:11               10742 Task               Job[Prefetcher]
4  sdbserver3:11820:12               10743 Task               Job[Prefetcher]
5  sdbserver3:11820:13               10744 Cluster            ""
6  sdbserver3:11820:14               10745 ClusterShard       ""
7  sdbserver3:11820:15               10746 ClusterLogNotify   ""
8  sdbserver3:11820:16               10747 ShardReader        ""
9  sdbserver3:11820:17               10748 ReplReader         ""
10  sdbserver3:11820:18              10749 SyncClockWorker    ""
11  sdbserver3:11820:19              10750 TCPListener        ""
12  sdbserver3:11820:2               10733 DpsRollback        ""
13  sdbserver3:11820:20              10751 RestListener       ""
14  sdbserver3:11820:21              10752 Task               Job[PageCleaner]
15  sdbserver3:11820:3               10734 Task               Job[Prefetcher]
16  sdbserver3:11820:4               10735 Task               Job[Prefetcher]
17  sdbserver3:11820:42              10847 ReplAgent          NodeID:1000,TID:1,Start:active
18  sdbserver3:11820:5               10736 Task               Job[Prefetcher]
19  sdbserver3:11820:59              23263 ShardAgent         NetID:1,TID:23262
20  sdbserver3:11820:6               10737 Task               Job[Prefetcher]
21  sdbserver3:11820:7               10738 Task               Job[Prefetcher]
22  sdbserver3:11820:8               10739 Task               Job[Prefetcher]</pre>

2.按‘Tab’键，可以看到屏幕左上方的‘displayMode’的值会发生切换

3.按‘r’键，在屏幕最下方输入‘2’，回车，设置刷新间隔时间，可以看到屏幕左上方的‘refresh’的值变为 2

<pre class="prettyprint lang-diy">
refresh= 2 secs              sdbtop 1.0         snapshotMode: GLOBAL
displayMode: ABSOLUTE        Sessions           snapshotModeInput: NULL
hostname: sdbserver3                            filtering Number: 0
servicename: 11810                              sortingWay: NULL sortingField: NULL
usrName: test                                   Refresh: F5, Quit: q, Help: h

SessionID                           TID Type               Name
&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;   &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;    &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;    &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;
1  sdbserver3:11820:1                10732 LogWriter          ""
2  sdbserver3:11820:10               10741 Task               Job[Prefetcher]
3  sdbserver3:11820:11               10742 Task               Job[Prefetcher]
4  sdbserver3:11820:12               10743 Task               Job[Prefetcher]
5  sdbserver3:11820:13               10744 Cluster            ""
6  sdbserver3:11820:14               10745 ClusterShard       ""
7  sdbserver3:11820:15               10746 ClusterLogNotify   ""
8  sdbserver3:11820:16               10747 ShardReader        ""
9  sdbserver3:11820:17               10748 ReplReader         ""
10  sdbserver3:11820:18              10749 SyncClockWorker    ""
11  sdbserver3:11820:19              10750 TCPListener        ""
12  sdbserver3:11820:2               10733 DpsRollback        ""
13  sdbserver3:11820:20              10751 RestListener       ""
14  sdbserver3:11820:21              10752 Task               Job[PageCleaner]
15  sdbserver3:11820:3               10734 Task               Job[Prefetcher]
16  sdbserver3:11820:4               10735 Task               Job[Prefetcher]
17  sdbserver3:11820:42              10847 ReplAgent          NodeID:1000,TID:1,Start:active
18  sdbserver3:11820:5               10736 Task               Job[Prefetcher]
19  sdbserver3:11820:59              23263 ShardAgent         NetID:1,TID:23262
20  sdbserver3:11820:6               10737 Task               Job[Prefetcher]
21  sdbserver3:11820:7               10738 Task               Job[Prefetcher]
22  sdbserver3:11820:8               10739 Task               Job[Prefetcher]</pre>

4.按‘A’键，并输入‘TID’，列表结果按照 TID 进行顺序排序

<pre class="prettyprint lang-diy">
refresh= 2 secs              sdbtop 1.0         snapshotMode: GLOBAL
displayMode: ABSOLUTE        Sessions           snapshotModeInput: NULL
hostname: sdbserver3                            filtering Number: 0
servicename: 11810                              sortingWay: NULL sortingField: NULL
usrName: test                                   Refresh: F5, Quit: q, Help: h

SessionID                           TID Type               Name
&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;   &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;    &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;    &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;
1  sdbserver3:11820:1                10732 LogWriter          ""
2  sdbserver3:11820:10               10741 Task               Job[Prefetcher]
3  sdbserver3:11820:11               10742 Task               Job[Prefetcher]
4  sdbserver3:11820:12               10743 Task               Job[Prefetcher]
5  sdbserver3:11820:13               10744 Cluster            ""
6  sdbserver3:11820:14               10745 ClusterShard       ""
7  sdbserver3:11820:15               10746 ClusterLogNotify   ""
8  sdbserver3:11820:16               10747 ShardReader        ""
9  sdbserver3:11820:17               10748 ReplReader         ""
10  sdbserver3:11820:18              10749 SyncClockWorker    ""
11  sdbserver3:11820:19              10750 TCPListener        ""
12  sdbserver3:11820:2               10733 DpsRollback        ""
13  sdbserver3:11820:20              10751 RestListener       ""
14  sdbserver3:11820:21              10752 Task               Job[PageCleaner]
15  sdbserver3:11820:3               10734 Task               Job[Prefetcher]
16  sdbserver3:11820:4               10735 Task               Job[Prefetcher]
17  sdbserver3:11820:42              10847 ReplAgent          NodeID:1000,TID:1,Start:active
18  sdbserver3:11820:5               10736 Task               Job[Prefetcher]
19  sdbserver3:11820:59              23263 ShardAgent         NetID:1,TID:23262
20  sdbserver3:11820:6               10737 Task               Job[Prefetcher]
21  sdbserver3:11820:7               10738 Task               Job[Prefetcher]
please input the displayName which need order by asc : TID</pre>

5.按‘N’键，并输入1，列表中将原来行号为1的记录过滤不显示

6.按‘W’键，返回没有按行号进行过滤前的列表信息

7.按‘C’键，并输入‘TID：10732”进行筛选，则只显示 TID 值为10732的记录

<pre class="prettyprint lang-diy">
refresh= 2 secs              sdbtop 1.0         snapshotMode: GLOBAL
displayMode: ABSOLUTE        Sessions           snapshotModeInput: NULL
hostname: sdbserver3                            filtering Number: 0
servicename: 11810                              sortingWay: NULL sortingField: NULL
usrName: test                                   Refresh: F5, Quit: q, Help: h

SessionID                           TID Type               Name
&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;   &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;    &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;    &#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;&#45;
1  sdbserver3:11820:1                10732 LogWriter          ""
2  sdbserver3:11820:10               10741 Task               Job[Prefetcher]
3  sdbserver3:11820:11               10742 Task               Job[Prefetcher]
4  sdbserver3:11820:12               10743 Task               Job[Prefetcher]
5  sdbserver3:11820:13               10744 Cluster            ""
6  sdbserver3:11820:14               10745 ClusterShard       ""
7  sdbserver3:11820:15               10746 ClusterLogNotify   ""
8  sdbserver3:11820:16               10747 ShardReader        ""
9  sdbserver3:11820:17               10748 ReplReader         ""
10  sdbserver3:11820:18              10749 SyncClockWorker    ""
11  sdbserver3:11820:19              10750 TCPListener        ""
12  sdbserver3:11820:2               10733 DpsRollback        ""
13  sdbserver3:11820:20              10751 RestListener       ""
14  sdbserver3:11820:21              10752 Task               Job[PageCleaner]
15  sdbserver3:11820:3               10734 Task               Job[Prefetcher]
16  sdbserver3:11820:4               10735 Task               Job[Prefetcher]
17  sdbserver3:11820:42              10847 ReplAgent          NodeID:1000,TID:1,Start:active
18  sdbserver3:11820:5               10736 Task               Job[Prefetcher]
19  sdbserver3:11820:59              23263 ShardAgent         NetID:1,TID:23262
20  sdbserver3:11820:6               10737 Task               Job[Prefetcher]
21  sdbserver3:11820:7               10738 Task               Job[Prefetcher]
please input the filter condition : TID:10732</pre>

8.按‘Q’键，返回没有按照筛选条件前的列表信息

9.按‘<’或者‘>’键，可以查看隐藏在左边或者右边的列
