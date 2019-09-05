package com.sequoiadb.faultmodel.fault;

import java.util.logging.Logger;

import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.commlib.Ssh;
import com.sequoiadb.exception.FaultException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.faultmodel.fault.Fault;

public class FaultBase extends Fault {

    protected String hostName;
    protected String svcName;
    protected Ssh ssh;
    protected int port = 22;
    protected String user = "root";
    protected String passwd = SdbTestBase.rootPwd;
    protected String localScriptPath = SdbTestBase.scriptDir;
    protected String remoteScriptPath = SdbTestBase.workDir;
    protected String faultScriptName = "faultInjection.sh";
    protected String restoreScriptName = "restoreFaultInjection.sh";
    protected String result = "";
    protected final static Logger log = Logger.getLogger(FaultBase.class.getName());

    public FaultBase(String hostName, String svcName, String name) {
        super(name);
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
