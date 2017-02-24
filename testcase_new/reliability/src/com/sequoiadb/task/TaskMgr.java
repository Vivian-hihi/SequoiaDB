/**
 * Copyright (c) 2017, SequoiaDB Ltd.
 * File Name:TaskMgr.java
 * 
 *
 *  @author wenjingwang
 * Date:2017-2-21下午4:54:48
 *  @version 1.00
 */
package com.sequoiadb.task ;

import java.util.HashMap ;
import java.util.List ;
import java.util.Map ;
import java.util.Map.Entry ;

import com.sequoiadb.exception.CommException ;

public class TaskMgr {
    private Map< String, Task > taskSet = new HashMap< String, Task >() ;

    public TaskMgr() {

    }

    /**
     * @param task
     *            任务
     */
    public void addTask( Task task ) {
        if ( !taskSet.containsKey( task.getName() ) ) {
            taskSet.put( task.getName(), task ) ;
        }
    }

    /**
     * @param task
     *            任务
     */
    public void removeTask( Task task ) {
        if ( taskSet.containsKey( task.getName() ) ) {
            taskSet.remove( task.getName() ) ;
        }
    }

    /**
     * @return 所有任务初始化成功，返回true，任一任务初始化失败，则返回false
     */
    public boolean init() {
        for ( Map.Entry< String, Task > entry : taskSet.entrySet() ) {
            if ( !entry.getValue().init() )
                return false ;
        }

        return true ;
    }

    /**
     * @return 所有任务反初始化成功，返回true，任一任务反初始化失败，则返回false
     */
    public boolean fini() {
        for ( Map.Entry< String, Task > entry : taskSet.entrySet() ) {
            if ( !entry.getValue().fini() )
                return false ;
        }

        return true ;
    }

    public void start() {
        for ( Map.Entry< String, Task > entry : taskSet.entrySet() ) {
            entry.getValue().start() ;
        }
    }

    public void join() {
        for ( Map.Entry< String, Task > entry : taskSet.entrySet() ) {
            try {
                entry.getValue().join() ;
            } catch ( InterruptedException e ) {
                // TODO Auto-generated catch block
                throw new CommException( e ) ;
            }
        }
    }

    /**
     * @param name
     *            任务名
     * @return 返回对应任务名的任务，不存在返回null
     */
    public Task getTaskByName( String name ) {
        if ( taskSet.containsKey( name ) ) {
            return taskSet.get( name ) ;
        }

        return null ;
    }

    /**
     * @param task
     *            执行完成的任务
     */
    public void Done( Task task ) {
        if ( task.getStatus() == Task.TaskStatus.TASKTHROWEXCEPTION ) {
            for ( Entry< String, Task > entry : taskSet.entrySet() ) {
                if ( !task.getName().equals( entry.getKey() ) ||
                      task.getClass().equals( FaultMakeTask.class )) {
                    entry.getValue().interrupt() ;
                }
            }
        }
    }

    public Map< String, List< Exception >> getExceptions() {
        join() ;
        HashMap< String, List< Exception >> map = new HashMap< String, List< Exception >>() ;
        for ( Map.Entry< String, Task > entry : taskSet.entrySet() ) {
            map.put( entry.getKey(), entry.getValue().getExceptionList() ) ;
        }
        return map ;
    }

    public String getErrorMsg() {
        String reStr = new String() ;
        for ( Map.Entry< String, Task > entry : taskSet.entrySet() ) {
            if ( !entry.getValue().isSuccess() ) {
                reStr += "Thread " + entry.getKey() + " ErrorMsg:\r\n" ;
                reStr += entry.getValue().getErrorMsg() ;
            }
        }
        return reStr ;
    }
}
