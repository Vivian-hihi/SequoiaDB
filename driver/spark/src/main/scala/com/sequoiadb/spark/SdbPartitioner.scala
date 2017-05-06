/*
 * Copyright 2017 SequoiaDB Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

package com.sequoiadb.spark

import com.sequoiadb.base.Sequoiadb
import com.sequoiadb.exception.BaseException
import org.bson.BSONObject
import org.bson.types.BasicBSONList

import scala.collection.JavaConversions._
import scala.collection.mutable
import scala.collection.mutable.ArrayBuffer
import scala.util.control.Breaks

/**
  * SequoiaDB partitioner
  *
  * @param config configurations
  * @param filter query filter
  */
abstract class SdbPartitioner(config: SdbConfig, filter: SdbFilter)
    extends Serializable with Logging {

    def computePartitions(): Array[SdbPartition]
}

private object SdbPartitioner extends Logging {
    def apply(config: SdbConfig, filter: SdbFilter): SdbPartitioner = {
        val mode = determinatePartitionMode(config, filter)
        logInfo(s"PartitionMode=$mode")
        mode match {
            case PartitionMode.Single =>
                new SdbSinglePartitioner(config, filter)
            case PartitionMode.Sharding =>
                new SdbShardingPartitioner(config, filter)
            case PartitionMode.Datablock =>
                new SdbDatablockPartitioner(config, filter)
            case _ =>
                throw new SdbException("Unknown partition mode: " + mode)
        }
    }

    private def determinatePartitionMode(config: SdbConfig, filter: SdbFilter): PartitionMode = {
        logInfo(s"config.partitionMode=${config.partitionMode}")
        config.partitionMode.toLowerCase match {
            case SdbConfig.PARTITION_MODE_SINGLE => PartitionMode.Single
            case SdbConfig.PARTITION_MODE_SHARDING => PartitionMode.Sharding
            case SdbConfig.PARTITION_MODE_DATABLOCK => PartitionMode.Datablock
            case SdbConfig.PARTITION_MODE_AUTO => autodetectPartitionMode(config, filter)
            case _ =>
                throw new SdbException("Unknown partition mode: " + config.partitionMode)
        }
    }

    // Sharding if query can use index, otherwise Datablock
    private def autodetectPartitionMode(config: SdbConfig, filter: SdbFilter): PartitionMode = {
        val sdb = new Sequoiadb(config.host, config.username, config.password, null)
        try {
            val shardings = getShardingInfo(sdb, config, filter)
            for (sharding <- shardings) {
                if (sharding.scanType == "ixscan") {
                    return PartitionMode.Sharding
                }
            }

            // partition num limit is too small, use Sharding mode
            if (config.partitionMaxNum > 0 && config.partitionMaxNum <= shardings.length) {
                return PartitionMode.Sharding
            }
        } finally {
            sdb.disconnect()
        }

        PartitionMode.Datablock
    }

    def getReplicaGroups(sdb: Sequoiadb): Map[String, List[String]] = {
        val map = new mutable.HashMap[String, List[String]]()

        try {
            val cursor = sdb.listReplicaGroups()
            while (cursor.hasNext) {
                val obj = cursor.getNext

                val groupName = obj.get("GroupName").asInstanceOf[String]
                val nodeList = ArrayBuffer[String]()

                val group = obj.get("Group").asInstanceOf[BasicBSONList]
                for (node <- group) {
                    val nodeObj = node.asInstanceOf[BSONObject]

                    val hostName = nodeObj.get("HostName")
                    val service = nodeObj.get("Service").asInstanceOf[BasicBSONList]
                    for (port <- service) {
                        val portObj = port.asInstanceOf[BSONObject]
                        val serviceType = portObj.get("Type").asInstanceOf[Int]
                        if (serviceType == 0) {
                            val serviceName = portObj.get("Name").asInstanceOf[String]

                            val node = hostName + ":" + serviceName
                            nodeList += node
                        }
                    }
                }

                map += (groupName -> nodeList.toList)
            }
        } catch {
            case e: BaseException =>
                if (!e.getErrorType.equals("SDB_RTN_COORD_ONLY")) {
                    throw e
                }
            case x: Throwable => throw x
        }

        map.toMap
    }

    def getShardingInfo(sdb: Sequoiadb, config: SdbConfig, filter: SdbFilter): List[ShardingInfo] = {
        val shardingList = ArrayBuffer[ShardingInfo]()

        val cs = if (sdb.isCollectionSpaceExist(config.collectionSpace)) {
            sdb.getCollectionSpace(config.collectionSpace)
        } else {
            throw new SdbException(s"Collection space is not existing: ${config.collectionSpace}")
        }

        val cl = if (cs.isCollectionExist(config.collection)) {
            cs.getCollection(config.collection)
        } else {
            throw new SdbException(s"Collection is not existing: " +
                s"${config.collectionSpace}.${config.collection}")
        }

        val matcher = filter.BSONObj()

        val cursor = cl.explain(matcher, null, null, null, 0, -1, 0, null)
        if (cursor == null) {
            return List[ShardingInfo]()
        }

        while (cursor.hasNext) {
            val obj = cursor.getNext

            val nodeName = obj.get("NodeName").asInstanceOf[String]
            val groupName = obj.get("GroupName").asInstanceOf[String]

            if (obj.containsField("SubCollections")) {
                val subCollections = obj.get("SubCollections").asInstanceOf[BasicBSONList]
                for (sub <- subCollections) {
                    val subObj = sub.asInstanceOf[BSONObject]

                    val name = subObj.get("Name").asInstanceOf[String]
                    val scanType = subObj.get("ScanType").asInstanceOf[String]

                    val shardingInfo = new ShardingInfo(name, nodeName, groupName, scanType)
                    shardingList += shardingInfo
                }
            } else {
                val name = obj.get("Name").asInstanceOf[String]
                val scanType = obj.get("ScanType").asInstanceOf[String]

                val shardingInfo = new ShardingInfo(name, nodeName, groupName, scanType)
                shardingList += shardingInfo
            }
        }

        shardingList.toList
    }

    def getQueryMeta(urls: List[String], csName: String, clName: String, config: SdbConfig): List[QueryMeta] = {
        val queryMetas = ArrayBuffer[QueryMeta]()

        val sdb = new Sequoiadb(urls, config.username, config.password, null)
        try {
            val cs = if (sdb.isCollectionSpaceExist(csName)) {
                sdb.getCollectionSpace(csName)
            } else {
                throw new SdbException(s"Collection space is not existing: $csName")
            }

            val cl = if (cs.isCollectionExist(clName)) {
                cs.getCollection(clName)
            } else {
                throw new SdbException(s"Collection is not existing: " +
                    s"$csName.$clName")
            }

            val cursor = cl.getQueryMeta(null, null, null, 0, -1, 0)
            while (cursor.hasNext) {
                val obj = cursor.getNext

                val hostName = obj.get("HostName").asInstanceOf[String]
                val serviceName = obj.get("ServiceName").asInstanceOf[String]

                val blocks = ArrayBuffer[Int]()

                val datablocks = obj.get("Datablocks").asInstanceOf[BasicBSONList]
                for (blockId <- datablocks) {
                    blocks += blockId.asInstanceOf[Int]
                }

                val meta = new QueryMeta(hostName + ":" + serviceName,
                    csName, clName, blocks.toList)
                queryMetas += meta
            }
        } finally {
            sdb.disconnect()
        }

        queryMetas.toList
    }

    // Host container for shufflePartitions
    private class Host(val host: String) {
        val nodes: ArrayBuffer[Node] = ArrayBuffer[Node]()
        val nodeMap: mutable.HashMap[Int, Node] = mutable.HashMap[Int, Node]()
        var partitionNum: Int = 0
        var refCount: Int = 0
    }

    // Node container for shufflePartitions
    private class Node(val host: String, val port: Int) {
        val partitions: ArrayBuffer[SdbPartition] = ArrayBuffer[SdbPartition]()
        var refCount: Int = 0
    }

    // shuffle partitions to rearrange partition position
    def shufflePartitions(partitions: Array[SdbPartition]): Array[SdbPartition] = {
        val hostMap = mutable.HashMap[String, Host]()

        // collect which host and node the partition is placed
        partitions.foreach { partition =>
            val url = partition.urls.head
            val u = url.split(':')
            val hostName = u(0).trim
            val port = u(1).trim.toInt

            val host = hostMap.get(hostName)
            if (host.isEmpty) {
                val newNode = new Node(hostName, port)
                newNode.partitions += partition

                val newHost = new Host(hostName)
                newHost.nodeMap += (port -> newNode)
                newHost.nodes += newNode
                newHost.partitionNum += 1

                hostMap += (hostName -> newHost)
            } else {
                val oldHost = host.get
                val node = oldHost.nodeMap.get(port)
                if (node.isEmpty) {
                    val newNode = new Node(hostName, port)
                    newNode.partitions += partition
                    oldHost.nodeMap += (port -> newNode)
                    oldHost.nodes += newNode
                    oldHost.partitionNum += 1
                } else {
                    node.get.partitions += partition
                    oldHost.partitionNum += 1
                }
            }
        }

        val hosts = ArrayBuffer[Host]()
        hostMap.values.foreach(host => hosts += host)

        val newPartitions = ArrayBuffer[SdbPartition]()

        // chose partition which is in the least used host and node one by one
        while (hosts.nonEmpty) {
            var minHost: Host = hosts.head

            hosts.foreach { host: Host =>
                if (host.refCount < minHost.refCount) {
                    minHost = host
                }
            }

            var minNode: Node = minHost.nodes.head
            minHost.nodes.foreach { node: Node =>
                if (node.refCount < minNode.refCount) {
                    minNode = node
                }
            }

            val partition = minNode.partitions.head
            newPartitions += partition
            minNode.partitions -= partition
            minNode.refCount += 1
            minHost.refCount += 1
            minHost.partitionNum -= 1

            if (minNode.partitions.isEmpty) {
                minHost.nodes -= minNode
                if (minHost.nodes.isEmpty) {
                    hosts -= minHost
                }
            }
        }

        if (newPartitions.size != partitions.length) {
            throw new SdbException("shufflePartitions")
        }

        for (i <- newPartitions.indices) {
            newPartitions(i).index = i
        }

        newPartitions.toArray
    }
}

/**
  * Sharding info of SequoiaDB collection
  *
  * @param name      sharding collection full name
  * @param nodeName  the url of SequoiaDB node that return the sharding info
  * @param groupName the group that hold the sharding collection
  * @param scanType  the query scan type
  */
private class ShardingInfo(val name: String,
                           val nodeName: String,
                           val groupName: String,
                           val scanType: String) {
    val csName: String = name.split('.')(0)
    val clName: String = name.split('.')(1)
}

/**
  * Query meta data of SequoiaDB
  *
  * @param url        url of SequoiaDB node that hode the datablocks
  * @param datablocks datablocks of partition
  */
private class QueryMeta(val url: String,
                        val csName: String,
                        val clName: String,
                        val datablocks: List[Int]) {
}

// Single partitioner generates only one partition using the host in SdbConfig
private[spark] class SdbSinglePartitioner(config: SdbConfig, filter: SdbFilter)
    extends SdbPartitioner(config, filter) {

    override def computePartitions(): Array[SdbPartition] = {
        Array(SdbPartition(
            config.host,
            config.collectionSpace,
            config.collection,
            filter,
            PartitionMode.Single))
    }
}

// Sharding partitioner generates partitions by SequoiaDB sharding
private[spark] class SdbShardingPartitioner(config: SdbConfig, filter: SdbFilter)
    extends SdbPartitioner(config, filter) {

    override def computePartitions(): Array[SdbPartition] = {
        val partitions = ArrayBuffer[SdbPartition]()

        val sdb = new Sequoiadb(config.host, config.username, config.password, null)
        try {
            val groups = SdbPartitioner.getReplicaGroups(sdb)

            val shardings = SdbPartitioner.getShardingInfo(sdb, config, filter)

            for (sharding <- shardings) {
                var urls: List[String] = null
                if (sharding.groupName == "") {
                    urls = List(sharding.nodeName)
                } else {
                    urls = groups(sharding.groupName)
                }

                val partition = SdbPartition(
                    groups(sharding.groupName),
                    sharding.csName,
                    sharding.clName,
                    filter,
                    PartitionMode.Sharding)
                partitions += partition
            }
        } finally {
            sdb.disconnect()
        }

        SdbPartitioner.shufflePartitions(partitions.toArray)
    }
}

// Datablock partitioner generates partitions by SequoiaDB collection's data blocks
private[spark] class SdbDatablockPartitioner(config: SdbConfig, filter: SdbFilter)
    extends SdbPartitioner(config, filter) {

    override def computePartitions(): Array[SdbPartition] = {
        var groups: Map[String, List[String]] = null
        var shardings: List[ShardingInfo] = null

        val sdb = new Sequoiadb(config.host, config.username, config.password, null)
        try {
            groups = SdbPartitioner.getReplicaGroups(sdb)
            shardings = SdbPartitioner.getShardingInfo(sdb, config, filter)
        } finally {
            sdb.disconnect()
        }

        val queryMetas = getQueryMetas(groups, shardings)

        var partitions: Array[SdbPartition] = null
        var blockNum = config.partitionBlockNum

        val breaker = new Breaks
        breaker.breakable {
            while (true) {
                partitions = generatePartitions(queryMetas, blockNum)

                // zero means no limit for partition max num
                if (config.partitionMaxNum == 0) {
                    breaker.break()
                }

                // partition num is satisfied or only one partition per sharding
                if (partitions.length < config.partitionMaxNum ||
                    partitions.length == shardings.size) {
                    breaker.break()
                }

                // we should increase block num of each partition to decrease partition num
                blockNum += 1
            }
        }

        logInfo(s"get ${partitions.length} partitions with $blockNum data blocks per partition")

        SdbPartitioner.shufflePartitions(partitions)
    }

    private def getQueryMetas(groups: Map[String, List[String]],
                              shardings: List[ShardingInfo])
    : Array[QueryMeta] = {
        val queryMetas = ArrayBuffer[QueryMeta]()

        for (sharding <- shardings) {
            var urls: List[String] = null
            if (sharding.groupName == "") {
                urls = List(sharding.nodeName)
            } else {
                urls = groups(sharding.groupName)
            }

            val metas = SdbPartitioner.getQueryMeta(urls, sharding.csName, sharding.clName, config)
            queryMetas ++= metas
        }

        queryMetas.toArray
    }

    private def generatePartitions(queryMetas: Array[QueryMeta],
                                   blockNum: Int)
    : Array[SdbPartition] = {
        val partitions = ArrayBuffer[SdbPartition]()

        for (meta <- queryMetas) {
            for (blocks <- meta.datablocks.sliding(blockNum, blockNum)) {
                val partition = SdbPartition(List(meta.url),
                    meta.csName,
                    meta.clName,
                    filter,
                    PartitionMode.Datablock,
                    blocks)
                partitions += partition
            }
        }

        partitions.toArray
    }
}