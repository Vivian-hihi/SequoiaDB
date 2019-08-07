package com.sequoiadb.fault;

import com.sequoiadb.task.FaultMakeTask;

public class RestartSdbseadapter extends FaultSdbseadapterBase {
    protected RestartSdbseadapter(String hostName, String svcName) {
        super(hostName, svcName, "RestartSdbseadapter", "-15");
    }

    /**
     * 默认持续时间 3s
     * 
     * @param hostName   主机名
     * @param svcName    适配器对应的节点的 svcname
     * @param maxDlay    最大延迟启动时间，单位：s
     * @param checkTimes 构造成功与否检查次数，每次检查完后 sleep 0.5s
     * @return
     */
    public static FaultMakeTask geFaultMakeTask(String hostName, String svcName, int maxDlay, int checkTimes) {
        FaultMakeTask task = null;
        RestartSdbseadapter rs = new RestartSdbseadapter(hostName, svcName);
        task = new FaultMakeTask(rs, maxDlay, 3, checkTimes);
        return task;
    }

    /**
     * 默认检查次数 300 次，持续时间 3s
     * 
     * @param hostName 主机名
     * @param svcName  适配器对应的节点的 svcname
     * @param maxDelay 最大延迟启动时间，单位：s
     * @return
     */
    public static FaultMakeTask geFaultMakeTask(String hostName, String svcName, int maxDelay) {
        return geFaultMakeTask(hostName, svcName, maxDelay, 300);
    }
}
