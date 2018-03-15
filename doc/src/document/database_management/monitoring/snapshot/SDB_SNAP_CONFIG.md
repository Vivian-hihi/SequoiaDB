##描述##

配置快照 SDB_SNAP_CONFIG 列出数据库中指定节点的配置信息。

每一个节点上的配置信息为一条记录。

##标示##

SDB_SNAP_CONFIG

###字段信息###

字段信息详见[数据库配置](database_management/runtime_configuration.md)一节。

##示例##

查看数据组 db1 中数据节点 20000 上的配置信息

```lang-javascript
> db.snapshot( SDB_SNAP_CONFIG, { GroupName:'db1', SvcName:'20000' } )
{
  "confpath": "/home/sequoiadb/conf/local/20000/",
  "dbpath": "/home/equoiadb/20000/",
  "indexpath": "/home/sequoiadb/20000/",
  "diagpath": "/home/sequoiadb/20000/diaglog/",
  "auditpath": "/home/sequoiadb/20000/diaglog/",
  "logpath": "/home/sequoiadb/20000/replicalog/",
  "bkuppath": "/home/sequoiadb/20000/bakfile/",
  "wwwpath": "/home/sequoiadb/web/",
  "lobpath": "/home/sequoiadb/20000/",
  "lobmetapath": "/home/sequoiadb/20000/",
  "maxpool": 50,
  "diagnum": 20,
  "auditnum": 20,
  "auditmask": "SYSTEM|DDL|DCL",
  "svcname": "20000",
  "replname": "20001",
  "catalogname": "20003",
  "shardname": "20002",
  "httpname": "20004",
  "omname": "20005",
  "diaglevel": 3,
  "role": "data",
  "logfilesz": 64,
  "logfilenum": 20,
  "logbuffsize": 1024,
  "numpreload": 0,
  "maxprefpool": 0,
  "maxsubquery": 0,
  "maxreplsync": 10,
  "replbucketsize": 32,
  "syncstrategy": "KeepNormal",
  "preferedinstance": "M",
  "preferedinstancemode": "random",
  "instanceid": 0,
  "dataerrorop": 1,
  "memdebug": "FALSE",
  "memdebugsize": 0,
  "indexscanstep": 100,
  "dpslocal": "FALSE",
  "traceon": "FALSE",
  "tracebufsz": 256,
  "transactionon": "FALSE",
  "transactiontimeout": 60,
  "sharingbreak": 7000,
  "startshifttime": 600,
  "catalogaddr": "ubuntu-zwb:30003,ubuntu-zwb:30013,ubuntu-zwb:30023",
  "tmppath": "/home/sequoiadb/20000/tmp/",
  "sortbuf": 256,
  "hjbuf": 128,
  "directioinlob": "FALSE",
  "sparsefile": "FALSE",
  "weight": 10,
  "auth": "TRUE",
  "planbuckets": 500,
  "optimeout": 300000,
  "overflowratio": 12,
  "extendthreshold": 32,
  "signalinterval": 0,
  "maxcachesize": 0,
  "maxcachejob": 10,
  "maxsyncjob": 10,
  "syncinterval": 10000,
  "syncrecordnum": 0,
  "syncdeep": "FALSE",
  "archiveon": "FALSE",
  "archivecompresson": "TRUE",
  "archivepath": "/home/sequoiadb/20000/archivelog/",
  "archivetimeout": 600,
  "archiveexpired": 240,
  "archivequota": 10,
  "omaddr": "",
  "dmschkinterval": 0,
  "cachemergesz": 0,
  "pagealloctimeout": 0,
  "perfstat": "FALSE",
  "optcostthreshold": 20,
  "enablemixcmp": "FALSE",
  "plancachelevel": 3,
  "maxconn": 0
}
Return 1 row(s).
```
