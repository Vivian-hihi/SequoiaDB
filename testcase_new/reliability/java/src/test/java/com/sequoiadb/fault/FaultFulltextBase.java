package com.sequoiadb.fault;

import java.util.logging.Logger;

import com.sequoiadb.commlib.Ssh;
import com.sequoiadb.exception.FaultException;
import com.sequoiadb.exception.ReliabilityException;

public class FaultFulltextBase extends Fault {

    protected String user;
    protected String password;
    protected Ssh ssh;
    protected String hostName;
    protected String svcName;
    protected int port;
    protected String localPath;
    protected String remotePath;
    protected String makeScriptName = "faultFulltext.sh";
    protected String restoreScriptName = "restoreFulltext.sh";
    protected String progName;
    protected String killRestart;
    protected String pid = "-1";
    protected String cmdDir;
    protected String cmdArgs;
    protected final static Logger log = Logger.getLogger(FaultFulltextBase.class.getName());

    public FaultFulltextBase(String name) {
        super(name);
    }

    @Override
    public void make() throws FaultException {
        if (svcName != null) {
            log.info("Make " + this.getName() + " " + hostName + ": " + svcName);
        } else {
            svcName = "";
            log.info("Make " + this.getName() + " " + hostName);
        }

        try {
            ssh.exec(remotePath + "/" + makeScriptName + " " + progName + " " + killRestart + " " + svcName);
            getMakeStdout();
        } catch (ReliabilityException e) {
            throw new FaultException(e);
        }
    }

    /**
     * 继承FaultFulltextBase需要实现该方法，用来解析make()的返回值，返回值pid:cmdDir
     * 
     * @exem String stdout = ssh.getStdout().trim(); pid = stdout.split(":")[0];
     *       cmdDir = stdout.split(":")[1];
     */
    protected void getMakeStdout() {
    }

    @Override
    public boolean checkMakeResult() throws FaultException {
        if (pid.equals("-1")) {
            return false;
        }
        try {
            ssh.exec("ps -ef | grep " + progName + " | grep -v grep | grep '" + svcName + "' | awk '{print $2}'");
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
        if (svcName != "") {
            log.info("Restore " + this.getName() + " " + hostName + ": " + svcName);
        } else {
            log.info("Restore " + this.getName() + " " + hostName);
        }

        try {
            beforeRestore();
            ssh.exec(remotePath + "/" + restoreScriptName + " " + cmdArgs);
        } catch (ReliabilityException e) {
            throw new FaultException(e);
        }
    }

    /**
     * 继承FaultFulltextBase需要实现该方法，在restore()之前给cmdArgs赋值，可以调用setRestoreArgs()方法，至少有一个参数且必须是进程名
     * 
     * @exem cmdArgs = setRestoreArgs(progName, cmdDir, sdbseadapterDir, svcName);
     */
    protected void beforeRestore() {
    }

    protected String setRestoreArgs(String... args) {
        String ret = "";
        for (String arg : args) {
            ret += " " + arg;
        }
        return ret;
    }

    @Override
    public boolean checkRestoreResult() throws FaultException {
        try {
            ssh.exec("ps -ef | grep " + progName + " | grep -v grep | grep '" + svcName + "' | awk '{print $2}'");
            if (ssh.getStdout().trim().length() <= 0) {
                return false;
            } else {
                return true;
            }
        } catch (ReliabilityException e) {
            throw new FaultException(e);
        }
    }

    @Override
    public void init() throws FaultException {
        try {
            ssh = new Ssh(hostName, user, password, port);
            ssh.scpTo(localPath + "/" + makeScriptName, remotePath + "/");
            ssh.scpTo(localPath + "/" + restoreScriptName, remotePath + "/");
            ssh.exec("chmod 777 " + remotePath + "/" + makeScriptName);
            ssh.exec("chmod 777 " + remotePath + "/" + restoreScriptName);
        } catch (ReliabilityException e) {
            throw new FaultException(e);
        }
    }

    @Override
    public void fini() throws FaultException {
        try {
            if (ssh != null) {
                ssh.exec("rm -rf " + remotePath + "/" + makeScriptName);
                ssh.exec("rm -rf " + remotePath + "/" + restoreScriptName);
            }
        } catch (ReliabilityException e) {
            throw new FaultException(e);
        }
    }
}
