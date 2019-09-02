package com.sequoiadb.faultmodel.th;

import com.sequoiadb.faultmodel.fault.Fault;
import com.sequoiadb.faultmodel.msg.FaultMQ;
import com.sequoiadb.faultmodel.msg.FaultMsg;

public class FaultWorkTh extends Thread {

    private volatile static FaultWorkTh workTh = new FaultWorkTh();

    private FaultWorkTh() {
    }

    @Override
    public void run() {
        try {
            Thread thisTh = Thread.currentThread();
            while (workTh == thisTh) {
                if (!checkMQ()) {
                    peek2WorkAndPop();
                } else {
                    try {
                        sleep(200);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
        } finally {
        }
    }

    private static void init() {
        FaultMQ.REQMSGQUEUE.clear();
        FaultMQ.RESPMSGQUEUE.clear();
    }

    private static void fini() {
        FaultMQ.REQMSGQUEUE.clear();
        FaultMQ.RESPMSGQUEUE.clear();
    }

    public static void startFault() {
        init();
        workTh.start();
    }

    public static void stopFault() {
        workTh = null;
        fini();
    }

    private boolean checkMQ() {
        return FaultMQ.REQMSGQUEUE.empty();
    }

    private FaultMsg peekFaultMsg() {
        return FaultMQ.REQMSGQUEUE.peek();
    }

    private void popFaultMsg() {
        FaultMQ.REQMSGQUEUE.pop();
    }

    private void push2RespMQ(FaultMsg msg) {
        FaultMQ.RESPMSGQUEUE.push(msg);
    }

    private void peek2WorkAndPop() {
        FaultMsg m = peekFaultMsg();
        Fault f = m.getFault();

        try {
            try {
                f.init();

                if ("make".equals(m.getMakeRestore())) {
                    sleep(m.getMaxDelay());

                    f.make();

                    if (f.checkMake()) {
                        m.setMakeStatus(1);
                    }
                } else if ("restore".equals(m.getMakeRestore())) {
                    f.restore();

                    if (f.checkRestore()) {
                        m.setRestoreStatus(1);
                    }
                }
            } finally {
                f.fini();
            }
        } catch (Exception e) {
            m.setMakeExp(e);
            System.out.println(m.getMakeStatus() + " : " + m.getMakeRestore());
            if (1 != m.getMakeStatus() && "make".equals(m.getMakeRestore())) {
                m.setMakeStatus(-1);
            }
            if (1 != m.getRestoreStatus() && "restore".equals(m.getMakeRestore())) {
                m.setRestoreStatus(-1);
            }
        } finally {
            popFaultMsg();
            push2RespMQ(m);
        }
    }
}
