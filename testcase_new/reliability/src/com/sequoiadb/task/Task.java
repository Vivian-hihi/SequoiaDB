/**
 * Copyright (c) 2017, SequoiaDB Ltd. File Name:Task.java
 *
 * @author wenjingwang Date:2017-2-21下午4:54:48
 * @version 1.00
 */
package com.sequoiadb.task;

import com.sequoiadb.exception.ReliabilityException;

public abstract class Task extends Thread {

    public enum TaskStatus {
        TASKSTART,
        TASKINTERRUPT,
        TASKTHROWEXCEPTION,
        TASKSTOP
    }

    protected TaskStatus status;
    private Exception exception;

    public Task() {
        super();
    }

    public Task(String name) {
        super(name);
    }

    public Exception getException() {
        return exception;
    }

    protected void setException(Exception exception) {
        this.exception = exception;
    }

    public abstract void init() throws ReliabilityException;

    @Deprecated
    public abstract void check() throws ReliabilityException;

    public abstract void fini() throws ReliabilityException;

    public void setStatus(TaskStatus status) {
        this.status = status;
    }

    public TaskStatus getStatus() {
        return this.status;
    }

    /**
     * 等待某一阶段的任务完成 注：只能用于线程方法中
     */
//    public void waitComplete() {
//        synchronized (this) {
//            try {
//                this.wait();
//            } catch (InterruptedException e) {
//                status = TaskStatus.TASKINTERRUPT;
//            }
//        }
//    }

    /**
     * 通知等待的任务，当前任务某一阶段的任务已经完成 注：只能用于线程方法中
     */
//    public void notifyComplete() {
//        synchronized (this) {
//            this.notify();
//        }
//    }

    /**
     * 通知所有等待的任务，当前任务某一阶段的任务已经完成 注：只能用于线程方法中
     */
//    public void notifyAllComplete() {
//    synchronized (this)
//    {
//        this.notifyAll();
//    }
//}
    public boolean isSuccess() {
        return this.getException() == null;
    }

    public String getErrorMsg() {
        String reStr = "";
        if (exception == null) {
            return reStr;
        }

        // errorMsg
        reStr += getName();
        reStr += " ErrMsg: ";
        reStr += exception.getMessage();
        reStr += "\r\n";

        reStr += "StackTrace: \r\n";
        // stackMsg
        StringBuffer stackBuffer = new StringBuffer();
        StackTraceElement[] stackElements = exception.getStackTrace();
        for (int j = 0; j < stackElements.length; j++) {
            stackBuffer.append(stackElements[j].toString()).append("\r\n");
        }
        reStr += stackBuffer.toString();
        return reStr;
    }
}
