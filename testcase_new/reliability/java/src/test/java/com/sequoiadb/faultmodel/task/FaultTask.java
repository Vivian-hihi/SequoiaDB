package com.sequoiadb.faultmodel.task;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.LinkedList;

import com.sequoiadb.exception.FaultException;
import com.sequoiadb.faultmodel.fault.Fault;
import com.sequoiadb.faultmodel.msg.FaultMQ;
import com.sequoiadb.faultmodel.msg.FaultMsg;
import com.sequoiadb.faultmodel.utils.FaultUtils;

public class FaultTask {

    private LinkedList<FaultMsg> faultMsgs;
    private String faultName;

    private FaultTask() {
        faultMsgs = new LinkedList<>();
    }

    private FaultTask(String faultName) {
        this();
        this.faultName = faultName;
    }

    public static FaultTask getFault(String faultName) {
        return new FaultTask(faultName);
    }

    private void push(FaultMsg m) {
        faultMsgs.push(m);
    }

    private boolean isMakeFini() {
        FaultMQ.REQMSGQUEUE.empty();
        if (!"prepare".equals(makeStatus())) {
            return true;
        }
        return false;
    }

    private boolean isRestoreFini() {
        if (!"prepare".equals(restoreStatus())) {
            return true;
        }
        return false;
    }

    private String makeStatus() {
        for (FaultMsg m : faultMsgs) {
            if (-1 == m.getMakeStatus()) {
                return "fail";
            }
        }
        for (FaultMsg m : faultMsgs) {
            if (0 == m.getMakeStatus()) {
                return "prepare";
            }
        }
        return "ok";
    }

    private String restoreStatus() {
        for (FaultMsg m : faultMsgs) {
            if (-1 == m.getRestoreStatus()) {
                return "fail";
            }
        }
        for (FaultMsg m : faultMsgs) {
            if (0 == m.getRestoreStatus()) {
                return "prepare";
            }
        }
        return "ok";
    }

    @SuppressWarnings({ "unchecked", "rawtypes" })
    public void makeFault(String... names) throws Exception {
        try {
            Class<? extends Fault> fault = (Class<? extends Fault>) Class
                    .forName("com.sequoiadb.faultmodel.fault.impl." + faultName);
            Fault f = null;
            Constructor c = null;
            FaultMsg m = null;

            String hostName = names[0];
            String svcName = names[1];
            c = fault.getDeclaredConstructor(String.class, String.class);
            f = (Fault) c.newInstance(hostName, svcName);
            m = new FaultMsg(hostName, svcName);
            if (null == f) {
                throw new FaultException("Can not match any fault");
            }
            m.setFault(f);
            m.setMakeRestore("make");

            FaultMQ.REQMSGQUEUE.push(m);
            push(m);

        } catch (IllegalAccessException | IllegalArgumentException | InvocationTargetException | FaultException
                | ClassNotFoundException | NoSuchMethodException | SecurityException | InstantiationException e) {
            throw new Exception(e);
        }
    }

    public void make(String hostName, String svcName) throws Exception {
        makeFault(hostName, svcName);
        while (!isMakeFini()) {
        }
        if (!FaultUtils.isAllMsgSuccess()) {
            throw new Exception(FaultUtils.getErrorMsg());
        }
    }

    public void restore() throws Exception {
        for (FaultMsg m : faultMsgs) {
            m.setMakeRestore("restore");
            FaultMQ.REQMSGQUEUE.push(m);
        }
        while (!isRestoreFini()) {
        }
        if (!FaultUtils.isAllMsgSuccess()) {
            throw new Exception(FaultUtils.getErrorMsg());
        }
    }
}
