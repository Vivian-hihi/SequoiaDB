/**
 * Copyright (c) 2017, SequoiaDB Ltd.
 * File Name:DiskFull.java
 * 
 *
 *  @author wenjingwang
 * Date:2017-2-21下午4:54:48
 *  @version 1.00
 */
package com.sequoiadb.fault;

import java.util.ArrayList;
import java.util.List;

import org.testng.annotations.Test;

import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.commlib.Ssh;
import com.sequoiadb.exception.CommException;
import com.sequoiadb.exception.ReliabilityException;

public class DiskFull extends Fault {
	private String hostName;
	private String user;
	private String passwd;
	private String padPath;
	private int port;
	private List<String> padFileList = new ArrayList<String>();
	private Ssh ssh;
	private String remotePath;

	private final String localScriptPath = "./script";
	private final String scriptName = "fillUpDisk.sh";

	@Test
	public static void test() throws ReliabilityException {
		DiskFull df = new DiskFull("192.168.31.33", 22, "root", "sequoiadb", "/tmp", "/tmp");
		df.init();
		System.out.println("sadasd");
		df.make();
		System.out.println(df.checkMakeResult());
		df.restore();
		System.out.println(df.checkRestoreResult());
		df.fini();
	}

	/**
	 * 使用root jenkins 22端口登录目标机
	 * 
	 * @param hostName
	 * @param padPath
	 *            填充文件的路径
	 */
	public DiskFull(String hostName, String padPath) {
		this(hostName, 22, "root", SdbTestBase.rootPwd, padPath, SdbTestBase.workDir);
	}

	public DiskFull(String hostName, int port, String user, String passwd, String padPath, String remotePath) {
		super("diskFull");
		this.hostName = hostName;
		this.user = user;
		this.passwd = passwd;
		this.padPath = padPath;
		this.remotePath = remotePath;
		this.port = port;
	}

	public void make() throws ReliabilityException {
		fillUpDisk(100);
	}

	public boolean checkMakeResult() throws ReliabilityException {
		ssh.exec("df " + padPath + " | sed '1d' |awk '{print $4}'");
		String stdOut = ssh.getStdout();
		if (stdOut.length() <= 0) {
			throw new CommException("can not find this path:" + padPath);
		}
		int padSize = Integer.parseInt(stdOut.substring(0, stdOut.length() - 1));
		if (padSize > 0) {
			return false;
		} else {
			return true;
		}
	}

	public void restore() throws ReliabilityException {
		for (int i = 0; i < padFileList.size();) {
			ssh.exec("rm -f " + padFileList.get(i));
			padFileList.remove(i);
		}
	}

	public boolean checkRestoreResult() {
		return padFileList.size() == 0 ? true : false;
	}

	public void fillUpDisk(int percent) throws ReliabilityException {
		ssh.exec(remotePath + "/" + scriptName + " " + padPath + " " + percent);
		if (ssh.getStdout().length() > 0) {
			String padFileName = ssh.getStdout().substring(0, ssh.getStdout().length() - 1);
			padFileList.add(padFileName);
		}
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
