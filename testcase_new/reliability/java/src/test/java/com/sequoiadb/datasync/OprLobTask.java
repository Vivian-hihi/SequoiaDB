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

import java.util.Random;

import org.bson.types.ObjectId;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.task.OperateTask;

public class OprLobTask extends OperateTask {
    private String clName ;
    private int repeatTimes = 100 ;
    private Random rnd = new Random() ;
    public OprLobTask(String clName) {
        this.clName = clName ;
    }
    
    public OprLobTask(String clName, int repeatTimes) {
        this.clName = clName ;
        this.repeatTimes = repeatTimes ;
    }
    
    @Override
    public void exec() throws Exception {
        int  opreateLobNum = 0; 
        try ( Sequoiadb db = new Sequoiadb( SdbTestBase.coordUrl, "",
                "" )) {
            DBCollection cl = db.getCollectionSpace( SdbTestBase.csName )
                    .getCollection( clName );
            int lobSize = rnd.nextInt( 1048576 );
            byte[] lobBytes = new byte[ lobSize ];
            rnd.nextBytes( lobBytes );
            
            for ( int i = 0; i < repeatTimes; i++ ) {
                DBLob wLob = null ;
                DBLob rLob = null ;
                ObjectId oid = null;
                try {
                    wLob = cl.createLob();
                    wLob.write( lobBytes );
                    oid = wLob.getID();
                    wLob.close();
                    wLob = null ;
                    
                    rLob = cl.openLob( oid );
                    byte[] rLobBytes = new byte[ lobSize ];
                    rLob.read( rLobBytes );
                }finally {
                    if(wLob != null) {
                        wLob.close(); 
                    }
                    
                    if (rLob != null ) {
                        rLob.close(); 
                    }
                }
                if (oid != null ) {
                    cl.removeLob( oid ); 
                }
                
                opreateLobNum++;
            }
        } catch ( BaseException e ) {
            System.out.println(
                    "write/remove lob nums is =" + opreateLobNum );
        }
    }
}
