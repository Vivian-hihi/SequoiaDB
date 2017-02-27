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
import com.sequoiadb.task.OperateTask ;

public class GroupWrapper {
    private ReplicaGroup group ;
    private List< NodeWrapper > nodes = new ArrayList< NodeWrapper >() ;
    private BasicBSONObject groupInfo ;

    public GroupWrapper( BasicBSONObject groupInfo, ReplicaGroup group ) {
        this.groupInfo = groupInfo ;
        this.group = group ;
    }

    public String getGroupName() {
        return this.groupInfo.getString( "GroupName" ) ;
    }

    public int getGroupID() {
        return this.groupInfo.getInt( "GroupID" ) ;
    }

    public void init() throws ReliabilityException {
        try {
            BasicBSONList nodesinfo = (BasicBSONList)groupInfo.get( "Group" ) ;
            for (int i = 0; i < nodesinfo.size(); ++i){
                BasicBSONObject nodeinfo = ( BasicBSONObject ) nodesinfo.get( i );
                String hostName = nodeinfo.getString( "HostName" ) ;
                String port = ( ( BasicBSONObject ) ( ( BasicBSONList ) nodeinfo
                        .get( "Service" ) ).get( 0 ) ).getString( "Name" ) ;
                
                NodeWrapper node = new NodeWrapper( this.group.getNode(
                        hostName, Integer.parseInt( port ) ), nodeinfo ) ;
                nodes.add( node ) ;
            }
        } catch ( BaseException e ) {
            throw new OperateException( e ) ;
        }
    }

    public NodeWrapper getMaster() throws ReliabilityException {
        for ( NodeWrapper node : nodes ) {
            if ( node.isMaster() ) {
                return node ;
            }
        }
        return null ;
    }

    public NodeWrapper getSlave() throws ReliabilityException {
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

    public GroupCheckResult checkBusiness() {
        GroupCheckResult checkRes = new GroupCheckResult();
        checkRes.groupName = getGroupName();
        checkRes.groupID = getGroupID();
        checkRes.primaryNode = groupInfo.getInt( "PrimaryNode" ) ;
    
        for ( NodeWrapper node : nodes ) {
            NodeCheckResult res = node.checkBusiness();
            checkRes.addNodeCheckResult( res );
        }
        
        return checkRes;
    }
}
