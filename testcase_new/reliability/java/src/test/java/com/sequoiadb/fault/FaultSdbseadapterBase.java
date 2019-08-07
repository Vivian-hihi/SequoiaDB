package com.sequoiadb.fault;

import java.util.logging.Logger;

import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.commlib.Ssh;
import com.sequoiadb.exception.FaultException;
import com.sequoiadb.exception.ReliabilityException;

public class FaultSdbseadapterBase extends Fault {
    protected String hostName;
    protected String svcName;
    protected String seadaptSvcName;
    protected String pid = "-1";
    protected String commandDir;
    protected String user;
    protected String password;
    protected int port;
    protected Ssh ssh;
    protected String killOrRestart;
    protected String remotePath = SdbTestBase.workDir;
    protected String scriptName = "faultSdbseadapter.sh";
    protected String seadaptDir = SdbTestBase.sdbseadapterDir;
    protected final String localScriptPath = SdbTestBase.scriptDir;
    protected final static Logger log = Logger.getLogger(FaultSdbseadapterBase.class.getName());

    protected FaultSdbseadapterBase(String hostName, String svcName, String className, String killOrRestart) {
        super(className);
        this.hostName = hostName;
        this.svcName = svcName;
        this.killOrRestart = killOrRestart;
        seadaptSvcName = svcName.substring(0, svcName.length() - 1);
        user = "root";
        password = SdbTestBase.rootPwd;
        port = 22;
    }

    @Override
    public void make() throws FaultException {
        log.info("target sdbseadapter: " + hostName + ":" + seadaptSvcName + "*");
        try {
            ssh.exec(remotePath + "/" + scriptName + " " + seadaptSvcName + " " + killOrRestart);
            String stdout = ssh.getStdout().trim();
            pid = stdout.split(":")[0];
            commandDir = stdout.split(":")[1];
        } catch (ReliabilityException e) {
            throw new FaultException(e);
        }
    }

    @Override
    public boolean checkMakeResult() throws FaultException {
        if (pid.equals("-1")) {
            return false;
        }
        try {
            ssh.exec("ps -ef | grep sdbseadapter | grep -v grep | grep " + seadaptSvcName + " | awk '{print $2}'");
            if (ssh.getStdout().trim().length() <= 0) {
                return true;
            } else {
                return false;
            }
        } catch (ReliabilityException e) {
            throw new FaultException(e);
        }
    }

    @Override
    public void restore() throws FaultException {
        try {
            ssh.exec("nohup " + commandDir + " -c " + seadaptDir + "/" + svcName + " 2>&1 &");
        } catch (ReliabilityException e) {
            throw new FaultException(e);
        }
    }

    @Override
    public boolean checkRestoreResult() throws FaultException {
        try {
            ssh.exec("ps -ef | grep sdbseadapter | grep -v grep | grep " + seadaptSvcName + " | awk '{print $2}'");
            if (ssh.getStdout().trim().length() <= 0) {
                return false;
            }
            return true;
        } catch (ReliabilityException e) {
            throw new FaultException(e);
        }
    }

    @Override
    public void init() throws FaultException {
        try {
            ssh = new Ssh(hostName, user, password, port);
            ssh.scpTo(localScriptPath + "/" + scriptName, remotePath + "/");
            ssh.exec("chmod 777 " + remotePath + "/" + scriptName);
        } catch (ReliabilityException e) {
            throw new FaultException(e);
        }
    }

    @Override
    public void fini() throws FaultException {

        try {
            if (ssh != null) {
                ssh.exec("rm -rf " + remotePath + "/" + scriptName);
            }
        } catch (ReliabilityException e) {
            throw new FaultException(e);
        }
    }
}
