/**
 * Copyright (c) 2017, SequoiaDB Ltd.
 * File Name:GroupMgr.java
 * 类的详细描述
 *
 *  @author wenjingwang
 * Date:2017-2-23上午10:19:55
 *  @version 1.00
 */
package com.sequoiadb.commlib ;

import java.util.ArrayList ;
import java.util.HashMap ;
import java.util.List ;
import java.util.Map ;
import java.util.Map.Entry ;

import org.bson.BSONObject ;
import org.bson.BasicBSONObject ;

import com.sequoiadb.base.DBCursor ;
import com.sequoiadb.base.Sequoiadb ;
import com.sequoiadb.exception.BaseException ;
import com.sequoiadb.exception.OperateException ;
import com.sequoiadb.exception.ReliabilityException ;

public class GroupMgr {
    private Map< String, GroupWrapper > name2group = new HashMap< String, GroupWrapper >() ;
    private Map< Integer, GroupWrapper > id2group = new HashMap< Integer, GroupWrapper >() ;
    private Sequoiadb sdb = null ;
    private static GroupMgr mgr = null ;

    private GroupMgr() {
        this.sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" ) ;
    }

    public void init() throws ReliabilityException {
        try{
        BSONObject nullObj = null ;
        DBCursor cursor = sdb.getList( Sequoiadb.SDB_LIST_GROUPS, nullObj,
                nullObj, nullObj ) ;
        while ( cursor.hasNext() ) {
            BasicBSONObject obj = ( BasicBSONObject ) cursor.getNext() ;
            String groupName = obj.getString( "GroupName" ) ;
            int groupId = obj.getInt( "GroupID" ) ;
            GroupWrapper group = new GroupWrapper( groupName,
                    sdb.getReplicaGroup( groupName ) ) ;
            group.init() ;
            name2group.put( groupName, group ) ;
            id2group.put( groupId, group ) ;
        }
        cursor.close() ;
        }catch(BaseException e){
            throw new OperateException(e);
        }
    }

    public List< GroupWrapper > getAllDataGroup() {
        List< GroupWrapper > dataGroups = new ArrayList< GroupWrapper >() ;
        for ( Entry< String, GroupWrapper > entry : name2group.entrySet() ) {
            if ( entry.getKey().equals( "SYSSpare" )
                    && entry.getKey().equals( "SYSCatalogGroup" )
                    && entry.getKey().equals( "SYSCoord" ) ) {
                dataGroups.add( entry.getValue() ) ;
            }
        }

        return dataGroups ;
    }

    public List< String > getAllDataGroupName() {
        List< String > names = new ArrayList< String >() ;
        for ( Entry< String, GroupWrapper > entry : name2group.entrySet() ) {
            if ( entry.getKey().equals( "SYSSpare" )
                    && entry.getKey().equals( "SYSCatalogGroup" )
                    && entry.getKey().equals( "SYSCoord" ) ) {
                names.add( entry.getKey() ) ;
            }
        }

        return names ;
    }

    public GroupWrapper getGroupByName( String name ) {
        if ( name2group.containsKey( name ) ) {
            return name2group.get( name ) ;
        } else {
            return null ;
        }
    }

    public GroupWrapper getGroupById( int id ) {
        if ( id2group.containsKey( id ) ) {
            return id2group.get( id ) ;
        } else {
            return null ;
        }
    }

    public static GroupMgr getInstance() throws ReliabilityException {
        if ( mgr == null ) {
            mgr = new GroupMgr() ;
            mgr.init() ;
        }
        return mgr ;
    }
}
