/**
 * Copyright (c) 2017, SequoiaDB Ltd.
 * File Name:BrokenNetwork.java
 * 
 *
 *  @author wenjingwang
 * Date:2017-2-21下午4:54:48
 *  @version 1.00
 */
package com.sequoiadb.fault;

import java.io.IOException;

import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.commlib.Ssh;
import com.sequoiadb.exception.FaultException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.task.FaultMakeTask;

public class BrokenNetwork extends Fault {
	private String hostName;
	private String user;
	private String passwd;
	private int duration;
	private int port = 22;
	private String remotePath;
	private Ssh ssh;
	private long brokenTime;
	private final String localScriptPath = "./script";
	private final String scriptName = "brokenNetwork.sh";

	/**
	 * 
	 * @param hostName
	 * @param user
	 * @param passwd
	 * @param remotePath
	 * @param duration
	 */
	public BrokenNetwork(String hostName, int duration) {
		super("brokenNetwork");
		this.hostName = hostName;
		this.user = "root";
		this.passwd = SdbTestBase.rootPwd;
		this.duration = duration;
		this.remotePath = SdbTestBase.workDir;
		this.port = 22;
	}

	public void make() throws FaultException {
		try {
			ssh.execBackground("nohup " + remotePath + "/" + scriptName + " " + duration + " &");
			brokenTime = System.currentTimeMillis();
		} catch (ReliabilityException e) {
			FaultException e1 = new FaultException(e);
			e1.setStackTrace(e.getStackTrace());
			throw e1;
		}
	}

	public boolean checkMakeResult() throws FaultException {
		int checkTime = 3;
		for (int i = 0; i < checkTime; i++) {
			if (ping() == false) {
				return true;
			}
		}
		return false;
	}

	public void restore() throws FaultException {
		long diff = System.currentTimeMillis() - brokenTime;
		if (diff < duration * 1000) {
			try {
				Thread.sleep(duration * 1000 - diff);
			} catch (InterruptedException e) {

			}
		}

	}

	public boolean checkRestoreResult() throws FaultException {
		int checkTime = 3;
		for (int i = 0; i < checkTime; i++) {
			if (ping()) {
				return true;
			}
		}
		return false;
	}

	public boolean ping() throws FaultException {
		String os = System.getProperties().getProperty("os.name");
		String cmd;
		if (os.startsWith("win") || os.startsWith("Win")) {
			cmd = "ping " + hostName + " -n 2 -w 2";
		} else {
			cmd = "ping " + hostName + " -c 2 -w 2";
		}
		Runtime rt = Runtime.getRuntime();
		try {
			Process pr = rt.exec(cmd);
			pr.waitFor();
			int exitcode = pr.exitValue();
			pr.destroy();
			if (exitcode == 0) {
				return true;
			} else {
				return false;
			}
		} catch (InterruptedException | IOException e) {
			throw new FaultException(e);
		}
	}

	@Override
	public boolean init() throws FaultException {
		try {
			ssh = new Ssh(hostName, user, passwd, port);
			try {
				ssh.exec("mkdir " + SdbTestBase.workDir);
			} catch (Exception e) {
			}
			ssh.scpTo(localScriptPath + "/" + scriptName, remotePath + "/");
			ssh.exec("chmod 777 " + remotePath + "/" + scriptName);
		} catch (ReliabilityException e) {
			FaultException e1 = new FaultException(e);
			e1.setStackTrace(e.getStackTrace());
			throw e1;
		}
		return true;
	}

	@Override
	public boolean fini() throws FaultException {

		try {
			if (ssh != null) {
				ssh.close();
			}
			ssh = new Ssh(hostName, user, passwd);
			ssh.exec("rm -rf " + remotePath + "/" + scriptName);
		} catch (ReliabilityException e) {
			FaultException e1 = new FaultException(e);
			e1.setStackTrace(e.getStackTrace());
			throw e1;
		} finally {
			ssh.close();
		}

		return true;
	}

	/**
	 * 
	 * @param hostName
	 * @param maxDelay
	 *            最大延迟启动时间
	 * @param duration
	 *            持续时间
	 * @param checkTimes
	 *            检查构造故障成功与否的次数（10）
	 * @return
	 */
	public static FaultMakeTask getFaultMakeTask(String hostName, int maxDelay, int duration, int checkTimes) {
		FaultMakeTask task = null;
		BrokenNetwork bn = new BrokenNetwork(hostName, duration);
		task = new FaultMakeTask(bn, maxDelay, duration, checkTimes);
		return task;
	}

}
