package com.sequoiadb.fault;

import com.sequoiadb.task.FaultMakeTask;

public class KillElasticSearch extends FaultElasticSearch {

    protected KillElasticSearch(String hostName) {
        super(hostName, "KillElasticSearch", "-9");
    }

    /**
     * 默认持续时间 3s
     * 
     * @param hostName   主机名
     * @param maxDlay    最大延迟启动时间，单位：s
     * @param checkTimes 构造成功与否检查次数，每次检查完后 sleep 0.5s
     * @return
     */
    public static FaultMakeTask geFaultMakeTask(String hostName, int maxDlay, int checkTimes) {
        FaultMakeTask task = null;
        KillElasticSearch ke = new KillElasticSearch(hostName);
        task = new FaultMakeTask(ke, maxDlay, 3, checkTimes);
        return task;
    }

    /**
     * 默认检查次数 120 次，持续时间 3s
     * 
     * @param hostName 主机名
     * @param maxDelay 最大延迟启动时间，单位：s
     * @return
     */
    public static FaultMakeTask geFaultMakeTask(String hostName, int maxDelay) {
        return geFaultMakeTask(hostName, maxDelay, 120);
    }
}
