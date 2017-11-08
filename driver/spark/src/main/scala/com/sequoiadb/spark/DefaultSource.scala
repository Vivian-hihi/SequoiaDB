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

import com.sequoiadb.base.{DBCollection, Sequoiadb}
import org.apache.spark.sql.sources._
import org.apache.spark.sql.types.StructType
import org.apache.spark.sql.{DataFrame, SQLContext, SaveMode}

import scala.collection.JavaConversions._

/**
  * Default source is loaded by Spark.
  * Allows creation of SequoiaDB based tables using
  * the syntax CREATE [TEMPORARY] TABLE ... USING com.sequoiadb.spark OPTIONS(...)
  * Required options are detailed in [[com.sequoiadb.spark.SdbConfig]]
  */
class DefaultSource extends DataSourceRegister
    with RelationProvider
    with SchemaRelationProvider
    with CreatableRelationProvider {

    override def shortName(): String = "sequoiadb"

    /**
      * Create relation without providing schema
      */
    override def createRelation(sqlContext: SQLContext,
                                parameters: Map[String, String]): BaseRelation = {
        SdbRelation(sqlContext, SdbConfig(parameters))
    }

    /**
      * Create relation with providing schema
      */
    override def createRelation(sqlContext: SQLContext,
                                parameters: Map[String, String],
                                schema: StructType): BaseRelation = {
        SdbRelation(sqlContext, SdbConfig(parameters), Option(schema))
    }

    /**
      * Creatable relation
      */
    override def createRelation(sqlContext: SQLContext,
                                mode: SaveMode,
                                parameters: Map[String, String],
                                data: DataFrame): BaseRelation = {
        val config = SdbConfig(parameters)

        // if it return true, that means we should write the data into collection
        if (isCollectionWritable(config, mode)) {
            // get schema for execution
            val schema = data.schema
            data.foreachPartition(it => {
                // always write through coord node which specified in config
                new SdbWriter(config).write(it, schema)
            })
        }

        SdbRelation(sqlContext, config, Option(data.schema))
    }

    // Check whether a collection is writable for the given mode
    // Return true for writable, return false for non-writable, throw exception for error
    private def isCollectionWritable(config: SdbConfig, mode: SaveMode): Boolean = {
        val sdb = new Sequoiadb(config.host, config.username, config.password, null)

        try {
            mode match {
                case SaveMode.Append =>
                    ensureCollection(sdb, config.collectionSpace, config.collection)
                    true
                case SaveMode.Overwrite =>
                    val cl = ensureCollection(sdb, config.collectionSpace, config.collection)
                    cl.truncate()
                    true
                case SaveMode.ErrorIfExists =>
                    if (isCollectionExist(sdb, config.collectionSpace, config.collection)) {
                        throw new SdbException(
                            String.format("Collection [%s.%s] is exist",
                                config.collectionSpace, config.collection)
                        )
                    }
                    ensureCollection(sdb, config.collectionSpace, config.collection)
                    true
                case SaveMode.Ignore => false
                case _ => false
            }
        } finally {
            sdb.disconnect()
        }
    }

    private def isCollectionExist(sdb: Sequoiadb, csName: String, clName: String): Boolean = {
        if (sdb.isCollectionSpaceExist(csName)) {
            val cs = sdb.getCollectionSpace(csName)
            if (cs.isCollectionExist(clName)) {
                return true
            }
        }

        false
    }

    // create CollectionSpace & Collection if they are not existing
    private def ensureCollection(sdb: Sequoiadb, csName: String, clName: String): DBCollection = {
        if (!sdb.isCollectionSpaceExist(csName)) {
            sdb.createCollectionSpace(csName)
        }
        val cs = sdb.getCollectionSpace(csName)
        if (!cs.isCollectionExist(clName)) {
            cs.createCollection(clName)
        }
        cs.getCollection(clName)
    }
}
