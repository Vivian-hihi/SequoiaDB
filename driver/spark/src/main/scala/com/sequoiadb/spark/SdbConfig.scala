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

import com.sequoiadb.net.ConfigOptions

/**
  * SequoiaDB configurations
  *
  * @param properties configurations in Map
  */
class SdbConfig(val properties: Map[String, String]) extends Serializable {
    private def notFound(name: String): Nothing = {
        throw new SdbException(s"Parameter $name is not specified")
    }

    private def invalidConfigValue(name: String, value: Any): Nothing = {
        throw new SdbException(s"Invalid value of parameter $name: $value")
    }

    require(
        SdbConfig.RequiredProperties.forall(properties.isDefinedAt),
        s"Not all required properties are defined! : ${
            SdbConfig.RequiredProperties.diff(
                properties.keys.toList.intersect(SdbConfig.RequiredProperties))
        }")

    // don't show password
    override def toString: String = (properties -- Seq(SdbConfig.Password)).toString()

    val host: List[String] = properties
        .getOrElse(SdbConfig.Host, notFound(SdbConfig.Host))
        .split(",").toList

    val collectionSpace: String = properties
        .getOrElse(SdbConfig.CollectionSpace, notFound(SdbConfig.CollectionSpace))

    val collection: String = properties
        .getOrElse(SdbConfig.Collection, notFound(SdbConfig.Collection))

    val username: String = properties
        .getOrElse(SdbConfig.Username, SdbConfig.DefaultUsername)

    val password: String = properties
        .getOrElse(SdbConfig.Password, SdbConfig.DefaultPassword)

    val samplingRatio: Double = properties.get(SdbConfig.SamplingRatio)
        .map(_.toDouble).getOrElse(SdbConfig.DefaultSamplingRatio)

    if (samplingRatio <= 0 || samplingRatio > 1.0) {
        invalidConfigValue(SdbConfig.SamplingRatio, samplingRatio)
    }

    val samplingNum: Long = properties.get(SdbConfig.SamplingNum)
        .map(_.toLong).getOrElse(SdbConfig.DefaultSamplingNum)

    if (samplingNum <= 0) {
        invalidConfigValue(SdbConfig.SamplingNum, samplingNum)
    }

    /**
      * sample _id for schema if true, ignore _id if false.
      */
    val samplingWithId: Boolean = properties.get(SdbConfig.SamplingWithId)
        .map(_.toBoolean).getOrElse(SdbConfig.DefaultSamplingWithId)

    /**
      * use single partition mode for schema sampling if true, use partitionMode if false.
      */
    val samplingSingle: Boolean = properties.get(SdbConfig.SamplingSingle)
        .map(_.toBoolean).getOrElse(SdbConfig.DefaultSamplingSingle)

    /**
      * bulk size when insert into SequoiaDB collection
      */
    val bulkSize: Int = properties.get(SdbConfig.BulkSize)
        .map(_.toInt).getOrElse(SdbConfig.DefaultBulkSize)

    if (bulkSize <= 0) {
        invalidConfigValue(SdbConfig.BulkSize, bulkSize)
    }

    val cursorType: String = properties
        .getOrElse(SdbConfig.CursorType, SdbConfig.DefaultCursorType)

    cursorType.toLowerCase match {
        case SdbConfig.CURSOR_TYPE_FAST =>
        case SdbConfig.CURSOR_TYPE_NORMAL =>
        case _ =>
            invalidConfigValue(SdbConfig.CursorType, cursorType)
    }

    val fastCursorBufSize: Int = properties.get(SdbConfig.FastCursorBufSize)
        .map(_.toInt).getOrElse(SdbConfig.DefaultFastCursorBufSize)

    if (fastCursorBufSize <= 0) {
        invalidConfigValue(SdbConfig.FastCursorBufSize, fastCursorBufSize)
    }

    val fastCursorDecoderNum: Int = properties.get(SdbConfig.FastCursorDecoderNum)
        .map(_.toInt).getOrElse(SdbConfig.DefaultFastCursorDecoderNum)

    if (fastCursorDecoderNum <= 0 || fastCursorDecoderNum > 16) {
        invalidConfigValue(SdbConfig.FastCursorDecoderNum, fastCursorDecoderNum)
    }

    private val scanType: String = properties
        .getOrElse(SdbConfig.ScanType, SdbConfig.SCAN_TYPE_AUTO)

    val partitionMode: String = properties
        .getOrElse(SdbConfig.PartitionMode, {
            // compatible with old edition option
            scanType.toLowerCase match {
                case SdbConfig.SCAN_TYPE_IXSCAN =>
                    SdbConfig.PARTITION_MODE_SHARDING
                case SdbConfig.SCAN_TYPE_TBSCAN =>
                    SdbConfig.PARTITION_MODE_DATABLOCK
                case _ => SdbConfig.DefaultPartitionMode
            }
        })

    partitionMode.toLowerCase match {
        case SdbConfig.PARTITION_MODE_SINGLE =>
        case SdbConfig.PARTITION_MODE_SHARDING =>
        case SdbConfig.PARTITION_MODE_DATABLOCK =>
        case SdbConfig.PARTITION_MODE_AUTO =>
        case _ =>
            invalidConfigValue(SdbConfig.PartitionMode, partitionMode)
    }

    val partitionBlockNum: Int = properties.get(SdbConfig.PartitionBlockNum)
        .map(_.toInt).getOrElse(SdbConfig.DefaultPartitionBlockNum)

    if (partitionBlockNum <= 0) {
        invalidConfigValue(SdbConfig.PartitionBlockNum, partitionBlockNum)
    }

    val partitionMaxNum: Int = properties.get(SdbConfig.PartitionMaxNum)
        .map(_.toInt).getOrElse(SdbConfig.DefaultPartitionMaxNum)

    // zero means no limit for partition max num when partitionMode is Datablock
    if (partitionMaxNum < 0) {
        invalidConfigValue(SdbConfig.PartitionMaxNum, partitionMaxNum)
    }

    val preferredLocation: Boolean = properties.get(SdbConfig.PreferredLocation)
        .map(_.toBoolean).getOrElse(SdbConfig.DefaultPreferredLocation)

    val shardingPartitionSingleNode: Boolean = properties.get(SdbConfig.ShardingPartitionSingleNode)
        .map(_.toBoolean).getOrElse(SdbConfig.DefaultShardingPartitionSingleNode)
}

object SdbConfig {
    //  Parameter names
    val Host = "host"
    val CollectionSpace = "collectionspace"
    val Collection = "collection"
    val Username = "username"
    val Password = "password"
    val SamplingRatio = "samplingratio"
    val SamplingNum = "samplingnum"
    val SamplingWithId = "samplingwithid"
    val SamplingSingle = "samplingsingle"
    val BulkSize = "bulksize"
    val CursorType = "cursortype"
    // fast, normal
    val FastCursorBufSize = "fastcursorbufsize"
    val FastCursorDecoderNum = "fastcursordecodernum"
    val PartitionMode = "partitionmode"
    // single, sharding, datablock, auto
    val PartitionBlockNum = "partitionblocknum"
    val PartitionMaxNum = "partitionmaxnum"
    val ShardingPartitionSingleNode = "shardingpartitionsinglenode"
    val PreferredLocation = "preferredlocation"

    // compatible with old edition option
    val ScanType = "scantype" // auto/ixscan/tbscan

    val CURSOR_TYPE_FAST = "fast"
    val CURSOR_TYPE_NORMAL = "normal"

    val PARTITION_MODE_SINGLE = "single"
    val PARTITION_MODE_SHARDING = "sharding"
    val PARTITION_MODE_DATABLOCK = "datablock"
    val PARTITION_MODE_AUTO = "auto"

    val SCAN_TYPE_IXSCAN = "ixscan"
    val SCAN_TYPE_TBSCAN = "tbscan"
    val SCAN_TYPE_AUTO = "auto"

    val AllProperties = List(
        Host,
        CollectionSpace,
        Collection,
        Username,
        Password,
        SamplingRatio,
        SamplingNum,
        SamplingWithId,
        SamplingSingle,
        BulkSize,
        CursorType,
        FastCursorBufSize,
        FastCursorDecoderNum,
        PartitionMode,
        PartitionBlockNum,
        PartitionMaxNum,
        ShardingPartitionSingleNode,
        PreferredLocation,
        ScanType
    )

    val RequiredProperties = List(
        Host,
        CollectionSpace,
        Collection
    )

    //  Default values
    val DefaultUsername = ""
    val DefaultPassword = ""
    val DefaultSamplingRatio = 1.0
    val DefaultSamplingNum = 1000L
    val DefaultSamplingWithId = false
    val DefaultSamplingSingle = true
    val DefaultBulkSize = 500
    val DefaultCursorType = "fast"
    val DefaultFastCursorBufSize = 500
    val DefaultFastCursorDecoderNum = 2
    val DefaultPartitionMode = "auto"
    val DefaultPartitionBlockNum = 4
    val DefaultPartitionMaxNum = 1000
    val DefaultShardingPartitionSingleNode = false
    val DefaultPreferredLocation = false

    def apply(parameters: Map[String, String]): SdbConfig = new SdbConfig(parameters)

    private[spark] val SdbConnectionOptions: ConfigOptions = {
        val opt = new ConfigOptions()
        opt.setConnectTimeout(3000)
        opt.setMaxAutoConnectRetryTime(0)
        opt
    }
}
