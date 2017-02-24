/**
 * Copyright (c) 2017, SequoiaDB Ltd.
 * File Name:GroupWrapper.java
 * 
 *
 *  @author wenjingwang
 * Date:2017-2-21下午4:54:48
 *  @version 1.00
 */
package com.sequoiadb.commlib ;

import java.util.ArrayList ;
import java.util.List ;
import java.util.Random ;

import org.bson.BasicBSONObject ;
import org.bson.types.BasicBSONList ;

import com.sequoiadb.base.ReplicaGroup ;
import com.sequoiadb.base.Sequoiadb ;
import com.sequoiadb.exception.BaseException ;
import com.sequoiadb.exception.OperateException ;
import com.sequoiadb.exception.ReliabilityException ;

public class GroupWrapper {
    private String groupName ;
    private Sequoiadb sdb ;
    private ReplicaGroup group ;
    private List< NodeWrapper > nodes = new ArrayList< NodeWrapper >() ;

    public GroupWrapper( String name, ReplicaGroup group ) {
        this.groupName = name ;
        this.group = group ;
    }

    public void init() throws ReliabilityException {
        try{
        BasicBSONObject obj = ( BasicBSONObject ) group.getDetail() ;
        BasicBSONList nodeobjs = ( BasicBSONList ) obj.get( "Group" ) ;
        for ( int i = 0; i < nodeobjs.size(); ++i ) {
            BasicBSONObject nodeobj = ( BasicBSONObject ) nodeobjs.get( i ) ;
            String hostName = nodeobj.getString( "HostName" ) ;
            String port = ( ( BasicBSONObject ) ( ( BasicBSONList ) nodeobj
                    .get( "Service" ) ).get( 0 ) ).getString( "Name" ) ;
            String dbPath = nodeobj.getString( "dbpath" ) ;
            NodeWrapper node = new NodeWrapper( this.group.getNode( hostName,
                    Integer.parseInt( port ) ), dbPath ) ;
            nodes.add( node ) ;
        }
        }
        catch(BaseException e){
            throw new OperateException(e);
        }
    }

    public NodeWrapper getMaster() {
        for ( int i = 0; i < nodes.size(); ++i ) {
            if ( nodes.get( i ).isMaster() ) {
                return nodes.get( i ) ;
            }
        }
        return null ;
    }

    public NodeWrapper getSlave() {
        Random random = new Random() ;

        int pos = random.nextInt( nodes.size() ) ;
        if ( !nodes.get( pos ).isMaster() ) {
            return nodes.get( pos ) ;
        } else if ( nodes.size() > 1 ) {
            return nodes.get( ( pos + 1 ) % nodes.size() ) ;
        } else {
            return null ;
        }
    }

    public int getNodeNum() {
        return nodes.size() ;
    }

    public boolean changePrimary() {
        return true ;
    }
}
