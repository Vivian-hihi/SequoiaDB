/**
 * Copyright (c) 2017, SequoiaDB Ltd.
 * File Name:Task.java
 * 
 *
 *  @author wenjingwang
 * Date:2017-2-21下午4:54:48
 *  @version 1.00
 */
package com.sequoiadb.task ;

import java.util.ArrayList ;
import java.util.Collections ;
import java.util.List ;

import com.sequoiadb.exception.CommException ;

public abstract class Task extends Thread {
    public enum TaskStatus {
        TASKSTART, TASKINTERRUPT, TASKTHROWEXCEPTION, TASKSTOP
    } ;

    protected int randomStartMaxDuration ;
    protected TaskStatus status ;
    protected List< Exception > exceptionList = Collections
            .synchronizedList( new ArrayList< Exception >() ) ;

    public Task( String name, int maxDuration ) {
        this.setName( name ) ;
        this.randomStartMaxDuration = maxDuration ;
    }

    public abstract boolean init() ;

    public abstract boolean fini() ;

    public void setStatus( TaskStatus status ) {
        this.status = status ;
    }

    public TaskStatus getStatus() {
        return this.status ;
    }

    /**
     * 等待某一阶段的任务完成
     * 
     */
    public void waitComplete() {
        synchronized ( this ) {
            try {
                this.wait() ;
            } catch ( InterruptedException e ) {
                status = TaskStatus.TASKINTERRUPT ;
            }
        }
    }

    /**
     * 通知等待的任务，当前任务某一阶段的任务已经完成
     * 
     */
    public void notifyComplete() {
        synchronized ( this ) {
            this.notify() ;
        }
    }

    /**
     * 通知所有等待的任务，当前任务某一阶段的任务已经完成
     * 
     */
    public void notifyAllComplete() {
        synchronized ( this ) {
            this.notifyAll() ;
        }
    }

    public List< Exception > getExceptionList() {
        return exceptionList ;
    }

    public boolean isSuccess() {
        try {
            join() ;
        } catch ( InterruptedException e ) {
            throw new CommException( e ) ;
        }
        if ( exceptionList.size() != 0 ) {
            return false ;
        }
        return true ;
    }

    public String getErrorMsg() {
        String reStr = new String() ;
        for ( int i = 0; i < exceptionList.size(); i++ ) {
            // errorMsg
            reStr += exceptionList.get( i ).getMessage() ;
            reStr += "\r\n" ;

            // stackMsg
            StringBuffer stackBuffer = new StringBuffer() ;
            StackTraceElement[] stackElements = exceptionList.get( i )
                    .getStackTrace() ;
            for ( int j = 0; j < stackElements.length; j++ ) {
                stackBuffer.append( stackElements[j].toString() ).append(
                        "\r\n" ) ;
            }
            reStr += stackBuffer.toString() ;
        }
        return reStr ;
    }
}
