package com.sequoiadb.faultmodel.msg;

import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicInteger;

public enum FaultMQ {
    REQMSGQUEUE, RESPMSGQUEUE;

    private ConcurrentLinkedQueue<FaultMsg> msgQueue = new ConcurrentLinkedQueue<>();
    private AtomicInteger hasPullCount = new AtomicInteger(0);

    public void push(FaultMsg msg) {
        msgQueue.add(msg);
    }

    public FaultMsg peek() {
        return msgQueue.peek();
    }

    public void pop() {
        msgQueue.poll();
        hasPullCount.incrementAndGet();
    }

    public boolean empty() {
        return msgQueue.isEmpty();
    }

    public void clear() {
        msgQueue.clear();
    }

    public int getLenth() {
        return msgQueue.size();
    }

    public int getPullCount() {
        return hasPullCount.get();
    }

    @Override
    public String toString() {
        return msgQueue.toString();
    }
}
