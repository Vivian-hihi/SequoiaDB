package com.sequoiadb.fault;

import java.util.logging.Logger;

import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.commlib.Ssh;
import com.sequoiadb.exception.FaultException;
import com.sequoiadb.exception.ReliabilityException;

public class FaultElasticSearch extends Fault {
    protected String hostName;
    protected String pid = "-1";
    protected String commandDir;
    protected String user;
    protected String password;
    protected int port;
    protected Ssh ssh;
    protected String killOrRestart;
    protected String remotePath = SdbTestBase.workDir;
    protected String scriptName = "faultElasticSearch.sh";
    protected String seadaptDir = SdbTestBase.sdbseadapterDir;
    protected final String localScriptPath = SdbTestBase.scriptDir;
    protected final static Logger log = Logger.getLogger(FaultElasticSearch.class.getName());

    protected FaultElasticSearch(String hostName, String className, String killOrRestart) {
        super(className);
        this.hostName = hostName;
        this.killOrRestart = killOrRestart;
        user = SdbTestBase.remoteUser;
        password = SdbTestBase.remotePwd;
        port = 22;
    }

    @Override
    public void make() throws FaultException {
        log.info("target elasticsearch: " + hostName);
        try {
            ssh.exec(remotePath + "/" + scriptName + " " + killOrRestart);
            String stdout = ssh.getStdout().trim();
            pid = stdout.split(":")[0];
            commandDir = stdout.split(":")[1] + "/bin/elasticsearch";
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
            ssh.exec("ps -ef | grep elasticsearch | grep -v grep | awk '{print $2}'");
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
            ssh.exec(commandDir + " -d");
        } catch (ReliabilityException e) {
            throw new FaultException(e);
        }
    }

    @Override
    public boolean checkRestoreResult() throws FaultException {
        try {
            ssh.exec("ps -ef | grep elasticsearch | grep -v grep | awk '{print $2}'");
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
