package com.sequoiadb.testcommon;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public abstract class SdbThreadBase implements Runnable {
    private List<Throwable> exceptionList = Collections.synchronizedList(new ArrayList<Throwable>());
    private List<Thread> threadList = new ArrayList<>();

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
     *  getBlockingMethod --
     *   获取当前线程阻塞在哪一个函数调用上
     *    
     * Parameters:
     *       无
     *
     * Returns:
     *       如果当前线程执行CL.update()阻塞，则返回update
     *       如果当前线程执行CL.query()阻塞，则返回query
     *--------------------------------------------------------------------------
     */
    public String getBlockingMethod(){
        assert threadList.size() == 1 ;
        
        final int oneSeonds = 5000 ;
        final int totalTimes = 3 ;
        int nullTimes = 0 ;
        int matchTimes = 0 ;
        int alreadyWaitTime = 0 ;
        String prevMethod = "" ;
        
        do{
            StackTraceElement[] stackElem = threadList.get( 0 ).getStackTrace() ;
            try {
                Thread.sleep( 5 ) ;
                alreadyWaitTime += 1;
            } catch ( InterruptedException e ) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
           
            if ( nullTimes >= totalTimes || alreadyWaitTime >= oneSeonds  ){
                return "" ;
            }
            
            if ( stackElem.length == 0 ) {
                nullTimes++ ;
                continue ;
            }
            
            prevMethod = stackElem[0].getMethodName() ;
            StackTraceElement[] currentStackElem = threadList.get( 0 ).getStackTrace() ;
            if ( currentStackElem.length == 0 ){
                return "" ;
            }
            
            String curMethod  = currentStackElem[0].getMethodName() ;
            if ( curMethod.equals( prevMethod )){
                matchTimes++ ;
            }
            
            if (  matchTimes >= totalTimes ){
                break ;
            }

            prevMethod = curMethod ;
        }while(true) ;
        
        return prevMethod ;
    }

    public abstract void exec() throws Exception;
    
    public static void main(String[] args){
        SdbThreadBase t = new SdbThreadBase(){

            @Override
            public void exec() throws Exception {
                // TODO Auto-generated method stub
                Thread.sleep(5000) ;
            }
        } ;
        
        t.start() ;
        t.getBlockingMethod() ;
        t.getBlockingMethod() ;
    }
}
