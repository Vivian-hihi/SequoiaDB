/**
 * Copyright (c) 2017, SequoiaDB Ltd.
 * File Name:OperateTask.java
 * 
 *
 *  @author wenjingwang
 * Date:2017-2-21下午4:54:48
 *  @version 1.00
 */
package com.sequoiadb.task ;

import com.sequoiadb.exception.ReliabilityException ;

public abstract class OperateTask extends Task {
    public enum faultStatus {
        INIT, MAKESUCCESS, MAKEFAILURE, RESTORESUCESS, RESTOREFAILURE, EXCEPTION
    } ;

    private TaskMgr mgr = null ;
    private static final int defaultDuration = 5 ;

    public OperateTask( String name ) {
        super( name, defaultDuration ) ;

        // TODO Auto-generated constructor stub
    }

    public void setMgr( TaskMgr mgr ) {
        this.mgr = mgr ;
    }

    public abstract void faultMakeNotify( faultStatus status ) ;

    public abstract void faultRestoreNotify( faultStatus status ) ;

    public abstract void Do() throws Exception ;

    public void run() {
        setStatus( Task.TaskStatus.TASKSTART ) ;
        try {
            Do() ;
        } catch ( Exception e ) {
            if ( e instanceof InterruptedException ) {
                setStatus( Task.TaskStatus.TASKINTERRUPT ) ;
            } else {
                setStatus( Task.TaskStatus.TASKTHROWEXCEPTION ) ;
                exception = ( ReliabilityException ) e ;
            }
            // exceptionList.add( e ) ;
        }
        if ( exception == null ) {
            setStatus( Task.TaskStatus.TASKSTOP ) ;
        }
        mgr.Done( this ) ;
    }

    public Task getTaskByName( String name ) {
        return mgr.getTaskByName( name ) ;
    }

}
