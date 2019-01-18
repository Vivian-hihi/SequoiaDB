package com.sequoiadb.testcommon;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;
import java.lang.Thread.State ;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public abstract class SdbThreadBase implements Runnable {
    private List<Throwable> exceptionList = Collections.synchronizedList(new ArrayList<Throwable>());
    private List<Thread> threadList = new ArrayList<>();
    private Integer sync = new Integer(0) ;
    private Object result = null ;
    
    public void start() {
        start(1);
    }

    public void start(int threadNum) {
        synchronized (this) {
            for (int i = 0; i < threadNum; i++) {
                Thread t = new Thread(this);
                threadList.add(t);
                t.start();
            }
        }
    }
    
    /*
     *--------------------------------------------------------------------------
     *
     *  getExecResult --
     *   获取线程的执行结果，只适应启动一个线程的情况
     *   必须在线程结束前通过setExecResult进行设置
     *   如果线程没有开始或者已经结束调用直接返回，否则会阻塞到结果被设置
     * Parameters:
     *
     * Returns:
     *       结果对象实例，如果是一个DBCursor，返回后通过
     *                 Object ret = getExecResult() ;
     *                 if ( ret instanceof DBCursor){
     *                    DBCursor cursor = (DBCursor)ret;
     *                 }
     *--------------------------------------------------------------------------
     */
    public Object getExecResult() throws InterruptedException{
        if ( this.threadList.isEmpty() ||
             this.threadList.get( 0 ).getState() == State.NEW ||
             this.threadList.get( 0 ).getState() == State.TERMINATED ){
            return this.result ;
        }
        
        synchronized( sync ){
            sync.wait();
        }
        return this.result ;
    }
    
    /*
     *--------------------------------------------------------------------------
     *
     *  setExecResult --
     *   设置线程的执行结果，只适合启单个线程的情况
     *    
     * Parameters:
     *       result: Object 可以是任意对象类型
     *
     * Returns:
     *       void 
     *--------------------------------------------------------------------------
     */
    public void setExecResult(Object result){
        assert this.threadList.size() == 1 ;
        this.result = result ;
        synchronized( sync ){
            sync.notifyAll();
        }
    }

    // 返回结果集
    public List<Throwable> getExceptions() {
        join();
        return exceptionList;
    }

    public String getErrorMsg() {
        join();
        StringBuilder buffer = new StringBuilder();
        for (Throwable exception : exceptionList) {
            buffer.append(getErrorMsg(exception));
        }
        return buffer.toString();
    }

    private String getErrorMsg(Throwable e) {
        if (e == null)
            return "";
        ByteArrayOutputStream bytes = new ByteArrayOutputStream();
        PrintStream printStream = new PrintStream(bytes);
        printStream.println();
        printStream.println("------  err msg start: ");
        e.printStackTrace(printStream);
        printStream.println("------  err msg end.");
        printStream.flush();
        return bytes.toString();
    }

    // join所有线程
    public void join() {
        synchronized (this) {
            for (Thread thread : threadList) {
                try {
                    thread.join();
                } catch (InterruptedException e) {
                    exceptionList.add(e);
                }
            }
            threadList.clear();
        }
    }

    public boolean isSuccess() {
        join();
        if (exceptionList.size() != 0) {
            return false;
        }
        return true;
    }

    public void run() {
        try {
            exec();
        } catch (Throwable e) {
            exceptionList.add(e);
        }
    }
    
    /*
     *--------------------------------------------------------------------------
     *
     *  matchBlockingMethod --
     *   当前线程是否阻塞在相应的调用上
     *    
     * Parameters:
     *       className: 类名 (DBCollection.class.getName())
     *       methodName: 方法名(query ...)
     *
     * Returns:
     *       如果当前线程执行CL.update()阻塞
     *         matchBlockingMethod(cl.getClass().getName(), "update")则返回true
     *       如果当前线程执行CL.query()阻塞，则返回true
     *         matchBlockingMethod(cl.getClass().getName(), "query")则返回true
     *       否则返回false 
     *--------------------------------------------------------------------------
     */
    public boolean matchBlockingMethod(String className, String methodName){
        assert threadList.size() == 1 ;
        
        final int fiveSeonds = 5000 ;
        final int totalTimes = 3 ;
        int nonMatchTimes = 0 ;
        int matchTimes = 0 ;
        int alreadyWaitTime = 0 ;
        boolean ret = true ;
        
        int pos = 0 ;
        do{
            if ( nonMatchTimes >= totalTimes || alreadyWaitTime >= fiveSeonds  ){
                ret = false ;
                break ;
            }
            
            if ( threadList.get( 0 ).getState() == State.TERMINATED ){
                ret = false ;
                break ;
            }
            
            try {
                Thread.sleep( 5 ) ;
                alreadyWaitTime += 1 ;
            } catch ( InterruptedException e ) {
                e.printStackTrace();
            }
            
            if ( threadList.get( 0 ).getState() == State.NEW ){
                continue ;
            }
            
            StackTraceElement[] stackElem = threadList.get( 0 ).getStackTrace() ;
            if ( pos != 0 ){
                stackElem = threadList.get( 0 ).getStackTrace() ;
                if ( stackElem.length == 0 
                     || stackElem.length <= pos ){
                    ret = false ;
                    break ;
                }
                
                if ( stackElem[pos].getClassName().equals( className ) 
                        && stackElem[pos].getMethodName().equals( methodName ) ){
                    ++matchTimes ;
                }
            }
            else{
                for ( pos = 0; pos < stackElem.length; ++pos ){
                    if ( stackElem[pos].getClassName().equals( className ) 
                        && stackElem[pos].getMethodName().equals( methodName ) ){
                        ++matchTimes ;
                        break ;
                    }
                }
                
                if ( pos == stackElem.length){
                    nonMatchTimes++;
                    pos = 0 ;
                }
            }
            
            if (  matchTimes >= totalTimes ){
                break ;
            }
            
        }while(true) ;
        
        return ret ;
    }

    public abstract void exec() throws Exception;
    
    public static void main(String[] args){
        
    }
}
