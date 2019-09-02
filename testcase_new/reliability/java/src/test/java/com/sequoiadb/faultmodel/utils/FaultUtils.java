package com.sequoiadb.faultmodel.utils;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.List;

import com.sequoiadb.faultmodel.msg.FaultMQ;
import com.sequoiadb.faultmodel.msg.FaultMsg;

public class FaultUtils {

    private static List<Exception> lExceptions = new ArrayList<>();

    public static boolean isAllMsgSuccess() {
        while (true) {
            int len = FaultMQ.REQMSGQUEUE.getLenth();
            if (0 != len) {
                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                continue;
            }

            int reqCount = FaultMQ.REQMSGQUEUE.getPullCount();
            int respCount = FaultMQ.RESPMSGQUEUE.getPullCount() + FaultMQ.RESPMSGQUEUE.getLenth();
            if (reqCount != respCount) {
                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                continue;
            }

            break;
        }

        lExceptions.clear();
        for (int i = 0; i < FaultMQ.RESPMSGQUEUE.getLenth(); i++) {
            FaultMsg m = FaultMQ.RESPMSGQUEUE.peek();
            if (null != m.getMakeExp()) {
                lExceptions.add(m.getMakeExp());
            }
            FaultMQ.RESPMSGQUEUE.pop();
        }

        return lExceptions.isEmpty();
    }

    public static String getErrorMsg() {
        StringBuffer reStr = new StringBuffer();
        for (Exception e : lExceptions) {
            if (e == null)
                return "";
            ByteArrayOutputStream bytes = new ByteArrayOutputStream();
            PrintStream printStream = new PrintStream(bytes);
            printStream.println();
            printStream.println("------Fault Make: " + Thread.currentThread().getName() + " err msg start: ");
            e.printStackTrace(printStream);
            printStream.println("------Fault Make: " + Thread.currentThread() + " err msg end.");
            printStream.flush();
            reStr.append(bytes.toString());
        }
        return reStr.toString();
    }
}
