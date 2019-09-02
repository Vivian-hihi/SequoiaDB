package com.sequoiadb.faultmodel.msg;

import com.sequoiadb.faultmodel.fault.Fault;

public class FaultMsg {

    private String faultName;
    private Fault fault;

    private double maxDelay;
    private double during;

    private volatile int makeStatus;
    private volatile int restoreStatus;
    private volatile String makeRestore;

    private String hostName;
    private String svcName;

    private Exception makeExp;

    public FaultMsg() {
        setMakeStatus(0);
        setRestoreStatus(0);
    }

    public FaultMsg(String hostName) {
        this();
        this.hostName = hostName;
    }

    public FaultMsg(String hostName, String svcName) {
        this();
        this.hostName = hostName;
        this.svcName = svcName;
    }

    public String getFaultName() {
        return faultName;
    }

    public void setFaultName(String faultName) {
        this.faultName = faultName;
    }

    public Fault getFault() {
        return fault;
    }

    public void setFault(Fault fault) {
        this.fault = fault;
    }

    public long getMaxDelay() {
        return (long) maxDelay;
    }

    public void setMaxDelay(double maxDelay) {
        this.maxDelay = maxDelay * 1000;
    }

    public long getDuring() {
        return (long) during;
    }

    public void setDuring(double during) {
        this.during = during * 1000;
    }

    public int getMakeStatus() {
        return makeStatus;
    }

    public void setMakeStatus(int makeStatus) {
        this.makeStatus = makeStatus;
    }

    public int getRestoreStatus() {
        return restoreStatus;
    }

    public void setRestoreStatus(int restoreStatus) {
        this.restoreStatus = restoreStatus;
    }

    public String getMakeRestore() {
        return makeRestore;
    }

    public void setMakeRestore(String makeRestore) {
        this.makeRestore = makeRestore;
    }

    public String getHostName() {
        return hostName;
    }

    public void setHostName(String hostName) {
        this.hostName = hostName;
    }

    public String getSvcName() {
        return svcName;
    }

    public void setSvcName(String svcName) {
        this.svcName = svcName;
    }

    public Exception getMakeExp() {
        return makeExp;
    }

    public void setMakeExp(Exception makeExp) {
        this.makeExp = makeExp;
    }

    @Override
    public String toString() {
        return "FaultMsg [faultName=" + faultName + ", fault=" + fault + ", maxDelay=" + maxDelay + ", during=" + during
                + ", makeStatus=" + makeStatus + ", restoreStatus=" + restoreStatus + ", makeRestore=" + makeRestore
                + ", hostName=" + hostName + ", svcName=" + svcName + ", makeExp=" + makeExp + "]";
    }
}
