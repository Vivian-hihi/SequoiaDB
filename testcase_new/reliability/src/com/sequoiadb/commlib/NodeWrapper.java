/**
 * Copyright (c) 2017, SequoiaDB Ltd.
 * File Name:NodeWrapper.java
 * 
 *
 *  @author wenjingwang
 * Date:2017-2-21下午4:54:48
 *  @version 1.00
 */
package com.sequoiadb.commlib ;

import org.bson.BSONObject ;
import org.bson.BasicBSONObject ;

import com.sequoiadb.base.DBCursor ;
import com.sequoiadb.base.Node ;
import com.sequoiadb.base.Sequoiadb ;
import com.sequoiadb.exception.BaseException ;
import com.sequoiadb.exception.OperateException ;
import com.sequoiadb.exception.ReliabilityException ;

public class NodeWrapper {
    public enum NodeStatus {
        STOP_SUCCESS, STOP_FAILURE, START_SUCCESS, START_FAILURE
    } ;

    private NodeStatus status ;
    private Node node ;
    private String dbPath ;

    public NodeWrapper( Node node, String dbPath ) {
        this.node = node ;
        this.dbPath = dbPath ;
    }

    private BasicBSONObject getDataBaseSnapshot() throws ReliabilityException {
        Sequoiadb sdb = null ;
        BasicBSONObject retObj = null ;
        try {
            sdb = node.connect() ;
            BSONObject nullObj = null ;
            DBCursor cursor = sdb.getSnapshot( Sequoiadb.SDB_SNAP_DATABASE,
                    nullObj, nullObj, nullObj ) ;
            while ( cursor.hasNext() ) {
                retObj = ( BasicBSONObject ) cursor.getNext() ;
            }
            cursor.close() ;
        } catch ( BaseException e ) {
            System.out.println( node.getNodeName() + " getSnapshot( "
                    + Sequoiadb.SDB_SNAP_DATABASE + ") failed "
                    + e.getErrorCode() ) ;
            throw new OperateException(e) ;
        } finally {
            if ( sdb != null ) {
                sdb.disconnect() ;
            }
        }
        return retObj ;
    }

    public boolean start() throws ReliabilityException{
        try {
            node.start() ;
            status = NodeStatus.START_SUCCESS ;
            
        } catch ( BaseException e ) {
            System.out.println( "start " + node.getNodeName() + " failed "
                    + e.getErrorCode() ) ;
            status = NodeStatus.START_FAILURE ;
            throw new OperateException(e) ;
        }
        return true ;
    }

    public boolean stop() throws ReliabilityException{
        try {
            node.stop() ;
            status = NodeStatus.STOP_SUCCESS ;
            
        } catch ( BaseException e ) {
            System.out.println( "stop " + node.getNodeName() + " failed "
                    + e.getErrorCode() ) ;
            status = NodeStatus.STOP_FAILURE ;
            throw new OperateException(e) ;
        }
        return true;
    }

    public boolean checkStop() {
        if ( status == NodeStatus.STOP_FAILURE ) {
            return false ;
        } else {
            return true ;
        }
    }

    public boolean checkStart() {
        if ( status == NodeStatus.START_FAILURE ) {
            return false ;
        } else {
            return true ;
        }
    }

    public boolean isNodeActive() throws ReliabilityException{
        try {
            Sequoiadb db = node.connect() ;
            BSONObject nullObj = null ;
            DBCursor cursor = db.getList( Sequoiadb.SDB_LIST_CONTEXTS_CURRENT,
                    nullObj, nullObj, nullObj ) ;
            while ( cursor.hasNext() ) {
                cursor.getNext() ;
            }
            cursor.close() ;
        } catch ( BaseException e ) {
            throw new OperateException(e) ;
        }
        return true;
    }

    public String hostName() {
        return node.getHostName() ;
    }

    public String svcName() {
        return Integer.toString( node.getPort() ) ;
    }

    public String dbPath() {
        return this.dbPath ;
    }

    public boolean isMaster() throws ReliabilityException{
        BasicBSONObject obj = getDataBaseSnapshot() ;
        if ( obj != null ) {
            return obj.getBoolean( "IsPrimary" ) ;
        }

        return false ;
    }
}
