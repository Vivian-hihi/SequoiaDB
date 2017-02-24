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

public abstract class OperateTask extends Task {
    public enum faultStatus {
        INIT, MAKESUCCESS, MAKEFAILURE, RESTORESUCESS, RESTOREFAILURE, EXCEPTION
    } ;

    private TaskMgr mgr ;

    public OperateTask( String name, int maxDuration ) {
        super( name, maxDuration ) ;
        // TODO Auto-generated constructor stub
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
            }
            exceptionList.add( e ) ;
        }
        setStatus( Task.TaskStatus.TASKSTOP ) ;
        mgr.Done( this ) ;
    }
    
    public Task getTaskByName(String name){
        return mgr.getTaskByName( name );
    }

}
