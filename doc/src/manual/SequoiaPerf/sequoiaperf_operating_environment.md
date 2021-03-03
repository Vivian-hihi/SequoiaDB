本文档主要从资源、环境要求、端口、浏览器、分辨率介绍的 sequoiaPerf 运行环境。


资源
----

每个 SequoiaPerf 实例均需要 8GB 的内存和 2GB 的磁盘，对于部署了[时间序列节点][stp]的数据库可以只需要 1GB 的磁盘。

环境要求
----

为了准确地收集和展示性能数据，SequoiaPerf 对部署环境存在以下需求：

- 需要先完成 SequoiaDB [集群部署][cluster]，且用于部署集群的服务器需要使用 NTP 同步时钟；
- 前端网页所在服务器需要与数据库集群所在的服务器进行时钟同步


端口
----

SequoiaPerf 网页使用的默认端口为 14000。


浏览器
----

SequoiaPerf 可以使用以下浏览器：

* Chrome，支持 Chrome 19.0 以上版本，建议使用当前较新的版本
* Firefox，支持 Firefox 22.0 以上版本，建议使用当前较新的版本
* IE7/8/9+，为了更好使用体验， 建议使用 IE9 或更高版本
* Microsoft Edge，建议使用较新版本
* 其他主流浏览器，建议使用较新的版本；国内浏览器通常是多内核，在不能正常使用可以尝试切换内核


分辨率和缩放
----

* SequoiaPerf 页面需要分辨率不小于 1024*768，为了更好使用体验，建议分辨率在 1366*768 或更高。
* SequoiaPerf 页面支持缩放，支持 80%、90%、100%、125% 的缩放，默认是 100%。


浏览器设置要求
----
* IE 浏览器设置了较高安全级别，需要把网站添加到可信站点。
* 部分浏览器需要关闭特殊模式，如：IE 要关闭兼容模式（兼容模式默认关闭）。
* 浏览器必须允许网站运行 Javascript （默认允许）。


SequoiaDB 集群要求
----

为保障多节点查询匹配和监控信息的准确性，用户在使用 SequoiaPerf 监控 SequoiaDB 集群时，需要在集群所包含的所有服务器中使用 NTP 同步时钟。

同时SequoiaPerf使用时序数据库来记录并保留各种事件和监控数据，为保证时序数据库中信息的连续性和准确性，部署SequoiaPerf的服务器也要求使用NTP与SequoiaDB集群服务器同步时钟。

[^_^]:
    本文使用的所有引用及链接
[cluster]:manual/Deployment/cluster_deployement.md
[stp]:manual/Distributed_Engine/Architecture/Stp/Readme.md
