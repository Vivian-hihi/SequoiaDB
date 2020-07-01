/**
 * Copyright (c) 2020, SequoiaDB Ltd.
 * File Name:OprLobTask.java
 *      将实现在各个测试用例类中的内部类抽取出来
 *
 *  @author wangwenjing
 * Date:2020年6月17日上午10:06:19
 *  @version 1.00
 */
package com.sequoiadb.datasync;

import org.bson.BSONObject;
import org.bson.util.JSON;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.datasync.OprLobTask;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.task.OperateTask;

public class CreateCLTask extends OperateTask {
    private int clNum = 500 ;
    private String namePrefix ;
    private String groupName = "";
    private int count = 0 ;
    public CreateCLTask(String prefix, String groupName) {
        this.namePrefix = prefix ;
        this.groupName = groupName ;
    }
    
    public CreateCLTask(String prefix, int clNum)
    {
       this.namePrefix = prefix ;
       this.clNum = clNum ;
    }

    public CreateCLTask(String prefix, String groupName, int clNum) {
        this.namePrefix = prefix ;
        this.groupName = groupName ;
        this.clNum = clNum ;
    }
    @Override
    public void exec() throws Exception {
        try ( Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl , "", "" )) {
           BSONObject option = ( BSONObject ) JSON
                        .parse( "{ ShardingKey: { a: 1 },"
                                + "ShardingType: 'hash', "
                                + "Partition: 2048, " + "ReplSize: 2, "
                                + "Compressed: true, "
                                + "CompressionType: 'lzw',"
                                + "IsMainCL: false, " + "AutoSplit: false, "
                                + (groupName.equals("") ? "": "Group: '" + groupName + "', ")
                                + "AutoIndexId: true, "
                                + "EnsureShardingIndex: true }" );

            CollectionSpace commCS = db
                    .getCollectionSpace( SdbTestBase.csName );
            for ( int i = 0; i < clNum; i++ ) {
                String clName = namePrefix + "_" + i;
                commCS.createCollection( clName, option );
                count++;
            }
        } catch ( BaseException e ) {
            System.out.println( "the create cl error i is =" + count );
        }
    }

    public int getCreateNum() {
        return count;
    }
}
