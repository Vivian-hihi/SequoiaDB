package com.sequoiadb.fault;

import org.testng.annotations.Test;

import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.commlib.Ssh;
import com.sequoiadb.exception.ReliabilityException;

public class KillNode extends Fault {
	private String hostName;
	private String svcName;
	private String user;
	private String passwd;
	private String pid = "-1";
	private Ssh ssh;
	private String remotePath;
	private int port;
	private final String localScriptPath = "./script";
	private final String scriptName = "killNode.sh";

	@Test
	public static void test() throws ReliabilityException {
		KillNode kn = new KillNode("192.168.31.31", 22, "11830", "root", "sequoiadb", "/tmp");
		kn.init();
		kn.make();
		for (int i = 0; i < 10; i++) {
			try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			System.out.println("check make:" + kn.checkMakeResult());
			
		}
		kn.restore();
		System.out.println(kn.checkRestoreResult());
		kn.fini();
	}

	public KillNode(String hostName, String svcName) {
		this(hostName, 22, svcName, "root", SdbTestBase.rootPwd, SdbTestBase.workDir);
	}

	public KillNode(String hostName, int port, String svcName, String user, String passwd, String remotePath) {
		super("killNode");
		this.hostName = hostName;
		this.svcName = svcName;
		this.user = user;
		this.passwd = passwd;
		this.remotePath = remotePath;
		this.port = port;
	}

	@Override
	public void make() throws ReliabilityException {
		ssh.exec(remotePath + "/" + scriptName + " " + svcName);
		pid = ssh.getStdout().substring(0, ssh.getStdout().length() - 1);
	}

	@Override
	public boolean checkMakeResult() throws ReliabilityException {
		if (pid.equals("-1")) {
			return false;
		}
		ssh.exec("lsof -i:" + svcName + " | sed '1d' | awk '{print $2}'");
		if (ssh.getStdout().length() <= 0) {
			return false;
		}
		String currentPid = ssh.getStdout().substring(0, ssh.getStdout().length() - 1);
		if (!pid.equals(currentPid)) {
			pid = currentPid;
			return true;
		} else {
			return false;
		}
	}

	@Override
	public void restore() {

		// TODO:如果节点没有异常重启，需要去启动这个节点吗？
	}

	@Override
	public boolean checkRestoreResult() throws ReliabilityException {
		ssh.exec("lsof -i:" + svcName + " | sed '1d' | awk '{print $2}'");
		if (ssh.getStdout().length() <= 0) {
			return false;
		}
		return true;
	}

	@Override
	public boolean init() throws ReliabilityException {
		if (ssh == null) {
			ssh = new Ssh(hostName, user, passwd, port);
		}
		ssh.scpTo(localScriptPath + "/" + scriptName, remotePath);
		ssh.exec("chmod 777 " + remotePath + "/" + scriptName);
		return true;
	}

	@Override
	public boolean fini() throws ReliabilityException {
		if (ssh != null) {
			ssh.exec("rm -rf " + remotePath + "/" + scriptName);
			ssh.close();
		}
		return true;
	}

}
