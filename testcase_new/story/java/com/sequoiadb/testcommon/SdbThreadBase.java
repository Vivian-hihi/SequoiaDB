package com.sequoiadb.testcommon;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

public abstract class SdbThreadBase implements Runnable {
    private List<Exception> exceptionList = Collections.synchronizedList(new ArrayList<Exception>());
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
    public List<Exception> getExceptions() {
        join();
        return exceptionList;
    }

    public String getErrorMsg() {
        join();
        StringBuilder buffer = new StringBuilder();
        for (Exception exception : exceptionList) {
            buffer.append(getErrorMsg(exception));
        }
        return buffer.toString();
    }

    private String getErrorMsg(Exception e) {
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
            Iterator<Thread> iter = threadList.iterator();
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
        } catch (Exception e) {
            exceptionList.add(e);
        }
    }

    public abstract void exec() throws Exception;
}
