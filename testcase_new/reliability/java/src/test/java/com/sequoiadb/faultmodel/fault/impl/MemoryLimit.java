package com.sequoiadb.faultmodel.fault.impl;

import java.util.logging.Logger;

import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.commlib.Ssh;
import com.sequoiadb.exception.FaultException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.faultmodel.fault.Fault;
import com.sequoiadb.faultmodel.fault.FaultName;

public class MemoryLimit extends Fault {

    private String hostName;
    private String svcName;
    private Ssh ssh;
    private int port = 22;
    private String user = "root";
    private String passwd = SdbTestBase.rootPwd;
    private String localScriptPath = SdbTestBase.scriptDir;
    private String remoteScriptPath = SdbTestBase.workDir;
    private String faultScriptName = "faultInjection.sh";
    private String restoreScriptName = "restoreFaultInjection.sh";
    private String result = "";
    private final static Logger log = Logger.getLogger(MemoryLimit.class.getName());

    public MemoryLimit(String hostName, String svcName) {
        super(FaultName.MEMORYLIMIT);
        this.hostName = hostName;
        this.svcName = svcName;
    }

    @Override
    public void make() throws FaultException {
        log.info(getName() + " make target node:[" + hostName + ":" + svcName + "]");
        try {
            resetResult();
            ssh.exec(remoteScriptPath + "/" + faultScriptName + " " + getName() + "  " + svcName);
            result = ssh.getStdout().trim();
        } catch (ReliabilityException e) {
            throw new FaultException(e);
        }
    }

    @Override
    public boolean checkMake() throws FaultException {
        String pid = result.substring(0, result.indexOf(":"));
        String output = result.substring(result.indexOf(":") + 1);
        if (pid.length() > 0) {
            if (output.length() > 0) {
                throw new FaultException(output);
            }
            return true;
        } else {
            result = "Can not match the pid for " + svcName;
            throw new FaultException(result);
        }
    }

    @Override
    public void restore() throws FaultException {
        log.info(getName() + " restore target node:[" + hostName + ":" + svcName + "]");
        try {
            resetResult();
            ssh.exec(remoteScriptPath + "/" + restoreScriptName + " " + getName() + "  " + svcName);
            result = ssh.getStdout().trim();
        } catch (ReliabilityException e) {
            throw new FaultException(e);
        }
    }

    @Override
    public boolean checkRestore() throws FaultException {
        String pid = result.substring(0, result.indexOf(":"));
        String output = result.substring(result.indexOf(":") + 1);
        if (pid.length() > 0) {
            if (output.length() > 0) {
                throw new FaultException(output);
            }
            return true;
        } else {
            result = "Can not match the pid for " + svcName;
            throw new FaultException(result);
        }
    }

    @Override
    public void init() throws FaultException {
        try {
            ssh = new Ssh(hostName, user, passwd, port);
            ssh.scpTo(localScriptPath + "/" + faultScriptName, remoteScriptPath);
            ssh.exec("chmod 777 " + remoteScriptPath + "/" + faultScriptName);
            ssh.scpTo(localScriptPath + "/" + restoreScriptName, remoteScriptPath);
            ssh.exec("chmod 777 " + remoteScriptPath + "/" + restoreScriptName);
        } catch (ReliabilityException e) {
            throw new FaultException(e);
        }
    }

    @Override
    public void fini() throws FaultException {
        try {
            if (null != ssh) {
                ssh.exec("rm -rf " + remoteScriptPath + "/" + faultScriptName);
                ssh.exec("rm -rf " + remoteScriptPath + "/" + restoreScriptName);
                ssh.disconnect();
            }
        } catch (ReliabilityException e) {
            throw new FaultException(e);
        }
    }

    private void resetResult() {
        if (result.length() > 0) {
            result = "";
        }
    }
}
