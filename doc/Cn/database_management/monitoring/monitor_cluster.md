1.连接到协调节点

<pre class="prettyprint lang-javascript">
$ /opt/sequoiadb/bin/sdb
> var db = new Sdb("localhost", 11810) ;</pre>

2.得到集群状态

<pre class="prettyprint lang-javascript">
> db.listReplicaGroups()
{
  "Group": [
    {
      "dbpath": "/opt/sequoiadb/database/cata/11800",
      "HostName": "vmsvr1",
      "Service": [
        {
          "Type": 0,
          "Name": "11800"
        },
        {
          "Type": 1,
          "Name": "11801"
        },
        {
          "Type": 2,
          "Name": "11802"
        },
        {
          "Type": 3,
          "Name": "11803"
        }
      ],
      "NodeID": 1
    },
    {
      "HostName": "vmsvr2",
      "dbpath": "/opt/sequoiadb/database/cata/11800",
      "Service": [
        {
          "Type": 0,
          "Name": "11800"
        },
        {
          "Type": 1,
          "Name": "11801"
        },
        {
          "Type": 2,
          "Name": "11802"
        },
        {
          "Type": 3,
          "Name": "11803"
        }
      ],
      "NodeID": 2
    },
    {
      "HostName": "vmsvr3",
      "dbpath": "/opt/sequoiadb/database/cata/11800",
      "Service": [
        {
          "Type": 0,
          "Name": "11800"
        },
        {
          "Type": 1,
          "Name": "11801"
        },
        {
          "Type": 2,
          "Name": "11802"
        },
        {
          "Type": 3,
          "Name": "11803"
        }
      ],
      "NodeID": 3
    }
  ],
  "GroupID": 1,
  "GroupName": "SYSCatalogGroup",
  "PrimaryNode": 1,
  "Role": 2,
  "Status": 1,
  "Version": 3,
  "_id": {
    "$oid": "558b9264de349a1b87451a1d"
  }
}
{
  "Group": [
    {
      "HostName": "vmsvr1",
      "dbpath": "/opt/sequoiadb/database/data/21100",
      "Service": [
        {
          "Type": 0,
          "Name": "21100"
        },
        {
          "Type": 1,
          "Name": "21101"
        },
        {
          "Type": 2,
          "Name": "21102"
        }
      ],
      "NodeID": 1000
    },
    {
      "HostName": "vmsvr2",
      "dbpath": "/opt/sequoiadb/database/data/21100",
      "Service": [
        {
          "Type": 0,
          "Name": "21100"
        },
        {
          "Type": 1,
          "Name": "21101"
        },
        {
          "Type": 2,
          "Name": "21102"
        }
      ],
      "NodeID": 1001
    },
    {
      "HostName": "vmsvr3",
      "dbpath": "/opt/sequoiadb/database/data/21100",
      "Service": [
        {
          "Type": 0,
          "Name": "21100"
        },
        {
          "Type": 1,
          "Name": "21101"
        },
        {
          "Type": 2,
          "Name": "21102"
        }
      ],
      "NodeID": 1002
    }
  ],
  "GroupID": 1000,
  "GroupName": "group1",
  "PrimaryNode": 1001,
  "Role": 0,
  "Status": 1,
  "Version": 4,
  "_id": {
    "$oid": "558b9295de349a1b87451a21"
  }
}
{
  "Group": [
    {
      "HostName": "vmsvr3",
      "dbpath": "/opt/sequoiadb/database/data/22100",
      "Service": [
        {
          "Type": 0,
          "Name": "22100"
        },
        {
          "Type": 1,
          "Name": "22101"
        },
        {
          "Type": 2,
          "Name": "22102"
        }
      ],
      "NodeID": 1003
    },
    {
      "HostName": "vmsvr1",
      "dbpath": "/opt/sequoiadb/database/data/22100",
      "Service": [
        {
          "Type": 0,
          "Name": "22100"
        },
        {
          "Type": 1,
          "Name": "22101"
        },
        {
          "Type": 2,
          "Name": "22102"
        }
      ],
      "NodeID": 1004
    },
    {
      "HostName": "vmsvr2",
      "dbpath": "/opt/sequoiadb/database/data/22100",
      "Service": [
        {
          "Type": 0,
          "Name": "22100"
        },
        {
          "Type": 1,
          "Name": "22101"
        },
        {
          "Type": 2,
          "Name": "22102"
        }
      ],
      "NodeID": 1005
    }
  ],
  "GroupID": 1001,
  "GroupName": "group2",
  "PrimaryNode": 1004,
  "Role": 0,
  "Status": 1,
  "Version": 4,
  "_id": {
    "$oid": "558b92b3de349a1b87451a22"
  }
}
Return 3 row(s).
Takes 0.12157s.</pre>
